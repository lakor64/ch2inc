/**
* @file typedef.hpp
* @author lakor64
* @date 03/01/2024
* @brief type definition
*/
#pragma once

#include "basicmember.hpp"
#include "linktype.hpp"

/**
* a C typedef
*/
class Typedef : public BasicMember
{
	friend CH2Parser;
public:
	/**
	* Default constructor
	*/
	explicit Typedef() : BasicMember(MemberType::Typedef), m_size(0), m_ref() {}

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
	/** size of the type in bits */
	int64_t m_size;
	/** link to the type it references */
	LinkType m_ref;
};
