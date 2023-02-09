cmake_minimum_required(VERSION 3.17)
project(ctune_plugin_alsa LANGUAGES C)

include(ExternalProject)

find_package(ALSA QUIET)

if(ALSA_FOUND)
    message(STATUS "ALSA detected")

    set(CTUNE_SOURCE_FILES
            ../../../libraries/logger/src/Logger.h
            ../../../src/ctune_err.h
            ../../../src/ctune_err.c
            ../../../src/audio/AudioOut.h)

    set(ALSA_SOURCE_FILES
            src/alsa.c )

    add_library(ctune_plugin_alsa SHARED ${CTUNE_SOURCE_FILES} ${ALSA_SOURCE_FILES})

    target_link_libraries(ctune_plugin_alsa PRIVATE ALSA::ALSA ctune_logger)

    set_target_properties(ctune_plugin_alsa
            PROPERTIES PREFIX        ""
            OUTPUT_NAME              alsa
            LIBRARY_OUTPUT_DIRECTORY "${PLUGINS_OUTPUT_DIRECTORY}/output")

    install(TARGETS     ctune_plugin_alsa
            EXPORT      ctune_plugin_alsa
            DESTINATION ${CTUNE_INSTALL_LIBDIR}/plugins/output)

endif(ALSA_FOUND)