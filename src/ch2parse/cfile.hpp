/**
* @file cfile.hpp
* @author lakor64
* @date 03/01/2024
* @brief C file container
*/
#pragma once

#include "basicmember.hpp"
#include "platform.hpp"
#include "primitive.hpp"

#include <string>
#include <unordered_map>

/**
* The serialized C file
*/
class CFile final
{
	friend CH2Parser;
public:
	/**
	* Default constructor
	*/
	explicit CFile() {}

	/**
	* Default deconstructor
	*/
	~CFile() { for (auto& x : m_types) delete x; m_types.clear(); }

	/**
	* Gets the loaded types of the header file
	* @return Array of types
	*/
	constexpr const auto& GetTypes() const { return m_types; }

	/**
	* Gets the file name of the file
	* @return File name
	*/
	constexpr const auto& GetFileName() const { return m_filename; }

private:
	/** name of the file */
	std::string m_filename;
	/** all the types found this file */
	std::vector<BasicMember*> m_types;
};
