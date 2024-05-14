/**
* @file primitive.hpp
* @author lakor64
* @date 03/01/2024
* @brief primitive type
*/
#pragma once

#include "basicmember.hpp"

#include <string>

/**
* Primitive type IDs
*/
enum class PrimitiveType
{
	/** invalid primitive */
	Invalid,
	/** 1-byte number */
	Byte,
	/** 2-bytes number */
	Word,
	/** 4-bytes number */
	DWord,
	/** 8-bytes number */
	QWord,
	/** 16-bytes number */
	OWord,
	/** 4-bytes floating number */
	Real4,
	/** 8-bytes floating number */
	Real8,
	/** 10-bytes floating number */
	Real10,
	/** pointer primitive */
	Pointer,
};

/**
* Primitive modificators
*/
enum class PrimitiveMods
{
	/** default type */
	Default,
	/** primitive is explicitally signed */
	Signed,
	/** primitive is explicitally unsigned */
	Unsigned,
};

/**
* Primitive types
*/
class Primitive final : public BasicMember
{
	friend CH2Parser;
public:
	/**
	* Default constructor
	*/
	explicit Primitive() : BasicMember(MemberType::Primitive), m_type(PrimitiveType::Invalid), m_mod(PrimitiveMods::Default) {}

	/**
	* Gets the type of the primitive
	* @return Primitive type
	*/
	constexpr auto GetType() const { return m_type; }

	/**
	* Gets the primitive modificators (eg: signed)
	* @return Primitive modificators
	*/
	constexpr auto GetMod() const { return m_mod; }
private:
	/**
	* Type of the primitive
	* @see PrimitiveType
	*/
	PrimitiveType m_type;
	/**
	* modificator of the primitive
	* @see PrimitiveMods
	*/
	PrimitiveMods m_mod;
};

/**
* Gets the default bit-size of the specified primitive
* @param t Primitive type
* @return Primitive size in bits
*/
uint8_t PrimitiveGetBitSize(PrimitiveType t);
