/**
* @file enum.hpp
* @author lakor64
* @date 03/01/2024
* @brief enum type
*/
#pragma once

#include "basicmember.hpp"
#include <vector>

class Enum;

/**
* A field of an enumerator
*/
class EnumField final : public BasicMember
{
	friend CH2Parser;
public:
	explicit EnumField() : BasicMember(MemberType::EnumField), m_size(0), m_value(0), m_parent(nullptr) {}
	
	constexpr auto GetBitSize() const { return m_size; }
	constexpr auto GetValue() const { return m_value; }
	constexpr const Enum* GetParent() const { return m_parent; }

private:
	/** size of the field in bits */
	int64_t m_size;
	/** value of the field */
	uint64_t m_value;
	/** parent of this field */
	Enum* m_parent;
};

/**
* A C enumator
*/
class Enum final : public BasicMember
{
	friend CH2Parser;
public:
	explicit Enum() : BasicMember(MemberType::Enum), m_size(0) {}
	~Enum() { for (auto& x : m_fields) delete x; m_fields.clear(); }

	constexpr auto GetSize() const { return m_size; }
	constexpr const auto& GetFields() const { return m_fields; }

private:
	/** size of the structure in bits */
	int64_t m_size;
	/** list of all fields inside the enumerator */
	std::vector<EnumField*> m_fields;
};
