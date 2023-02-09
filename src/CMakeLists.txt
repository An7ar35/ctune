cmake_minimum_required(VERSION 3.17)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-Wall")
set(CMAKE_C_FLAGS "-Wextra")
set(CMAKE_C_FLAGS_RELEASE "-O2")

include(ExternalProject)


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
        src/enum/Flag.c
        src/enum/ListCategory.c
        src/enum/ListCategory.h
        src/enum/MouseInterval.c
        src/enum/MouseInterval.h
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
        src/ui/window/RSListWin.c
        src/ui/window/RSListWin.h
        src/ui/window/BrowserWin.c
        src/ui/window/BrowserWin.h
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
        src/ui/widget/BorderWin.h

        src/audio/FileOut.h src/audio/FileOut.h src/enum/PluginType.h)
