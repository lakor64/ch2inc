/**
* @file define.hpp
* @author lakor64
* @date 03/01/2024
* @brief define type
*/
#pragma once

#include "basicmember.hpp"

/**
* A C preprocessor definition
* @note only basic definitions are supported, no function-like definition is actually supported
*/
class Define final : public BasicMember
{
public:
	explicit Define() : BasicMember(MemberType::Define) {}

	constexpr const auto& GetValue() const { return m_value; }
private:
	/** value of the define */
	std::string m_value;
};
