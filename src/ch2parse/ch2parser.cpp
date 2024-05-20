/**
* @file ch2parser.cpp
* @author lakor64
* @date 03/01/2024
* @brief clang parser
*/
#include "ch2parser.hpp"
#include "utility.hpp"
#include "clangutils.hpp"
#include "struct.hpp"

#include <algorithm>
#include <clang-c/Index.h>

void CH2Parser::AddPrimitive(const std::string& name, PrimitiveType type, PrimitiveMods mod)
{
	auto p = new Primitive();
	p->m_name = name;
	p->m_type = type;
	p->m_mod = mod;
	m_types.insert_or_assign(name, p);
}

void CH2Parser::AddBasics(const PlatformInfo& plat)
{
	static std::vector<std::string> g_types = {
		"char",
		"short",
		"int",
		"long",
		"long long",
		"float",
		"double",
		"long double",
		"wchar_t",
		"__int128",
		"*",
	};

	for (const auto& name : g_types)
	{
		const auto primitive_type = Utility::GetPrimitiveTypeForPlatform(name, plat);

		if (PrimitiveType::Real4 == primitive_type ||
			PrimitiveType::Real8 == primitive_type ||
			PrimitiveType::Pointer == primitive_type)
		{
			AddPrimitive(name, primitive_type, PrimitiveMods::Default);
			continue;
		}

		for (int i = static_cast<int>(PrimitiveMods::Default); i <= static_cast<int>(PrimitiveMods::Unsigned); i++)
		{
			std::string newName;
			const auto mod = static_cast<PrimitiveMods>(i);

			if (mod == PrimitiveMods::Unsigned)
				newName = "unsigned " + name;
			else if (mod == PrimitiveMods::Signed)
				newName = "signed " + name;
			else
				newName = name;

			AddPrimitive(newName, primitive_type, mod);
		}
	}
}

void CH2Parser::Visit(const std::string& in, int clang_argc, const char** clang_argv, CFile& file, const PlatformInfo& plt)
{

	m_cf = &file;

	// add basic primitives
	AddBasics(plt);

	// create index

#if CINDEX_VERSION_MINOR > 63 || CINDEX_VERSION_MAJOR > 0
	CXIndexOptions opts = {};
	opts.Size = sizeof(opts);
	opts.DisplayDiagnostics = 1;

	auto index = clang_createIndexWithOptions(&opts);
#else
	auto index = clang_createIndex(0, 1);
#endif

	if (!index)
	{
		m_lasterr = CH2ErrorCodes::IndexError;
		return;
	}

	// create translation unit
	uint32_t flags = CXTranslationUnit_DetailedPreprocessingRecord | CXTranslationUnit_SkipFunctionBodies;

	const auto ec = clang_parseTranslationUnit2(index, in.c_str(), clang_argv, clang_argc,
		nullptr, 0, 
		flags,
		&m_unit);

	if (ec != CXError_Success)
	{
		m_lasterr = Utility::CXErrorToCH2Error(ec);
		return;
	}

	for (int i = 0; i < clang_argc; i++)
	{
		if (clang_argv[i][0] == '-' && clang_argv[i][1] == 'D' && clang_argv[i][2] != '\0')
		{
			std::string def = clang_argv[i] + 2;
			size_t eqpos = def.find("=");
			if (eqpos == std::string::npos)
				eqpos = def.size();

			m_defs.emplace_back(def.substr(0, eqpos));
		}
	}

	// start visiting
	auto cursor = clang_getTranslationUnitCursor(m_unit);
	clang_visitChildren(cursor, [](CXCursor c, CXCursor parent, CXClientData data)
		-> CXChildVisitResult {
			return ((CH2Parser*)data)->ParseChild(c, parent);
		}, this);

	/*
	* During nested structures, clang parses a nested after the parent structure
	* therefore we can get to a point where a parent structure is before the child structure
	* This function amis to fix that.
	*/
	FixupDecls();
}

BasicMember* CH2Parser::FindType(const std::string& name)
{
	const auto& it = m_types.find(name);
	if (it == m_types.end())
		return nullptr;

	return it->second;
}

bool CH2Parser::SetupLink(CXType type, LinkType& ref)
{
	CXType baseType = type;

	// find the real type to link
	while (IsTypePointer(baseType))
	{
		baseType = clang_getPointeeType(baseType);
		ref.pointers++;
	}

	if (baseType.kind == CXType_Void && IsTypePointer(type))
	{
		ref.ref_type = FindType("*"); // primitive pointer type
		return ref.ref_type != nullptr;
	}

	ClangStr baseTypeStr(clang_getTypeSpelling(baseType));
	std::string baseTypeName = baseTypeStr.Get();

	if (clang_isConstQualifiedType(baseType)) // remove "const " from name
	{
		const auto p = baseTypeName.find("const");
		baseTypeName = baseTypeName.substr(p + 6);
	}

	if (clang_isVolatileQualifiedType(baseType)) // remove "volatile " from name
	{
		const auto p = baseTypeName.find("volatile");
		baseTypeName = baseTypeName.substr(p + 9);
	}

	if (clang_isRestrictQualifiedType(baseType)) // remove "restrict " from name
	{
		const auto p = baseTypeName.find("restrict");
		baseTypeName = baseTypeName.substr(p + 9);
	}

	ref.ref_type = FindType(baseTypeName);
	return ref.ref_type != nullptr;
}

BasicMember* CH2Parser::FindType(CXType type)
{
	ClangStr name(clang_getTypeSpelling(type));
	return FindType(name);
}

BasicMember* CH2Parser::FindType(CXCursor c)
{
	return FindType(clang_getCursorType(c));
}

bool CH2Parser::SetupVariable(Variable& v, CXType type, CXCursor c)
{
	if (c.kind != CXType_Invalid && c.kind != CXCursor_FirstInvalid && c.kind != CXCursor_LastInvalid)
	{
		// we only care about argument names in a function not a typedef
		ClangStr argumentName(clang_getCursorSpelling(c));
		v.m_name = argumentName.Get();
	}

	if (!SetupLink(type, v.m_ref))
	{
		return false;
	}

	v.m_restrict = clang_isRestrictQualifiedType(type);
	v.m_volatile = clang_isVolatileQualifiedType(type);
	v.m_size = clang_Type_getSizeOf(type) * 8;

	return true;
}


CXChildVisitResult CH2Parser::ParseChild(CXCursor cursor, CXCursor parent)
{
	const auto kind = clang_getCursorKind(cursor);
	BasicMember* member = nullptr;
	bool skip = false, skipadd = false;

	switch (kind)
	{
	case CXCursor_StructDecl:
	case CXCursor_UnionDecl:
		member = VisitStructOrUnion(cursor, kind == CXCursor_UnionDecl);
		break;

	case CXCursor_FieldDecl:
		member = VisitField(cursor, parent);
		skipadd = true;
		break;

	case CXCursor_TypedefDecl:
		member = VisitTypedef(cursor);
		break;

	case CXCursor_EnumDecl:
		member = VisitEnum(cursor);
		break;

	case CXCursor_EnumConstantDecl:
		member = VisitEnumDecl(cursor, parent);
		skipadd = true;
		break;

	case CXCursor_MacroDefinition:
	{
		skip = true;
		// skip builtin macros and function-like macros
		if (clang_Cursor_isMacroBuiltin(cursor) == 0 && clang_Cursor_isMacroFunctionLike(cursor) == 0)
		{
			// do not parse builin macros ereditated by system headers
			const auto& locationSource = clang_getCursorLocation(cursor);
			if (clang_Location_isInSystemHeader(locationSource) <= 0)
			{
				skip = false;
				// skip macros passed by the cli
				ClangStr name(clang_getCursorSpelling(cursor));

				for (const auto& def : m_defs)
				{
					if (def == name.Get())
					{
						skip = true;
						break;
					}
				}
			}
		}

		if (!skip)
			member = VisitMacroDef(cursor);

		if (!member)
			return CXChildVisit_Recurse; // skip unsupported macros

		break;
	}
	case CXCursor_FunctionDecl:
		member = VisitFunc(cursor);
		break;

	case CXCursor_VarDecl:
		member = VisitVarDecl(cursor);
		break;


	case CXCursor_IntegerLiteral: // skip, we don't read literals
	case CXCursor_FloatingLiteral: // skip, we don't read literals
	case CXCursor_StringLiteral:
	case CXCursor_CharacterLiteral:
	case CXCursor_ImaginaryLiteral:
	case CXCursor_FixedPointLiteral:
	case CXCursor_ObjCStringLiteral:
	case CXCursor_CompoundLiteralExpr:
	case CXCursor_CXXNullPtrLiteralExpr:
	case CXCursor_InclusionDirective: // skip inclusions as they are part of preprocessor
	case CXCursor_ParmDecl: // we have parsed them already
	case CXCursor_MacroExpansion: // skip macro expansions, we do not support %ifdef %else %endif and neither h2inc did
	case CXCursor_TypeRef:
		skip = true;
		break;

	default:
		break;
	}

	if (m_lasterr != CH2ErrorCodes::None)
	{
		if (member)
			delete member;

		return CXChildVisit_Break;
	}

	if (skip)
		return CXChildVisit_Continue;

	if (!skipadd)
	{
		if (!member)
			return CXChildVisit_Break;

		const auto& it = m_types.find(member->GetName());

		if (it != m_types.end())
		{
			delete member;
			/*
			* libclang parses nested structures two times, probably this is done because
			*  it resolved the nested structure and then inform us to return at the parsing
			*   of the original structure
			*/
			return CXChildVisit_Recurse;
		}

		m_types.insert_or_assign(member->GetName(), member);
		m_cf->m_types.push_back(member);
	}

	return CXChildVisit_Recurse;
}

void CH2Parser::FixupDecls()
{
	// 1. order items by id
	std::unordered_map<std::string, size_t> order_map;
	auto& types = m_cf->m_types;

	for (size_t i = 0; i < types.size(); i++)
	{
		order_map.insert_or_assign(types[i]->GetName(), i);
	}

	auto& it = types.begin();

	std::vector<BasicMember*> push_members;

	for (; it != types.end(); )
	{
		bool erased = false;

		if ((*it)->GetTypeID() == MemberType::Struct || (*it)->GetTypeID() == MemberType::Union)
		{
			const auto& s = dynamic_cast<Struct*>((*it));
			for (const auto& e : s->GetFields())
			{
				if (e->GetRef().ref_type->GetTypeID() == MemberType::Struct || e->GetRef().ref_type->GetTypeID() == MemberType::Union)
				{
					const auto& s2 = dynamic_cast<Struct*>(e->GetRef().ref_type);
					
					if (order_map[s2->GetName()] > order_map[s->GetName()])
					{
						// erase the type as we have to push at the end
						it = types.erase(it);
						push_members.emplace_back(s);
						erased = true;
						break;
					}
				}
			}
		}

		if (!erased)
			it++;
	}

	for (const auto& m : push_members)
	{
		m_cf->m_types.emplace_back(m);
	}
}
