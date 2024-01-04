/**
* @file dynlib.hpp
* @author lakor64
* @date 01/01/2024
* @brief dynamic library
*/
#pragma once

#ifndef DISABLE_DYNLIB

using DynLib = void*;

DynLib dynlib_load(const char* file);
void dynlib_free(DynLib lib);
void* dynlib_getfunc(DynLib lib, const char* funcname);

#endif
