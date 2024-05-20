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

// TODO: Refactor this crappy code to properly use Variable

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

// TODO: this should be WriteVariable
void MasmDriver::WriteVariable(const Variable& v)
{
	std::string typeName = "";
	bool usetags = false;

	if (v.GetRef().pointers)
	{
		typeName = "@t_" + std::to_string(m_total_used_typedef);
		m_total_used_typedef++;
	}
	else
	{
		CopyName(typeName, v.GetRef());

		if (v.GetRef().ref_type->GetTypeID() == MemberType::Struct || v.GetRef().ref_type->GetTypeID() == MemberType::Union)
		{
			usetags = true;
		}
	}

	writefmt(m_cfg.fp, "{}\t\t{}\t\t", v.GetName(), typeName);

	const auto& az = v.GetArraySizes();

	if (az.size() > 0 && !usetags) // apperently for MASM only basic types are supported
	{
		for (const auto& dups : az)
		{
			writefmt(m_cfg.fp, " {}t DUP (", dups);
		}

		writefmt(m_cfg.fp, "?");

		for (const auto& dups : az)
		{
			writefmt(m_cfg.fp, ")");
		}
	}
	else
	{
		writefmt(m_cfg.fp, " {}\n", usetags ? "<>" : "?");
	}
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
			WriteVariable(*field);
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
			writefmt(m_cfg.fp, "rec@{}_{}\t\tRECORD\t", structname, totalprct);

			bool writeretn = false;

			if (processed != PrimitiveGetBitSize(prim))
			{
				// this adds the missing padding

				// @0@x:5
				writefmt(m_cfg.fp, "@0@{}:{}", structname, PrimitiveGetBitSize(prim) - processed);
				writeretn = true;
			}

			while (!m_bitstack.empty())
			{
				if (!writeretn)
					writeretn = true;
				else
					writefmt(m_cfg.fp, ",\n\t\t\t\t");

					// p@x:3
				const auto top = m_bitstack.top();
				writefmt(m_cfg.fp, "{}@{}:{}", top->GetName(), structname, top->GetSize());

				m_bitstack.pop();
			}

			// @bit_0  rec@x_0 <>
			writefmt(m_cfg.fp, "\n"
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
		{
			if (link.ref_type->GetTypeID() == MemberType::Primitive)
				fullname += get_primitive_name(*dynamic_cast<Primitive*>(link.ref_type));
			else
				fullname += link.ref_type->GetName();
		}
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

		writefmt(m_cfg.fp, "@t_{}\t\tTYPEDEF\t\t{}\n", m_total_preprocess_typedef, fullname);
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
	writefmt(m_cfg.fp,
		"\n"
		"option expr32\n"
		"option casemap:none\n"
		"\n"
		"; Begin of the file\n"
	);
}

void MasmDriver::WriteFileEnd(void)
{
	writefmt(m_cfg.fp, "; End of the file\n");
}

void MasmDriver::WriteSingleComment(const std::string& comment)
{
	writefmt(m_cfg.fp, "; {}\n", comment);
}

void MasmDriver::WriteMultiComment(const std::vector<std::string>& v)
{
	writefmt(m_cfg.fp, "COMMENT @$?\n\n");
	for (const auto& comment : v)
	{
		writefmt(m_cfg.fp, comment);
	}
	writefmt(m_cfg.fp, "\n@$?\n");
}

void MasmDriver::WriteTypeDef(const Typedef& type)
{
	std::string typeName = "";
	CopyName(typeName, type.GetRef());

	writefmt(m_cfg.fp, "{}\t\tTYPEDEF\t\t{}\n\n", type.GetName(), typeName);
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
	writefmt(m_cfg.fp, "{}\t\tSTRUCT {}t\n", name, stru.GetAlign() / 8);
	WriteStructMembers(stru);
	writefmt(m_cfg.fp, "{}\t\tENDS\n\n", name);
}

void MasmDriver::WriteUnion(const Union& fnc)
{
	const std::string_view name = fnc.GetName().c_str() + 6;
	PreprocessStruct(dynamic_cast<const Struct&>(fnc));
	writefmt(m_cfg.fp, "{}\t\tUNION\n", name);
	WriteStructMembers(dynamic_cast<const Struct&>(fnc));
	writefmt(m_cfg.fp, "{}\t\tENDS\n\n", name);
}

void MasmDriver::WriteEnum(const Enum& fnc)
{
	for (const auto& p : fnc.GetFields())
	{
		writefmt(m_cfg.fp, "{}\t\tEQU\t\t{}t\n", p->GetName(), p->GetValue());
	}
}

void MasmDriver::WriteFunction(const Function& fnc)
{
	std::string_view callType;
	int isfirst = 1;

	if (m_cfg.platform.GetBits() != 64) // MASM x64 does not use C/PASCAL
	{
		const auto callTypeC = get_calling_string(fnc.GetCallType());
		if (!callTypeC)
		{
			// MASM unsupported types
			callType = CallType2Str(fnc.GetCallType());

			if (m_cfg.verbose)
				writefmt(m_cfg.fp, "; function {} ignored as it uses an unsupported call type ({})\n\n", fnc.GetName(), callType);
	
			return;
		}
		else
			callType = callTypeC;
	}

	writefmt(m_cfg.fp, "@proto_{}\t\tTYPEDEF\t\tPROTO {} ", m_total_protos, callType);

	// MASM does not write the return type so we skip that

	for (const auto& arg : fnc.GetArguments())
	{
		std::string type = "";

		if (isfirst)
			isfirst = 0;
		else
			writefmt(m_cfg.fp, ", ");

		CopyName(type, arg.GetRef());
		writefmt(m_cfg.fp, ":{}", type);
	}

	if (fnc.IsVariadic())
		writefmt(m_cfg.fp, ", :VARARG");

	writefmt(m_cfg.fp, "\n{}\t\t{}\t\t@proto_{}\n\n", fnc.GetName(), fnc.IsTypedef() ? "TYPEDEF PTR" : "PROTO", m_total_protos);
	m_total_protos++;
}

void MasmDriver::WriteDefine(const Define& def)
{
	const auto& deftype = def.GetDefineType();

	std::string postfix = "";
	std::string prefix = "";
	std::string cmd = "EQU";

	switch (deftype)
	{
	case DefineType::None:
	case DefineType::Float:
	case DefineType::Binary:
	//case DefineType::String:
		if (m_cfg.verbose)
			writefmt(m_cfg.fp, "; Unsupported macro {}\n", def.GetName());
	
		return;

	case DefineType::String:
	case DefineType::Text:
		prefix = "<";
		postfix = ">";
		cmd = "TEXTEQU";
		break;

	case DefineType::Integer:
		postfix = "t";
		break;

	case DefineType::Hexadecimal:
		postfix = "h";
		break;

	case DefineType::Octal:
		postfix = "o";
		break;
	}

	writefmt(m_cfg.fp, "{}\t\t{}\t\t{}{}{}\n", def.GetName(), cmd, prefix, def.GetValue(), postfix);
}

void MasmDriver::WriteGlobalVar(const GlobalVar& def)
{
	if (def.GetStorageType() == StorageType::Static)
	{
		if (!m_data_written)
		{
			// writes .DATA if we find static variables
			writefmt(m_cfg.fp, "\n.DATA\n\n");
			m_data_written = true;
		}

		PreprocessType(def.GetRef());
		WriteVariable(def);
		writefmt(m_cfg.fp, "\n");
	}
	else
	{
		// EXTERNDEF

		std::string name = "";
		CopyName(name, def.GetRef());
		writefmt(m_cfg.fp, "EXTERNDEF\t\tC\t{}:{}\n\n", def.GetName(), name);
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
