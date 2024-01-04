/**
* @file masmdriver.hpp
* @author lakor64
* @date 03/01/2024
* @brief MASM driver functions
*/
#pragma once

#include <driver.hpp>

class MasmDriver final : public Driver
{
public:
	explicit MasmDriver()
		: Driver()
		, m_total_preprocess_typedef(0)
		, m_total_used_typedef(0)
		, m_total_protos(0)
	{}

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
	void WriteGlobalVar(const Typedef& def) override;

private:
	void PreprocessStruct(const Struct& stru);
	void WriteStructMembers(const Struct& stru);

	/** total preprocessed custom typedefs */
	int64_t m_total_preprocess_typedef;
	/** total used custom typedefs */
	int64_t m_total_used_typedef;
	/** total function protos */
	int64_t m_total_protos;
};
