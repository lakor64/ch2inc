/**
* @file drivermain.cpp
* @author lakor64
* @date 03/01/2024
* @brief MASM driver entrypoint
*/
#include "masmdriver.hpp"

/**
* Entrypoint of the driver
* @return Implementation of a ch2inc driver
*/
extern "C" Driver* CH2DriverEntrypoint(void)
{
	return new MasmDriver();
}
