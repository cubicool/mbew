project(mbew-vorbis)

# Satisfy vorbis's internal find_package(OGG) with our bundled submodule.
set(OGG_FOUND TRUE)
set(OGG_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/ext/mbew-ogg/ogg/include")
set(OGG_LIBRARIES mbew-ogg)

add_subdirectory("vorbis" EXCLUDE_FROM_ALL)

foreach(VAR VORBIS_PUBLIC_HEADERS VORBIS_HEADERS VORBIS_SOURCES)
    get_directory_property(VAR_FILES DIRECTORY "vorbis/lib" DEFINITION ${VAR})

    foreach(VF ${VAR_FILES})
        list(APPEND MBEW_VORBIS_FILES "vorbis/lib/${VF}")
    endforeach()
endforeach()

add_library(mbew-vorbis STATIC ${MBEW_VORBIS_FILES})

target_include_directories(mbew-vorbis PUBLIC "${PROJECT_SOURCE_DIR}/vorbis/include")

target_link_libraries(mbew-vorbis PUBLIC mbew-ogg m)
