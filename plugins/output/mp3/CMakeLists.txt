cmake_minimum_required(VERSION 3.17)
project(ctune_plugin_mp3 LANGUAGES C)

include(ExternalProject)
find_package(lame QUIET)

if(LAME_FOUND)
    message(STATUS "Lame detected")

    set(CTUNE_SOURCE_FILES
            ../../../src/datastructure/String.h
            ../../../src/datastructure/String.c
            ../../../src/ctune_err.h
            ../../../src/ctune_err.c
            ../../../src/audio/FileOut.h)

    set(LAME_MP3_SOURCE_FILES
            src/mp3lame.c )

    add_library(ctune_plugin_mp3 SHARED ${CTUNE_SOURCE_FILES} ${LAME_MP3_SOURCE_FILES})

    target_link_libraries(ctune_plugin_mp3 PRIVATE mp3lame ctune_logger)

    set_target_properties(ctune_plugin_mp3
            PROPERTIES PREFIX        ""
            OUTPUT_NAME              mp3
            LIBRARY_OUTPUT_DIRECTORY "${PLUGINS_OUTPUT_DIRECTORY}/output")

    install(TARGETS     ctune_plugin_mp3
            EXPORT      ctune_plugin_mp3
            DESTINATION ${CTUNE_INSTALL_LIBDIR}/plugins/output)

endif(LAME_FOUND)