cmake_minimum_required(VERSION 3.20)
project(ch2inc)

set(CMAKE_CXX_STANDARD 17)

if (NOT DEFINED LLVM_ROOT)
    message(FATAL_ERROR "Variable LLVM_ROOT not set")
endif()

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
endif()

find_package(cxxopts CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_path(EXPRTK_INCLUDE_DIRS "exprtk.hpp")

set(LIBCLANG_NAME "clang")

if (WIN32 AND EXISTS "${LLVM_ROOT}/lib/libclang.lib")
    set(LIBCLANG_NAME "libclang")
endif()

if (DEFINED CH2_STATIC_DRIVER)
    set(CH2_DRIVER_TYPE STATIC)

    if ("${CH2_STATIC_DRIVER}" STREQUAL "masm")
        set(CH2_DRIVER_NAME "ch2drvmasm")
    else()
        message(FATAL "Invalid static driver ${CH2_STATIC_DRIVER}")
    endif()
else()
    set(CH2_DRIVER_TYPE SHARED)
    set(CH2_NO_STATIC_DRIVER 1)
endif()

add_subdirectory(src)
