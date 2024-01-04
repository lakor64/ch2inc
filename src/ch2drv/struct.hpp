/**
* @file struct.hpp
* @author lakor64
* @date 03/01/2024
* @brief struct/union type
*/
#pragma once

#include "linktype.hpp"
#include "basicmember.hpp"
#include <vector>

class Struct;

class StructField final : public BasicMember
{
	friend CH2Parser;
public:
	explicit StructField() : BasicMember(MemberType::StructField), m_size(0), m_ref(), m_parent(nullptr) {}

	constexpr auto GetSize() const { return m_size; }
	constexpr auto& GetRef() const { return m_ref; }
	constexpr const Struct* GetParent() const { return m_parent; }

private:
	/** size of the field in bits */
	int64_t m_size;
	/** type reference */
	LinkType m_ref;
	/** parent of this field */
	Struct* m_parent;
};

/**
* A C structure
*/
class Struct : public BasicMember
{
	friend CH2Parser;
public:
	explicit Struct() : BasicMember(MemberType::Struct), m_align(0), m_size(0) {}
	~Struct() override { for (auto& x : m_fields) delete x; m_fields.clear(); }

	constexpr auto GetAlign() const { return m_align; }
	constexpr auto GetSize() const { return m_size; }
	constexpr const auto& GetFields() const { return m_fields; }

protected:
	/** alignment of the structure in bits */
	int64_t m_align;
	/** total size of the structure in bits */
	int64_t m_size;
	/** list of all fields inside the structure */
	std::vector<StructField*> m_fields;
};

/**
* A C union
*/
class Union final : public Struct
{
	friend CH2Parser;
public:
	explicit Union() { m_type = MemberType::Union; }
};
