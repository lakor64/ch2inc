#include "NasmWriter.h"
#include "Utility.h"

#include <vector>
#include <algorithm>

bool NasmWriter::Open(const std::string& str)
{
	Close();
	m_file.open(str);
	return m_file.is_open();
}

void NasmWriter::Close()
{
	m_file.close();
}

void NasmWriter::WriteComment(const std::string& comment)
{
	std::string c = ReplaceAll(comment, "\n", "\n; ");
	m_file << "; " << c << "\n";
}

void NasmWriter::WriteNewLine()
{
	m_file << "\n";
}

/* struct writing */
void NasmWriter::WriteStructStart(const std::string& name, size_t sz, size_t align)
{
	WriteStructReal(name, false, sz, align);
}

void NasmWriter::WriteStructReal(const std::string& name, bool isUnion, size_t sz, size_t)
{
	m_file << "STRUC " << name << "\n";

	// remove struct/union types
	auto typedefName = ReplaceAll(name, "union ", "");
	typedefName = ReplaceAll(typedefName, "struct ", "");

	// allow to declare struct/unions in fields
	NasmTypeDef td;
	td.name = typedefName;
	td.primitive = isUnion ? PrimitiveTypeId::union_type : PrimitiveTypeId::struct_type;

	if (isUnion)
	{
		td.shadowType = ".UNION_SHADOWTYPE_" + gen_random(10);

		m_file << td.shadowType << "\tRESB " << sz << "\n";
	}

	m_typeDefs.insert_or_assign(typedefName, td);
}

void NasmWriter::WriteField(const std::string& name, const std::string& type, const std::string& parentType, size_t bitSize)
{
	auto x = GetRealType(type);
	auto y = GetRealType(parentType);

	if (bitSize != -1)
	{
		m_file << "%error BitFields are not supported in NASM: " << name << "\n";
		return;
	}

	auto it = m_typeDefs.find(type);

	/*
		this are struct or unions, we don't have a real way
		to tell nasm what size are the types we define more or less
		so we just need to make a byte array that is the same size as the
		complex type
	*/

	std::string strType;

	if (it != m_typeDefs.end())
	{
		strType = "RESB\t";
		strType += x + "_size";
	}
	else
	{
		auto p = GetPrimitiveTypeForPlatform(x, m_platform);
		// transform primitive type to basic type if any, or threat it as a typedef (define)
		if (p.id == PrimitiveTypeId::invalid)
			strType = x;
		else 
			strType = getResType(p) + " 1";
	}

	it = m_typeDefs.find(y);

	if (it == m_typeDefs.end())
	{
		m_file << "%error invalid type " << y << " for field " << name;
		return;
	}
	
	m_file << "." << name << "\t";

	if (!it->second.shadowType.empty())
		m_file << "EQU\t" << it->second.shadowType; // this is an union
	else
		m_file << strType;

	m_file << "\n";
}

void NasmWriter::WriteStructEnd(const std::string&)
{
	m_file << "ENDSTRUC\n";
}

/* union writing */
void NasmWriter::WriteUnionStart(const std::string& name, size_t sz, size_t align)
{
	WriteStructReal(name, true, sz, align);
}

void NasmWriter::WriteUnionEnd(const std::string& name)
{
	WriteStructEnd(name);
}

void NasmWriter::WriteTypeDef(const std::string& name, const std::string& type)
{
	std::string pstr;

	auto p = GetPrimitiveTypeForPlatform(type, m_platform);
	if (p.id == PrimitiveTypeId::invalid)
	{
		pstr = GetRealType(type);
	}
	else
	{
		pstr = getResType(p) + " 1";
	}

	m_file << "%define\t" << name << "\t" << pstr << "\n";
}

std::string NasmWriter::getResType(PrimitiveType p)
{
	switch (p.id)
	{
	case PrimitiveTypeId::byte:
		return "RESB";
	case PrimitiveTypeId::word:
		return "RESW";
	case PrimitiveTypeId::dword:
		return "RESD";
	case PrimitiveTypeId::qword:
		return "RESQ";
	default: // we should NEVER hit this
		break;
	}

	return "%error Invalid primitive type " + std::to_string((int)p.id);
}

void NasmWriter::WriteEnumEnd()
{
	// nasm doesn't need this
}

void NasmWriter::WriteEnumStart(const std::string&, size_t)
{
	// nasm doesn't need this
}

void NasmWriter::WriteEnumField(const std::string& name, size_t value, size_t)
{
	m_file << name << " EQU " << value << "\n";
}

std::string NasmWriter::GetRealType(const std::string& type)
{
	if (type.find('*') != -1) // pointer type
	{
		if (m_platform.bits == 32)
			return "RESD 1";
		else if (m_platform.bits == 64)
			return "RESQ 1";

		return "RESW 1";
	}

	std::string x = type;
	DoTypeFix(x);
	// remove struct and union
	x = ReplaceAll(x, "struct ", "");
	x = ReplaceAll(x, "union ", "");

	return x;
}

void NasmWriter::WriteFunctionBegin(const std::string& name, const std::string& ret_type,
	const std::string& callConv, int info, int argnum)
{
	m_file << "extern ";

	if ((callConv == "cdecl" || callConv == "stdcall") && m_platform.type == PlatformType::Windows)
		m_file << "_"; // required for cdecl symbols
	else if ((callConv == "fastcall") && m_platform.type == PlatformType::Windows)
		m_file << "@";

	m_file << name;

	if ((callConv == "fastcall" || callConv == "stdcall" || callConv == "vectorcall") && m_platform.type == PlatformType::Windows)
	{
		// required for stdcall/fastcall/vectorcall
		m_file << "@";
		if (callConv == "vectorcall")
			m_file << "@";

		m_file << (argnum * (m_platform.bits / 8));
	}

	m_file << "\n";
}

void NasmWriter::WriteFunctionEnd()
{
	// nasm doesn't need this
}

void NasmWriter::WriteFileEnd()
{
	// nasm doesn't need this
}

void NasmWriter::WriteFileStart()
{
	// TODO: should be add .cpu i386 and similar?
}
