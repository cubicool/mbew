project(mbew-vpx)

include(ExternalProject)

if(WIN32)
	include("${CMAKE_CURRENT_LIST_DIR}/CMakeLists-windows.cmake")
	return()
endif()

find_program(MBEW_VPX_MAKE NAMES gmake make REQUIRED)

set(MBEW_VPX_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libvpx")
set(MBEW_VPX_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/libvpx-build")
set(MBEW_VPX_UPSTREAM_LIBRARY "${MBEW_VPX_BINARY_DIR}/libvpx.a")
set(MBEW_VPX_POSTFIX "")
set(MBEW_VPX_CFLAGS "${CMAKE_C_FLAGS}")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(MBEW_VPX_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
	string(APPEND MBEW_VPX_CFLAGS " ${CMAKE_C_FLAGS_DEBUG}")
endif()

set(MBEW_VPX_LIBRARY
	"${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}mbew-vpx${MBEW_VPX_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}"
)

file(MAKE_DIRECTORY "${MBEW_VPX_BINARY_DIR}")

set(MBEW_VPX_BUILD_JOB_SERVER_AWARE)

if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.28)
	list(APPEND MBEW_VPX_BUILD_JOB_SERVER_AWARE BUILD_JOB_SERVER_AWARE TRUE)
endif()

# libvpx owns its generated configuration and complete, target-specific source list.
ExternalProject_Add(mbew-vpx-build
	SOURCE_DIR "${MBEW_VPX_SOURCE_DIR}"
	BINARY_DIR "${MBEW_VPX_BINARY_DIR}"
	CONFIGURE_COMMAND
		${CMAKE_COMMAND} -E env
		"CC=${CMAKE_C_COMPILER}"
		"AR=${CMAKE_AR}"
		"RANLIB=${CMAKE_RANLIB}"
		"CFLAGS=${MBEW_VPX_CFLAGS}"
		"${MBEW_VPX_SOURCE_DIR}/configure"
		--target=generic-gnu
		--enable-vp8-encoder
		--disable-vp9-encoder
		--disable-examples
		--disable-tools
		--disable-docs
		--disable-unit-tests
		--disable-shared
		--enable-static
	BUILD_COMMAND "${MBEW_VPX_MAKE}"
	BUILD_BYPRODUCTS "${MBEW_VPX_UPSTREAM_LIBRARY}"
	INSTALL_COMMAND ""
	TEST_COMMAND ""
	UPDATE_COMMAND ""
	${MBEW_VPX_BUILD_JOB_SERVER_AWARE}
)

# Keep the archive name produced by MBEW's former in-tree CMake target.
add_custom_command(
	OUTPUT "${MBEW_VPX_LIBRARY}"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MBEW_VPX_UPSTREAM_LIBRARY}" "${MBEW_VPX_LIBRARY}"
	DEPENDS mbew-vpx-build "${MBEW_VPX_UPSTREAM_LIBRARY}"
)

add_custom_target(mbew-vpx-archive DEPENDS "${MBEW_VPX_LIBRARY}")

add_library(mbew-vpx STATIC IMPORTED GLOBAL)

set_target_properties(mbew-vpx PROPERTIES
	IMPORTED_LOCATION "${MBEW_VPX_LIBRARY}"
	INTERFACE_INCLUDE_DIRECTORIES "${MBEW_VPX_SOURCE_DIR};${MBEW_VPX_BINARY_DIR}"
)

add_dependencies(mbew-vpx mbew-vpx-archive)

if(NOT APPLE)
	target_link_libraries(mbew-vpx INTERFACE m)
endif()
