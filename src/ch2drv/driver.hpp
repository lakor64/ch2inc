/**
* @file driver.hpp
* @author lakor64
* @date 03/01/2024
* @brief driver functions
*/
#pragma once

#include "platform.hpp"
#include "define.hpp"
#include "globalvar.hpp"
#include "function.hpp"
#include "enum.hpp"
#include "struct.hpp"

#include <vector>
#include <string>

/**
* Driver configuration
*/
struct DriverConfig
{
	/**
	* Default constructor
	*/
	explicit DriverConfig() : verbose(false), fp(nullptr) {}

	/**
	* Verbose error message logging
	*/
	bool verbose;

	/**
	* Platform info
	*/
	PlatformInfo platform;

	/**
	* File pointer
	*/
	FILE* fp;
};

/**
* Interface of a driver
* A driver is a component that implements the format of how the file is written to the disk
*/
class Driver
{
public:
	/**
	* Default deconstructor
	*/
	virtual ~Driver() = default;

	/**
	* Gets the name of the driver
	* @return name of the driver
	*/
	virtual const char* GetName() = 0;

	/**
	* Gets the version of the driver
	* @return version string
	*/
	virtual const char* GetVersion() = 0;

	/**
	* Gets the author of the driver
	* @return name of the author
	*/
	virtual const char* GetAuthor() = 0;

	/**
	* Adds extra defines, if any, to the parser command line
	* @param defs define array to modify
	*/
	virtual void AppendExtraDefines(std::vector<std::string>& defs) = 0;

	/**
	* Writes the start marker of the file
	*/
	virtual void WriteFileStart() = 0;

	/**
	* Writes the end marker of the file
	*/
	virtual void WriteFileEnd() = 0;

	/**
	* Writes a new comment
	* @param comment comment string
	*/
	virtual void WriteSingleComment(const std::string& comment) = 0;

	/**
	* Writes a multi comment
	* @param comments comments to write
	*/
	virtual void WriteMultiComment(const std::vector<std::string>& comments) = 0;

	/**
	* Writes a type definition
	* @param td Typedef type
	*/
	virtual void WriteTypeDef(const Typedef& td) = 0;
	
	/**
	* Writes a struct
	* @param stru Struct type
	*/
	virtual void WriteStruct(const Struct& stru) = 0;

	/**
	* Writes an union
	* @param union Union type
	*/
	virtual void WriteUnion(const Union& stru) = 0;

	/**
	* Writes an enumeration
	* @param enu Enum type
	*/
	virtual void WriteEnum(const Enum& enu) = 0;

	/**
	* Writes a function
	* @param fnc Function type
	*/
	virtual void WriteFunction(const Function& fnc) = 0;

	/**
	* Writes a define
	* @param def Define type
	*/
	virtual void WriteDefine(const Define& def) = 0;

	/**
	* Writes a global variable
	* @param def Variable type
	*/
	virtual void WriteGlobalVar(const GlobalVar& def) = 0;

	/**
	* Sets the driver config
	* @param cfg Driver config
	*/
	virtual void SetConfig(const DriverConfig& cfg) { m_cfg = cfg; }

protected:
	/**
	* Default constructor
	*/
	explicit Driver() {}

	/**
	* Driver config
	*/
	DriverConfig m_cfg;
};


/** callback that defines the entrypoint of the driver */
using DriverEntrypointFunc = Driver*(*)(void);

/** driver entrypoint */
#define DRIVER_ENTRYPOINT CH2DriverEntrypoint

/** driver entrypoint name */
#define DRIVER_ENTRYPOINT_NAME "CH2DriverEntrypoint"

#ifdef DISABLE_DYNLIB
/**
* Entrypoint of the driver
* @return the driver information
* @note the caller must manually free the driver pointer
*/
extern "C" Driver* CH2DriverEntrypoint(void);
#endif
