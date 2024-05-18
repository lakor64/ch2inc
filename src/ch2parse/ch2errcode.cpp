/**
* @file ch2errcode.cpp
* @author lakor64
* @date 03/01/2024
* @brief error codes
*/
#include "ch2errcode.hpp"

const char* CH2ErrorCodeStr(CH2ErrorCodes err)
{
	switch (err)
	{
	//case CH2ErrorCodes::Unknown
	default:
		break;
	case CH2ErrorCodes::IndexError:
		return "Parser index error";
	case CH2ErrorCodes::OutOfMemory:
		return "Out of memory";
	case CH2ErrorCodes::Crashed:
		return "Parser crashed";
	case CH2ErrorCodes::ReadError:
		return "Error during file read";
	case CH2ErrorCodes::InvalidArgs:
		return "Invalid parser arguments";
	case CH2ErrorCodes::MissingType:
		return "Missing a type declaration";
	case CH2ErrorCodes::MissingParent:
		return "Missing a parent";
	case CH2ErrorCodes::BadParent:
		return "Wrong parent type";
	case CH2ErrorCodes::BadCallType:
		return "Wrong call type";
	case CH2ErrorCodes::EvalError:
		return "Evalutation error";
	case CH2ErrorCodes::ValueError:
		return "Value error";
	}

	return "Unknown";
}
