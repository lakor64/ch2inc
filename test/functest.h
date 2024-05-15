#pragma once

extern int __stdcall a(int a, int b, int c, int d);
extern int __stdcall b(void);

extern float __cdecl c(void);
extern float __cdecl d(float a, float b, int c, int d);

#ifdef __CH2INC__
extern void __fastcall e(void);
extern void __fastcall f(int a, int b, int c, int d);
#endif

extern void __cdecl g(int a, ...);

extern void h(void* x, void **** y, int ******* a);

//extern void __vectorcall g(void);
//extern void __vectorcall h(int a, int b, int c, int d);

extern void j(volatile int a);

#ifdef __CH2INC__
extern void k(int* restrict p);
#endif
