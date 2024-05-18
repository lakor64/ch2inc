/**
* @file define.hpp
* @author lakor64
* @date 03/01/2024
* @brief define type
*/
#pragma once

#include "basicmember.hpp"

/**
* Type of the define
*/
enum class DefineType
{
	/**
	* No define type
	*/
	None,

	/**
	* Integer type
	*/
	Integer,

	/**
	* Hexadecimal integer
	*/
	Hexadecimal,

	/**
	* Octal integer
	*/
	Octal,

	/**
	* Binary integer
	*/
	Binary,

	/**
	* Floating point integer
	*/
	Float,

	/**
	* String type
	*/
	String,

	/**
	* Raw text type
	*/
	Text,
};

/**
* A C preprocessor definition
* @note only basic definitions are supported, no function-like definition is actually supported
*/
class Define final : public BasicMember
{
	friend CH2Parser;
public:
	/**
	* Default constructor
	*/
	explicit Define() : BasicMember(MemberType::Define), m_defType(DefineType::None) {}

	/**
	* Gets the value of the define
	* @return Define value
	*/
	constexpr const auto& GetValue() const { return m_value; }

	/**
	* Gets the define type
	* @return the define type
	*/
	constexpr auto GetDefineType() const { return m_defType; }
private:
	/**
	* value of the define
	*/
	std::string m_value;

	/**
	* Type of the define
	*/
	DefineType m_defType;
};
