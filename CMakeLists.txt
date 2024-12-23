cmake_minimum_required(VERSION 3.17)
project(ctune
        LANGUAGES    C
        VERSION      1.3.3
        DESCRIPTION  "Curses based Internet radio tuner"
        HOMEPAGE_URL "eadavison.com")

set(PROJECT_AUTHOR "E.A.Davison")
set(PROJECT_YEARS  "2020-24")

set(CMAKE_C_STANDARD 99         CACHE INTERNAL "")
set(CMAKE_C_FLAGS "-Wall"       CACHE INTERNAL "")
set(CMAKE_C_FLAGS "-Wextra"     CACHE INTERNAL "")
set(CMAKE_C_FLAGS_RELEASE "-O2" CACHE INTERNAL "")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif(NOT CMAKE_BUILD_TYPE)

include(ExternalProject)
include(GNUInstallDirs)

#Build directories
set(CMAKE_OUTPUT_DIRECTORY          "${CMAKE_CURRENT_SOURCE_DIR}/build"         CACHE INTERNAL "")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_OUTPUT_DIRECTORY}/bin"             CACHE INTERNAL "")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  "${CMAKE_OUTPUT_DIRECTORY}/lib"             CACHE INTERNAL "")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  "${CMAKE_OUTPUT_DIRECTORY}/lib"             CACHE INTERNAL "")
set(CMAKE_EXTERNAL_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/external"      CACHE INTERNAL "")
set(PLUGINS_OUTPUT_DIRECTORY        "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/plugins" CACHE INTERNAL "")

#External imports/dependencies directories
set(CMAKE_RESOURCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/resource")
set(CMAKE_LIBRARY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(TEMP_DIR_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tmp")
include_directories(libraries)

#Installation directories
set(CTUNE_INSTALL_BINDIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}                 CACHE INTERNAL "")
set(CTUNE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME} CACHE INTERNAL "")
set(CTUNE_INSTALL_MANDIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_MANDIR}/man1            CACHE INTERNAL "")

#Path linkage
set(CMAKE_INSTALL_RPATH ${CTUNE_INSTALL_LIBDIR})
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

#Import external CMake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
include(FetchContent)


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
        PREFIX         "${JSONC_DOWNLOAD_PATH}"
        SOURCE_DIR     "${JSONC_DOWNLOAD_PATH}/src/libjsonc-build"
        GIT_REPOSITORY "https://github.com/json-c/json-c.git"
        CMAKE_ARGS     "-DBUILD_STATIC_LIBS=ON"
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
        src/datastructure/CircularBuffer.c
        src/datastructure/CircularBuffer.h
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
        src/dto/PluginInfo.h
        src/dto/RadioStationVote.c
        src/dto/RadioStationVote.h
        src/dto/UIConfig.h
        src/dto/UIConfig.c
        src/dto/Field.h
        src/enum/ByCategory.h
        src/enum/ByCategory.c
        src/enum/FileState.h
        src/enum/Flag.h
        src/enum/Flag.c
        src/enum/ListCategory.c
        src/enum/ListCategory.h
        src/enum/MouseInterval.c
        src/enum/MouseInterval.h
        src/enum/PlaybackCtrl.c
        src/enum/PlaybackCtrl.h
        src/enum/PluginType.h
        src/enum/PluginType.c
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
        src/audio/FileOut.h
        src/audio/FileOut.h
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
        src/ui/EventQueue.c
        src/ui/EventQueue.h
        src/ui/Resizer.c
        src/ui/Resizer.h
        src/ui/datastructure/WindowProperty.h
        src/ui/datastructure/WindowMargin.h
        src/ui/datastructure/KeyBind.h
        src/ui/datastructure/KeyInfo.h
        src/ui/definitions/Icons.c
        src/ui/definitions/Icons.h
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
        src/ui/enum/EventType.c
        src/ui/enum/EventType.h
        src/ui/enum/FormExit.h
        src/ui/enum/IconID.h
        src/ui/enum/PanelID.h
        src/ui/enum/PanelID.c
        src/ui/enum/TextID.h
        src/ui/enum/InputKeyID.h
        src/ui/types/WinCtrlMask.h
        src/ui/types/WinCtrlMask.c
        src/ui/types/ScrollMask.c
        src/ui/types/ScrollMask.h
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
        src/ui/dialog/SetOutputDir.c
        src/ui/dialog/SetOutputDir.h
        src/ui/window/RSListWin.c
        src/ui/window/RSListWin.h
        src/ui/window/BrowserWin.c
        src/ui/window/BrowserWin.h
        src/ui/window/MainWin.c
        src/ui/window/MainWin.h
        src/ui/widget/Dialog.h
        src/ui/widget/Dialog.c
        src/ui/widget/Form.c
        src/ui/widget/Form.h
        src/ui/widget/ScrollWin.c
        src/ui/widget/ScrollWin.h
        src/ui/widget/SlideMenu.c
        src/ui/widget/SlideMenu.h
        src/ui/widget/ScrollBar.c
        src/ui/widget/ScrollBar.h
        src/ui/widget/BorderWin.c
        src/ui/widget/BorderWin.h )

set(TEST_FILES
        tests/main.c
        src/datastructure/CircularBuffer.c
        src/datastructure/CircularBuffer.h
        tests/datastructure/CircularBuffer.c)
#==================================== CTUNE LOGGER LIBRARY ========================================#
add_subdirectory(libraries/logger)
link_directories(libraries/logger)

#===================================== AUDIO INPUT PLUGINS ========================================#
set(CTUNE_PLUGIN_LIST_AUDIO_PLAYER "")

add_subdirectory(plugins/input/ffmpeg)
add_subdirectory(plugins/input/vlc)

get_directory_property(FFMPEG_FOUND DIRECTORY plugins/input/ffmpeg DEFINITION FFMPEG_FOUND)
get_directory_property(VLC_FOUND    DIRECTORY plugins/input/vlc    DEFINITION VLC_FOUND)

if(FFMPEG_FOUND)
    link_directories(plugins/input/ffmpeg)
    list(APPEND CTUNE_PLUGIN_LIST_AUDIO_PLAYER ctune_plugin_ffmpeg)
    add_definitions(-DFFMPEG_FOUND)
endif(FFMPEG_FOUND)

if(VLC_FOUND)
    link_directories(plugins/input/vlc)
    list(APPEND CTUNE_PLUGIN_LIST_AUDIO_PLAYER ctune_plugin_vlc)
    add_definitions(-DVLC_FOUND)
endif(VLC_FOUND)

list(LENGTH CTUNE_PLUGIN_LIST_AUDIO_PLAYER CTUNE_PLUGIN_LIST_AUDIO_PLAYER_COUNT)

if(CTUNE_PLUGIN_LIST_AUDIO_PLAYER_COUNT EQUAL 0)
    message(FATAL_ERROR "no supported input libraries detected")
else()
    list(GET CTUNE_PLUGIN_LIST_AUDIO_PLAYER 0 CTUNE_DEFAULT_PLAYER_PLUGIN)
    get_target_property(CTUNE_DEFAULT_PLAYER_PLUGIN_NAME ${CTUNE_DEFAULT_PLAYER_PLUGIN} OUTPUT_NAME)
    message(NOTICE "Setting '${CTUNE_DEFAULT_PLAYER_PLUGIN_NAME}' as default player.")
endif()

#================================= SOUND FILE OUTPUT PLUGINS ======================================#
set(CTUNE_PLUGIN_LIST_AUDIO_FILE   "")

add_subdirectory(plugins/output/mp3)
add_subdirectory(plugins/output/wave)

get_directory_property(LAME_FOUND DIRECTORY plugins/output/mp3  DEFINITION LAME_FOUND)
get_directory_property(WAVE_FOUND DIRECTORY plugins/output/wave DEFINITION WAVE_FOUND)

if(LAME_FOUND)
    link_directories(plugins/output/mp3)
    list(APPEND CTUNE_PLUGIN_LIST_AUDIO_FILE ctune_plugin_mp3)
    add_definitions(-DLAME_FOUND)
endif ()

if(WAVE_FOUND)
    link_directories(plugins/output/wave)
    list(APPEND CTUNE_PLUGIN_LIST_AUDIO_FILE ctune_plugin_wave)
    add_definitions(-DWAVE_FOUND)
endif(WAVE_FOUND)

list(LENGTH CTUNE_PLUGIN_LIST_AUDIO_FILE CTUNE_PLUGIN_LIST_AUDIO_FILE_COUNT)

if(CTUNE_PLUGIN_LIST_AUDIO_FILE_COUNT EQUAL 0)
    message(FATAL_ERROR "no supported recording libraries detected")
else()
    list(GET CTUNE_PLUGIN_LIST_AUDIO_FILE 0 CTUNE_DEFAULT_AUDIO_FILE_PLUGIN)
    get_target_property(CTUNE_DEFAULT_AUDIO_FILE_PLUGIN_NAME ${CTUNE_DEFAULT_AUDIO_FILE_PLUGIN} OUTPUT_NAME)
    message(NOTICE "Setting '${CTUNE_DEFAULT_AUDIO_FILE_PLUGIN_NAME}' as default recorder.")
endif()

#==================================== AUDIO OUTPUT PLUGINS ========================================#
set(CTUNE_PLUGIN_LIST_AUDIO_SRV "")

add_subdirectory(plugins/output/pipewire)
add_subdirectory(plugins/output/pulseaudio)
add_subdirectory(plugins/output/alsa)
add_subdirectory(plugins/output/sdl)
add_subdirectory(plugins/output/sndio)

get_directory_property(ALSA_FOUND       DIRECTORY plugins/output/alsa       DEFINITION ALSA_FOUND)
get_directory_property(PULSEAUDIO_FOUND DIRECTORY plugins/output/pulseaudio DEFINITION PULSEAUDIO_FOUND)
get_directory_property(PIPEWIRE_FOUND   DIRECTORY plugins/output/pipewire   DEFINITION PIPEWIRE_FOUND)
get_directory_property(SDL2_FOUND       DIRECTORY plugins/output/sdl        DEFINITION SDL2_FOUND)
get_directory_property(SNDIO_FOUND      DIRECTORY plugins/output/sndio      DEFINITION SNDIO_FOUND)

if(PULSEAUDIO_FOUND)
    link_directories(plugins/output/pulseaudio)
    list(APPEND CTUNE_PLUGIN_LIST_AUDIO_SRV ctune_plugin_pulseaudio)
    add_definitions(-DPULSEAUDIO_FOUND)
endif(PULSEAUDIO_FOUND)

if(ALSA_FOUND)
    link_directories(plugins/output/alsa)
    list(APPEND CTUNE_PLUGIN_LIST_AUDIO_SRV ctune_plugin_alsa)
    add_definitions(-DALSA_FOUND)
endif()

if(PIPEWIRE_FOUND)
    link_directories(plugins/output/pipewire)
    list(APPEND CTUNE_PLUGIN_LIST_AUDIO_SRV ctune_plugin_pipewire)
    add_definitions(-DPIPEWIRE_FOUND)
endif(PIPEWIRE_FOUND)

if(SDL2_FOUND)
    link_directories(plugins/output/sdl)
    list(APPEND CTUNE_PLUGIN_LIST_AUDIO_SRV ctune_plugin_sdl)
    add_definitions(-DSDL2_FOUND)
endif()

if(SNDIO_FOUND)
    link_directories(plugins/output/sndio)
    list(APPEND CTUNE_PLUGIN_LIST_AUDIO_SRV ctune_plugin_sndio)
    add_definitions(-DSNDIO_FOUND)
endif()

list(LENGTH CTUNE_PLUGIN_LIST_AUDIO_SRV CTUNE_PLUGIN_LIST_AUDIO_SRV_COUNT)

if(CTUNE_PLUGIN_LIST_AUDIO_SRV_COUNT EQUAL 0)
    message(FATAL_ERROR "no supported sound server libraries detected")
else()
    list(GET CTUNE_PLUGIN_LIST_AUDIO_SRV 0 CTUNE_DEFAULT_AUDIO_SRV_PLUGIN)
    get_target_property(CTUNE_DEFAULT_AUDIO_SRV_PLUGIN_NAME ${CTUNE_DEFAULT_AUDIO_SRV_PLUGIN} OUTPUT_NAME)
    message(NOTICE "Setting '${CTUNE_DEFAULT_AUDIO_SRV_PLUGIN_NAME}' as default sound server.")
endif()

#========================== CMAKE VARIABLES DEFINITION CREATOR FOR SOURCE =========================#
configure_file(src/cmake/cmake_vars.h ${CMAKE_BINARY_DIR}/generated-src/project_version.h )
include_directories( ${CMAKE_BINARY_DIR}/generated-src/ )

##============================================ MAN PAGE ============================================#
add_subdirectory(docs)

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

add_dependencies(ctune ${CTUNE_PLUGIN_LIST_AUDIO_PLAYER})
add_dependencies(ctune ${CTUNE_PLUGIN_LIST_AUDIO_FILE})
add_dependencies(ctune ${CTUNE_PLUGIN_LIST_AUDIO_SRV})

if(GENERATE_DOCS)
    add_dependencies(ctune ctune_documentation)
endif(GENERATE_DOCS)

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
install(TARGETS     ctune
        DESTINATION ${CTUNE_INSTALL_BINDIR})

#========================================== UNINSTALL =============================================#
if(TARGET uninstall)
    configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/inputs/CMakeUninstall.in"
            "${CMAKE_CURRENT_SOURCE_DIR}/CMakeUninstall.cmake"
            IMMEDIATE @ONLY)

    add_custom_target(uninstall
            COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/CMakeUninstall.cmake)
endif()