# Find SNDIO
#
# @var SNDIO_FOUND        SNDIO found flag
# @var SNDIO_LIBRARIES    location of the library
# @var SNDIO_INCLUDE_DIRS location of the include files

set(SNDIO_ROOT_DIR
        "${SNDIO_ROOT_DIR}"
        CACHE
        PATH
        "Directory(ies) to search for SNDIO"
)

find_path(SNDIO_INCLUDE_DIRS
        NAMES
            sndio.h
        PATHS
            /usr/local/include
            /usr/include
            /opt/local/include
#        HINTS ${SNDIO_ROOT_DIR}
)

find_library(SNDIO_LIBRARIES
        NAMES
            sndio
        PATHS
            /usr/lib
            /usr/local/lib
            /usr/lib64
            /opt/local/lib
#        HINTS ${SNDIIO_ROOT_DIR}
)


mark_as_advanced(SNDIO_INCLUDE_DIRS)
mark_as_advanced(SNDIO_LIBRARIES)


include (FindPackageHandleStandardArgs)

find_package_handle_standard_args (
        sndio
        REQUIRED_VARS
            SNDIO_INCLUDE_DIRS
            SNDIO_LIBRARIES
)


