/**
* @file strconv.hpp
* @author lakor64
* @date 03/01/2024
* @brief MASM string converter
*/
#pragma once

#include <function.hpp>
#include <primitive.hpp>

/**
* Converts a primitive to a string for MASM
* @param p Primitive to convert
* @return name of the primitive
*/
static const char* get_primitive_name(const Primitive& p)
{
	switch (p.GetType())
	{
	case PrimitiveType::Byte:
		if (p.GetMod() == PrimitiveMods::Unsigned)
			return "BYTE";
		return "SBYTE";
	case PrimitiveType::Word:
		if (p.GetMod() == PrimitiveMods::Unsigned)
			return "WORD";
		return "SWORD";
	case PrimitiveType::DWord:
		if (p.GetMod() == PrimitiveMods::Unsigned)
			return "DWORD";
		return "SDWORD";
	case PrimitiveType::QWord:
		if (p.GetMod() == PrimitiveMods::Unsigned)
			return "QWORD";
		return "SQWORD";
	case PrimitiveType::OWord:
		return "OWORD"; // I can't find SOWORD in MSDN so I think it doesn't exists
	case PrimitiveType::Real4:
		return "REAL4";
	case PrimitiveType::Real8:
		return "REAL8";
	case PrimitiveType::Real10:
		return "REAL10";
	case PrimitiveType::Pointer:
		return ""; // skip, we can't write this
	default:
		break;
	}

	return p.GetName().c_str();
}

static const char* get_calling_string(CallType c)
{
	switch (c)
	{
	case CallType::Cdecl:
		return "C";
	case CallType::Stdcall:
		return "STDCALL";
	case CallType::Pascal:
		return "PASCAL";
	default:
		break;
	}

	return nullptr;
}
