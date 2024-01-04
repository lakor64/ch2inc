/**
* @file ch2inc.hpp
* @author lakor64
* @date 03/01/2024
* @brief main ch2inc app
*/
#pragma once

#include "options.hpp"
#include "driver.hpp"
#include "dynlib.hpp"

#include <ch2parser.hpp>
#include <cxxopts.hpp>

/**
* Main bootstrap of the application
*/
class CH2Inc final
{
public:
	/**
	* Default constructor
	*/
	explicit CH2Inc();

	/**
	* Default deconstructor
	*/
	~CH2Inc();

	/**
	* Runs the program
	*/
	int Run(int argc, char** argv);

private:
	/**
	* Parses the command line
	*/
	int ParseCli(int argc, char** argv);

	/**
	* Shows the help message
	*/
	void ShowHelp();

	/**
	* Sets up the driver
	*/
	bool SetupDriver();

	/**
	* Adds default platform defines
	*/
	void AddDefaultData();

	/** ch2 parser */
	CH2Parser m_parser;

	/** serialized file */
	CFile m_file;

	/** options parser */
	cxxopts::Options m_opt;

	/** serialized options */
	Options m_sopts;

	/** output file */
	FILE* m_fp;

	/** driver entrypoint */
	DriverEntrypointFunc m_drvep;

	/** driver functions */
	Driver* m_drvfnc;

#ifndef DISABLE_DYNLIB
	/** driver library */
	DynLib m_drv;
#endif

};
