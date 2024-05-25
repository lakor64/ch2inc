/**
* @file masmdriver.hpp
* @author lakor64
* @date 03/01/2024
* @brief MASM driver functions
*/
#pragma once

#include <driver.hpp>
#include <vector>

class MasmDriver final : public Driver
{
public:
	/**
	* Default constructor
	*/
	explicit MasmDriver()
		: Driver()
		, m_total_preprocess_typedef(0)
		, m_total_used_typedef(0)
		, m_total_protos(0)
		, m_data_written(false)
	{}

	/**
	* Default destructor
	*/
	~MasmDriver() override = default;

	/**
	* Gets the name of the driver
	* @return name of the driver
	*/
	const char* GetName() override;

	/**
	* Gets the version of the driver
	* @return version string
	*/
	const char* GetVersion() override;

	/**
	* Gets the author of the driver
	* @return name of the author
	*/
	const char* GetAuthor() override;

	/**
	* Adds extra defines, if any, to the parser command line
	* @param defs define array to modify
	*/
	void AppendExtraDefines(std::vector<std::string>& defs) override;

	/**
	* Writes the start marker of the file
	*/
	void WriteFileStart() override;

	/**
	* Writes the end marker of the file
	*/
	void WriteFileEnd() override;

	/**
	* Writes a new comment
	* @param comment comment string
	*/
	void WriteSingleComment(const std::string& comment) override;

	/**
	* Writes a multi comment
	* @param comments comments to write
	*/
	void WriteMultiComment(const std::vector<std::string>& comments) override;

	/**
	* Writes a type definition
	* @param td Typedef type
	*/
	void WriteTypeDef(const Typedef& td) override;

	/**
	* Writes a struct
	* @param stru Struct type
	*/
	void WriteStruct(const Struct& stru) override;

	/**
	* Writes an union
	* @param union Union type
	*/
	void WriteUnion(const Union& stru) override;

	/**
	* Writes an enumeration
	* @param enu Enum type
	*/
	void WriteEnum(const Enum& enu) override;

	/**
	* Writes a function
	* @param fnc Function type
	*/
	void WriteFunction(const Function& fnc) override;

	/**
	* Writes a define
	* @param def Define type
	*/
	void WriteDefine(const Define& def) override;

	/**
	* Writes a global variable
	* @param def Variable type
	*/
	void WriteGlobalVar(const GlobalVar& def) override;

private:
	/**
	* Writes a function typedef
	* @param fnc Function type
	*/
	void WriteFunctionTypedef(const Function& fnc);

	/**
	* Adjust the type name of a link name
	* @param dst Destination string name
	* @param link Link type to check
	*/
	void CopyName(std::string& dst, const LinkType& link);

	/**
	* This code preprocess all the struct members in search 
	* of typedef to generate BEFORE starting to write the structure
	* @param stru Structure to preprocess
	*/
	void PreprocessStruct(const Struct& stru);

	/**
	* MASM does not directly support type declarations of pointers, so we need to generate
	* a new typedef and put them there.
	* This code will preprocess the variable and verify if we need to insert a TYPEDEF before the
	* type delcaration.
	* @param link Link to preprocess
	*/
	void PreprocessVariable(const Variable& link);

	/**
	* Writes the members of a structure or union
	* @param stru Structure to parse
	*/
	void WriteStructMembers(const Struct& stru);

	/**
	* Writes a variable
	* @param link Type link
	* @param field_name Type name
	*/
	void WriteVariable(const Variable& v);

	/** total preprocessed custom typedefs */
	int64_t m_total_preprocess_typedef;
	/** total used custom typedefs */
	int64_t m_total_used_typedef;
	/** total function protos */
	int64_t m_total_protos;
	/** total used tags */
	std::vector<std::string> m_tag_link;
	/** written the .DATA command */
	bool m_data_written;
};
