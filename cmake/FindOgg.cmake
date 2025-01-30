# FindOgg.cmake
# Locate the Ogg library
# Sets:
#   OGG_FOUND
#   OGG_INCLUDE_DIRS
#   OGG_LIBRARIES

find_path(OGG_INCLUDE_DIR
    NAMES ogg/ogg.h
    PATH_SUFFIXES include
)

find_library(OGG_LIBRARY
    NAMES ogg
    PATH_SUFFIXES lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Ogg REQUIRED_VARS OGG_LIBRARY OGG_INCLUDE_DIR)

if(OGG_FOUND)
    set(OGG_LIBRARIES ${OGG_LIBRARY})
    set(OGG_INCLUDE_DIRS ${OGG_INCLUDE_DIR})
endif()
