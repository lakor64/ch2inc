cmake_minimum_required(VERSION 3.20)
project(ch2inc)

set(CMAKE_CXX_STANDARD 17)

if (NOT DEFINED LLVM_ROOT)
    message(FATAL_ERROR "Variable LLVM_ROOT not set")
endif()

find_package(cxxopts CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

add_subdirectory(src)
