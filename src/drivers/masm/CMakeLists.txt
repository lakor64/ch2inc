file(GLOB SRC "*.cpp" "*.hpp")

if (WIN32)
    list(APPEND SRC "ch2drvmasm.def")
endif()

add_library(ch2drvmasm SHARED ${SRC})
target_link_libraries(ch2drvmasm PRIVATE ch2drv fmt::fmt-header-only)
