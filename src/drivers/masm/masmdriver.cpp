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


void MasmDriver::CopyName(std::string& dst, const LinkType& link)
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
		const std::string n = link.ref_type->GetName().c_str() + 6; // remove "union "
		for (size_t k = 0; k < m_tag_link.size(); k++)
		{
			if (m_tag_link[k] == n)
			{
				dst += "@tag_ " + std::to_string(k);
				return;
			}
		}
		dst += n;
	}
	else if (link.ref_type->GetTypeID() == MemberType::Struct)
	{
		const std::string n = link.ref_type->GetName().c_str() + 7; // remove "struct "
		for (size_t k = 0; k < m_tag_link.size(); k++)
		{
			if (m_tag_link[k] == n)
			{
				dst += "@tag_" + std::to_string(k);
				return;
			}
		}
		dst += n;
	}
	else
	{
		dst += link.ref_type->GetName();
	}
}

void MasmDriver::WriteType(const LinkType& ref, const std::string_view& field_name)
{
	std::string typeName = "";
	bool usetags = false;

	if (ref.pointers)
	{
		typeName = "@t_" + std::to_string(m_total_used_typedef);
		m_total_used_typedef++;
	}
	else
	{
		CopyName(typeName, ref);

		if (ref.ref_type->GetTypeID() == MemberType::Struct || ref.ref_type->GetTypeID() == MemberType::Union)
		{
			usetags = true;
		}
	}

	writefmt(m_fp, "{}\t\t{}\t\t{}\n", field_name, typeName, usetags ? "<>" : "?");

}

void MasmDriver::WriteStructMembers(const Struct& stru)
{
	int64_t totalprct = 0;
	const auto& fields = stru.GetFields();

	for (size_t i = 0; i < fields.size(); i++)
	{
		const auto& field = fields[i];

		if (field->GetSize() == -1) // bitsize > 0 works differently
		{
			WriteType(field->GetRef(), field->GetName());
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

			const std::string_view structname = stru.GetName().c_str() + 7;
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
			writefmt(m_fp, "rec@{}_{}\t\tRECORD\t", structname, totalprct);

			bool writeretn = false;

			if (processed != PrimitiveGetBitSize(prim))
			{
				// this adds the missing padding

				// @0@x:5
				writefmt(m_fp, "@0@{}:{}", structname, PrimitiveGetBitSize(prim) - processed);
				writeretn = true;
			}

			while (!m_bitstack.empty())
			{
				if (!writeretn)
					writeretn = true;
				else
					writefmt(m_fp, ",\n\t\t\t\t");

					// p@x:3
				const auto top = m_bitstack.top();
				writefmt(m_fp, "{}@{}:{}", top->GetName(), structname, top->GetSize());

				m_bitstack.pop();
			}

			// @bit_0  rec@x_0 <>
			writefmt(m_fp, "\n"
							"@bit_{}\t\trec@{}_{} <>\n", totalprct, structname, totalprct);

			totalprct++;
		}
	}
}

void MasmDriver::PreprocessType(const LinkType& link)
{
	if (link.pointers)
	{
		std::string fullname = "";

		for (long long i = 0; i < link.pointers; i++)
		{
			fullname += "PTR ";
		}

		if (link.ref_type->GetName() != "*")
			fullname += link.ref_type->GetName();
		else
			fullname = fullname.erase(fullname.size() - 2);

		if (link.ref_type->GetTypeID() == MemberType::Union)
		{
			const auto p = fullname.find("union ");
			fullname = fullname.erase(p, 6);
		}
		else if (link.ref_type->GetTypeID() == MemberType::Struct)
		{
			const auto p = fullname.find("struct ");
			fullname = fullname.erase(p, 7);
		}

		writefmt(m_fp, "@t_{}\t\tTYPEDEF\t\t{}\n", m_total_preprocess_typedef, fullname);
		m_total_preprocess_typedef++;
	}
}

void MasmDriver::PreprocessStruct(const Struct& stru)
{
	for (const auto& field : stru.GetFields())
	{
		PreprocessType(field->GetRef());
	}
}

void MasmDriver::WriteFileStart(void)
{
	writefmt(m_fp,
		"\n"
		"option expr32\n"
		"option casemap:none\n"
		"\n"
		"; Begin of the file\n"
	);
}

void MasmDriver::WriteFileEnd(void)
{
	writefmt(m_fp, "; End of the file\n");
}

void MasmDriver::WriteSingleComment(const std::string& comment)
{
	writefmt(m_fp, "; {}\n", comment);
}

void MasmDriver::WriteMultiComment(const std::vector<std::string>& v)
{
	writefmt(m_fp, "COMMENT @$?\n\n");
	for (const auto& comment : v)
	{
		writefmt(m_fp, comment);
	}
	writefmt(m_fp, "\n@$?\n");
}

void MasmDriver::WriteTypeDef(const Typedef& type)
{
	std::string typeName = "";
	CopyName(typeName, type.GetRef());

	writefmt(m_fp, "{}\t\tTYPEDEF\t\t{}\n\n", type.GetName(), typeName);
}

void MasmDriver::WriteStruct(const Struct& stru)
{
	PreprocessStruct(stru);
	std::string name = stru.GetName().c_str() + 7;

	if (stru.IsUnnamed())
	{
		m_tag_link.emplace_back(name);
		name = "@tag_" + std::to_string(m_tag_link.size() - 1);		
	}

	// we need to remove "struct " part of the name, that's why there's + 7
	writefmt(m_fp, "{}\t\tSTRUCT {}t\n", name, stru.GetAlign() / 8);
	WriteStructMembers(stru);
	writefmt(m_fp, "{}\t\tENDS\n\n", name);
}

void MasmDriver::WriteUnion(const Union& fnc)
{
	const std::string_view name = fnc.GetName().c_str() + 6;
	PreprocessStruct(dynamic_cast<const Struct&>(fnc));
	writefmt(m_fp, "{}\t\tUNION\n", name);
	WriteStructMembers(dynamic_cast<const Struct&>(fnc));
	writefmt(m_fp, "{}\t\tENDS\n\n", name);
}

void MasmDriver::WriteEnum(const Enum& fnc)
{
	for (const auto& p : fnc.GetFields())
	{
		writefmt(m_fp, "{}\t\tEQU\t\t{}t\n", p->GetName(), p->GetValue());
	}
}

void MasmDriver::WriteFunction(const Function& fnc)
{
	std::string_view callType;
	int isfirst = 1;

	if (m_platform.GetBits() != 64) // MASM x64 does not use C/PASCAL
	{
		const auto callTypeC = get_calling_string(fnc.GetCallType());
		if (!callTypeC)
		{
			// MASM unsupported types
			callType = CallType2Str(fnc.GetCallType());
			writefmt(m_fp, "; function {} ignored as it uses an unsupported call type ({})\n\n", fnc.GetName(), callType);
			return;
		}
		else
			callType = callTypeC;
	}

	writefmt(m_fp, "@proto_{}\t\tTYPEDEF\t\tPROTO {} ", m_total_protos, callType);

	// MASM does not write the return type so we skip that

	for (const auto& arg : fnc.GetArguments())
	{
		std::string type = "";

		if (isfirst)
			isfirst = 0;
		else
			writefmt(m_fp, ", ");

		CopyName(type, arg.ref);
		writefmt(m_fp, ":{}", type);
	}

	if (fnc.IsVariadic())
		writefmt(m_fp, ", :VARARG");

	writefmt(m_fp, "\n{}\t\tPROTO\t\t@proto_{}\n\n", fnc.GetName(), m_total_protos);
	m_total_protos++;
}

void MasmDriver::WriteDefine(const Define& def)
{
	// TODO
}

void MasmDriver::WriteGlobalVar(const GlobalVar& def)
{
	if (def.GetStorageType() == StorageType::Static)
	{
		if (!m_data_written)
		{
			// writes .DATA if we find static variables
			writefmt(m_fp, "\n.DATA\n\n");
			m_data_written = true;
		}

		PreprocessType(def.GetRef());
		WriteType(def.GetRef(), def.GetName());
		writefmt(m_fp, "\n");
	}
	else
	{
		// EXTERNDEF

		std::string name = "";
		CopyName(name, def.GetRef());
		writefmt(m_fp, "EXTERNDEF\t\tC\t{}:{}\n\n", def.GetName(), name);
	}
}

void MasmDriver::AppendExtraDefines(std::vector<std::string>& defs)
{
	defs.push_back("MASM");
	defs.push_back("__MASM__");
}

const char* MasmDriver::GetName() { return "MASM CH2 Driver"; }

const char* MasmDriver::GetVersion() { return __TIMESTAMP__; }

const char* MasmDriver::GetAuthor() { return "lakor64"; }
