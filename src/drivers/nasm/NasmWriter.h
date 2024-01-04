#pragma once

#include "IWriter.h"
#include "NasmTypeDef.h"

#include <fstream>
#include <unordered_map>

class NasmWriter final : public IWriter
{
public:
	/**
	* Default constructor
	*/
	explicit NasmWriter() = default;


	/**
	* Default deconstructor
	*/
	~NasmWriter() override {
		Close();
	}

	/**
	* Opens a file to write output to
	* @param str File path
	* @return true if the file was opened, otherwise false
	*/
	bool Open(const std::string& str) override;

	/**
	* Closes the opened file
	*/
	void Close() override;

	/**
	* Writes a new line
	*/
	void WriteNewLine() override;

	/**
	* Writes a new comment, this is function is used for both single
	* and multi line comments
	* @param comment comment string
	*/
	void WriteComment(const std::string& comment) override;

	/**
	* Writes a typedefinition
	* @param name Name of the typedef
	* @param type String of the type (eg. usigned int)
	*/
	void WriteTypeDef(const std::string& name, const std::string& type) override;

	/**
	* Starts writing a struct
	* @param name Name of the struct
	* @param sizeOf Total size of the struct
	* @param align Alignment of the structure
	*/
	void WriteStructStart(const std::string& name, size_t sizeOf, size_t align) override;

	/**
	* Closes the writing of a struct
	* @param name Name of the union
	*/
	void WriteStructEnd(const std::string& name) override;

	/**
	* Starts writing an union
	* @param name Name of the union
	* @param sizeOf Total size of the union
	* @param align Alignment of the structure
	*/
	void WriteUnionStart(const std::string& name, size_t sizeOf, size_t align) override;

	/**
	* Closes the writing of an union
	* @param name Name of the union
	*/
	void WriteUnionEnd(const std::string& name) override;

	/**
	* Writes a struct or union field
	* @param name Name of the field
	* @param type String of the type (eg. usigned int)
	* @param bitSize Size in bits of the type
	*/
	void WriteField(const std::string& name, const std::string& type, const std::string& parentType, size_t bitSize) override;

	/**
	* Open the writing of an enumeration
	* @param name Enum name
	* @param size Size of the enumerator (eg enum X : uint32_t then it's 4)
	*/
	void WriteEnumStart(const std::string& name, size_t size) override;

	/**
	* Writes an enumerator field
	* @param name Name of the field
	* @param value Value of the field
	* @param size Size of the field
	*/
	void WriteEnumField(const std::string& name, size_t value, size_t size) override;

	/**
	* Closes the writing of an enumeration
	*/
	void WriteEnumEnd() override;

	/**
	* Opens the writing of a function
	* @param name Function name
	* @param ret_type Function return type
	* @param callConv Calling convention
	* @param info Extra function info
	*/
	void WriteFunctionBegin(const std::string& name, const std::string& ret_type,
		const std::string& callConv, int info, int argnum) override;

	/**
	* Closes the writing of a function
	*/
	void WriteFunctionEnd() override;

	/**
	* Writes the start marker of the file
	*/
	void WriteFileStart() override;

	/**
	* Writes the end marker of the file
	*/
	void WriteFileEnd() override;

private:

	/**
	* Gets a NASM string rapresentation of the primitive
	* @param p Primitive
	* @return string (eg: RESB)
	*/
	std::string getResType(PrimitiveType p);

	/**
	* Real function that writes a NASM struct (STRUC)
	* @param name Name of the structure
	* @param isUnion true if we are writing an union, otherwise false
	* @param sz Size of the struct
	* @param align Alignment of the struct
	*/
	void WriteStructReal(const std::string& name, bool isUnion, size_t sz, size_t align);

	/**
	* Gets the real type form a C type (without const or similar things, also resolved pointers)
	* @param type C type
	* @return compatible nasm type
	*/
	std::string GetRealType(const std::string& type);

	/**
	* Opened file
	*/
	std::ofstream m_file;

	/**
	* List of typedefs or struct defs
	*/
	std::unordered_map<std::string, NasmTypeDef> m_typeDefs;
};
