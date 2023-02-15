cmake_minimum_required(VERSION 3.17)
project(ctune_plugin_pulseaudio LANGUAGES C)

include(ExternalProject)

find_package(PulseAudio QUIET)

if(PULSEAUDIO_FOUND)
    MESSAGE(STATUS "PulseAudio detected")

    include_directories(${CMAKE_BINARY_DIR}/generated-src/)

    set(CTUNE_SOURCE_FILES
            ../../../src/ctune_err.h
            ../../../src/ctune_err.c
            ../../../src/utils/Timeout.h
            ../../../src/utils/Timeout.c
            ../../../src/audio/AudioOut.h)

    set(PULSE_SOURCE_FILES
            src/pulse.c )

    add_library(ctune_plugin_pulseaudio SHARED ${CTUNE_SOURCE_FILES} ${PULSE_SOURCE_FILES})

    target_link_libraries(ctune_plugin_pulseaudio PRIVATE pulse ctune_logger)

    set_target_properties(ctune_plugin_pulseaudio
            PROPERTIES PREFIX        ""
            OUTPUT_NAME              pulse
            LIBRARY_OUTPUT_DIRECTORY "${PLUGINS_OUTPUT_DIRECTORY}/output")

    install(TARGETS     ctune_plugin_pulseaudio
            EXPORT      ctune_plugin_pulseaudio
            DESTINATION ${CTUNE_INSTALL_LIBDIR}/plugins/output)

endif(PULSEAUDIO_FOUND)