/**
* @file variable.hpp
* @author lakor64
* @date 20/05/2024
* @brief variable declaration
*/
#pragma once

#include "basicmember.hpp"
#include "linktype.hpp"

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

private:
	/** If the variable is volatile */
	bool m_volatile;
	/** If the variable is restricted */
	bool m_restrict;
	/** size of the type in bits */
	int64_t m_size;
	/** link to the type it references */
	LinkType m_ref;
};
