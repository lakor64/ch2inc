/**
* @file ch2errcode.hpp
* @author lakor64
* @date 03/01/2024
* @brief error codes
*/
#pragma once

/**
* @enum CH2ErrorCodes
* Error codes for the parser
*/
enum class CH2ErrorCodes
{
	/**
	* No error was found
	*/
	None,
	/**
	* Unable to create clang index
	*/
	IndexError,
	/**
	* Parser ran out of memory
	*/
	OutOfMemory,
	/**
	* Unknown error oncurred
	*/
	Unknown,
	/**
	* Clang parser crashed
	*/
	Crashed,
	/**
	* AST error during file reading
	*/
	ReadError,
	/**
	* Invalid clang arguments
	*/
	InvalidArgs,
	/**
	* Missing type link for a declaration
	*/
	MissingType,
	/**
	* Missing parent link on a child type
	*/
	MissingParent,
	/**
	* Bad type parent link
	*/
	BadParent,
	/**
	* Invalid calling convention
	*/
	BadCallType,
};

/**
* Converts an error code to it's string rapresentation
* @param err Error code to convert
* @return String rapresentation of the error code
*/
const char* CH2ErrorCodeStr(CH2ErrorCodes err);
