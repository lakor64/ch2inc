/**
* @file primitive.cpp
* @author lakor64
* @date 04/01/2024
* @brief primitive helpers
*/
#include "primitive.hpp"

uint8_t PrimitiveGetBitSize(PrimitiveType t)
{
	switch (t)
	{
	case PrimitiveType::Byte:
		return 8;
	case PrimitiveType::Word:
		return 16;
	case PrimitiveType::DWord:
	case PrimitiveType::Real4:
		return 32;
	case PrimitiveType::QWord:
	case PrimitiveType::Real8:
		return 64;
	case PrimitiveType::Real10:
		return 80;
	default:
		break;
	}

	return 0;
}
