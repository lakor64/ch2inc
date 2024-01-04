file(GLOB SRC "*.cpp" "*.hpp")
add_library(ch2drv STATIC ${SRC})
target_include_directories(ch2drv PUBLIC "${CMAKE_CURRENT_LIST_DIR}")
