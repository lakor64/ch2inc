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
	/**
	* Default constructor
	*/
	explicit PlatformInfo() : m_type(PlatformType::Invalid), m_bits(0), m_real10(false), m_defct(CallType::Cdecl) {}

	/**
	* Platform configuration constructor
	* @param name Platform name
	* @param bits Platform bits
	* @param haveReal10 true if the platform supports 80-bit float values
	* @param defcf Default calling convention
	*/
	explicit PlatformInfo(const char* name, const char* bits, bool haveReal10, CallType defcf) : m_type(PlatformType::Invalid), m_bits(0) {
		Set(name, bits, haveReal10, defcf);
	}

	/**
	* Sets the platform configuration
	* @param name Platform name
	* @param bits Platform bits
	* @param haveReal10 true if the platform supports 80-bit float values
	* @param defcf Default calling convention
	*/
	void Set(const char* name, const char* bits, bool haveReal10, CallType defcf);

	/**
	* Gets the default bits size of the platform
	* @return bit-size in int
	*/
	constexpr auto GetBits() const { return m_bits; }

	/**
	* Gets the type of this platform
	* @return Platform type
	* @see PlatformType
	*/
	constexpr auto GetType() const { return m_type; }

	/**
	* Checks if the platform can support "long double" as a 64-bit type
	* true if the platform does support it, otherwise false
	*/
	constexpr auto HaveReal10() const { return m_real10; }
	
	/**
	* Checks if the platform configuration is valid
	* @return true if it valid, otherwise false
	*/
	constexpr bool IsValid() const { return m_bits != 0 && m_type != PlatformType::Invalid; }

	/**
	* Gets the default calling type for the platform
	* @return Default calling type
	*/
	constexpr CallType GetDefaultCallType() const { return m_defct; }

private:
	/**
	* type of the platform
	* @see PlatformType
	*/
	PlatformType m_type;

	/**
	* number of bits in this platform (eg: 32)
	*/
	uint32_t m_bits;

	/** 
	* when false REAL10 is threated as REAL8 (no support for 80-bit float values)
	*/
	bool m_real10;

	/**
	* default calltype
	*/
	CallType m_defct;
};
