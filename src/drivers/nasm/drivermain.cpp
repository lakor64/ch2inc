/**
* @file drivermain.c
* @author lakor64
* @date 03/01/2024
* @brief NASM driver entrypoint
*/
#include "nasmdriver.hpp"

/**
* Entrypoint of the driver
* @return a filled structure of driver information
*/
extern "C" Driver* CH2DriverEntrypoint(void)
{
	return new NasmDriver();
}
