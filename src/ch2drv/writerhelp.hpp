/**
* @file writefmt.hpp
* @author lakor64
* @date 14/05/2024
* @brief Simple writer helper
*/
#pragma once

#include <format>
#include <string_view>

static void writefmt(FILE* fp, const std::string_view& fmt)
{
	fwrite(fmt.data(), fmt.size(), 1, fp);
}

template <typename... Args>
static void writefmt(FILE* fp, const std::string_view& fmt, Args&&... args)
{
	auto f = std::vformat(fmt, std::make_format_args(args...));
	writefmt(fp, f);
}
