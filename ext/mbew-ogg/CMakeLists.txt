project(mbew-ogg)

add_subdirectory("ogg" EXCLUDE_FROM_ALL)

foreach(VAR OGG_HEADERS OGG_SOURCES)
    get_directory_property(VAR_FILES DIRECTORY "ogg" DEFINITION ${VAR})

    foreach(VF ${VAR_FILES})
        list(APPEND MBEW_OGG_FILES "ogg/${VF}")
    endforeach()
endforeach()

add_library(mbew-ogg STATIC ${MBEW_OGG_FILES})

target_include_directories(mbew-ogg PUBLIC "${PROJECT_SOURCE_DIR}/ogg/include")
