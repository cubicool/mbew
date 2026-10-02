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

if(WIN32)
	set(MBEW_COMPILE_OPTIONS
		/W4
		/WX
		/wd4100
		/analyze
		/analyze:external-
		# The MSVC STL/SDK headers are reached via the INCLUDE env var, not -I,
		# so they aren't automatically treated as external; without this,
		# /analyze produces false positives (e.g. C6031) deep in <xutility>
		# et al. when heavily-templated code (pybind11) instantiates them.
		/external:env:INCLUDE
	)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	set(MBEW_COMPILE_OPTIONS
		-W
		-Wall
		-Wshadow
		-Wconversion
		-Wsign-conversion
		-Wfloat-conversion
		# -Wpedantic
		-Wextra
		-Werror
		-Wno-unused-parameter
	)
else()
	set(MBEW_COMPILE_OPTIONS)
endif()

add_library(mbew STATIC
	src/mbew.h
	src/mbew.hpp
	src/mbew-private.h
	src/mbew.c
	src/mbew-read.c
	src/mbew-format.c
	src/mbew-io.c
	src/mbew-iterate.c
	src/mbew-write.c
	src/mbew-string.c
)

target_include_directories(mbew PUBLIC "src")
target_link_libraries(mbew PUBLIC mbew-vpx mbew-nestegg mbew-vorbis)
target_compile_options(mbew PRIVATE ${MBEW_COMPILE_OPTIONS})

add_subdirectory("examples")
