# Find FFMPEG
#
# @var FFMPEG_FOUND        FFMPEG found flag
# @var FFMPEG_LIBRARIES    location of the library
# @var FFMPEG_INCLUDE_DIRS location of the include files

set(SYS_LIB_SEARCH_PATHS
        /usr/local/include
        /usr/include
        /opt/local/include)

set(SYS_HEADER_SEARCH_PATHS
        /usr/lib
        /usr/local/lib
        /usr/lib64
        /opt/local/lib)

#libavcodec encoding/decoding library
find_path(AVCODEC_INCLUDE_DIR NAMES libavcodec/avcodec.h PATHS ${SYS_HEADER_SEARCH_PATHS})
find_library(AVCODEC_LIBRARY NAMES avcodec PATHS ${SYS_LIB_SEARCH_PATHS})

#libavfilter graph-based frame editing library
find_path(AVFILTER_INCLUDE_DIR NAMES libavfilter/avfilter.h PATHS ${SYS_HEADER_SEARCH_PATHS})
find_library(AVFILTER_LIBRARY NAMES avfilter PATHS ${SYS_LIB_SEARCH_PATHS})

#libavformat I/O and muxing/demuxing library
find_path(AVFORMAT_INCLUDE_DIR NAMES libavformat/avformat.h PATHS ${SYS_HEADER_SEARCH_PATHS})
find_library(AVFORMAT_LIBRARY NAMES avformat PATHS ${SYS_LIB_SEARCH_PATHS})

#libavdevice special devices muxing/demuxing library
find_path(AVDEVICE_INCLUDE_DIR NAMES libavdevice/avdevice.h PATHS ${SYS_HEADER_SEARCH_PATHS})
find_library(AVDEVICE_LIBRARY NAMES avdevice PATHS ${SYS_LIB_SEARCH_PATHS})

#libavutil common utility library
find_path(AVUTIL_INCLUDE_DIR NAMES libavutil/avutil.h PATHS ${SYS_HEADER_SEARCH_PATHS})
find_library(AVUTIL_LIBRARY NAMES avutil PATHS ${SYS_LIB_SEARCH_PATHS})

#libswresample audio resampling, format conversion and mixing
find_path(SWRESAMPLE_INCLUDE_DIR NAMES libswresample/swresample.h PATHS ${SYS_HEADER_SEARCH_PATHS})
find_library(SWRESAMPLE_LIBRARY NAMES swresample PATHS ${SYS_LIB_SEARCH_PATHS})

#libpostproc post processing library
find_path(POSTPROC_INCLUDE_DIR NAMES libpostproc/postprocess.h PATHS ${SYS_HEADER_SEARCH_PATHS})
find_library(POSTPROC_LIBRARY NAMES postproc PATHS ${SYS_LIB_SEARCH_PATHS})

#libswscale color conversion and scaling library
find_path(SWSCALE_INCLUDE_DIR NAMES libswscale/swscale.h PATHS ${SYS_HEADER_SEARCH_PATHS})
find_library(SWSCALE_LIBRARY NAMES swscale PATHS ${SYS_LIB_SEARCH_PATHS})

set(FFMPEG_INCLUDE_DIRS
        ${AVCODEC_INCLUDE_DIR}
        ${AVFILTER_INCLUDE_DIR}
        ${AVFORMAT_INCLUDE_DIR}
        ${AVDEVICE_INCLUDE_DIR}
        ${AVUTIL_INCLUDE_DIR}
        ${SWRESAMPLE_INCLUDE_DIR}
        ${POSTPROC_INCLUDE_DIR}
        ${SWSCALE_INCLUDE_DIR}
)

list(REMOVE_DUPLICATES FFMPEG_INCLUDE_DIRS)

set(FFMPEG_LIBRARIES
        ${AVCODEC_LIBRARY}
        ${AVFILTER_LIBRARY}
        ${AVFORMAT_LIBRARY}
        ${AVDEVICE_LIBRARY}
        ${AVUTIL_LIBRARY}
        ${SWRESAMPLE_LIBRARY}
        ${POSTPROC_LIBRARY}
        ${SWSCALE_LIBRARY}
)

if(FFMPEG_INCLUDE_DIRS AND FFMPEG_LIBRARIES)
    set(FFMPEG_FOUND 1 CACHE INTERNAL "FFMPEG found" FORCE)
endif()

mark_as_advanced(FFMPEG_INCLUDE_DIRS)
mark_as_advanced(FFMPEG_LIBRARIES)


include (FindPackageHandleStandardArgs)

find_package_handle_standard_args (
        ffmpeg
        REQUIRED_VARS
        FFMPEG_INCLUDE_DIRS
        FFMPEG_LIBRARIES
)