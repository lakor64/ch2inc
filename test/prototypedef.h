#pragma once

typedef void(*aa)(void);
typedef void(*bb)(int, int, int);
typedef int(__stdcall*cc)(void);
typedef float(***dd)(float, double, char*);

typedef void __cdecl br_putline_cbfn(char *str, void *arg);
typedef void __stdcall br_putline_cbfn2(char *str, void *arg);
