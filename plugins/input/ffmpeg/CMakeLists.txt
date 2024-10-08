cmake_minimum_required(VERSION 3.17)
project(ctune_plugin_ffmpeg LANGUAGES C)

include(ExternalProject)
include(Findffmpeg)

find_package(ffmpeg QUIET)

if(FFMPEG_FOUND)
    MESSAGE(STATUS "FFMpeg detected")

    set(CTUNE_SOURCE_FILES
            ../../../src/ctune_err.h
            ../../../src/ctune_err.c
            ../../../src/enum/PlaybackCtrl.h
            ../../../src/enum/PlaybackCtrl.c
            ../../../src/utils/Timeout.h
            ../../../src/utils/Timeout.c
            ../../../src/audio/AudioOut.h
            ../../../src/player/Player.h)

    set(FFMPEG_SOURCE_FILES
            src/ffmpeg.c )

    add_library(ctune_plugin_ffmpeg SHARED ${CTUNE_SOURCE_FILES} ${FFMPEG_SOURCE_FILES})

    target_link_libraries(ctune_plugin_ffmpeg
            ${FFMPEG_LIBRARIES}
            ctune_logger)

    set_target_properties(ctune_plugin_ffmpeg
            PROPERTIES PREFIX        ""
            OUTPUT_NAME              ffmpeg
            LIBRARY_OUTPUT_DIRECTORY "${PLUGINS_OUTPUT_DIRECTORY}/input")

    install(TARGETS     ctune_plugin_ffmpeg
            EXPORT      ctune_plugin_ffmpeg
            DESTINATION ${CTUNE_INSTALL_LIBDIR}/plugins/input)

endif(FFMPEG_FOUND)