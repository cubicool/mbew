cmake_minimum_required(VERSION 3.10)
project(mbew)

if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

option(MBEW32 "TEMPORARY 32bit/X86 flag" OFF)

add_subdirectory("ext/mbew-vpx")
add_subdirectory("ext/mbew-nestegg")
add_subdirectory("ext/mbew-ogg")
add_subdirectory("ext/mbew-vorbis")
add_subdirectory("ext/mbew-opus")

add_library(mbew STATIC
    src/mbew.h
    src/mbew.hpp
    src/mbew-private.h
    src/mbew.c
    src/mbew-format.c
    src/mbew-io.c
    src/mbew-iterate.c
    src/mbew-string.c
)

target_include_directories(mbew PUBLIC "src")

target_link_libraries(mbew PUBLIC mbew-vpx mbew-nestegg mbew-vorbis)

add_subdirectory("examples")
