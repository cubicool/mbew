PROJECT(mbew)

CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

IF(WIN32)
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
ENDIF()

SET(MBEW32 OFF CACHE BOOL "TEMPORARY 32bit/X86 flag")

ADD_SUBDIRECTORY("ext/mbew-vpx")
ADD_SUBDIRECTORY("ext/mbew-nestegg")
ADD_SUBDIRECTORY("ext/mbew-ogg")
ADD_SUBDIRECTORY("ext/mbew-vorbis")
ADD_SUBDIRECTORY("ext/mbew-opus")

SET(MBEW_SRC_FILES
	"src/mbew.c"
	"src/mbew-format.c"
	"src/mbew-io.c"
	"src/mbew-iterate.c"
	"src/mbew-string.c"
)

SET(MBEW_PUBLIC_HEADER_FILES
	"src/mbew.h"
	"src/mbew.hpp"
)

SET(MBEW_PRIVATE_HEADER_FILES
	"src/mbew-private.h"
)

SET(MBEW_HEADER_FILES
	${MBEW_PUBLIC_HEADER_FILES}
	${MBEW_PRIVATE_HEADER_FILES}
)

SET(MBEW_LIBRARY_TYPE STATIC)

GET_DIRECTORY_PROPERTY(MBEW_VPX_INCLUDE_DIRS
	DIRECTORY "ext/mbew-vpx"
	DEFINITION MBEW_VPX_INCLUDE_DIRS
)

GET_DIRECTORY_PROPERTY(MBEW_NESTEGG_INCLUDE_DIRS
	DIRECTORY "ext/mbew-nestegg"
	DEFINITION MBEW_NESTEGG_INCLUDE_DIRS
)

GET_DIRECTORY_PROPERTY(MBEW_VORBIS_INCLUDE_DIRS
	DIRECTORY "ext/mbew-vorbis"
	DEFINITION MBEW_VORBIS_INCLUDE_DIRS
)

INCLUDE_DIRECTORIES(
	"src"
	${MBEW_VPX_INCLUDE_DIRS}
	${MBEW_NESTEGG_INCLUDE_DIRS}
	${MBEW_VORBIS_INCLUDE_DIRS}
)

SOURCE_GROUP("Header Files" FILES ${MBEW_HEADER_FILES})
SOURCE_GROUP("Source Files" FILES ${MBEW_SRC_FILES})

ADD_LIBRARY(mbew ${MBEW_LIBRARY_TYPE} ${MBEW_HEADER_FILES} ${MBEW_SRC_FILES})

TARGET_LINK_LIBRARIES(mbew mbew-vpx mbew-nestegg mbew-vorbis)

ADD_SUBDIRECTORY("examples")

# GCC
# CMAKE_C_FLAGS_INIT="'-W -Wall -Wno-unused-parameter'"
# CMAKE_CXX_FLAGS_INIT="'-W -Wall -Wno-unused-parameter'"

# Clang
# CMAKE_C_FLAGS_INIT="'-W -Wall -Wno-unused-parameter -Wno-error=unused-command-line-argument -Qunused-arguments'"
# CMAKE_CXX_FLAGS_INIT="'-W -Wall -Wno-unused-parameter -Wno-error=unused-command-line-argument -Qunused-arguments'"
