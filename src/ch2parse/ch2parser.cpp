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
#include "struct.hpp"
#include "globalvar.hpp"
#include "define.hpp"
#include "clangutils.hpp"

#include <algorithm>
#include <clang-c/Index.h>
#include <exprtk.hpp>

// TODO: Separate and simplify this big chunk of garbage

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
			m_defs.emplace_back(clang_argv[i] + 2);
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

BasicMember* CH2Parser::VisitStructOrUnion(CXCursor c, bool isUnion)
{
	Struct* rt;
	
	if (!isUnion)
		rt = new Struct();
	else
		rt = new Union();

	const auto type = clang_getCursorType(c);
	ClangStr name(clang_getTypeSpelling(type));

	rt->m_name = name.Get();
	rt->m_size = clang_Type_getSizeOf(type) * 8;
	rt->m_align = clang_Type_getAlignOf(type) * 8;

	if (rt->m_name.find("(unnamed") != std::string::npos)
		rt->m_unnamed = true;

	return rt;
}

BasicMember* CH2Parser::VisitField(CXCursor c, CXCursor p)
{
	auto rt = new StructField();

	ClangStr name(clang_getCursorSpelling(c));
	auto type = clang_getCursorType(c);

	if (type.kind == CXType_Elaborated)
		type = clang_Type_getNamedType(type);

	const auto parent_type = clang_getCursorType(p);

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

	// fields are sequential, cannot use a map
	for (const auto& f : rt->m_parent->m_fields)
	{
		if (f->GetName() == name.Get())
		{
			// field was already added, skip
			// this is required due to how nested structure parse are handled (they are parsed two times)
			// the sad thing is that we cannot know if the structure is going to be parsed again
			delete rt;
			return nullptr;
		}
	}

	ClangStr typeName(clang_getTypeSpelling(type));

	rt->m_name = name.Get();
	rt->m_size = clang_getFieldDeclBitWidth(c);


	if (!SetupLink(type, rt->m_ref))
	{
		m_lasterr = CH2ErrorCodes::MissingType;
		delete rt;
		return nullptr;
	}
	
	rt->m_parent->m_fields.emplace_back(rt);

	return rt;
}

BasicMember* CH2Parser::VisitTypedef(CXCursor c)
{
	auto rt = new Typedef();

	ClangStr name(clang_getCursorSpelling(c));
	const auto type = clang_getCursorType(c);

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
	const auto sizeType = clang_getEnumDeclIntegerType(c);

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
	const auto type = clang_getCursorType(c);
	ClangStr name(clang_getTypeSpelling(type));

	rt->m_name = name.Get();

	const auto sizeType = clang_getEnumDeclIntegerType(c);
	rt->m_size = clang_Type_getSizeOf(sizeType);

	return rt;
}

BasicMember* CH2Parser::VisitFunc(CXCursor c)
{
	ClangStr name(clang_getCursorSpelling(c));
	const auto type = clang_getCursorType(c);
	auto rt = new Function();
	
	const auto argLen = clang_Cursor_getNumArguments(c);

	rt->m_name = name.Get();
	rt->m_storage = Utility::CXStorageTypeToCH2StorageType(clang_Cursor_getStorageClass(c));

	/* 
	* ISO C forbids "void a(...)", and MS H2INC doesn't threat "void a()" as variadic.
	* I don't think we should do this as well, so make sure a function can be variadic if libClang tells us so and we have at least one argument
	*/
	rt->m_variadic = clang_isFunctionTypeVariadic(type) > 0 && argLen > 0;
	rt->m_calltype = Utility::CXCallConvToCH2CallConv(clang_getFunctionTypeCallingConv(type));
	if (rt->m_calltype == CallType::Invalid)
	{
		m_lasterr = CH2ErrorCodes::BadCallType;
		delete rt;
		return nullptr;
	}

	const auto returnType = clang_getResultType(type);
	
	if (returnType.kind != CXType_Void)
	{
		if (!SetupLink(returnType, rt->m_ret.ref))
		{
			m_lasterr = CH2ErrorCodes::MissingType;
			delete rt;
			return nullptr;
		}
	}

	for (int i = 0; i < argLen; i++)
	{
		FuncData d;
		const auto argCursor = clang_Cursor_getArgument(c, i);
		const auto argumentType = clang_getCursorType(argCursor);

		if (!SetupLink(argumentType, d.ref))
		{
			m_lasterr = CH2ErrorCodes::MissingType;
			delete rt;
			return nullptr;
		}

		ClangStr argumentName(clang_getCursorSpelling(argCursor));
		d.name = argumentName.Get();
		d.restric = clang_isRestrictQualifiedType(argumentType);
		d.volatil = clang_isVolatileQualifiedType(argumentType);

		rt->m_arguments.push_back(d);
	}

	return rt;
}

BasicMember* CH2Parser::VisitVarDecl(CXCursor c)
{
	auto rt = new GlobalVar();

	ClangStr name(clang_getCursorSpelling(c));
	const auto type = clang_getCursorType(c);

	rt->m_name = name.Get();
	rt->m_size = clang_Type_getSizeOf(type) * 8;

	if (!SetupLink(type, rt->m_ref))
	{
		m_lasterr = CH2ErrorCodes::MissingType;
		delete rt;
		return nullptr;
	}

	rt->m_storage = Utility::CXStorageTypeToCH2StorageType(clang_Cursor_getStorageClass(c));
	rt->m_volatile = clang_isVolatileQualifiedType(type);
	rt->m_restrict = clang_isRestrictQualifiedType(type);

	return rt;
}

BasicMember* CH2Parser::VisitMacroDef(CXCursor c)
{
	auto rt = new Define();

	CXSourceRange range = clang_getCursorExtent(c);

	unsigned int numTokens = 0;
	CXToken* tokens = nullptr;
	clang_tokenize(m_unit, range, &tokens, &numTokens);

	ClangStr name(clang_getCursorSpelling(c));
	rt->m_name = name.Get();
	bool eval = false;

	for (auto i = 0U; i < numTokens; i++)
	{
		if (i == 0)
			continue; // skip name

		auto kind = clang_getTokenKind(tokens[i]);
		ClangStr value(clang_getTokenSpelling(m_unit, tokens[i]));
		std::string value_str = value.Get();

		switch (kind)
		{
		case CXToken_Punctuation:
			if (rt->m_defType == DefineType::Hexadecimal || rt->m_defType == DefineType::Integer || rt->m_defType == DefineType::Octal
				|| rt->m_defType == DefineType::Binary || rt->m_defType == DefineType::Float)
			{
				if (value_str == "+" || value_str == "-" || value_str == "*" || value_str == "/")
					eval = true;
			}

			rt->m_value += value_str;
			break;
		case CXToken_Identifier:
		{
			const auto& ref = FindType(value_str);

			if (!ref || ref->GetTypeID() != MemberType::Define)
			{
				rt->m_value += value_str;
				rt->m_defType = DefineType::Text;
				continue;
			}
			else
			{
				const auto& dd = dynamic_cast<Define*>(ref);
				rt->m_value += dd->GetValue();

				if (rt->m_defType == DefineType::None)
					rt->m_defType = dd->GetDefineType();

				if (dd->GetDefineType() != rt->m_defType)
					rt->m_defType = DefineType::Text;
			}
			break;
		}
		case CXToken_Literal:
		{
			// does the token starts with a string literal?
			if (value_str[0] == '"')
			{
				rt->m_defType = DefineType::String;

				const auto e = value_str.find_last_of('"');
				if (e == std::string::npos)
				{
					// invalid string
					m_lasterr = CH2ErrorCodes::ValueError;
					clang_disposeTokens(m_unit, tokens, numTokens);
					delete rt;
					return nullptr;
				}

				value_str = value_str.substr(1, e - 1);
				rt->m_value += value_str;
			}
			else if (isdigit(value_str[0]))
			{
				// we might have an int literal

				if (value_str[0] == '0' && value_str[1] == 'x')
				{
					rt->m_defType = DefineType::Hexadecimal;
					value_str = value_str.substr(2);
				}
				else if (value_str[0] == '0' && value_str[1] == 'b')
				{
					rt->m_defType = DefineType::Binary;
					value_str = value_str.substr(2);
				}
				else if (value_str[0] == '0' && value_str.size() > 1)
				{
					rt->m_defType = DefineType::Octal;
					value_str = value_str.substr(1);
				}
				else
					rt->m_defType = DefineType::Integer;

				if (value_str.find(".") != std::string::npos || value_str[value_str.size() - 1] == 'f')
					rt->m_defType = DefineType::Float;

				size_t m = 0;
				for (; m < value_str.size(); m++)
				{
					if (!isdigit(value_str[m]) && value_str[m] != '.')
						break;
				}

				// remove ULL and similar marks
				value_str = value_str.substr(0, m);

				rt->m_value += value_str;
			}
			else if (rt->m_defType == DefineType::None)
			{
				clang_disposeTokens(m_unit, tokens, numTokens);
				delete rt;
				return nullptr;
			}
			break;
		}
		default:
			clang_disposeTokens(m_unit, tokens, numTokens);
			delete rt;
			return nullptr;
		}
	}

	clang_disposeTokens(m_unit, tokens, numTokens);

	if (eval)
		EvalDefine(rt);

	return rt;
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

void CH2Parser::EvalDefine(Define* def)
{
	exprtk::expression<double> expression;
	exprtk::parser<double> parser;

	if (!parser.compile(def->GetValue(), expression))
	{
		m_lasterr = CH2ErrorCodes::EvalError;
		return;
	}

	auto res = expression.value();

	std::stringstream stream;
	
	if (def->GetDefineType() == DefineType::Float)
		stream << res;
	else
		stream << std::hex << uint32_t(res);

	def->m_defType = DefineType::Hexadecimal;
	def->m_value = stream.str();
}
