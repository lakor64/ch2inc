cmake_minimum_required(VERSION 3.20)
project(ch2inc)

set(CMAKE_CXX_STANDARD 20)

if (NOT DEFINED LLVM_ROOT)
    message(FATAL_ERROR "Variable LLVM_ROOT not set")
endif()

find_package(cxxopts CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

set(LIBCLANG_NAME "clang")

if (WIN32 AND EXISTS "${LLVM_ROOT}/lib/libclang.lib")
    set(LIBCLANG_NAME "libclang")
endif()

add_subdirectory(src)
