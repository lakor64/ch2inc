/**
* @file ch2parser.hpp
* @author lakor64
* @date 03/01/2024
* @brief clang parser
*/
#pragma once

#include "ch2errcode.hpp"
#include "cfile.hpp"
#include "linktype.hpp"

#include <clang-c/Index.h>

/**
* This class uses libclang AST to perform the visiting of a C header file and
* serializes data back to the specific CFile
*/
class CH2Parser final
{
public:
	/**
	* Default constructor
	*/
	explicit CH2Parser() : m_lasterr(CH2ErrorCodes::None), m_cf(nullptr) {}

	/**
	* Default deconstructor
	*/
	~CH2Parser() = default;

	/**
	* Visits the specified file and serializes it into a file
	* @param in Input file
	* @param clang_argc number of c arguments to pass to clang
	* @param clang_argv argument pointer to pass to clang
	* @param file cfile to save data
	* @param plat Platform configuration
	*/
	void Visit(const std::string& in, int clang_argc, const char** clang_argv, CFile& file, const PlatformInfo& plat);

	constexpr auto GetLastError() { return m_lasterr; }

	/**
	* Checks if there was an error during parsing
	* @return true if there was an error while parsing
	*/
	constexpr bool HaveError() const { return m_lasterr != CH2ErrorCodes::None; }

private:

	/**
	* Parses a single child in the AST
	* @param cursor Current cursor
	* @param parent Parent cursor
	* @return Result of the parsing
	*/
	CXChildVisitResult ParseChild(CXCursor cursor, CXCursor parent);

	/**
	* Visits a struct type
	* @param c Cursor to the data
	* @param isUnion true if it's an union
	*/
	BasicMember* VisitStructOrUnion(CXCursor c, bool isUnion);

	/**
	* Visits a field of a struct or union
	* @param c Cursor to the data
	* @param p Parent of the data
	*/
	BasicMember* VisitField(CXCursor c, CXCursor p);

	/**
	* Visits a type definition
	* @param c Cursor to the data
	*/
	BasicMember* VisitTypedef(CXCursor c);

	/**
	* Visits an enum type
	* @param c Cursor to the data
	*/
	BasicMember* VisitEnum(CXCursor c);

	/**
	* Visits an enum declaration
	* @param c Cursor to the data
	* @param p Parent to the cursor
	*/
	BasicMember* VisitEnumDecl(CXCursor c, CXCursor p);

	/**
	* Visits a preprocessor definition
	* @param c Cursor to the data
	*/
	BasicMember* VisitMacroDef(CXCursor c);

	/**
	* Visits a variable declaration
	* @param c Cursor to the data
	*/
	BasicMember* VisitVarDecl(CXCursor c);

	/**
	* Visits a function
	* @param c Cursor to the data
	*/
	BasicMember* VisitFunc(CXCursor c);

	bool SetupLink(CXType type, LinkType& ref);

	BasicMember* FindType(const std::string& name);
	BasicMember* FindType(CXType type);
	BasicMember* FindType(CXCursor type);

	void AddPrimitive(const std::string& name, PrimitiveType type, PrimitiveMods mod);
	void AddBasics(const PlatformInfo& plat);

	/**
	* Last error
	*/
	CH2ErrorCodes m_lasterr;

	/**
	* CFile link
	*/
	CFile* m_cf;

	/** key-value reference of all the types */
	std::unordered_map<std::string, BasicMember*> m_types;
};
