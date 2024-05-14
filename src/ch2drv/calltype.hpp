/**
* @file calltype.hpp
* @author lakor64
* @date 03/01/2024
* @brief call type
*/
#pragma once

/**
* types of function call
*/
enum class CallType
{
	/** invalid type */
	Invalid,
	/** x86 stdcall */
	Stdcall,
	/** x86 cdecl, on x64 this is the default call */
	Cdecl,
	/** x86 fastcall */
	Fastcall,
	/** x86 vectorcall */
	Vectorcall,
	/** x86 pascal */
	Pascal,
};

/**
* Converts the library calltype to it's name rapresentation
* @param ct Calltype to convert
* @return string rapresentation of the calltype
*/
const char* CallType2Str(CallType ct);