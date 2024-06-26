/**
* @file struct.hpp
* @author lakor64
* @date 03/01/2024
* @brief struct/union type
*/
#pragma once

#include "variable.hpp"
#include <vector>

class Struct;

/**
* A C structure/union field
*/
class StructField final : public Variable
{
	friend CH2Parser;
public:
	/**
	* Default constructor
	*/
	explicit StructField() : Variable(), m_parent(nullptr) { m_type = MemberType::StructField; }

	/**
	* Gets the parent of this field
	* @return Pointer to the Struct parent
	*/
	constexpr const Struct* GetParent() const { return m_parent; }

private:
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
	/**
	* Default constructor
	*/
	explicit Struct() : BasicMember(MemberType::Struct), m_align(0), m_size(0), m_unnamed(false) {}

	/**
	* Default destructor
	*/
	~Struct() override { for (auto& x : m_fields) delete x; m_fields.clear(); }

	/**
	* Gets the alignment of this structure
	* @return Alignemtn of the structure in bits
	*/
	constexpr auto GetAlign() const { return m_align; }

	/**
	* Gets the total size of this structure
	* @return Structure size in bits
	*/
	constexpr auto GetSize() const { return m_size; }

	/**
	* Gets the fields of this structure
	* @return Array of fields
	*/
	constexpr const auto& GetFields() const { return m_fields; }

	/**
	* Checks if the structure is unnamed
	* @return true if it's unnamed, otherwise false
	*/
	constexpr auto IsUnnamed() const { return m_unnamed; }

protected:
	/** alignment of the structure in bits */
	int64_t m_align;
	/** total size of the structure in bits */
	int64_t m_size;
	/** list of all fields inside the structure */
	std::vector<StructField*> m_fields;
	/** if it's an unamed struct */
	bool m_unnamed;
};

/**
* A C union
*/
class Union final : public Struct
{
	friend CH2Parser;
public:
	/**
	* Default constructor
	*/
	explicit Union() { m_type = MemberType::Union; }
};
