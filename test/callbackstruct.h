#pragma once

struct x
{
    void (__stdcall*a)(void);
    void (__cdecl*b)(void);
    void (**c)(int a);
    void (*d)(const char* v, const char* l);
};
