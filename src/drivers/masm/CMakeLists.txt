file(GLOB SRC "*.cpp" "*.hpp")

if (WIN32 AND CH2_NO_STATIC_DRIVER)
    list(APPEND SRC "ch2drvmasm.def")
endif()

add_library(ch2drvmasm ${CH2_DRIVER_TYPE} ${SRC})
target_link_libraries(ch2drvmasm PRIVATE ch2drv fmt::fmt-header-only)
