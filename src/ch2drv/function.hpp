/**
* @file function.hpp
* @author lakor64
* @date 03/01/2024
* @brief function type
*/
#pragma once

#include "calltype.hpp"
#include "linktype.hpp"
#include "storagetype.hpp"
#include "variable.hpp"
#include <string>
#include <vector>

/**
* A C function declaration
*/
class Function final : public BasicMember
{
	friend CH2Parser;
public:
	/**
	* Default constructor
	*/
	explicit Function() : BasicMember(MemberType::Function), m_calltype(CallType::Invalid), m_variadic(false), m_ret(), m_storage(StorageType::None), m_pointers(-1) {}

	/**
	* Gets the calling convention of this function
	* @return Calling convention of the function
	*/
	constexpr auto GetCallType() const { return m_calltype; }

	/**
	* Checks if the function is a variadic function or not
	* @return true if the function is variadic, otherwise false
	*/
	constexpr auto IsVariadic() const  { return m_variadic; }

	/**
	* Gets the return type of this function
	* @return Return type
	*/
	constexpr const auto& GetReturnType() const { return m_ret; }

	/**
	* Gets the arguments of this function
	* @return Array of arguments
	*/
	constexpr const auto& GetArguments() const { return m_arguments; }

	/**
	* Gets the storage type of the variable
	* @return Variable storage type
	*/
	constexpr StorageType GetStorageType() const { return m_storage; }

	/**
	* Checks if the function is a typedef
	* @return true if it's a typedef not a real function, otherwise false
	*/
	constexpr auto& IsTypedef() const { return m_typedef; }

	/**
	* Gets the number of typedef pointers in the function
	* @return true if it have pointers, otherwise false
	*/
	constexpr auto GetPointers() const { return m_pointers; }

private:
	/**
	* type of the function call
	* @see CallType
	*/
	CallType m_calltype;
	/**
	* if the function is variadic
	*/
	bool m_variadic;
	/**
	* return type of a function
	* @note if the ref_type is nullptr then the return type is "void"
	*/
	Variable m_ret;
	/**
	* list of all arguments of the function
	*/
	std::vector<Variable> m_arguments;

	/**
	* Type of storage
	*/
	StorageType m_storage;

	/**
	* If the function is a typedef prototype
	*/
	bool m_typedef;

	/**
	* Pointers size
	*/
	int m_pointers;
};
