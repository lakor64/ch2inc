/**
* @file options.hpp
* @author lakor64
* @date 03/01/2024
* @brief app options
*/
#pragma once

#include <platform.hpp>

#include <vector>
#include <string>

/**
* Simple structure to hold options
*/
struct Options
{
	/**
	* Default constructor
	*/
	explicit Options() : info(), nologo(false), msvc(false), verbose(false) {}

	/** Platform info */
	PlatformInfo info;
	/** File input */
	std::string input;
	/** File output */
	std::string output;
	/** List of includes */
	std::vector<std::string> includes;
	/** List of defines */
	std::vector<std::string> defines;
	/** Extra flags to pass to clang */
	std::vector<std::string> extra;
	/** List of undefines */
	std::vector<std::string> undef;
	/** Disables printing of logo */
	bool nologo;
	/** Run in MSVC compatibility mode */
	bool msvc;
	/** Verbose logging */
	bool verbose;
#ifndef DISABLE_DYNLIB
	/** Driver name */
	std::string driver;
#endif
};
