cmake_minimum_required(VERSION 3.17)
project(ctune_plugin_vlc LANGUAGES C)

include(ExternalProject)
include(Findvlc)

find_package(vlc QUIET)

if(VLC_FOUND)
    set(CTUNE_SOURCE_FILES
            ../../../src/ctune_err.h
            ../../../src/ctune_err.c
            ../../../src/utils/Timeout.h
            ../../../src/utils/Timeout.c
            ../../../src/audio/AudioOut.h
            ../../../src/player/Player.h)

    set(VLC_SOURCE_FILES
            src/vlc.c )

    add_library(ctune_plugin_vlc SHARED ${CTUNE_SOURCE_FILES} ${VLC_SOURCE_FILES})

    target_link_libraries(ctune_plugin_vlc
            ${VLC_LIBRARIES}
            ctune_logger)

    set_target_properties(ctune_plugin_vlc
            PROPERTIES PREFIX        ""
            OUTPUT_NAME              vlc
            LIBRARY_OUTPUT_DIRECTORY "${PLUGINS_OUTPUT_DIRECTORY}/input")

    install(TARGETS     ctune_plugin_vlc
            EXPORT      ctune_plugin_vlc
            DESTINATION ${CTUNE_INSTALL_LIBDIR}/plugins/input)

endif(VLC_FOUND)

