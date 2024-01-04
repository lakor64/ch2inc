#pragma once

struct x
{
	char a : 3;
	char p : 5;
	char c;
	short d;
	int o : 28;
	short k;
	short k1 : 14;
	short k2 : 10;
	short k3 : 13;
	short k4 : 9;
	short k5 : 7;
	int pq;
	short k7 : 7;
	short k6 : 9;
};

struct y 
{
	char o : 5;
	int a;
	int b : 20;
};

#pragma pack(1)
struct z
{
	int a : 20;
	short q : 10;
	short l : 6;
	char p : 2;
	char q2 : 3;
	char aa : 2;
	char oo : 1;
};
