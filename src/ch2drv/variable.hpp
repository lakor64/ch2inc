/**
* @file variable.hpp
* @author lakor64
* @date 20/05/2024
* @brief variable declaration
*/
#pragma once

#include "basicmember.hpp"
#include "linktype.hpp"
#include <vector>

/**
* A variable, either in a function as argument or global variable
*/
class Variable : public BasicMember
{
	friend CH2Parser;

public:
	/**
	* Default constructor
	*/
	explicit Variable() : BasicMember(MemberType::GlobalVar), m_volatile(false), m_restrict(false), m_size(0), m_ref() {}

	/**
	* Checks if the variable is volatile
	* @return true if the variable is volatile, otherwise false
	*/
	constexpr auto IsVolatile() const { return m_volatile; }

	/**
	* Checks if the variable is restricted
	* @return true if the variable is restricted, otherwise false
	*/
	constexpr auto IsRestricted() const { return m_restrict; }


	/**
	* Gets the size of the typedef in bits
	* @return Size of the typedef in bits
	*/
	constexpr auto GetSize() const { return m_size; }

	/**
	* Gets the reference of this type
	* @return Type reference
	*/
	const auto& GetRef() const { return m_ref; }

	/**
	* Gets all the array sizes
	* @return All the recorded array sizes
	* @note if you have an array which size > 0 and their value is -1,
	*  it means it's an incomplete array (like b[]), otherwise threat it
	*  as a normal array.
	* A value that's not an array would have m_array.size() be 0
	*/
	constexpr const auto& GetArraySizes() const { return m_array; }

private:
	/** If the variable is volatile */
	bool m_volatile;
	/** If the variable is restricted */
	bool m_restrict;
	/** size of the type in bits */
	int64_t m_size;
	/** link to the type it references */
	LinkType m_ref;
	/** array sizes */
	std::vector<int> m_array;
};
