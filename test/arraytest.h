#pragma once

static int g_a[3];
static int g_b[3][4][6];

static float* g_c[2];

extern int g_d[5];

int g_e[7];

struct ae
{
	int pp;
};

struct ae mem[3];

static void hello_func(int* a, int b[], int d[4]);
