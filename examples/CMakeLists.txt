FUNCTION(ADD_EXAMPLE _name)
	GET_FILENAME_COMPONENT(EXAMPLE_NAME ${_name} NAME_WE)
	GET_FILENAME_COMPONENT(EXAMPLE_EXT ${_name} EXT)

	ADD_EXECUTABLE("mbew-example-${EXAMPLE_NAME}" "mbew-example-${EXAMPLE_NAME}${EXAMPLE_EXT}")

	TARGET_LINK_LIBRARIES("mbew-example-${EXAMPLE_NAME}" mbew ${ARGV1} ${ARGV2} ${ARGV3})

	IF(NOT WIN32)
		TARGET_LINK_LIBRARIES("mbew-example-${EXAMPLE_NAME}" pthread m)
	ENDIF()
ENDFUNCTION()

ADD_EXAMPLE(c++.cpp)
ADD_EXAMPLE(iterate.c)
ADD_EXAMPLE(properties.c)
ADD_EXAMPLE(src-memory.c)
ADD_EXAMPLE(strings.c)

IF(NOT WIN32)
	INCLUDE_DIRECTORIES("/usr/include/cairo")
	ADD_EXAMPLE(video-cairo.c cairo)
	ADD_EXAMPLE(video-osg.cpp osg osgViewerd)
	ADD_EXAMPLE(video-sdl.c SDL)
	ADD_EXAMPLE(audio-ao.c ao)
	ADD_EXAMPLE(audio-sdl.c SDL)
ENDIF()

