/**
* @file function.hpp
* @author lakor64
* @date 03/01/2024
* @brief function type
*/
#pragma once

#include "calltype.hpp"
#include "linktype.hpp"
#include <string>
#include <vector>

/**
* Data in the function, either an argument or return type
*/
struct FuncData
{
	explicit FuncData() : ref() {}

	/** reference type */
	LinkType ref;
	/** name info */
	std::string name;
};

/**
* A C function declaration
*/
class Function final : public BasicMember
{
	friend CH2Parser;
public:
	explicit Function() : BasicMember(MemberType::Function), m_calltype(CallType::Invalid), m_variadic(false), m_ret() {}

	constexpr auto GetCallType() const { return m_calltype; }
	constexpr auto IsVariadic() const  { return m_variadic; }
	constexpr const auto& GetReturnType() const { return m_ret; }
	constexpr const auto& GetArguments() const { return m_arguments; }

private:
	/**
	* type of the function call
	* @see CallType
	*/
	CallType m_calltype;
	/** if the function is variadic */
	bool m_variadic;
	/**
	* return type of a function
	* @note if the ref_type is nullptr then the return type is "void"
	*/
	FuncData m_ret;
	/** list of all arguments of the function */
	std::vector<FuncData> m_arguments;
};
