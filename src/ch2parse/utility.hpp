/**
* @file utility.hpp
* @author lakor64
* @date 03/01/2024
* @brief Utility functions
*/
#pragma once

#include "platform.hpp"
#include "primitive.hpp"
#include "ch2errcode.hpp"
#include "calltype.hpp"
#include "storagetype.hpp"

#include <clang-c/CXErrorCode.h>
#include <clang-c/Index.h>

namespace Utility
{
	/**
	* Gets the primitive type for the specified platform
	* @param name Name of the type
	* @param platform Platform configuration
	* @return fetched primitive type
	*/
	PrimitiveType GetPrimitiveTypeForPlatform(const std::string& name, const PlatformInfo& platform);

	/**
	* Translates a libclang error into the library errors
	* @param ec Error to translate
	* @return library error
	*/
	CH2ErrorCodes CXErrorToCH2Error(CXErrorCode ec);

	/**
	* Converts a clang calling conversion to a library calling convention
	* @param c Calling convention to translate
	* @return library calling convention
	*/
	CallType CXCallConvToCH2CallConv(CXCallingConv c);

	/**
	* Converts a clang storage type to a library storage type
	* @param c Storage class to convert
	* @return library storage type
	*/
	StorageType CXStorageTypeToCH2StorageType(CX_StorageClass c);
}
