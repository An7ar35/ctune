cmake_minimum_required(VERSION 3.17)
project(ctune_plugin_sndio LANGUAGES C)

include(ExternalProject)
include(Findsndio)

if(SNDIO_FOUND)
    message(STATUS "sndio detected")

    include_directories( ${CMAKE_BINARY_DIR}/generated-src/ )

    set(CTUNE_SOURCE_FILES
            ../../../src/ctune_err.h
            ../../../src/ctune_err.c
            ../../../src/audio/AudioOut.h)

    set(SNDIO_SOURCE_FILES
            src/sndio.c )

    add_library(ctune_plugin_sndio SHARED ${CTUNE_SOURCE_FILES} ${SNDIO_SOURCE_FILES})

    target_link_libraries(ctune_plugin_sndio PRIVATE sndio ctune_logger)

    set_target_properties(ctune_plugin_sndio
            PROPERTIES PREFIX        ""
            OUTPUT_NAME              sndio
            LIBRARY_OUTPUT_DIRECTORY "${PLUGINS_OUTPUT_DIRECTORY}/output")

    install(TARGETS     ctune_plugin_sndio
            EXPORT      ctune_plugin_sndio
            DESTINATION ${CTUNE_INSTALL_LIBDIR}/plugins/output)

endif(SNDIO_FOUND)
 
