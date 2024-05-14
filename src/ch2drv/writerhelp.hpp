/**
* @file writefmt.hpp
* @author lakor64
* @date 14/05/2024
* @brief Simple writer helper
*/
#pragma once

#include <fmt/format.h>
#include <string_view>

/**
* Writes a string to a C file
* @param fp C file pointer
* @param fmt String to write
*/
static void writefmt(FILE* fp, const std::string_view& fmt)
{
	fwrite(fmt.data(), fmt.size(), 1, fp);
}

/**
* Writes a formatted string to a C file
* @param fp C file pointer
* @param fmt String to format
* @param args Arguments to format
*/
template <typename... Args>
static void writefmt(FILE* fp, const std::string_view& fmt, Args&&... args)
{
	auto f = fmt::vformat(fmt, fmt::make_format_args(args...));
	writefmt(fp, f);
}
