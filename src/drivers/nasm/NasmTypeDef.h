#pragma once

#include "PrimitiveType.h"

#include <string>

/**
* Internal container of a typedef
*/
struct NasmTypeDef final
{
	/**
	* Name of the typedef
	*/
	std::string name;

	/**
	* Complex type name (used for struct or unions)
	* Only use this if primitiveType == complex
	*/
	std::string typeName;

	/**
	* Primitive type, only use if the type is not invalid
	*/
	PrimitiveTypeId primitive;

	/**
	* Union shadow type for equ
	* 
	*/
	std::string shadowType;

	/**
	* Default constructor
	*/
	explicit NasmTypeDef() 
		: primitive(PrimitiveTypeId::invalid)
	{
		name = "";
		typeName = "";
		shadowType = "";
	}
};
