/**
* @file basicmember.hpp
* @author lakor64
* @date 03/01/2024
* @brief basic member
*/
#pragma once

#include <string>

/**
* Member type ID
*/
enum class MemberType
{
	/**
	* A C type definition
	* @see typedef_t
	*/
	Typedef,
	/**
	* A C structure
	* @see struct_t
	*/
	Struct,
	/**
	* A C union
	* @see struct_t
	*/
	Union,
	/**
	* A C struct field
	* @see field_t
	*/
	StructField,
	/**
	* A C enumeration
	* @see enum_t
	*/
	Enum,
	/**
	* A C enumeration field
	* @see enumfield_t
	*/
	EnumField,
	/**
	* A C function declaration
	* @see function_t
	*/
	Function,
	/**
	* Argument or return type of a function, essentially the function data
	* @see typedef_t
	*/
	FunctionType,
	/**
	* A global variable declaration
	* @see typedef_t
	*/
	GlobalVar,
	/**
	* A C preprocessor define
	* @see define_t
	*/
	Define,
	/**
	* A special primitive type (such as int,unsigned int)
	* This is used to keep tracking of the basic types that someone can construct
	* @see primitive_t
	*/
	Primitive,
};

class CH2Parser;

/**
* Basic member of all the fields inside a file
*/
class BasicMember
{
	friend CH2Parser;
public:
	virtual ~BasicMember() = default;

	constexpr const auto& GetName() const { return m_name; }
	constexpr auto GetTypeID() const { return m_type; }
protected:
	explicit BasicMember(MemberType type) : m_type(type) {}

	/** name of the file */
	std::string m_name;
	/**
	* id of the type
	* @see MemberType
	*/
	MemberType m_type;
};
