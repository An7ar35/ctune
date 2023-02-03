#include "Settings.h"

#include <stdio.h>
#include <errno.h>

#include "fs.h"
#include "XDG.h"
#include "Plugin.h"
#include "../datastructure/HashMap.h"
#include "../logger/Logger.h"
#include "../dto/RadioStationInfo.h"
#include "../parser/JSON.h"
#include "../parser/KVPairs.h"
#include "../utils/utilities.h"
#include "project_version.h"

#define CFG_KEY_RESUME_VOL                      "Resume::Volume"
#define CFG_KEY_LAST_STATION_PLAYED_UUID        "Resume::UUID"
#define CFG_KEY_LAST_STATION_PLAYED_SRC         "Resume::SourceID"
#define CFG_KEY_INPUT_LIB                       "IO::Plugin::Player"
#define CFG_KEY_OUTPUT_LIB                      "IO::Plugin::SoundServer"
#define CFG_KEY_OVERWRITE_PLAYLOG               "IO::OverwritePlayLog"
#define CFG_KEY_STREAM_TIMEOUT                  "IO::StreamTimeout"
#define CFG_KEY_NETWORK_TIMEOUT                 "IO::NetworkTimeout"
#define CFG_KEY_UI_MOUSE                        "UI::Mouse"
#define CFG_KEY_UI_MOUSE_RESOLUTION             "UI::Mouse::Resolution"
#define CFG_KEY_UI_MOUSE_RESOLUTION_PRESET      "UI::Mouse::Resolution::Preset"
#define CFG_KEY_UI_UNICODE_ICONS                "UI::UnicodeIcons"
#define CFG_KEY_UI_FAVTAB_SHOW_THEMING          "UI::Favourites::ShowTheme"
#define CFG_KEY_UI_FAVTAB_USE_CUSTOM_THEMING    "UI::Favourites::UseCustomTheme"
#define CFG_KEY_UI_FAVTAB_LRG                   "UI::Favourites::UseLargeRows"
#define CFG_KEY_UI_FAVTAB_SORTBY                "UI::Favourites::SortBy"
#define CFG_KEY_UI_SEARCHTAB_LRG                "UI::Search::UseLargeRows"
#define CFG_KEY_UI_BROWSERTAB_LRG               "UI::Browser::UseLargeRows"
#define CFG_KEY_UI_THEME_PRESET                 "UI::Theme::preset"
#define CFG_KEY_UI_THEME                        "UI::Theme"
#define CFG_KEY_UI_THEME_ROW                    "UI::Theme::row"
#define CFG_KEY_UI_THEME_ROW_SELECTED_FOCUSED   "UI::Theme::row::selected::focused"
#define CFG_KEY_UI_THEME_ROW_SELECTED_UNFOCUSED "UI::Theme::row::selected::unfocused"
#define CFG_KEY_UI_THEME_ROW_FAVOURITE_LOCAL    "UI::Theme::row::favourite::local"
#define CFG_KEY_UI_THEME_ROW_FAVOURITE_REMOTE   "UI::Theme::row::favourite::remote"
#define CFG_KEY_UI_THEME_ICON_PLAYBACK_ON       "UI::Theme::icon::playback::on"
#define CFG_KEY_UI_THEME_ICON_PLAYBACK_OFF      "UI::Theme::icon::playback::off"
#define CFG_KEY_UI_THEME_ICON_QUEUED            "UI::Theme::icon::queued"
#define CFG_KEY_UI_THEME_FIELD_INVALID          "UI::Theme::field::invalid"
#define CFG_KEY_UI_THEME_BUTTON                 "UI::Theme::button"
#define CFG_KEY_UI_THEME_BUTTON_INVALID         "UI::Theme::button::invalid"
#define CFG_KEY_UI_THEME_BUTTON_VALIDATED       "UI::Theme::button::validated"


#define CTUNE_PLAYER_PLUGIN_COUNT       2
#define CTUNE_SOUND_OUTPUT_PLUGIN_COUNT 4
#define CTUNE_LOCK_FILENAME             "ctune.lock"

/**
 * List of supported input plugin names (i.e.: players)
 */
static const char * player_plugins[CTUNE_PLAYER_PLUGIN_COUNT] = {
    "ffmpeg",
    "vlc",
};

/**
 * List of supported output plugin names (i.e.: sound servers)
 */
static const char * sound_output_plugins[CTUNE_SOUND_OUTPUT_PLUGIN_COUNT] = {
    "pulse",
    "sdl",
    "alsa",
    "sndio",
};

static struct ctune_Settings_Cfg {
    bool             loaded;
    const char *     file_name;
    int              resume_volume;
    bool             play_log_overwrite;
    int              timeout_stream_val;
    int              timeout_network_val;

    struct {
        const char * sys_lib_path;

        struct {
            const char * dir_name;
            const char * dflt_name;
            String_t     name;
        } player;

        struct {
            const char * dir_name;
            const char * dflt_name;
            String_t     name;
        } sound_server;

    } io_libs;

    ctune_UIConfig_t ui;

    struct {
        String_t           uuid;
        ctune_StationSrc_e src;
    } last_station;
} config;

static struct ctune_Settings_Fav {
    const char * file_name;
    const char * backup_name;
    HashMap_t    favs[CTUNE_STATIONSRC_COUNT];

    ctune_RadioStationInfo_SortBy_e sort_id;
} favourites;

/**
 * [PRIVATE] Gets the total number of favourites stations in the collections
 * @return Tally of all favourites
 */
static size_t ctune_Settings_favouriteCount( void ) {
    size_t count = 0;

    for( int i = 0; i < CTUNE_STATIONSRC_COUNT; ++i ) {
        count += HashMap.size( &favourites.favs[i] );
    }

    return count;
}

/**
 * Initialises all the variables for the Settings instance
 */
static void ctune_Settings_init( void ) {
    favourites = (struct ctune_Settings_Fav) {
        .file_name   = "ctune.fav",
        .backup_name = "ctune.fav.bck",
        .sort_id     = CTUNE_RADIOSTATIONINFO_SORTBY_NONE,
    };

    config = (struct ctune_Settings_Cfg) {
        .loaded                 = false,
        .file_name              = "ctune.cfg",
        .resume_volume          = 100,
        .play_log_overwrite     = true,
        .timeout_stream_val     = 5, //in seconds
        .timeout_network_val    = 8, //in seconds

        .io_libs = {
            .sys_lib_path           = CTUNE_SYSLIB_PATH,
            .player.dir_name        = "/player/",
            .player.dflt_name       = CTUNE_PLUGIN_PLAYER_DFLT,
            .player.name            = { NULL, 0 },
            .sound_server.dir_name  = "/output/",
            .sound_server.dflt_name = CTUNE_PLUGIN_SNDSRV_DFLT,
            .sound_server.name      = { NULL, 0 },
        },

        .last_station = {
            .uuid = { ._raw = NULL, ._length = 0 },
            .src  = CTUNE_STATIONSRC_LOCAL,
        },
    };

    config.ui = ctune_UIConfig.create(); //init with default variables and theme
}

//=================================== CTUNE RUNTIME LOCK ===========================================

/**
 * Creates the lock file
 * @return Success
 */
static bool ctune_Settings_rtlock_lock( void ) {
    String_t lockfile_path = String.init();

    ctune_XDG.resolveDataFilePath( CTUNE_LOCK_FILENAME, &lockfile_path );

    int file_state = ctune_fs.getFileState( lockfile_path._raw, NULL );

    switch( file_state ) {
        case CTUNE_FILE_NOTFOUND: {
            FILE * lockfile = fopen( lockfile_path._raw, "w" );

            if( lockfile ) {
                fclose( lockfile );
                CTUNE_LOG( CTUNE_LOG_MSG, "[ctune_Settings_rtlock_lock()] Lock file created: %s", lockfile_path._raw );

            } else {
                fprintf( stderr, "Failed to create lock file: %s\n", lockfile_path._raw );
                file_state = CTUNE_FILE_ERR;
            }
        } break;

        case CTUNE_FILE_FOUND: {
            fprintf( stdout, "ERROR    : Lock file found: %s\n"
                             "ISSUE    : Another instance of ctune may be running already.\n"
                             "SOLUTIONS: a) Close other instance and try launching again.\n"
                             "           b) Kill other instance, delete lock file and try launching again.\n"
                             "           c) Reboot system, delete lock file and try launching again.\n"
                             "           d) Report bug.\n",
                             lockfile_path._raw
            );
        } break;

        default: //CTUNE_FILE_ERR
            fprintf( stdout, "Error raised when trying to find lock file: %s\n" ,lockfile_path._raw );
        break;
    }

    String.free( &lockfile_path );
    return ( file_state == CTUNE_FILE_NOTFOUND );
}

/**
 * Deletes the lock file
 * @return Success
 */
static bool ctune_Settings_rtlock_unlock( void ) {
    String_t lockfile_path = String.init();
    bool     error_state = false;

    ctune_XDG.resolveDataFilePath( CTUNE_LOCK_FILENAME, &lockfile_path );

    const int file_state = ctune_fs.getFileState( lockfile_path._raw, NULL );

    if( file_state == CTUNE_FILE_FOUND ) {
        if( remove( lockfile_path._raw ) == 0 ) {
            CTUNE_LOG( CTUNE_LOG_MSG, "[ctune_Settings_rtlock_unlock()] Lock file deleted successfully." );

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_Settings_rtlock_unlock()] Failed to remove lock file (\"%s\"): %s",
                       lockfile_path._raw,
                       strerror( errno )
            );

            error_state = true;
        }

    } else if( file_state == CTUNE_FILE_ERR ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Settings_rtlock_unlock()] Failed to get lock file state (\"%s\").",
                   lockfile_path._raw
        );

        error_state = true;
    }

    String.free( &lockfile_path );
    return !( error_state );
}

//===================================== CTUNE CONFIG ===============================================
/**
 * Loads the setting file
 * @return Success
 */
static bool ctune_Settings_loadCfg() {
    String_t file_path    = String.init();
    String_t file_content = String.init();
    String_t key          = String.init();
    String_t val          = String.init();
    bool     error_state  = false;

    ctune_XDG.resolveCfgFilePath( config.file_name, &file_path );

    if( !ctune_fs.readFile( file_path._raw, &file_content ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_Settings_loadCfg()] Error reading configuration file into String_t object." );
        error_state = true;
        goto end_fail;
    }

    char * ptr, * tmp;
    size_t n = 1;

    ptr = strtok_r( file_content._raw, "\n", &tmp ); //Note: POSIX specific

    do {
        CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_Settings_loadCfg()] Line #%lu: \"%s\"", n, ptr );

        bool error = false;

        if( ctune_Parser_KVPairs.parse( ptr, '=', &key, &val ) ) {
            if( strcmp( CFG_KEY_LAST_STATION_PLAYED_UUID, key._raw ) == 0 ) { //uuid string
                error = !ctune_Parser_KVPairs.validateUUID( &val, &config.last_station.uuid );

            } else if( strcmp( CFG_KEY_LAST_STATION_PLAYED_SRC, key._raw ) == 0 ) {
                int tmp_val = 0;
                error = !ctune_Parser_KVPairs.validateInteger( &val, &tmp_val );

                if( !error ) {
                    if( tmp_val >= 0 && tmp_val < CTUNE_STATIONSRC_COUNT ) {
                        config.last_station.src = ( ctune_StationSrc_e ) tmp_val;

                    } else {
                        error = true;

                        CTUNE_LOG( CTUNE_LOG_ERROR,
                                   "[ctune_Settings_loadCfg()] Line #%lu: value (%i) is out of range of the ctune_StationSrc_e type.",
                                   n, tmp_val
                        );
                    }
                }

            } else if( strcmp( CFG_KEY_RESUME_VOL, key._raw ) == 0 ) { //int
                int vol = config.resume_volume;

                if( !( error = !ctune_Parser_KVPairs.validateInteger( &val, &vol ) ) ) {
                    ctune_Settings.cfg.setVolume( vol );
                }

            } else if( strcmp( CFG_KEY_INPUT_LIB, key._raw ) == 0 ) { //string
                int index = -1;

                if( ( index = ctune_Parser_KVPairs.validateString( &val, player_plugins, CTUNE_PLAYER_PLUGIN_COUNT, false ) ) >= 0 ) {
                    String.set( &config.io_libs.player.name, player_plugins[ index ] );

                } else {
                    CTUNE_LOG( CTUNE_LOG_FATAL,
                               "[ctune_Settings_loadCfg()] Line #%lu: value (\"%s\") is not a valid input plugin.",
                               n, val._raw
                    );

                    error_state = true;
                }

            } else if( strcmp( CFG_KEY_OUTPUT_LIB, key._raw ) == 0 ) { //string
                int index = -1;

                if( ( index = ctune_Parser_KVPairs.validateString( &val, sound_output_plugins, CTUNE_SOUND_OUTPUT_PLUGIN_COUNT, false ) ) >= 0 ) {
                    String.set( &config.io_libs.sound_server.name, sound_output_plugins[ index ] );

                } else {
                    CTUNE_LOG( CTUNE_LOG_FATAL,
                               "[ctune_Settings_loadCfg()] Line #%lu: value (\"%s\") is not a valid output plugin.",
                               n, val._raw
                    );

                    error_state = true;
                }

            } else if( strcmp( CFG_KEY_OVERWRITE_PLAYLOG, key._raw ) == 0 ) { //bool
                error = !ctune_Parser_KVPairs.validateBoolean( &val, &config.play_log_overwrite );

            } else if( strcmp( CFG_KEY_UI_FAVTAB_LRG, key._raw ) == 0 ) { //bool
                error = !ctune_Parser_KVPairs.validateBoolean( &val, &config.ui.fav_tab.large_rows );

            } else if( strcmp( CFG_KEY_UI_FAVTAB_SORTBY, key._raw ) == 0 ) { //int
                int tmp_val = 0;
                error = !ctune_Parser_KVPairs.validateInteger( &val, &tmp_val );

                if( !error ) {
                    if( tmp_val >= CTUNE_RADIOSTATIONINFO_SORTBY_NONE && tmp_val < CTUNE_RADIOSTATIONINFO_SORTBY_COUNT ) {
                        favourites.sort_id = ( ctune_RadioStationInfo_SortBy_e ) tmp_val;

                    } else {
                        error = true;

                        CTUNE_LOG( CTUNE_LOG_ERROR,
                                   "[ctune_Settings_loadCfg()] Line #%lu: value (%i) is out of range of the ctune_RadioStationInfo_SortBy_e type.",
                                   n, tmp_val
                        );
                    }
                }

            } else if( strcmp( CFG_KEY_UI_SEARCHTAB_LRG, key._raw ) == 0 ) { //bool
                error = !ctune_Parser_KVPairs.validateBoolean( &val, &config.ui.search_tab.large_rows );

            } else if( strcmp( CFG_KEY_UI_BROWSERTAB_LRG, key._raw ) == 0 ) { //bool
                error = !ctune_Parser_KVPairs.validateBoolean( &val, &config.ui.browse_tab.large_rows );

            } else if( strcmp( CFG_KEY_STREAM_TIMEOUT, key._raw ) == 0 ) { //int
                error = !ctune_Parser_KVPairs.validateInteger( &val, &config.timeout_stream_val );

            } else if( strcmp( CFG_KEY_UI_MOUSE, key._raw ) == 0 ) { //bool
                error = !ctune_Parser_KVPairs.validateBoolean( &val, &config.ui.mouse.enabled );

            } else if( strcmp( CFG_KEY_UI_MOUSE_RESOLUTION, key._raw ) == 0 ) { //int
                error = !ctune_Parser_KVPairs.validateInteger( &val, &config.ui.mouse.resolution );

            } else if( strcmp( CFG_KEY_UI_MOUSE_RESOLUTION_PRESET, key._raw ) == 0 ) {
                int preset = CTUNE_MOUSERESOLUTION_NOT_SET;

                error = !ctune_Parser_KVPairs.validateInteger( &val, &preset );

                if( !error && preset >= 0 ) {
                    ctune_UIConfig.mouse.setResolutionPreset( &config.ui, preset );
                }

            } else if( strcmp( CFG_KEY_UI_UNICODE_ICONS, key._raw ) == 0 ) { //bool
                error = !ctune_Parser_KVPairs.validateBoolean( &val, &config.ui.unicode_icons );

            } else if( strcmp( CFG_KEY_UI_FAVTAB_SHOW_THEMING, key._raw ) == 0 ) { //bool
                error = !ctune_Parser_KVPairs.validateBoolean( &val, &config.ui.fav_tab.theme_favourites );

            } else if( strcmp( CFG_KEY_UI_FAVTAB_USE_CUSTOM_THEMING, key._raw ) == 0 ) { //bool
                error = !ctune_Parser_KVPairs.validateBoolean( &val, &config.ui.fav_tab.custom_theming );

            } else if( strcmp( CFG_KEY_NETWORK_TIMEOUT, key._raw ) == 0 ) { //int
                error = !ctune_Parser_KVPairs.validateInteger( &val, &config.timeout_network_val );

            } else if( strcmp( CFG_KEY_UI_THEME_PRESET, key._raw ) == 0 ) { //string
                ctune_UIPreset_e preset = ctune_Parser_KVPairs.validateString( &val, ctune_UIPreset.presetList(), CTUNE_UIPRESET_COUNT, false );

                if( !( error = ( preset == CTUNE_UIPRESET_UNKNOWN ) ) ) {
                    config.ui.theme.preset = preset;

                    if( preset != CTUNE_UIPRESET_CUSTOM ) {
                        config.ui.theme.preset_pallet = ctune_ColourTheme.init( preset );
                    }
                }

            } else if( strcmp( CFG_KEY_UI_THEME, key._raw ) == 0 ) { //colour str pair
                error = !ctune_Parser_KVPairs.validateColourPair( &val, &config.ui.theme.custom_pallet.foreground, &config.ui.theme.custom_pallet.background );

            } else if( strcmp( CFG_KEY_UI_THEME_ROW, key._raw ) == 0 ) { //colour str pair
                error = !ctune_Parser_KVPairs.validateColourPair( &val, &config.ui.theme.custom_pallet.rows.foreground, &config.ui.theme.custom_pallet.rows.background );

            } else if( strcmp( CFG_KEY_UI_THEME_ROW_SELECTED_FOCUSED, key._raw ) == 0 ) { //colour str pair
                error = !ctune_Parser_KVPairs.validateColourPair( &val, &config.ui.theme.custom_pallet.rows.selected_focused_fg, &config.ui.theme.custom_pallet.rows.selected_focused_bg );

            } else if( strcmp( CFG_KEY_UI_THEME_ROW_SELECTED_UNFOCUSED, key._raw ) == 0 ) { //colour str pair
                error = !ctune_Parser_KVPairs.validateColourPair( &val, &config.ui.theme.custom_pallet.rows.selected_unfocused_fg, &config.ui.theme.custom_pallet.rows.selected_unfocused_bg );

            } else if( strcmp( CFG_KEY_UI_THEME_ROW_FAVOURITE_LOCAL, key._raw ) == 0 ) { //colour str
                error = !ctune_Parser_KVPairs.validateColour( &val, &config.ui.theme.custom_pallet.rows.favourite_local_fg );

            } else if( strcmp( CFG_KEY_UI_THEME_ROW_FAVOURITE_REMOTE, key._raw ) == 0 ) { //colour str
                error = !ctune_Parser_KVPairs.validateColour( &val, &config.ui.theme.custom_pallet.rows.favourite_remote_fg );

            } else if( strcmp( CFG_KEY_UI_THEME_ICON_PLAYBACK_ON, key._raw ) == 0 ) { //colour str
                error = !ctune_Parser_KVPairs.validateColour( &val, &config.ui.theme.custom_pallet.icons.playback_on );

            } else if( strcmp( CFG_KEY_UI_THEME_ICON_PLAYBACK_OFF, key._raw ) == 0 ) { //colour str
                error = !ctune_Parser_KVPairs.validateColour( &val, &config.ui.theme.custom_pallet.icons.playback_off );

            } else if( strcmp( CFG_KEY_UI_THEME_ICON_QUEUED, key._raw ) == 0 ) { //colour str
                error = !ctune_Parser_KVPairs.validateColour( &val, &config.ui.theme.custom_pallet.icons.queued_station );

            } else if( strcmp( CFG_KEY_UI_THEME_FIELD_INVALID, key._raw ) == 0 ) { //colour str
                error = !ctune_Parser_KVPairs.validateColour( &val, &config.ui.theme.custom_pallet.field.invalid_fg );

            } else if( strcmp( CFG_KEY_UI_THEME_BUTTON, key._raw ) == 0 ) { //colour str pair
                error = !ctune_Parser_KVPairs.validateColourPair( &val, &config.ui.theme.custom_pallet.button.foreground, &config.ui.theme.custom_pallet.button.background );

            } else if( strcmp( CFG_KEY_UI_THEME_BUTTON_INVALID, key._raw ) == 0 ) { //colour str
                error = !ctune_Parser_KVPairs.validateColour( &val, &config.ui.theme.custom_pallet.button.invalid_fg );

            } else if( strcmp( CFG_KEY_UI_THEME_BUTTON_VALIDATED, key._raw ) == 0 ) { //colour str
                error = !ctune_Parser_KVPairs.validateColour( &val, &config.ui.theme.custom_pallet.button.validated_fg );

            } else {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_Settings_loadCfg()] Line #%lu: key \"%s\" not recognised.",
                           n, key._raw
                );

                error_state = true;
            }

            if( error ) {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_Settings_loadCfg()] Line #%lu: failed parsing (K=\"%s\", V=\"%s\".",
                           n, key._raw, val._raw
                );

                error_state = true;
            }

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_Settings_loadCfg()] Line #%lu: failed to parse.", n );
            error_state = true;
        }

        ++n;

    } while ( ( ptr = strtok_r( NULL, "\n", &tmp ) ) != NULL );

    config.loaded = true;

    end_fail:
        String.free( &key );
        String.free( &val );
        String.free( &file_path );
        String.free( &file_content );
        return !( error_state );
}

/**
 * Checks if the settings have been loaded
 */
static bool ctune_Settings_isLoaded( void ) {
    return config.loaded;
}

/**
 * Writes the current settings to the cfg file
 * @return Success
 */
static bool ctune_Settings_writeCfg() {
    String_t file_path   = String.init();
    FILE   * file        = NULL;
    bool     error_state = false;

    ctune_XDG.resolveCfgFilePath( config.file_name, &file_path );

    file = fopen( file_path._raw, "w" );

    if( file == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_Settings_writeCfg()] Error opening configuration file (\"%s\").", file_path._raw );
        error_state = true;
        goto end;
    }

    int ret[32];

    ret[ 0] = fprintf( file, "%s=%s\n", CFG_KEY_LAST_STATION_PLAYED_UUID, config.last_station.uuid._raw );
    ret[ 1] = fprintf( file, "%s=%i\n", CFG_KEY_LAST_STATION_PLAYED_SRC, config.last_station.src );
    ret[ 2] = fprintf( file, "%s=%d\n", CFG_KEY_RESUME_VOL, config.resume_volume );
    ret[ 3] = fprintf( file, "%s=%s\n", CFG_KEY_INPUT_LIB, ( String.empty( &config.io_libs.player.name ) ? "" : config.io_libs.player.name._raw ) );
    ret[ 4] = fprintf( file, "%s=%s\n", CFG_KEY_OUTPUT_LIB, ( String.empty( &config.io_libs.sound_server.name ) ? "" : config.io_libs.sound_server.name._raw ) );
    ret[ 5] = fprintf( file, "%s=%s\n", CFG_KEY_OVERWRITE_PLAYLOG, ( config.play_log_overwrite ? "true" : "false" ) );
    ret[ 6] = fprintf( file, "%s=%d\n", CFG_KEY_STREAM_TIMEOUT, config.timeout_stream_val );
    ret[ 7] = fprintf( file, "%s=%d\n", CFG_KEY_NETWORK_TIMEOUT, config.timeout_network_val );

    ret[ 8] = fprintf( file, "%s=%s\n", CFG_KEY_UI_MOUSE, ( config.ui.mouse.enabled ? "true" : "false" ) );
    ret[ 9] = fprintf( file, "%s=%i\n", CFG_KEY_UI_MOUSE_RESOLUTION, config.ui.mouse.resolution );
    ret[10] = fprintf( file, "%s=%i\n", CFG_KEY_UI_MOUSE_RESOLUTION_PRESET, config.ui.mouse.preset );
    ret[11] = fprintf( file, "%s=%s\n", CFG_KEY_UI_UNICODE_ICONS, ( config.ui.unicode_icons ? "true" : "false" ) );
    ret[12] = fprintf( file, "%s=%s\n", CFG_KEY_UI_FAVTAB_SHOW_THEMING, ( config.ui.fav_tab.theme_favourites ? "true" : "false" ) );
    ret[13] = fprintf( file, "%s=%s\n", CFG_KEY_UI_FAVTAB_USE_CUSTOM_THEMING, ( config.ui.fav_tab.custom_theming ? "true" : "false" ) );
    ret[14] = fprintf( file, "%s=%s\n", CFG_KEY_UI_FAVTAB_LRG, ( config.ui.fav_tab.large_rows ? "true" : "false" ) );
    ret[15] = fprintf( file, "%s=%i\n", CFG_KEY_UI_FAVTAB_SORTBY, favourites.sort_id );
    ret[16] = fprintf( file, "%s=%s\n", CFG_KEY_UI_SEARCHTAB_LRG, ( config.ui.search_tab.large_rows ? "true" : "false" ) );
    ret[17] = fprintf( file, "%s=%s\n", CFG_KEY_UI_BROWSERTAB_LRG, ( config.ui.browse_tab.large_rows ? "true" : "false" ) );

    ret[18] = fprintf( file, "%s=%s\n", CFG_KEY_UI_THEME_PRESET, ctune_UIPreset.str( config.ui.theme.preset ) );
    ret[19] = fprintf( file, "%s={%s,%s}\n", CFG_KEY_UI_THEME, ctune_ColourTheme.str( config.ui.theme.custom_pallet.foreground, true ), ctune_ColourTheme.str( config.ui.theme.custom_pallet.background, true ) );
    ret[20] = fprintf( file, "%s={%s,%s}\n", CFG_KEY_UI_THEME_ROW, ctune_ColourTheme.str( config.ui.theme.custom_pallet.rows.foreground, true ), ctune_ColourTheme.str( config.ui.theme.custom_pallet.rows.background, true ) );
    ret[21] = fprintf( file, "%s={%s,%s}\n", CFG_KEY_UI_THEME_ROW_SELECTED_FOCUSED, ctune_ColourTheme.str( config.ui.theme.custom_pallet.rows.selected_focused_fg, true ), ctune_ColourTheme.str( config.ui.theme.custom_pallet.rows.selected_focused_bg, true ) );
    ret[22] = fprintf( file, "%s={%s,%s}\n", CFG_KEY_UI_THEME_ROW_SELECTED_UNFOCUSED, ctune_ColourTheme.str( config.ui.theme.custom_pallet.rows.selected_unfocused_fg, true ), ctune_ColourTheme.str( config.ui.theme.custom_pallet.rows.selected_unfocused_bg, true ) );
    ret[23] = fprintf( file, "%s=%s\n", CFG_KEY_UI_THEME_ROW_FAVOURITE_LOCAL, ctune_ColourTheme.str( config.ui.theme.custom_pallet.rows.favourite_local_fg, true ) );
    ret[24] = fprintf( file, "%s=%s\n", CFG_KEY_UI_THEME_ROW_FAVOURITE_REMOTE, ctune_ColourTheme.str( config.ui.theme.custom_pallet.rows.favourite_remote_fg, true ) );

    ret[25] = fprintf( file, "%s=%s\n", CFG_KEY_UI_THEME_ICON_PLAYBACK_ON, ctune_ColourTheme.str( config.ui.theme.custom_pallet.icons.playback_on, true ) );
    ret[26] = fprintf( file, "%s=%s\n", CFG_KEY_UI_THEME_ICON_PLAYBACK_OFF, ctune_ColourTheme.str( config.ui.theme.custom_pallet.icons.playback_off, true ) );
    ret[27] = fprintf( file, "%s=%s\n", CFG_KEY_UI_THEME_ICON_QUEUED, ctune_ColourTheme.str( config.ui.theme.custom_pallet.icons.queued_station, true ) );

    ret[28] = fprintf( file, "%s=%s\n", CFG_KEY_UI_THEME_FIELD_INVALID, ctune_ColourTheme.str( config.ui.theme.custom_pallet.field.invalid_fg, true ) );

    ret[29] = fprintf( file, "%s={%s,%s}\n", CFG_KEY_UI_THEME_BUTTON, ctune_ColourTheme.str( config.ui.theme.custom_pallet.button.foreground, true ), ctune_ColourTheme.str( config.ui.theme.custom_pallet.button.background, true ) );
    ret[30] = fprintf( file, "%s=%s\n", CFG_KEY_UI_THEME_BUTTON_INVALID, ctune_ColourTheme.str( config.ui.theme.custom_pallet.button.invalid_fg, true ) );
    ret[31] = fprintf( file, "%s=%s\n", CFG_KEY_UI_THEME_BUTTON_VALIDATED, ctune_ColourTheme.str( config.ui.theme.custom_pallet.button.validated_fg, true ) );

    for( size_t item_no = 0; item_no < 16; ++item_no ) {
        if( ret[item_no] < 0 ) {
            CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_Settings_writeCfg()] Error writing to configuration file (\"%s\"): item #%lu", file_path._raw, item_no );
            error_state = true;
        }
    }

    end:
        if( file != NULL )
            fclose( file );

        String.free( &file_path );
        return !( error_state );
}

/**
 * Gets the saved volume
 * @return Volume (0-100)
 */
static int ctune_Settings_getVolume() {
    return config.resume_volume;
}

/**
 * Set the volume to save in configuration
 * @param vol Volume (0-100)
 * @return volume post-change
 */
static int ctune_Settings_setVolume( int vol ) {
    return ( config.resume_volume = ( vol > 100 ? 100 : ( vol < 0 ? 0 : vol ) ) );
}

/**
 * Modifies the stored configuration volume
 * @param delta Delta value to modify by
 * @return Volume post-modification
 */
static int ctune_Settings_modVolume( int delta ) {
    return ctune_Settings_setVolume( config.resume_volume + delta );
}

/**
 * Gets the last station played' UUID
 * @param src Pointer to value to store where the last station source was from
 * @return UUID or NULL if none was saved
 */
static const char * ctune_Settings_getLastPlayedUUID( void ) {
    return config.last_station.uuid._raw;
}

/**
 * Gets hte last station played' source
 * @return ctune_StationSrc_e enum value
 */
static ctune_StationSrc_e ctune_Settings_getLastPlayedSrc( void ) {
    return config.last_station.src;
}

/**
 * Sets the UUID of the last station played to be saved in the configuration
 * @param uuid Radio station UUID to save
 * @param src  Source for Station UUID
 */
static void ctune_Settings_setLastPlayedStation( const char * uuid, ctune_StationSrc_e src ) {
    if( String.set( &config.last_station.uuid, uuid ) )
        config.last_station.src = src;
}

/**
 * Gets the playback log file overwrite preference
 * @return Overwrite flag
 */
static bool ctune_Settings_playbackLogOverwrite( void ) {
    return config.play_log_overwrite;
}

/**
 * Gets the timout value in seconds for connecting to and playing a stream
 * @return Timeout value in seconds
 */
static int ctune_Settings_getStreamTimeoutVal( void ) {
    return config.timeout_stream_val;
}

/**
 * Sets teh timeout value in seconds for connecting to and playing a stream
 * @param val Timeout value in seconds (1-10 inclusive)
 * @return Success
 */
static bool ctune_Settings_setStreamTimeoutVal( int val ) {
    if( 0 < val && val <= 10) {
        CTUNE_LOG( CTUNE_LOG_DEBUG,
                   "[ctune_Settings_setStreamTimeoutVal( %i )] Stream timeout value set: %i -> %i",
                   val, config.timeout_stream_val, val
        );

        config.timeout_stream_val = val;
        return true;
    }

    return false;
}

/**
 * Gets the timout value in seconds for querying a network service
 * @return Timeout value in seconds
 */
static int ctune_Settings_getNetworkTimeoutVal( void ) {
    return config.timeout_network_val;
}

/**
 * Gets the mouse support requirement
 * @return "Enable mouse" state
 */
static bool ctune_Settings_enableMouse( void ) {
    return config.ui.mouse.enabled;
}

/**
 * Gets the UI configuration
 * @return ctune_UIConfig object
 */
static ctune_UIConfig_t ctune_Settings_getUIConfig( void ) {
    return config.ui;
}

/**
 * Sets the UI configuration
 * @return Success
 */
static bool ctune_Settings_setUIConfig( const ctune_UIConfig_t * cfg ) {
    return ctune_UIConfig.copy( cfg, &config.ui );
}

//==================================== CTUNE FAVOURITES ============================================
/**
 * [PRIVATE] Creates a backup of the current favourites
 * @param fav_path Favourites file path
 * @param backup_path Target backup file path
 * @return Success
 */
static bool ctune_Settings_backupFavourites( const char * fav_path, const char * backup_path ) {
    bool error_state = false;

    if( ctune_fs.getFileState( backup_path, NULL ) == CTUNE_FILE_FOUND ) {
        if( remove( backup_path ) != 0 ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_Settings_backupFavourites()] Failed to remove old backup file (\"%s\"): %s",
                       backup_path,
                       strerror( errno )
            );
            error_state = true;
            goto end;
        }
    }

    if( rename( fav_path, backup_path ) != 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Settings_backupFavourites()] Failed to rename favourites into backup file (\"%s\" -> \"%s\"): %s",
                   fav_path,
                   backup_path,
                   strerror( errno )
        );
        error_state = true;
        goto end;
    }

    if( !ctune_fs.duplicateFile( backup_path, fav_path, 1024 ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Settings_backupFavourites()] Failed to duplicate file: \"%s\" -> \"%s\"",
                   backup_path,
                   fav_path
        );

        //revert renaming
        if( rename( backup_path, fav_path ) != 0 ) {
            CTUNE_LOG( CTUNE_LOG_FATAL,
                       "[ctune_Settings_backupFavourites()] Failed to rename backup file back to favourites (\"%s\" -> \"%s\"): %s",
                       backup_path,
                       fav_path,
                       strerror( errno )
            );
        }

        error_state = true;
        goto end;
    }

    end:
        return !( error_state );
}

/**
 * Load favourite stations from file
 * @return Success
 */
static bool ctune_Settings_loadFavourites() {
    for( int i = 0; i < CTUNE_STATIONSRC_COUNT; ++i ) {
        favourites.favs[i] = HashMap.init( ctune_RadioStationInfo.free,
                                           ctune_RadioStationInfo.dup,
                                           ctune_RadioStationInfo.hash,
                                           ctune_RadioStationInfo.sameUUID );
    }

    bool     error_state  = false;
    String_t file_path    = String.init();
    String_t backup_path  = String.init();
    String_t file_content = String.init();
    Vector_t station_list = Vector.init( sizeof( ctune_RadioStationInfo_t ), ctune_RadioStationInfo.freeContent );

    ctune_XDG.resolveCfgFilePath( favourites.file_name, &file_path );
    ctune_XDG.resolveCfgFilePath( favourites.backup_name, &backup_path );

    if( !ctune_fs.readFile( file_path._raw, &file_content ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_Settings_loadFavourites()] Failed to load file content." );
        error_state = true;
        goto end;

    }

    if( !ctune_Settings_backupFavourites( file_path._raw, backup_path._raw ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_Settings_loadFavourites()] Failed create backup." );
    } else {
        CTUNE_LOG( CTUNE_LOG_MSG, "[ctune_Settings_loadFavourites()] Backup successful." );
    }

    if( !ctune_parser_JSON.parseToRadioStationList( &file_content, &station_list ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_Settings_loadFavourites()] Error converting file content to collection of stations (\"%s\").", file_path._raw );
        error_state = true;
        goto end;
    }

    size_t count[CTUNE_STATIONSRC_COUNT];
    size_t tally = 0;

    for( int i = 0; i < CTUNE_STATIONSRC_COUNT; ++i )
        count[i] = 0;

    for( size_t i = 0; i < Vector.size( &station_list ); ++i ) {
        ctune_RadioStationInfo_t * station = (ctune_RadioStationInfo_t *) Vector.at( &station_list, i );
        ctune_RadioStationInfo.set.favourite( station, true );

        const ctune_StationSrc_e src = ctune_RadioStationInfo.get.stationSource( station );

        if( HashMap.add( &favourites.favs[src], ctune_RadioStationInfo.get.stationUUID( station ), station ) ) {
            ++count[src];
            ++tally;
        }
    }

    for( int i = 0; i < CTUNE_STATIONSRC_COUNT; ++i ) {
        CTUNE_LOG( CTUNE_LOG_MSG,
                   "[ctune_Settings_loadFavourites()] Loaded %lu '%s' favourite stations.",
                   count[i], ctune_StationSrc.str( i ) );
    }

    CTUNE_LOG( CTUNE_LOG_MSG,
               "[ctune_Settings_loadFavourites()] Loaded %lu/%lu favourite stations.",
               tally, Vector.size( &station_list ) );

    end:
        String.free( &file_path );
        String.free( &backup_path );
        String.free( &file_content );
        Vector.clear_vector( &station_list );
        return !( error_state );
}

/**
 * Writes the current favourite radio stations to the favourites file
 * @return Success
 */
static bool ctune_Settings_saveFavourites() {
    CTUNE_LOG( CTUNE_LOG_MSG,
               "[ctune_Settings_saveFavourites()] Saving %lu favourite station(s) to file \"%s\".",
               ctune_Settings_favouriteCount(), favourites.file_name
    );

    bool     error_state  = false;
    String_t file_path    = String.init();
    FILE *   file         = NULL;
    String_t json         = String.init();
    Vector_t station_list = Vector.init( sizeof( ctune_RadioStationInfo_t ), ctune_RadioStationInfo.freeContent );

    ctune_XDG.resolveCfgFilePath( favourites.file_name, &file_path );

    file = fopen( file_path._raw , "w" );

    if( !file ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_Settings_saveFavourites()] Error opening file \"%s\": %s", file_path._raw, strerror( errno ) );
        error_state = true;
        goto end;
    }

    for( int i = 0; i < CTUNE_STATIONSRC_COUNT; ++i ) {
        HashMap.export( &favourites.favs[ i ], &station_list, ctune_RadioStationInfo.init, ctune_RadioStationInfo.copy );
    }

    if( !ctune_parser_JSON.parseRadioStationListToJSON( &station_list, &json ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_Settings_saveFavourites()] Failed to parse stations to JSON format." );
        error_state = true;
        goto end;
    }

    if( fprintf( file, "%s", json._raw ) < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_Settings_saveFavourites()] Error writing to file \"%s\": %s", favourites.file_name, strerror( errno ) );
        error_state = true;
        goto end;
    }

    end:
        String.free( &file_path );
        String.free( &json );
        Vector.clear_vector( &station_list );
        fclose( file );
        return !( error_state );
}

/**
 * Syncs a collection used for display the internal map state
 * @param stations View container
 * @return Success
 */
static bool ctune_Settings_refreshFavourites( Vector_t * stations ) {
    if( !Vector.empty( stations ) ) {
        CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_Settings_refreshFavourites( %p )] Vector is not empty - clearing...", stations );

        if( !Vector.reinit( stations ) ) {
            CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_Settings_refreshFavourites( %p )] Failed re-initialisation of vector.", stations );
            return false; //EARLY RETURN
        }
    }

    for( int i = 0; i < CTUNE_STATIONSRC_COUNT; ++i )
        HashMap.export( &favourites.favs[ i ], stations, ctune_RadioStationInfo.init, ctune_RadioStationInfo.copy );

    int (*sort_fn)( const void *, const void * ) = ctune_RadioStationInfo.getComparator( favourites.sort_id );

    if( favourites.sort_id != CTUNE_RADIOSTATIONINFO_SORTBY_NONE && sort_fn != NULL ) {
        CTUNE_LOG( CTUNE_LOG_DEBUG,
                   "[ctune_Settings_refreshFavourites( %p )] Sorting called (%i): %s",
                   stations, favourites.sort_id, ctune_RadioStationInfo.sortAttrStr( favourites.sort_id )
        );

        Vector.sort( stations, sort_fn );
    }

    return true;
}

/**
 * Sets the sorting attribute for the display list
 * @param attr ctune_RadioStationInfo_SortBy_e ID
 */
static void ctune_Settings_setSortingAttribute( ctune_RadioStationInfo_SortBy_e attr ) {
    favourites.sort_id = attr;
}

/**
 * Checks if a station is in the 'favourites' list
 * @param uuid UUID string
 * @param src  Radio station provenance
 * @return Favourite state
 */
static bool ctune_Settings_isFavourite( const char * uuid, ctune_StationSrc_e src ) {
    return ( HashMap.at( &favourites.favs[src], uuid ) != NULL );
}

/**
 * Gets the pointer to a favourite RSI inside the HashMap collection
 * @param uuid UUID of the RSI
 * @param src  Radio station provenance
 * @return Pointer to radio station object
 */
static const ctune_RadioStationInfo_t * ctune_Settings_getFavourite( const char * uuid, ctune_StationSrc_e src ) {
    return HashMap.at( &favourites.favs[src], uuid );
}

/**
 * Adds a new station to the 'favourites' list
 * @param rsi Pointer to a RadioStationInfo_t DTO
 * @param src  Radio station provenance
 * @return Success
 */
static bool ctune_Settings_addStation( const ctune_RadioStationInfo_t * rsi, ctune_StationSrc_e src ) {
    CTUNE_LOG( CTUNE_LOG_MSG,
               "[ctune_Settings_addStation( %p, %i )] "
               "Adding to favourites: <%s>",
               rsi, src, ( rsi != NULL ? ctune_RadioStationInfo.get.stationUUID( rsi ) : "NULL" )
    );

    return HashMap.add( &favourites.favs[src], ctune_RadioStationInfo.get.stationUUID( rsi ), rsi );
}

/**
 * Removes a station from the 'favourites' list
 * @param rsi Pointer to a RadioStationInfo_t DTO
 * @param src  Radio station provenance
 * @return Success
 */
static bool ctune_Settings_removeStation( const ctune_RadioStationInfo_t * rsi, ctune_StationSrc_e src ) {
    CTUNE_LOG( CTUNE_LOG_MSG,
               "[ctune_Settings_removeStation( %p, %i )] "
               "Removing from '%s' favourites: <%s>",
               rsi, src, ctune_StationSrc.str( src ), ( rsi != NULL ? ctune_RadioStationInfo.get.stationUUID( rsi ) : "NULL" )
    );

    return HashMap.remove( &favourites.favs[src], ctune_RadioStationInfo.get.stationUUID( rsi ) );
}

/**
 * Gets the audio server plugin
 * @return Pointer to the loaded plugin
 */
static ctune_AudioOut_t * ctune_Settings_getAudioServer( void ) {
    String_t dir_path = String.init();

    String.append_back( &dir_path, config.io_libs.sys_lib_path );
    String.append_back( &dir_path, config.io_libs.sound_server.dir_name );

    ctune_AudioOut_t * plugin = NULL;

    if( String.empty( &config.io_libs.sound_server.name ) ) {
        String.set( &config.io_libs.sound_server.name, config.io_libs.sound_server.dflt_name );
        plugin = ctune_Plugin.loadAudioOut( dir_path._raw, config.io_libs.sound_server.dflt_name );

    } else {
        plugin = ctune_Plugin.loadAudioOut( dir_path._raw, config.io_libs.sound_server.name._raw );
    }

    if( plugin == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Settings_getAudioServer()] Failed to get audio server plugin '%s' in '%s'",
                   config.io_libs.sound_server.name._raw, dir_path._raw
        );
    }

    String.free( &dir_path );
    return plugin;
}

/**
 * Gets the player plugin
 * @return Pointer to the loaded plugin
 */
static ctune_Player_t * ctune_Settings_getPlayer( void ) {
    String_t dir_path = String.init();

    String.append_back( &dir_path, config.io_libs.sys_lib_path );
    String.append_back( &dir_path, config.io_libs.player.dir_name );

    ctune_Player_t * plugin = NULL;

    if( String.empty( &config.io_libs.player.name ) ) {
        String.set( &config.io_libs.player.name, config.io_libs.player.dflt_name );
        plugin = ctune_Plugin.loadPlayer( dir_path._raw, config.io_libs.player.dflt_name );

    } else {
        plugin = ctune_Plugin.loadPlayer( dir_path._raw, config.io_libs.player.name._raw );
    }

    if( plugin == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Settings_getPlayer()] Failed to get player plugin '%s' in '%s'",
                   config.io_libs.player.name._raw, dir_path._raw
        );
    }

    String.free( &dir_path );
    return plugin;
}

/**
 * De-allocates anything stored on the heap
 */
static void ctune_Settings_free() {
    ctune_XDG.free();
    String.free( &config.io_libs.player.name );
    String.free( &config.io_libs.sound_server.name );
    String.free( &config.last_station.uuid );

    ctune_Plugin.free();

    for( int i = 0; i < CTUNE_STATIONSRC_COUNT; ++i ) {
        HashMap.clear( &favourites.favs[ i ] );
    }

    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_Settings_free()] Settings freed." );
}


/**
 * Namespace constructor
 */
const struct ctune_Settings_Instance ctune_Settings = {
    .init = &ctune_Settings_init,

    .rtlock = {
        .lock                 = &ctune_Settings_rtlock_lock,
        .unlock               = &ctune_Settings_rtlock_unlock,
    },

    .favs = {
        .loadFavourites       = &ctune_Settings_loadFavourites,
        .saveFavourites       = &ctune_Settings_saveFavourites,
        .isFavourite          = &ctune_Settings_isFavourite,
        .getFavourite         = &ctune_Settings_getFavourite,
        .refreshView          = &ctune_Settings_refreshFavourites,
        .setSortingAttribute  = &ctune_Settings_setSortingAttribute,
        .addStation           = &ctune_Settings_addStation,
        .removeStation        = &ctune_Settings_removeStation
    },

    .cfg = {
        .loadCfg                = &ctune_Settings_loadCfg,
        .isLoaded               = &ctune_Settings_isLoaded,
        .writeCfg               = &ctune_Settings_writeCfg,
        .getVolume              = &ctune_Settings_getVolume,
        .setVolume              = &ctune_Settings_setVolume,
        .modVolume              = &ctune_Settings_modVolume,
        .getLastPlayedUUID      = &ctune_Settings_getLastPlayedUUID,
        .getLastPlayedSrc       = &ctune_Settings_getLastPlayedSrc,
        .setLastPlayedStation   = &ctune_Settings_setLastPlayedStation,
        .playbackLogOverwrite   = &ctune_Settings_playbackLogOverwrite,
        .getStreamTimeoutVal    = &ctune_Settings_getStreamTimeoutVal,
        .setStreamTimeoutVal    = &ctune_Settings_setStreamTimeoutVal,
        .getNetworkTimeoutVal   = &ctune_Settings_getNetworkTimeoutVal,
        .enableMouse            = &ctune_Settings_enableMouse,
        .getUIConfig            = &ctune_Settings_getUIConfig,
        .setUIConfig            = &ctune_Settings_setUIConfig,
    },

    .plugins = {
        .getAudioServer = &ctune_Settings_getAudioServer,
        .getPlayer      = &ctune_Settings_getPlayer,
    },

    .free = &ctune_Settings_free,
};