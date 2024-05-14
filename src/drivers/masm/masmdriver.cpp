/**
* @file masmdriver.cpp
* @author lakor64
* @date 03/01/2024
* @brief MASM driver functions
*/
#include "masmdriver.hpp"
#include "strconv.h"
#include <writerhelp.hpp>
#include <stack>

static void copy_fixed_name(std::string& dst, const LinkType& link)
{
	for (long long i = 0; i < link.pointers; i++)
	{
		dst += "PTR ";
	}

	if (!link.ref_type) // for "void" types
		return;

	if (link.ref_type->GetTypeID() == MemberType::Primitive)
	{
		if (link.ref_type->GetName() != "*") // do not write the pointer name if it's flagged as such
			dst += get_primitive_name(*dynamic_cast<Primitive*>(link.ref_type));
	}
	else if (link.ref_type->GetTypeID() == MemberType::Union)
	{
		dst += link.ref_type->GetName().c_str() + 6; // remove "union "
	}
	else if (link.ref_type->GetTypeID() == MemberType::Struct)
	{
		dst += link.ref_type->GetName().c_str() + 7; // remove "struct "
	}
	else
	{
		dst += link.ref_type->GetName();
	}
}

/**
* Writes the members of a structure or union
* @param stru Structure to parse
*/
void MasmDriver::WriteStructMembers(const Struct& stru)
{
	int64_t totalprct = 0;
	const auto& fields = stru.GetFields();

	for (size_t i = 0; i < fields.size(); i++)
	{
		const auto& field = fields[i];

		if (field->GetSize() == -1) // bitsize > 0 works differently
		{
			std::string typeName = "";
			bool usetags = false;

			if (field->GetRef().pointers)
			{
				typeName = "@t_" + std::to_string(m_total_used_typedef);
			}
			else
			{
				copy_fixed_name(typeName, field->GetRef());

				if (field->GetRef().ref_type->GetTypeID() == MemberType::Struct || field->GetRef().ref_type->GetTypeID() == MemberType::Union)
				{
					usetags = true;
				}
			}

			writefmt(m_fp, "%s\t\t%s\t\t%s\n", field->GetName().c_str(), typeName.c_str(), usetags ? "<>" : "?");
		}
		else
		{
			/*
				MASM writes thing in BE, our CPU is in LE so what we need to do is:
				- iterate trough all types which bitSize != -1
				- store all the types in a FIFO queue so we top last -> first
				- check if the processed total data of our bitwise matches the base type requested
				  if not, then we need to add padding
				- iterare trough all of our stack and write data from last to begin
			*/

			const auto structname = stru.GetName().c_str() + 7;
			std::stack<StructField*> m_bitstack;
			auto prim = dynamic_cast<const Primitive*>(field->GetRef().ref_type)->GetType();
			int64_t processed = field->GetSize();
			size_t k;
			m_bitstack.push(field);

			for (k = i + 1; k < fields.size(); k++)
			{
				auto field2 = fields[k];
				auto prim2 = dynamic_cast<const Primitive*>(field2->GetRef().ref_type);
				if (!prim2 || prim2->GetTypeID() != MemberType::Primitive)
					break;

				if (prim2->GetType() != prim || field2->GetSize() == -1)
					break;

				if (processed + field2->GetSize() > PrimitiveGetBitSize(prim))
					break;

				m_bitstack.push(fields[k]);
				processed += fields[k]->GetSize();
				i++;

				if (processed == PrimitiveGetBitSize(prim))
					break;
			}

			// rec@x_0   RECORD
			writefmt(m_fp, "rec@%s_%lld\t\tRECORD\t", structname, totalprct);

			bool writeretn = false;

			if (processed != PrimitiveGetBitSize(prim))
			{
				// this adds the missing padding

				// @0@x:5
				writefmt(m_fp, "@0@%s:%lld", structname, PrimitiveGetBitSize(prim) - processed);
				writeretn = true;
			}

			while (!m_bitstack.empty())
			{
				if (!writeretn)
					writeretn = true;
				else
					writesng(m_fp, ",\n\t\t\t\t");

					// p@x:3
				const auto top = m_bitstack.top();
				writefmt(m_fp, "%s@%s:%lld", top->GetName().c_str(), structname, top->GetSize());

				m_bitstack.pop();
			}

			// @bit_0  rec@x_0 <>
			writefmt(m_fp, "\n"
							"@bit_%lld\t\trec@%s_%lld <>\n", totalprct, structname, totalprct);

			totalprct++;
		}
	}
}

/**
* MASM does not directly support struct members of pointers, so we need to generate
* a new typedef and put them there. This code preprocess all the struct members in search
* of typedef to generate BEFORE starting to write the structure
* @param stru Structure to preprocess
*/
void MasmDriver::PreprocessStruct(const Struct& stru)
{
	for (const auto& field : stru.GetFields())
	{
		if (field->GetRef().pointers)
		{
			std::string fullname = "";

			for (long long i = 0; i < field->GetRef().pointers; i++)
			{
				fullname += "PTR ";
			}

			if (field->GetRef().ref_type->GetName() != "*")
				fullname += field->GetRef().ref_type->GetName();
			else
				fullname = fullname.erase(fullname.size() - 2);

			writefmt(m_fp, "@t_%lld\t\tTYPEDEF\t\t%s\n", m_total_preprocess_typedef, fullname.c_str());
			m_total_preprocess_typedef++;
		}
	}
}

void MasmDriver::WriteFileStart(void)
{
	writesng(m_fp,
		"\n"
		"option expr32\n"
		"option casemap:none\n"
		"\n"
		"; Begin of the file\n"
	);
}

void MasmDriver::WriteFileEnd(void)
{
	writesng(m_fp, "; End of the file\n");
}

void MasmDriver::WriteSingleComment(const std::string& comment)
{
	writefmt(m_fp, "; %s\n", comment.c_str());
}

void MasmDriver::WriteMultiComment(const std::vector<std::string>& v)
{
	writesng(m_fp, "COMMENT @$?\n\n");
	for (const auto& comment : v)
	{
		writesng(m_fp, comment.c_str());
	}
	writesng(m_fp, "\n@$?\n");
}

void MasmDriver::WriteTypeDef(const Typedef& type)
{
	std::string typeName = "";
	copy_fixed_name(typeName, type.GetRef());

	writefmt(m_fp, "%s\t\tTYPEDEF\t\t%s\n\n", type.GetName().c_str(), typeName.c_str());
}

void MasmDriver::WriteStruct(const Struct& stru)
{
	PreprocessStruct(stru);
	// we need to remove "struct " part of the name, that's why there's + 7
	writefmt(m_fp, "%s\t\tSTRUCT %lldt\n", stru.GetName().c_str() + 7, stru.GetAlign() / 8);
	WriteStructMembers(stru);
	writefmt(m_fp, "%s\t\tENDS\n\n", stru.GetName().c_str() + 7);
}

void MasmDriver::WriteUnion(const Union& fnc)
{
	PreprocessStruct(dynamic_cast<const Struct&>(fnc));
	writefmt(m_fp, "%s\t\tUNION\n", fnc.GetName().c_str() + 6);
	WriteStructMembers(dynamic_cast<const Struct&>(fnc));
	writefmt(m_fp, "%s\t\tENDS\n\n", fnc.GetName().c_str() + 6);
}

void MasmDriver::WriteEnum(const Enum& fnc)
{
	for (const auto& p : fnc.GetFields())
	{
		writefmt(m_fp, "%s\t\tEQU\t\t%llut\n", p->GetName().c_str(), p->GetValue());
	}
}

void MasmDriver::WriteFunction(const Function& fnc)
{
	const char* callType = "";
	int isfirst = 1;

	if (m_platform.GetBits() != 64) // MASM x64 does not use C/PASCAL
	{
		callType = get_calling_string(fnc.GetCallType());
		if (!callType)
		{
			// MASM unsupported types
			writefmt(m_fp, "; function %s ignored as it uses an unsupported call type (%s)\n\n", fnc.GetName().c_str(), CallType2Str(fnc.GetCallType()));
			return;
		}
	}

	writefmt(m_fp, "@proto_%lld\t\tTYPEDEF\t\tPROTO %s ", m_total_protos, callType);

	// MASM does not write the return type so we skip that

	for (const auto& arg : fnc.GetArguments())
	{
		std::string type = "";

		if (isfirst)
			isfirst = 0;
		else
			writesng(m_fp, ", ");

		copy_fixed_name(type, arg.ref);
		writefmt(m_fp, ":%s", type.c_str());
	}

	if (fnc.IsVariadic())
		writesng(m_fp, ", :VARARG");

	writefmt(m_fp, "\n%s\t\tPROTO\t\t@proto_%lld\n\n", fnc.GetName().c_str(), m_total_protos);
	m_total_protos++;
}

void MasmDriver::WriteDefine(const Define& def)
{
	// TODO
}

void MasmDriver::WriteGlobalVar(const Typedef& def)
{
	// TODO!
}

void MasmDriver::AppendExtraDefines(std::vector<std::string>& defs)
{
	defs.push_back("MASM");
	defs.push_back("__MASM__");
}

const char* MasmDriver::GetName() { return "MASM CH2 Driver"; }

const char* MasmDriver::GetVersion() { return __TIMESTAMP__; }

const char* MasmDriver::GetAuthor() { return "lakor64"; }
