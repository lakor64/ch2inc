#pragma once

#define TEST_0 0
#define TEST_1 1

#if TEST_0
#define TEST_2 0
#else
#define TEST_2 1
#endif

#define TEST_3(yyy) yyy * 2
