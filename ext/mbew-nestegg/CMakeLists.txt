project(mbew-nestegg)

add_library(mbew-nestegg STATIC
    nestegg/include/nestegg/nestegg.h
    nestegg/src/nestegg.c
)

target_include_directories(mbew-nestegg PUBLIC "nestegg/include")
