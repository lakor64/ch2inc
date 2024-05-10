#pragma once

extern int printf(const char* fmt, ...);

static void __p()
{
    printf("eeee\n");
}

void __a(void);

extern int g_all;

static int g_r4;

static float g_appsp = 403534.4f;

#ifdef __CH2INC__
extern register int g_r1;
#endif

extern volatile int g_r2;
int g_r3;
