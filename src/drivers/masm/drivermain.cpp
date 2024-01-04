/**
* @file drivermain.c
* @author lakor64
* @date 03/01/2024
* @brief MASM driver entrypoint
*/
#include "masmdriver.hpp"

/**
* Entrypoint of the driver
* @return a filled structure of driver information
*/
extern "C" Driver* CH2DriverEntrypoint(void)
{
	return new MasmDriver();
}
