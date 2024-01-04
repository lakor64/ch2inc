#pragma once

struct a
{
	unsigned int b;
	int c;
};

struct b
{
	short a;
	struct a p;
};

struct c
{
	double p;
};

union d
{
	struct c _c;
	struct b _b;
};
