# Find VLC
#
# @var VLC_FOUND        VLC found flag
# @var VLC_LIBRARIES    location of the library
# @var VLC_INCLUDE_DIRS location of the include files

set(VLC_ROOT_DIR
        "${VLC_ROOT_DIR}"
        CACHE
        PATH
        "Directory(ies) to search for VLC"
)

find_path(VLC_INCLUDE_DIRS
        NAMES
            vlc/vlc.h
        PATHS
            /usr/local/include
            /usr/include
            /opt/local/include
        HINTS
            ${VLC_ROOT_DIR}
)

find_library(VLC_LIBRARIES
        NAMES
            vlc
        PATHS
            /usr/lib
            /usr/local/lib
            /usr/lib64
            /opt/local/lib
        HINTS
            ${VLC_ROOT_DIR}
)

if(VLC_INCLUDE_DIRS AND VLC_LIBRARIES)
    set(VLC_FOUND 1 CACHE INTERNAL "libVLC found" FORCE)
endif()

mark_as_advanced(VLC_INCLUDE_DIRS)
mark_as_advanced(VLC_LIBRARIES)

include (FindPackageHandleStandardArgs)

find_package_handle_standard_args (
        vlc
        REQUIRED_VARS
        VLC_INCLUDE_DIRS
        VLC_LIBRARIES
)