/**
* @file globalvar.hpp
* @author lakor64
* @date 15/05/2024
* @brief global variables type
*/
#pragma once

#include "typedef.hpp"
#include "storagetype.hpp"

/**
* A C global variable
*/
class GlobalVar final : public Typedef
{
	friend CH2Parser;
public:
	/**
	* Default constructor
	*/
	explicit GlobalVar() : Typedef(), m_storage(StorageType::None), m_volatile(false) { m_type = MemberType::GlobalVar; }

	/**
	* Gets the storage type of the variable
	* @return Variable storage type
	*/
	constexpr StorageType GetStorageType() const { return m_storage; }

	/**
	* Checks if the variable is volatile
	* @return true if the variable is volatile, otherwise false
	*/
	constexpr auto IsVolatile() const { return m_volatile; }

private:
	/** Variable storage type */
	StorageType m_storage;
	/** If the variable is volatile */
	bool m_volatile;
	/** If the variable is restricted */
	bool m_restrict;
};
