/**
* @file platform.hpp
* @author lakor64
* @date 03/01/2024
* @brief platform specific helper/structures
*/
#pragma once

#include <stdint.h>
#include "calltype.hpp"

/**
* Types of platforms the parser support
*/
enum class PlatformType
{
	/** invalid platform */
	Invalid,
	/** MS-DOS/DOS based platforms */
	DOS,
	/** Windows/NT based platforms */
	Win,
	/** MacOSX/Darwin based platforms */
	Darwin,
	/** Linux based platforms */
	Linux,
	/** OS/2 based platforms */
	OS2,
};

/**
* Simple class that contains the information for the current platform
*/
class PlatformInfo final
{
public:
	explicit PlatformInfo() : m_type(PlatformType::Invalid), m_bits(0), m_real10(false), m_defct(CallType::Cdecl) {}
	explicit PlatformInfo(const char* name, const char* bits, bool haveReal10, CallType defcf) : m_type(PlatformType::Invalid), m_bits(0) {
		Set(name, bits, haveReal10, defcf);
	}

	void Set(const char* name, const char* bits, bool haveReal10, CallType defcf);

	constexpr auto GetBits() const { return m_bits; }
	constexpr auto GetType() const { return m_type; }
	constexpr auto HaveReal10() const { return m_real10; }
	constexpr bool IsValid() const { return m_bits != 0 && m_type != PlatformType::Invalid; }
	constexpr CallType GetDefaultCallType() const { return m_defct; }

private:
	/**
	* type of the platform
	* @see PlatformType
	*/
	PlatformType m_type;
	/** number of bits in this platform (eg: 32) */
	uint32_t m_bits;
	/** when false REAL10 is threated as REAL8 */
	bool m_real10;
	/** default calltype */
	CallType m_defct;
};
