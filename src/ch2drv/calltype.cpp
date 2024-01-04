/**
* @file calltype.cpp
* @author lakor64
* @date 03/01/2024
* @brief call type
*/
#include "calltype.hpp"

const char* CallType2Str(CallType ct)
{
	switch (ct)
	{
	default:
	//case CallType::Invalid:
		break;
	case CallType::Stdcall:
		return "stdcall";
	case CallType::Cdecl:
		return "cdecl";
	case CallType::Fastcall:
		return "fastcall";
	case CallType::Vectorcall:
		return "vectorcall";
	case CallType::Pascal:
		return "pascal";
	}

	return "invalid";
}
