/**
* @file ch2parser_visit.cpp
* @author lakor64
* @date 20/05/2024
* @brief llvm clang visit
*/
#include "ch2parser.hpp"
#include "utility.hpp"
#include "function.hpp"
#include "enum.hpp"
#include "struct.hpp"
#include "globalvar.hpp"
#include "define.hpp"
#include "clangutils.hpp"

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

	if (!SetupVariable(*rt, type, c))
	{
		m_lasterr = CH2ErrorCodes::MissingType;
		delete rt;
		return nullptr;
	}

	// fields are sequential, cannot use a map
	for (const auto& f : rt->m_parent->m_fields)
	{
		if (f->GetName() == rt->m_name)
		{
			// field was already added, skip
			// this is required due to how nested structure parse are handled (they are parsed two times)
			// the sad thing is that we cannot know if the structure is going to be parsed again
			delete rt;
			return nullptr;
		}
	}

	rt->m_size = clang_getFieldDeclBitWidth(c);
	rt->m_parent->m_fields.emplace_back(rt);

	return rt;
}

BasicMember* CH2Parser::VisitTypedef(CXCursor c)
{
	auto undertype = clang_getTypedefDeclUnderlyingType(c);
	const auto type = clang_getCursorType(c);

	if (undertype.kind == CXType_Pointer)
		undertype = clang_getPointeeType(undertype);

	if (undertype.kind == CXType_FunctionProto)
	{
		// libclang is trying to find a function proto for something that prolly doesn't exist
		return VisitFunc(c, undertype, true);
	}
	
	// that part is done to resolve function proto, we need to reset
	//  the code flow to the previous thing
	// TODO: is there a better way than with this hack?

	undertype = clang_getTypedefDeclUnderlyingType(c);

	auto rt = new Typedef();

	if (!SetupVariable(*rt, undertype, c))
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
	return VisitFunc(c, clang_getCursorType(c), false);
}

BasicMember* CH2Parser::VisitFunc(CXCursor c, CXType type, bool isTypedef)
{
	ClangStr name(clang_getCursorSpelling(c));
	auto rt = new Function();

	const auto argLen = clang_getNumArgTypes(type);

	rt->m_name = name.Get();
	rt->m_storage = Utility::CXStorageTypeToCH2StorageType(clang_Cursor_getStorageClass(c));
	rt->m_typedef = isTypedef;

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
		if (!SetupVariable(rt->m_ret, returnType))
		{
			m_lasterr = CH2ErrorCodes::MissingType;
			delete rt;
			return nullptr;
		}
	}

	for (int i = 0; i < argLen; i++)
	{
		Variable d;
		const auto argCursor = clang_Cursor_getArgument(c, i);
		const auto argType = clang_getArgType(type, i);

		if (!SetupVariable(d, argType, argCursor))
		{
			m_lasterr = CH2ErrorCodes::MissingType;
			delete rt;
			return nullptr;
		}

		rt->m_arguments.push_back(d);
	}

	return rt;
}


BasicMember* CH2Parser::VisitVarDecl(CXCursor c)
{
	auto rt = new GlobalVar();
	const auto type = clang_getCursorType(c);

	if (!SetupVariable(*rt, type, c))
	{
		m_lasterr = CH2ErrorCodes::MissingType;
		delete rt;
		return nullptr;
	}

	rt->m_storage = Utility::CXStorageTypeToCH2StorageType(clang_Cursor_getStorageClass(c));

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
