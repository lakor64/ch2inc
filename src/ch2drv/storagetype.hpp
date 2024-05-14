/**
* @file storagetype.hpp
* @author lakor64
* @date 15/05/2024
* @brief function or variable storage type
*/
#pragma once

/**
* Types of storage
*/
enum class StorageType
{
	/**
	* No storage type
	*/
	None,

	/**
	* Static storage type
	*/
	Static,

	/**
	* External linkage storage type
	*/
	Extern,

	/**
	* Register storage type
	*/
	Register,
};
