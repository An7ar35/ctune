cmake_minimum_required(VERSION 3.17)
project(ctune_logger LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-Wall")
set(CMAKE_C_FLAGS "-Wextra")
set(CMAKE_C_FLAGS_RELEASE "-O2")

include(ExternalProject)

find_package(Threads REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG BOOL 1)

include_directories( ${CMAKE_BINARY_DIR}/generated-src/ )

set(CTUNE_SOURCE_FILES
        ../../src/utils/Timeout.c
        ../../src/utils/Timeout.h
        ../../src/ctune_err.c
        ../../src/ctune_err.h
        ../../src/datastructure/String.c
        ../../src/datastructure/String.h)

set(LOGGER_SOURCE_FILES
        src/Logger.h
        src/Logger.c
        src/LogQueue.c
        src/LogQueue.h
        src/LogWriter.c
        src/LogWriter.h)

add_library(ctune_logger SHARED
        ${CTUNE_SOURCE_FILES}
        ${LOGGER_SOURCE_FILES})

target_link_libraries(ctune_logger pthread)

set_target_properties(ctune_logger
        PROPERTIES PREFIX        ""
        OUTPUT_NAME              logger
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

install(TARGETS     ctune_logger
        DESTINATION ${CTUNE_INSTALL_LIBDIR})