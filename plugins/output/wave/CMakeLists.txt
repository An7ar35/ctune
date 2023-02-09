cmake_minimum_required(VERSION 3.17)
project(ctune_plugin_wave LANGUAGES C)

set(WAVE_FOUND TRUE)

message(STATUS "Wave detected")

set(CTUNE_SOURCE_FILES
        ../../../libraries/logger/src/Logger.h
        ../../../src/ctune_err.h
        ../../../src/ctune_err.c
        ../../../src/audio/FileOut.h)

set(WAVE_SOURCE_FILES
        src/wave.c )

add_library(ctune_plugin_wave SHARED ${CTUNE_SOURCE_FILES} ${WAVE_SOURCE_FILES})

target_link_libraries(ctune_plugin_wave PRIVATE ctune_logger)

set_target_properties(ctune_plugin_wave
        PROPERTIES PREFIX        ""
        OUTPUT_NAME              wave
        LIBRARY_OUTPUT_DIRECTORY ${PLUGINS_OUTPUT_DIRECTORY}/output)

install(TARGETS     ctune_plugin_wave
        EXPORT      ctune_plugin_wave
        DESTINATION ${CTUNE_INSTALL_LIBDIR}/plugins/output)