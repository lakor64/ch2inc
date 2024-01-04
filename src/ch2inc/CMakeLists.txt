file(GLOB SRC "*.cpp" "*.hpp")
add_executable(ch2inc ${SRC} ${NASM} ${MASM})
target_link_libraries(ch2inc PRIVATE cxxopts::cxxopts ch2parse)
