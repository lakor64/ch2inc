file(GLOB SRC "*.cpp" "*.hpp")
add_executable(ch2inc ${SRC})
target_link_libraries(ch2inc PRIVATE cxxopts::cxxopts ch2parse)

if (NOT CH2_NO_STATIC_DRIVER)
    target_compile_definitions(ch2inc PRIVATE -DDISABLE_DYNLIB)
    target_link_libraries(ch2inc PRIVATE ${CH2_DRIVER_NAME})
endif()
