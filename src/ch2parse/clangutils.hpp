/**
* @file clangutils.hpp
* @author lakor64
* @date 03/01/2024
* @brief clang utility types
*/
#pragma once

#include <clang-c/CXString.h>

class ClangStr final
{
public:
	explicit ClangStr(CXString str)
	{
		m_str = clang_getCString(str);
		clang_disposeString(str);
	}

	const char* Get() const { return m_str.c_str(); }

	operator const char*() const { return Get(); }
	operator std::string() const { return m_str; }

private:
	std::string m_str;
};

static inline bool IsTypePointer(CXType type)
{
	return type.kind == CXType_Pointer ||                     // If cursor_type is a pointer
		type.kind == CXType_LValueReference ||              // or an LValue Reference (&)
		type.kind == CXType_RValueReference;
}
