/**
* @file platform.cpp
* @author lakor64
* @date 03/01/2024
* @brief platform specific helper/structures
*/
#include "platform.hpp"
#include <string>
#include <algorithm>

void PlatformInfo::Set(const char* name, const char* bits, bool haveReal10, CallType defcf)
{
	std::string cxxname = name;

	if (bits[0] == '6' && bits[1] == '4')
		m_bits = 64;
	else if (bits[0] == '3' && bits[1] == '2')
		m_bits = 32;
	else if (bits[0] == '1' && bits[1] == '6')
		m_bits = 16;
	else
		m_bits = 0;

	std::transform(cxxname.begin(), cxxname.end(), cxxname.begin(),
		[](unsigned char c) { return std::tolower(c); });

	if (cxxname == "win")
		m_type = PlatformType::Win;
	else if (cxxname == "dos")
		m_type = PlatformType::DOS;
	else if (cxxname == "os2")
		m_type = PlatformType::OS2;
	else if (cxxname == "linux")
		m_type = PlatformType::Linux;
	else if (cxxname == "darwin")
		m_type = PlatformType::Darwin;
	else
		m_type = PlatformType::Invalid;

	if ((m_type == PlatformType::Darwin || m_type == PlatformType::Linux) && m_bits == 16)
	{
		m_type = PlatformType::Invalid;
	}

	m_real10 = haveReal10;

	if (defcf != CallType::Invalid)
		m_defct = defcf;
}

