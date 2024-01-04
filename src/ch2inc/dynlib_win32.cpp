/**
* @file dynlib_win32.cpp
* @author lakor64
* @date 01/01/2024
* @brief dynamic library support for Windows NT
*/
#include "dynlib.hpp"

#if !defined(DISABLE_DYNLIB) && defined(_WIN32)

#define WIN32_LEAN_AND_MEAN 1
#define STRICT 1
#include <Windows.h>


DynLib dynlib_load(const char* file)
{
	return (DynLib)LoadLibraryA(file);
}

void dynlib_free(DynLib lib)
{
	if (lib)
		FreeLibrary((HMODULE)lib);
}

void* dynlib_getfunc(DynLib lib, const char* funcname)
{
	return (void*)GetProcAddress((HMODULE)lib, funcname);
}

#endif
