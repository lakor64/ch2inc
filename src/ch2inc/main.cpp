/**
* @file main.cpp
* @author lakor64
* @date 03/01/2024
* @brief main ch2inc app
*/
#include "ch2inc.hpp"

/**
* Main entrypoint for the application
* @param argc Number of arguments
* @param argv Arguments pointer
* @return exit code
*/
int main(int argc, char** argv)
{
	CH2Inc app;
	return app.Run(argc, argv);
}
