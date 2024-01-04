#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static void writesng(FILE* fp, const char* ch)
{
	fwrite(ch, strlen(ch), 1, fp);
}

static void writefmt(FILE* fp, const char* fmt, ...)
{
	char buffer[0x200];
	va_list vl;

	va_start(vl, fmt);
	vsprintf_s(buffer, _countof(buffer), fmt, vl);
	va_end(vl);

	fwrite(buffer, strlen(buffer), 1, fp);
}
