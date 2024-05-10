#pragma once

#define TEST_0 0
#define TEST_1 1

#if TEST_0
#define TEST_2 0
#else
#define TEST_2 1
#endif

#define TEST_4 "ciao"
#define TEST_5 "a"
#define TEST_6 "tutti"
#define TEST_7 TEST_4 TEST_5 TEST_6

#ifdef TEST_0
#define TEST_8 TEST_4 TEST_6
#else
#define TEST_8 TEST_4 TEST_5
#endif

#undef TEST_0
#define TEST_0 1

#ifdef TEST_0
#define TEST_11 TEST_4 TEST_6
#else
#define TEST_11 TEST_4 TEST_5
#endif

#define TEST_3(yyy) yyy * 2

#define TEST_9 TEST_0 + 4
#define TEST_10 this->ciao()

#define TEST_11 L"aa"

#ifdef TEST_0
	extern void a(void);
#else
	extern void b(void);
#endif

#define TEST_12 qq

extern void TEST_12 (int p);
