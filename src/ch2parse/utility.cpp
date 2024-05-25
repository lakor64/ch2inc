/**
* @file utility.cpp
* @author lakor64
* @date 03/01/2024
* @brief Utility functions
*/
#include "utility.hpp"

PrimitiveType Utility::GetPrimitiveTypeForPlatform(const std::string& name, const PlatformInfo& platform)
{
	if (name == "char")
		return PrimitiveType::Byte;
	else if (name == "short")
		return PrimitiveType::Word;
	else if (name == "int")
	{
		if (platform.GetBits() == 16)
			return PrimitiveType::Word;

		return PrimitiveType::DWord;
	}
	else if (name == "long")
	{
		if ((platform.GetType() == PlatformType::Linux || platform.GetType() == PlatformType::Darwin) && platform.GetBits() == 64)
			return PrimitiveType::QWord;

		return PrimitiveType::DWord;
	}
	else if (name == "long long")
		return PrimitiveType::QWord;
	else if (name == "float")
		return PrimitiveType::Real4;
	else if (name == "double")
		return PrimitiveType::Real8;
	else if (name == "long double")
	{
		return platform.HaveReal10() ? PrimitiveType::Real10 : PrimitiveType::Real8;
	}
	else if (name == "wchar_t")
	{
		if (platform.GetType() == PlatformType::Linux || platform.GetType() == PlatformType::Darwin)
			return PrimitiveType::DWord;

		return PrimitiveType::Word;
	}
	else if (name == "__int128")
		return PrimitiveType::OWord;
	else if (name == "*")
	{
		switch (platform.GetBits())
		{
		case 16:
			return PrimitiveType::Word;
		case 32:
			return PrimitiveType::DWord;
		case 64:
			return PrimitiveType::QWord;
		default:
			break;
		}
	}

	return PrimitiveType::Invalid;
}

CH2ErrorCodes Utility::CXErrorToCH2Error(CXErrorCode ec)
{
	switch (ec)
	{
		//case CXError_Failure:
	default:
		break;
	case CXError_ASTReadError:
		return CH2ErrorCodes::ReadError;
	case CXError_Crashed:
		return CH2ErrorCodes::Crashed;
	case CXError_InvalidArguments:
		return CH2ErrorCodes::InvalidArgs;
	case CXError_Success:
		return CH2ErrorCodes::None;
	}

	return CH2ErrorCodes::Unknown;
}

CallType Utility::CXCallConvToCH2CallConv(CXCallingConv c)
{
	switch (c)
	{
	case CXCallingConv_C:
		return CallType::Cdecl;
	case CXCallingConv_X86StdCall:
		return CallType::Stdcall;
	case CXCallingConv_X86FastCall:
		return CallType::Fastcall;
	case CXCallingConv_X86Pascal:
		return CallType::Pascal;
	case CXCallingConv_X86VectorCall:
	case CXCallingConv_AArch64VectorCall:
		return CallType::Vectorcall;
	case CXCallingConv_Win64:
		return CallType::Cdecl;
	default:
		break;
	}

	return CallType::Invalid;
}

StorageType Utility::CXStorageTypeToCH2StorageType(CX_StorageClass c)
{
	switch (c)
	{
	case CX_SC_Extern:
	case CX_SC_PrivateExtern:
		return StorageType::Extern;
	case CX_SC_Register:
		return StorageType::Register;
	case CX_SC_Static:
		return StorageType::Static;
	default:
		break;
	}

	return StorageType::None;
}

std::string Utility::GetPrimitiveNameFromSize(int size)
{
	switch (size)
	{
	case 8:
		return "char";
	case 16:
		return "short";
	case 32:
		return "int";
	case 64:
		return "long long";
	case 80:
		return "long double";
	default:
		break;
	}

	return "";
}