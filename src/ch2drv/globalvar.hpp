/**
* @file globalvar.hpp
* @author lakor64
* @date 15/05/2024
* @brief global variables type
*/
#pragma once

#include "variable.hpp"
#include "typedef.hpp"
#include "storagetype.hpp"

/**
* A C global variable
*/
class GlobalVar final : public Variable
{
	friend CH2Parser;
public:
	/**
	* Default constructor
	*/
	explicit GlobalVar() : Variable(), m_storage(StorageType::None) { m_type = MemberType::GlobalVar; }

	/**
	* Gets the storage type of the variable
	* @return Variable storage type
	*/
	constexpr StorageType GetStorageType() const { return m_storage; }

private:
	/** Variable storage type */
	StorageType m_storage;
};
