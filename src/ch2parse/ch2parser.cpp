/**
* @file ch2parser.cpp
* @author lakor64
* @date 03/01/2024
* @brief clang parser
*/
#include "ch2parser.hpp"
#include "utility.hpp"
#include "function.hpp"
#include "enum.hpp"
#include "typedef.hpp"
#include "struct.hpp"
#include "clangutils.hpp"

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
		auto primitive_type = Utility::GetPrimitiveTypeForPlatform(name, plat);

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
			auto mod = static_cast<PrimitiveMods>(i);

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

	CXTranslationUnit unit;
	auto ec = clang_parseTranslationUnit2(index, in.c_str(), clang_argv, clang_argc,
		nullptr, 0, 
		flags,
		&unit);

	if (ec != CXError_Success)
	{
		m_lasterr = Utility::CXErrorToCH2Error(ec);
		return;
	}

	// start visiting
	auto cursor = clang_getTranslationUnitCursor(unit);
	clang_visitChildren(cursor, [](CXCursor c, CXCursor parent, CXClientData data)
		-> CXChildVisitResult {
			return ((CH2Parser*)data)->ParseChild(c, parent);
		}, this);
}

BasicMember* CH2Parser::FindType(const std::string& name)
{
	auto it = m_types.find(name);
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

	ClangStr baseTypeName(clang_getTypeSpelling(baseType));
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

BasicMember* CH2Parser::VisitStructOrUnion(CXCursor c, bool isUnion)
{
	Struct* rt;
	
	if (!isUnion)
		rt = new Struct();
	else
		rt = new Union();

	auto type = clang_getCursorType(c);
	ClangStr name(clang_getTypeSpelling(type));

	rt->m_name = name.Get();
	rt->m_size = clang_Type_getSizeOf(type) * 8;
	rt->m_align = clang_Type_getAlignOf(type) * 8;

	return rt;
}

BasicMember* CH2Parser::VisitField(CXCursor c, CXCursor p)
{
	auto rt = new StructField();

	ClangStr name(clang_getCursorSpelling(c));
	auto type = clang_getCursorType(c);
	auto parent_type = clang_getCursorType(p);

	ClangStr typeName(clang_getTypeSpelling(type));

	rt->m_name = name.Get();
	rt->m_size = clang_getFieldDeclBitWidth(c);

	if (!SetupLink(type, rt->m_ref))
	{
		m_lasterr = CH2ErrorCodes::MissingType;
		delete rt;
		return nullptr;
	}

	auto parent = FindType(p);

	if (!parent)
	{
		m_lasterr = CH2ErrorCodes::MissingParent;
		delete rt;
		return nullptr;
	}

	if (parent->GetTypeID() != MemberType::Struct && parent->GetTypeID() != MemberType::Union)
	{
		m_lasterr = CH2ErrorCodes::BadParent;
		delete rt;
		return nullptr;
	}

	rt->m_parent = dynamic_cast<Struct*>(parent);
	rt->m_parent->m_fields.push_back(rt);

	return rt;
}

BasicMember* CH2Parser::VisitTypedef(CXCursor c)
{
	auto rt = new Typedef();

	ClangStr name(clang_getCursorSpelling(c));
	auto type = clang_getCursorType(c);

	rt->m_name = name.Get();
	rt->m_size = clang_Type_getSizeOf(type) * 8;

	auto undertype = clang_getTypedefDeclUnderlyingType(c);

	if (!SetupLink(undertype, rt->m_ref))
	{
		m_lasterr = CH2ErrorCodes::MissingType;
		delete rt;
		return nullptr;
	}

	return rt;
}

BasicMember* CH2Parser::VisitEnumDecl(CXCursor c, CXCursor p)
{
	auto rt = new EnumField();
	auto sizeType = clang_getEnumDeclIntegerType(c);

	ClangStr name(clang_getCursorSpelling(c));

	rt->m_name = name.Get();
	rt->m_size = clang_Type_getSizeOf(sizeType) * 8;
	rt->m_value = clang_getEnumConstantDeclUnsignedValue(c);

	auto parent = FindType(p);

	if (!parent)
	{
		m_lasterr = CH2ErrorCodes::MissingParent;
		delete rt;
		return nullptr;
	}

	if (parent->GetTypeID() != MemberType::Enum)
	{
		m_lasterr = CH2ErrorCodes::BadParent;
		delete rt;
		return nullptr;
	}

	rt->m_parent = dynamic_cast<Enum*>(parent);
	rt->m_parent->m_fields.push_back(rt);

	return rt;
}

BasicMember* CH2Parser::VisitEnum(CXCursor c)
{
	auto rt = new Enum();
	ClangStr name(clang_getCursorSpelling(c));

	rt->m_name = name.Get();

	auto sizeType = clang_getEnumDeclIntegerType(c);
	rt->m_size = clang_Type_getSizeOf(sizeType);

	return rt;
}

BasicMember* CH2Parser::VisitFunc(CXCursor c)
{
	ClangStr name(clang_getCursorSpelling(c));
	auto type = clang_getCursorType(c);
	auto rt = new Function();
	
	rt->m_name = name.Get();
	rt->m_variadic = clang_isFunctionTypeVariadic(type) > 0;
	rt->m_calltype = Utility::CXCallConvToCH2CallConv(clang_getFunctionTypeCallingConv(type));
	if (rt->m_calltype == CallType::Invalid)
	{
		m_lasterr = CH2ErrorCodes::BadCallType;
		delete rt;
		return nullptr;
	}

	auto returnType = clang_getResultType(type);
	
	if (returnType.kind != CXType_Void)
	{
		if (!SetupLink(returnType, rt->m_ret.ref))
		{
			m_lasterr = CH2ErrorCodes::MissingType;
			delete rt;
			return nullptr;
		}
	}

	auto argLen = clang_Cursor_getNumArguments(c);

	for (int i = 0; i < argLen; i++)
	{
		FuncData d;
		auto argCursor = clang_Cursor_getArgument(c, i);
		auto argumentType = clang_getCursorType(argCursor);

		if (!SetupLink(argumentType, d.ref))
		{
			m_lasterr = CH2ErrorCodes::MissingType;
			delete rt;
			return nullptr;
		}

		ClangStr argumentName(clang_getTypeSpelling(argumentType));
		d.name = argumentName.Get();

		rt->m_arguments.push_back(d);
	}

	return rt;
}

BasicMember* CH2Parser::VisitVarDecl(CXCursor c)
{
	// TODO
	return nullptr;
}

BasicMember* CH2Parser::VisitMacroDef(CXCursor c)
{
	// TODO
	return nullptr;
}

CXChildVisitResult CH2Parser::ParseChild(CXCursor cursor, CXCursor parent)
{
	auto kind = clang_getCursorKind(cursor);
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
		if (clang_Cursor_isMacroBuiltin(cursor) || clang_Cursor_isMacroFunctionLike(cursor))
			skip = true; // skip macros that we cannot parse
		else
			member = VisitMacroDef(cursor);

		break;

	case CXCursor_FunctionDecl:
		member = VisitFunc(cursor);
		break;

	case CXCursor_VarDecl:
		member = VisitVarDecl(cursor);
		break;


	case CXCursor_IntegerLiteral: // skip, where there are the literals anyway...
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

	if (!member)
		return CXChildVisit_Break;

	if (!skipadd)
	{
		auto it = m_types.find(member->GetName());

		if (it != m_types.end())
		{
			delete member;
			return CXChildVisit_Break;
		}

		m_types.insert_or_assign(member->GetName(), member);
		m_cf->m_types.push_back(member);
	}

	return CXChildVisit_Recurse;
}
