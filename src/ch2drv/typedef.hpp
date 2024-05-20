/**
* @file typedef.hpp
* @author lakor64
* @date 03/01/2024
* @brief type definition
*/
#pragma once

#include "variable.hpp"
#include "linktype.hpp"

/**
* a C typedef
*/
class Typedef : public Variable
{
	friend CH2Parser;
public:
	/**
	* Default constructor
	*/
	explicit Typedef() : Variable() { m_type = MemberType::Typedef; }
};
