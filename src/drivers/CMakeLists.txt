if ("${CH2_STATIC_DRIVER}" STREQUAL "masm" OR CH2_NO_STATIC_DRIVER)
    add_subdirectory(masm)
endif()

#add_subdirectory(nasm)
