/**
* @file ch2errcode.hpp
* @author lakor64
* @date 03/01/2024
* @brief error codes
*/
#pragma once

enum class CH2ErrorCodes
{
	None,
	IndexError,
	OutOfMemory,
	Unknown,
	Crashed,
	ReadError,
	InvalidArgs,
	MissingType,
	MissingParent,
	BadParent,
	BadCallType,
};

const char* CH2ErrorCodeStr(CH2ErrorCodes err);
