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

#include <clang-c/CXErrorCode.h>
#include <clang-c/Index.h>

namespace Utility
{
	PrimitiveType GetPrimitiveTypeForPlatform(const std::string& name, const PlatformInfo& platform);

	/**
	* Translates a libclang error into the library errors
	* @param ec Error to translate
	* @return library error
	*/
	CH2ErrorCodes CXErrorToCH2Error(CXErrorCode ec);

	CallType CXCallConvToCH2CallConv(CXCallingConv c);
}
