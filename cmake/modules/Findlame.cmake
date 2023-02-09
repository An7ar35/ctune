# Find LAME
#
# @var LAME_FOUND        LAME found flag
# @var LAME_LIBRARIES    location of the library
# @var LAME_INCLUDE_DIRS location of the include files

set(LAME_ROOT_DIR "${LAME_ROOT_DIR}"
        CACHE PATH "Directory(ies) to search for LAME" )

find_path(LAME_INCLUDE_DIRS
        NAMES lame/lame.h
        PATHS /usr/local/include
              /usr/include
              /opt/local/include
        HINTS ${LAME_ROOT_DIR}
)

find_library(LAME_LIBRARIES
        NAMES mp3lame
        PATHS /usr/lib
              /usr/local/lib
              /usr/lib64
              /opt/local/lib
        HINTS ${LAME_ROOT_DIR}
)

if(LAME_INCLUDE_DIRS AND LAME_LIBRARIES)
    set(LAME_FOUND 1 CACHE INTERNAL "Lame found" FORCE)
else()
    set(LAME_FOUND 0 CACHE INTERNAL "Lame not found" FORCE)
endif()

mark_as_advanced(LAME_INCLUDE_DIRS)
mark_as_advanced(LAME_LIBRARIES)

include (FindPackageHandleStandardArgs)

find_package_handle_standard_args (
        lame
        REQUIRED_VARS
        LAME_INCLUDE_DIRS
        LAME_LIBRARIES
        LAME_FOUND
)