/**
* @file dynlib.hpp
* @author lakor64
* @date 01/01/2024
* @brief dynamic library
*/
#pragma once

#ifndef DISABLE_DYNLIB

/**
* @typedef DynLib
* Definition of a dynamic library
*/
using DynLib = void*;

/**
* Loads a dynamic library
* @param file Location of the dynamic library
* @return A dynamic library referece or NULL if no library was loaded
*/
DynLib dynlib_load(const char* file);

/**
* Frees the memory allocated by a dynamic library reference
* @param lib Dynamic library reference to free
*/
void dynlib_free(DynLib lib);

/**
* Gets the exported function of a dynamic library
* @param lib Dynamic library reference
* @param funcname Function name
* @return A pointer to the function or NULL if the function was not found
*/
void* dynlib_getfunc(DynLib lib, const char* funcname);

#endif
