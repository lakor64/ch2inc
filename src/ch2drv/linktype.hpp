/**
* @file linktype.hpp
* @author lakor64
* @date 03/01/2024
* @brief link type
*/
#pragma once

#include "basicmember.hpp"

/**
* Link types
*/
struct LinkType
{
	explicit LinkType() : ref_type(nullptr), pointers(0) {}

	/** base type link reference (if it's a pointer this will be the base type) */
	BasicMember* ref_type;
	/** number of pointers if any */
	long long pointers;
};
