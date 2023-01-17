cmake_minimum_required(VERSION 3.17)
project(ctune
        LANGUAGES    C
        VERSION      1.1.4
        DESCRIPTION  "Curses based Internet radio tuner"
        HOMEPAGE_URL "eadavison.com")

set(PROJECT_AUTHOR "E.A.Davison")
set(PROJECT_YEARS  "2020-23")

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-Wall")
set(CMAKE_C_FLAGS "-Wextra")
set(CMAKE_C_FLAGS_RELEASE "-O2")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

include(ExternalProject)
include(GNUInstallDirs)

#Build directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build/lib")
set(CMAKE_EXTERNAL_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/external")

#External imports/dependencies directories
set(CMAKE_RESOURCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/resource")
set(CMAKE_LIBRARY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(TEMP_DIR_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tmp")

#Installation directories
set(CTUNE_INSTALL_BINDIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})
set(CTUNE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME})
set(CTUNE_INSTALL_MANDIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_MANDIR}/man1)

#Path linkage
set(CMAKE_INSTALL_RPATH ${CTUNE_INSTALL_LIBDIR})
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

#Import external CMake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
include(FetchContent)
include(Findsndio)
include(Findffmpeg)
include(Findvlc)

set(CMAKE_VERBOSE_MAKEFILE ON)

#========================================== PRE-CHECKS ============================================#
include(FindGit)
find_package(Git)

if (NOT Git_FOUND)
    message(FATAL_ERROR "Required package 'Git' not found on system.")
endif ()

#======================================== LIB IMPORTS =============================================#
# ======= #
# OpenSSL #
# ======= #
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

# ============== #
# JSON-C library #
# ============== #
set(JSONC_DOWNLOAD_PATH "${TEMP_DIR_PATH}/libjsonc")
set(JSONC_INSTALL_PATH "${CMAKE_EXTERNAL_OUTPUT_DIRECTORY}")

add_custom_target(make-jsonc-install-dir ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory ${JSONC_INSTALL_PATH}
)

ExternalProject_Add(libjsonc
        PREFIX              "${JSONC_DOWNLOAD_PATH}"
        SOURCE_DIR          "${JSONC_DOWNLOAD_PATH}/src/libjsonc-build"
        GIT_REPOSITORY      "https://github.com/json-c/json-c.git"
        CMAKE_ARGS          "-DBUILD_STATIC_LIBS=ON"
                            "-DBUILD_SHARED_LIBS=OFF"
                            "-DCMAKE_INSTALL_PREFIX:PATH=${JSONC_INSTALL_PATH}"
                            "-DHAVE_ARC4RANDOM=OFF"
                            "-DCMAKE_BUILD_TYPE=release"
)

set(JSONC_LIBRARIES
        ${CMAKE_EXTERNAL_OUTPUT_DIRECTORY}/lib/libjson-c.a
        bsd
)

# ============ #
# Curl library #
# ============ #
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})

# ================================ #
# nCurses - for the user interface #
# ================================ #
set(${CURSES_NEED_NCURSES} TRUE)
find_package(Curses REQUIRED)
include_directories(${NCURSES_INCLUDEDIR} ${CURSES_INCLUDE_DIR})

#========================================== SOURCE FILES ==========================================#
set(SOURCE_FILES
        src/main.c
        src/Controller.c
        src/Controller.h
        src/cli/CLI.c
        src/cli/CLI.h
        src/network/RadioBrowser.c
        src/network/RadioBrowser.h
        src/network/NetworkUtils.c
        src/network/NetworkUtils.h
        src/utils/utilities.c
        src/utils/utilities.h
        src/ctune_err.h
        src/ctune_err.c
        src/utils/Timeout.c
        src/utils/Timeout.h
        src/datastructure/StrList.c
        src/datastructure/StrList.h
        src/datastructure/ServerList.c
        src/datastructure/ServerList.h
        src/datastructure/String.c
        src/datastructure/String.h
        src/datastructure/Vector.c
        src/datastructure/Vector.h
        src/datastructure/HashMap.c
        src/datastructure/HashMap.h
        src/datastructure/Deque.c
        src/datastructure/Deque.h
        src/dto/ArgOptions.c
        src/dto/ArgOptions.h
        src/dto/RadioStationInfo.c
        src/dto/RadioStationInfo.h
        src/dto/RadioBrowserFilter.c
        src/dto/RadioBrowserFilter.h
        src/dto/CategoryItem.h
        src/dto/CategoryItem.c
        src/dto/ColourTheme.h
        src/dto/ColourTheme.c
        src/dto/ServerStats.c
        src/dto/ServerStats.h
        src/dto/ServerConfig.c
        src/dto/ServerConfig.h
        src/dto/ClickCounter.c
        src/dto/ClickCounter.h
        src/dto/NewRadioStation.h
        src/dto/NewRadioStation.c
        src/dto/RadioStationVote.c
        src/dto/RadioStationVote.h
        src/dto/UIConfig.h
        src/dto/UIConfig.c
        src/dto/Field.h
        src/enum/ByCategory.h
        src/enum/ByCategory.c
        src/enum/FileState.h
        src/enum/Flag.h
        src/enum/ListCategory.c
        src/enum/ListCategory.h
        src/enum/PlaybackCtrl.h
        src/enum/SearchCtrl.h
        src/enum/StationAttribute.c
        src/enum/StationAttribute.h
        src/enum/StationSrc.h
        src/enum/StationSrc.c
        src/enum/UIPreset.h
        src/enum/UIPreset.c
        src/parser/JSON.c
        src/parser/JSON.h
        src/parser/KVPairs.c
        src/parser/KVPairs.h
        src/audio/AudioOut.h
        src/audio/channel_position.h
        src/audio/OutputFormat.h
        src/player/Player.h
        src/player/RadioPlayer.c
        src/player/RadioPlayer.h
        src/fs/fs.h
        src/fs/fs.c
        src/fs/PlaybackLog.c
        src/fs/PlaybackLog.h
        src/fs/Settings.c
        src/fs/Settings.h
        src/fs/Plugin.c
        src/fs/Plugin.h
        src/fs/XDG.c
        src/fs/XDG.h
        src/ui/UI.c
        src/ui/UI.h
        src/ui/Resizer.c
        src/ui/Resizer.h
        src/ui/datastructure/WindowProperty.h
        src/ui/datastructure/WindowMargin.h
        src/ui/datastructure/KeyBind.h
        src/ui/datastructure/KeyInfo.h
        src/ui/definitions/KeyBinding.c
        src/ui/definitions/KeyBinding.h
        src/ui/definitions/KeyDescription.c
        src/ui/definitions/KeyDescription.h
        src/ui/definitions/Language.c
        src/ui/definitions/Language.h
        src/ui/definitions/Theme.c
        src/ui/definitions/Theme.h
        src/ui/enum/ActionID.h
        src/ui/enum/ContextID.h
        src/ui/enum/FormExit.h
        src/ui/enum/PanelID.h
        src/ui/enum/TextID.h
        src/ui/enum/KeyboardKeyID.h
        src/ui/dialog/ContextHelp.c
        src/ui/dialog/ContextHelp.h
        src/ui/dialog/RSInfo.c
        src/ui/dialog/RSInfo.h
        src/ui/dialog/RSFind.c
        src/ui/dialog/RSFind.h
        src/ui/dialog/RSEdit.c
        src/ui/dialog/RSEdit.h
        src/ui/dialog/OptionsMenu.c
        src/ui/dialog/OptionsMenu.h
        src/ui/window/RSListWin.c
        src/ui/window/RSListWin.h
        src/ui/window/BrowserWin.c
        src/ui/window/BrowserWin.h
        src/ui/widget/Dialog.h
        src/ui/widget/Dialog.c
        src/ui/widget/ScrollWin.c
        src/ui/widget/ScrollWin.h
        src/ui/widget/SlideMenu.c
        src/ui/widget/SlideMenu.h
        src/ui/widget/ScrollBar.c
        src/ui/widget/ScrollBar.h
        src/ui/widget/BorderWin.c
        src/ui/widget/BorderWin.h)

set(LOGGER_SOURCE_FILES
        src/logger/Logger.h
        src/logger/Logger.c
        src/logger/LogQueue.c
        src/logger/LogQueue.h
        src/logger/LogWriter.c
        src/logger/LogWriter.h
        src/utils/Timeout.c
        src/utils/Timeout.h
        src/datastructure/String.c
        src/datastructure/String.h)

#==================================== CTUNE LOGGER LIBRARY ========================================#
add_library(ctune_logger SHARED ${LOGGER_SOURCE_FILES})

target_link_libraries(ctune_logger pthread)

set_target_properties(ctune_logger
        PROPERTIES PREFIX        ""
        OUTPUT_NAME              logger
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

#===================================== AUDIO INPUT PLUGINS ========================================#
option(USING_FFMPEG "Compile FFMPEG player plugin" OFF)
option(USING_VLC    "Compile with VLC"             OFF)

set(CTUNE_INPUTS_COUNT          0)
set(CTUNE_PLAYER_PLUGIN_TARGETS "")
set(CTUNE_PLAYER_PLUGIN_DFLT    "")
set(CTUNE_PLAYER_PLUGIN_DIR     player)

if(FFMPEG_FOUND)
    include_directories(${FFMPEG_INCLUDE_DIR})

    MESSAGE(STATUS "FFMpeg detected")
    MATH(EXPR CTUNE_INPUTS_COUNT "${CTUNE_INPUTS_COUNT}+1")

    add_library(ctune_plugin_in_ffmpeg SHARED src/player/ffmpeg.c)

    set_target_properties(ctune_plugin_in_ffmpeg
            PROPERTIES PREFIX        ""
            OUTPUT_NAME              ffmpeg
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CTUNE_PLAYER_PLUGIN_DIR})

    target_link_libraries(ctune_plugin_in_ffmpeg PRIVATE
            ${FFMPEG_LIBRARIES}
            ctune_logger)

    set(USING_FFMPEG ON)
    add_definitions(-DUSING_FFMPEG)
    list(APPEND CTUNE_PLAYER_PLUGIN_TARGETS ctune_plugin_in_ffmpeg)

    set(CTUNE_PLAYER_PLUGIN_DFLT ffmpeg)
endif()

if(VLC_FOUND)
    include_directories(${VLC_INCLUDE_DIR})

    MESSAGE(STATUS "VLC detected")
    MATH(EXPR CTUNE_INPUTS_COUNT "${CTUNE_INPUTS_COUNT}+1")

    add_library(ctune_plugin_in_vlc SHARED
            src/player/vlc.c)

    set_target_properties(ctune_plugin_in_vlc
            PROPERTIES PREFIX        ""
            OUTPUT_NAME              vlc
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CTUNE_PLAYER_PLUGIN_DIR})

    target_link_libraries(ctune_plugin_in_vlc PRIVATE
            ${VLC_LIBRARIES}
            ctune_logger)

    set(USING_VLC ON)
    add_definitions(-DUSING_VLC)
    list(APPEND CTUNE_PLAYER_PLUGIN_TARGETS ctune_plugin_in_vlc)

    if(USING_FFMPEG EQUAL OFF)
        set(CTUNE_PLAYER_PLUGIN_DFLT vlc)
    endif()
endif()

if(CTUNE_INPUTS_COUNT EQUAL 0)
    message(FATAL_ERROR "no supported input libraries detected")
else()
    message(NOTICE "Found ${CTUNE_INPUTS_COUNT} supported input libraries for the player.")
    message(NOTICE "Setting '${CTUNE_PLAYER_PLUGIN_DFLT}' as default player.")
endif()

#==================================== AUDIO OUTPUT PLUGINS ========================================#
find_package(PulseAudio QUIET)
find_package(ALSA QUIET)
find_package(SDL2 QUIET)

set(CTUNE_OUTPUTS_COUNT         0)
set(CTUNE_OUTPUT_PLUGIN_TARGETS "")
set(CTUNE_OUTPUT_PLUGIN_DFLT    "")
set(CTUNE_OUTPUT_PLUGIN_DIR     output)

option(USING_PULSEAUDIO "Compile PulseAudio output plugin" OFF)
option(USING_ALSA       "Compile ALSA output plugin"       OFF)
option(USING_SDL        "Compile SDL2 output plugin"       OFF)
option(USING_SNDIO      "Compile SNDIO output plugin"      OFF)

if(PULSEAUDIO_FOUND)
    MESSAGE(STATUS "PulseAudio detected")
    MATH(EXPR CTUNE_OUTPUTS_COUNT "${CTUNE_OUTPUTS_COUNT}+1")

    add_library(ctune_plugin_out_pulseaudio SHARED src/audio/pulse.c)

    set_target_properties(ctune_plugin_out_pulseaudio
            PROPERTIES PREFIX        ""
            OUTPUT_NAME              pulse
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CTUNE_OUTPUT_PLUGIN_DIR})

    target_link_libraries(ctune_plugin_out_pulseaudio PRIVATE
            pulse
            ctune_logger)

    set(USING_PULSEAUDIO ON)
    add_definitions(-DUSING_PULSEAUDIO)
    list(APPEND CTUNE_OUTPUT_PLUGIN_TARGETS ctune_plugin_out_pulseaudio)

    set(CTUNE_OUTPUT_PLUGIN_DFLT pulse)
endif()

if(ALSA_FOUND)
    message(STATUS "ALSA detected")
    MATH(EXPR CTUNE_OUTPUTS_COUNT "${CTUNE_OUTPUTS_COUNT}+1")

    add_library(ctune_plugin_out_alsa SHARED src/audio/alsa.c)

    set_target_properties(ctune_plugin_out_alsa
            PROPERTIES PREFIX        ""
            OUTPUT_NAME              alsa
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CTUNE_OUTPUT_PLUGIN_DIR})

    target_link_libraries(ctune_plugin_out_alsa PRIVATE
            ALSA::ALSA
            ctune_logger)

    set(USING_ALSA ON)
    add_definitions(-DUSING_ALSA)
    list(APPEND CTUNE_OUTPUT_PLUGIN_TARGETS ctune_plugin_out_alsa)

    if(USING_PULSEAUDIO EQUAL OFF)
        set(CTUNE_OUTPUT_PLUGIN_DFLT alsa)
    endif()
endif()

if(SDL2_FOUND)
    message(STATUS "SDL2 detected")
    MATH(EXPR CTUNE_OUTPUTS_COUNT "${CTUNE_OUTPUTS_COUNT}+1")

    add_library(ctune_plugin_out_sdl SHARED src/audio/sdl.c)

    set_target_properties(ctune_plugin_out_sdl
            PROPERTIES PREFIX        ""
            OUTPUT_NAME              sdl
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CTUNE_OUTPUT_PLUGIN_DIR})

    target_link_libraries(ctune_plugin_out_sdl PRIVATE
            SDL2::SDL2
            ctune_logger)

    set(USING_SDL ON)
    add_definitions(-DUSING_SDL)
    list(APPEND CTUNE_OUTPUT_PLUGIN_TARGETS ctune_plugin_out_sdl)

    if(USING_PULSEAUDIO EQUAL OFF AND USING_ALSA EQUAL OFF)
        set(CTUNE_OUTPUT_PLUGIN_DFLT sdl)
    endif()
endif()

if(SNDIO_FOUND)
    message(STATUS "sndio detected")
    MATH(EXPR CTUNE_OUTPUTS_COUNT "${CTUNE_OUTPUTS_COUNT}+1")

    add_library(ctune_plugin_out_sndio SHARED src/audio/sndio.c)

    set_target_properties(ctune_plugin_out_sndio
            PROPERTIES PREFIX        ""
            OUTPUT_NAME              sndio
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CTUNE_OUTPUT_PLUGIN_DIR})

    target_link_libraries(ctune_plugin_out_sndio PRIVATE
            sndio
            ctune_logger)

    set(USING_SNDIO ON)
    add_definitions(-DUSING_SNDIO)
    list(APPEND CTUNE_OUTPUT_PLUGIN_TARGETS ctune_plugin_out_sndio)

    if(USING_PULSEAUDIO EQUAL OFF AND USING_ALSA EQUAL OFF AND USING_SDL EQUAL OFF)
        set(CTUNE_OUTPUT_PLUGIN_DFLT sndio)
    endif()
endif()

if(CTUNE_OUTPUTS_COUNT EQUAL 0)
    message(FATAL_ERROR "no supported sound server detected")
else()
    message(NOTICE "Found ${CTUNE_OUTPUTS_COUNT} supported sound server libraries.")
    message(NOTICE "Setting '${CTUNE_OUTPUT_PLUGIN_DFLT}' as default sound output.")
endif()


#========================== CMAKE VARIABLES DEFINITION CREATOR FOR SOURCE =========================#
configure_file(src/cmake/cmake_vars.h ${CMAKE_BINARY_DIR}/generated-src/project_version.h )
include_directories( ${CMAKE_BINARY_DIR}/generated-src/ )

#============================================ MAN PAGE ============================================#
find_program(PANDOC pandoc)
find_program(GZIP gzip)

set(MAN_SRC_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/docs/man)
set(MAN_OUT_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/build/man)
set(MAN_SRC_FILENAME ${CMAKE_PROJECT_NAME}.1.md)
set(MAN_OUT_FILENAME ${CMAKE_PROJECT_NAME}.1)
set(MAN_INSTALL_DIR  ${CTUNE_INSTALL_MANDIR})

if(PANDOC AND GZIP)
    add_custom_target(man ALL
            COMMAND ${CMAKE_COMMAND} -E make_directory ${MAN_OUT_DIR}
    )

    add_custom_command(TARGET man
            COMMAND pandoc
            ARGS -s -t man -V hyphenate=false ${MAN_SRC_DIR}/${MAN_SRC_FILENAME} -o ${MAN_OUT_DIR}/${MAN_OUT_FILENAME}
    )

    add_custom_command(TARGET man
            COMMAND gzip
            ARGS -f ${MAN_OUT_DIR}/${MAN_OUT_FILENAME}
    )
else()
    message(WARNING "Could not find pandoc and/or gzip - man page will not be generated.")
endif()

#======================================== CTUNE EXECUTABLE ========================================#
#(debuging) prints cmake vars
#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

add_executable(ctune ${SOURCE_FILES})
add_dependencies(ctune ctune_logger libjsonc)
include_directories(${CMAKE_EXTERNAL_OUTPUT_DIRECTORY}/include)

if(USING_FFMPEG)
    add_dependencies(ctune ctune_plugin_in_ffmpeg)
endif()

if(USING_VLC)
    add_dependencies(ctune ctune_plugin_in_vlc)
endif()

if(USING_SDL)
    add_dependencies(ctune ctune_plugin_out_sdl)
endif()

if (USING_PULSEAUDIO)
    add_dependencies(ctune ctune_plugin_out_pulseaudio)
endif ()

if(USING_ALSA)
    add_dependencies(ctune ctune_plugin_out_alsa)
endif()

if(USING_SNDIO)
    add_dependencies(ctune ctune_plugin_out_sndio)
endif()

if(PANDOC AND GZIP)
    add_dependencies(ctune man)
endif()

target_link_libraries(ctune PRIVATE
        ctune_logger            #CTUNE_LOG(..)
        ${JSONC_LIBRARIES}      #(json-c/*.h) for `parse/JSON.c`
        ${NCURSES_LIBRARIES}    #(ncurses.h) for `ui/*`
        ${CURSES_LIBRARIES}     #(ncurses.h) for `ui/*`
        panel                   #(panel.h) ncurses panel lib for `ui/*`
        form                    #(form.h) ncurses form lib for `ui/*`
        pthread                 #threading
        OpenSSL::SSL            #(openssl/*.h) for `network/NetworkUtils.c`
        OpenSSL::Crypto         #(openssl/*.h) for `network/NetworkUtils.c`
        m                       #(math.h) for `utils/utilities.c`
        uuid                    #(uuid/uuid.h) for `utils/utilities.c`
        curl                    #(curl/curl.h) for `network/NetworkUtils.c`
        dl                      #(dlfcn.h) for `fs/Plugin.c`
)

#=========================================== INSTALL ==============================================#
install(TARGETS
            ctune
        DESTINATION
            ${CTUNE_INSTALL_BINDIR}
)

install(TARGETS
            ctune_logger
        DESTINATION
            ${CTUNE_INSTALL_LIBDIR}
)

install(TARGETS
            ${CTUNE_PLAYER_PLUGIN_TARGETS}
        DESTINATION
            ${CTUNE_INSTALL_LIBDIR}/${CTUNE_PLAYER_PLUGIN_DIR}
)

install(TARGETS
            ${CTUNE_OUTPUT_PLUGIN_TARGETS}
        DESTINATION
            ${CTUNE_INSTALL_LIBDIR}/${CTUNE_OUTPUT_PLUGIN_DIR}
)

if(PANDOC AND GZIP)
    install(FILES
                ${MAN_OUT_DIR}/${MAN_OUT_FILENAME}.gz
            DESTINATION
                ${CTUNE_INSTALL_MANDIR}
    )

    install(CODE
            "execute_process(COMMAND ${CMAKE_SOURCE_DIR}/cmake/scripts/update_mandb.sh ARGS ${CMAKE_BUILD_TYPE})"
    )
endif()

#========================================== UNINSTALL =============================================#
if(TARGET uninstall)
    configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/inputs/CMakeUninstall.in"
            "${CMAKE_CURRENT_SOURCE_DIR}/CMakeUninstall.cmake"
            IMMEDIATE @ONLY)

    add_custom_target(uninstall
            COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/CMakeUninstall.cmake)
endif()