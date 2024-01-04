#pragma once

typedef int a;
typedef int* b;
typedef b c;

struct p
{
	a _a;
	c _b;
	c* _c;
	c*** _e;
};

struct o
{
	c** _a;
	int _b;
};

typedef struct p l;
