/**
* @file clangcli.hpp
* @author lakor64
* @date 03/01/2024
* @brief clang cli constructor
*/
#pragma once

#include "options.hpp"

#include <vector>
#include <string>

/**
* Simple class to help making arguments to pass to clang
*/
struct ClangCli
{
	/**
	* Default constructor
	* @param opts Options to construct
	*/
	explicit ClangCli(const Options& opts);

	/**
	* Default deconstructor
	*/
	~ClangCli();

	/**
	* Number of converted arguments
	*/
	int argc;

	/**
	* Pointer to the argument data
	*/
	char** argv;
};
