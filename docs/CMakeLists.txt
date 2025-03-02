cmake_minimum_required(VERSION 3.17)

project(ctune_documentation LANGUAGES NONE)

#============================================ MAN PAGE ============================================#
find_program(PANDOC pandoc)
find_program(GZIP gzip)

set(MAN_SRC_DIR         ${CMAKE_CURRENT_SOURCE_DIR}/man)
set(MAN_OUT_DIR         ${CMAKE_OUTPUT_DIRECTORY}/man)
set(MAN_OUT_FILENAME    ${CMAKE_PROJECT_NAME}.1)
set(MAN_OUT_FILENAME_GZ ${MAN_OUT_FILENAME}.gz)
set(MAN_SRC_FILENAME    ${CMAKE_PROJECT_NAME}.1.md)
set(MAN_INSTALL_DIR     ${CTUNE_INSTALL_MANDIR})

set(GENERATE_DOCS ON CACHE INTERNAL PARENT_SCOPE)

if(PANDOC AND GZIP)
    add_custom_target(ctune_documentation ALL
            COMMAND ${CMAKE_COMMAND} -E make_directory ${MAN_OUT_DIR}
    )

    add_custom_command(
            PRE_BUILD
            TARGET  ctune_documentation
            COMMAND pandoc
            ARGS    -s -t man -V hyphenate=false ${MAN_SRC_DIR}/${MAN_SRC_FILENAME} -o ${MAN_SRC_DIR}/${MAN_OUT_FILENAME}
    )

    add_custom_command(
            PRE_BUILD
            TARGET  ctune_documentation
            COMMAND gzip
            ARGS    -f ${MAN_SRC_DIR}/${MAN_OUT_FILENAME}
    )

else()
    message(WARNING "Could not find pandoc and/or gzip - repo copy of man page will be used instead.")

endif()

add_custom_command(
        POST_BUILD
        TARGET ctune_documentation
        COMMAND cp
        ARGS -f ${MAN_SRC_DIR}/${MAN_OUT_FILENAME_GZ} ${MAN_OUT_DIR}/${MAN_OUT_FILENAME_GZ}
)

install(FILES ${MAN_OUT_DIR}/${MAN_OUT_FILENAME_GZ}
        TYPE  MAN
)

install(CODE "execute_process(COMMAND ${CMAKE_SOURCE_DIR}/cmake/scripts/update_mandb.sh ARGS ${CMAKE_BUILD_TYPE})")