cmake_minimum_required(VERSION 3.17)
project(ctune_plugin_sdl LANGUAGES C )

include(ExternalProject)
find_package(SDL2 QUIET)

if(SDL2_FOUND)
    message(STATUS "SDL2 detected")

    set(CTUNE_SOURCE_FILES
            ../../../src/ctune_err.h
            ../../../src/ctune_err.c
            ../../../src/audio/AudioOut.h)

    set(SDL_SOURCE_FILES
            src/sdl.c )

    add_library(ctune_plugin_sdl SHARED ${CTUNE_SOURCE_FILES} ${SDL_SOURCE_FILES})

    target_link_libraries(ctune_plugin_sdl PRIVATE SDL2::SDL2 ctune_logger)

    set_target_properties(ctune_plugin_sdl
            PROPERTIES PREFIX        ""
            OUTPUT_NAME              sdl
            LIBRARY_OUTPUT_DIRECTORY "${PLUGINS_OUTPUT_DIRECTORY}/output")

    install(TARGETS     ctune_plugin_sdl
            EXPORT      ctune_plugin_sdl
            DESTINATION ${CTUNE_INSTALL_LIBDIR}/plugins/output)

endif(SDL2_FOUND)