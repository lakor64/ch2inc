/**
* @file clangcli.cpp
* @author lakor64
* @date 03/01/2024
* @brief clang cli constructor
*/
#include "clangcli.hpp"

/**
* Simple utility to allocate the argv data and write data to it
* @param p number of arguments that are inside argv
* @param argv Container of strings to modify
* @param s string to copy
* @param mk Prefix to add to the string result
*/
static void p_copy(int& p, char** argv, std::string s, const char* mk)
{
	size_t sz, sz2 = strlen(mk);
	sz = sz2 + s.size() + 1;
	argv[p] = new char[sz];
#ifdef _WIN32
	strcpy_s(argv[p], sz, mk);
	strcpy_s(argv[p] + sz2, sz - sz2, s.c_str());
#else
	strcpy(argv[p], mk);
	strcpy(argv[p] + sz2, s.c_str());
#endif
	p++;
}

ClangCli::~ClangCli()
{
	for (auto i = 0; i < argc; i++)
	{
		delete[] argv[i];
	}

	delete[] argv;
	argv = nullptr;
	argc = 0;
}

ClangCli::ClangCli(const Options& opts)
{
	argc = (int)(opts.defines.size() + opts.includes.size() + opts.extra.size() + opts.undef.size());
	argv = new char* [argc];

	int p = 0;

	for (const auto& x : opts.defines)
	{
		p_copy(p, argv, x, "-D");
	}

	for (const auto& x : opts.includes)
	{
		p_copy(p, argv, x, "-I");
	}

	for (const auto& x : opts.undef)
	{
		p_copy(p, argv, x, "-U");
	}

	for (const auto& x : opts.extra)
	{
		p_copy(p, argv, x, "");
	}
}
