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
#include <string>
#include <vector>

/**
* Data in the function, either an argument or return type
*/
struct FuncData
{
	explicit FuncData() : ref(), volatil(false), restric(false) {}

	/** reference type */
	LinkType ref;
	/** name info */
	std::string name;
	/** if it's volatile */
	bool volatil;
	/** if it's restrict */
	bool restric;
};

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
	explicit Function() : BasicMember(MemberType::Function), m_calltype(CallType::Invalid), m_variadic(false), m_ret(), m_storage(StorageType::None) {}

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
	FuncData m_ret;
	/**
	* list of all arguments of the function
	*/
	std::vector<FuncData> m_arguments;

	/**
	* Type of storage
	*/
	StorageType m_storage;
};
