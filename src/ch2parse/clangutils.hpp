/**
* @file clangutils.hpp
* @author lakor64
* @date 03/01/2024
* @brief clang utility types
*/
#pragma once

#include <clang-c/CXString.h>

/**
* Simple utility wrapper of a clang string
*/
class ClangStr final
{
public:
	/**
	* Default constructor
	* @param str clang string to fetch
	*/
	explicit ClangStr(CXString str)
	{
		m_str = clang_getCString(str);
		clang_disposeString(str);
	}

	/**
	* Gets the string name
	*/
	const char* Get() const { return m_str.c_str(); }

	/**
	* Get operator override c string
	*/
	operator const char*() const { return Get(); }

	/**
	* Get operator override c++ string
	*/
	operator std::string() const { return m_str; }

private:
	/**
	* Retrived string
	*/
	std::string m_str;
};

/**
* Checks if the specified type is a pointer type
* @param type Type to check
* @return true if the type is a pointer, otherwise false
*/
static constexpr bool IsTypePointer(CXType type)
{
	return type.kind == CXType_Pointer ||                     // If cursor_type is a pointer
		type.kind == CXType_LValueReference ||              // or an LValue Reference (&)
		type.kind == CXType_RValueReference;
}
