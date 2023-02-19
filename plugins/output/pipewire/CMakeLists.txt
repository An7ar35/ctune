cmake_minimum_required(VERSION 3.17)
project(ctune_plugin_pipewire LANGUAGES C)

include(ExternalProject)
include(Findpipewire)

if(PIPEWIRE_FOUND)
    MESSAGE(STATUS "PipeWire detected")

    set(CTUNE_SOURCE_FILES
            ../../../src/ctune_err.h
            ../../../src/ctune_err.c
            ../../../src/utils/Timeout.h
            ../../../src/utils/Timeout.c
            ../../../src/audio/AudioOut.h
    )

    set(PW_SOURCE_FILES
            src/pipewire.c
    )

    add_library(ctune_plugin_pipewire
            SHARED ${CTUNE_SOURCE_FILES}
                   ${PW_SOURCE_FILES}
    )

    target_include_directories(ctune_plugin_pipewire
            PRIVATE ${CMAKE_BINARY_DIR}/generated-src/
                    ${PIPEWIRE_INCLUDE_DIR}
    )

    target_link_libraries(ctune_plugin_pipewire
            PRIVATE PipeWire::PipeWire
                    ctune_logger
    )

    set_target_properties(ctune_plugin_pipewire
            PROPERTIES PREFIX                   ""
                       OUTPUT_NAME              pipewire
                       LIBRARY_OUTPUT_DIRECTORY "${PLUGINS_OUTPUT_DIRECTORY}/output"
    )

    install(TARGETS     ctune_plugin_pipewire
            EXPORT      ctune_plugin_pipewire
            DESTINATION ${CTUNE_INSTALL_LIBDIR}/plugins/output
    )

endif(PIPEWIRE_FOUND)