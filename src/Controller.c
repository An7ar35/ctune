#include "Controller.h"

#include <pthread.h>

#include "logger/src/Logger.h"
#include "ctune_err.h"
#include "fs/Settings.h"
#include "fs/PlaybackLog.h"
#include "player/RadioPlayer.h"
#include "network/RadioBrowser.h"
#include "network/NetworkUtils.h"

/**
 * Controller variables
 */
static struct {
    ctune_UIConfig_t     ui_config;
    ctune_ServerList_t   radio_browser_servers;

    struct { /* CALLBACKS METHODS */
        void (* station_change_cb)( const ctune_RadioStationInfo_t * );
        void (* song_change_cb)( const char * );
        void (* volume_change_cb)( int );
        void (* search_state_change_cb)( bool state );
        void (* resize_cb)( void );

    } cb;

} controller = {
    .cb = {
        .station_change_cb        = NULL,
        .song_change_cb           = NULL,
        .volume_change_cb         = NULL,
        .search_state_change_cb   = NULL,
    },
};

/**
 * Local cache
 */
struct {
    String_t last_played_song;

} cache = {
    .last_played_song = { ._raw = NULL, ._length = 0 },
};

/* ============================================ PRIVATE ========================================= */

/**
 * [PRIVATE] Signals a song change event to the controller
 * @param str Song description string
 */
static void ctune_Controller_songChangeEvent( const char * str ) {
    if( !String.empty( &cache.last_played_song ) &&
        strcmp( str, cache.last_played_song._raw ) == 0 )
    { //avoid duplicates - sometimes station signal metadata change that's not related to the song description
        return; //EARLY RETURN
    }

    String.set( &cache.last_played_song, str );

    if( controller.cb.song_change_cb != NULL )
        controller.cb.song_change_cb( str );

    String_t line = String.init();

    if( !ctune_timestampLocal( &line, "%x - %X    " ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Controller_songChangeEvent( \"%s\" )] Failed to create timestamp.",
                   str
        );
    }

    String.append_back( &line, str );

    ctune_PlaybackLog.writeln( line._raw );

    String.free( &line );
}

/**
 * [PRIVATE] Signals a volume change event to the controller
 * @param vol Volume (0-100)
 */
static void ctune_Controller_volumeChangeEvent( int vol ) {
    if( controller.cb.volume_change_cb != NULL ) {
        controller.cb.volume_change_cb( vol );
    }

    ctune_Settings.cfg.setVolume( vol );
}

/* ============================================ PUBLIC ========================================== */

/**
 * Initiate cTune
 * @return Success
 */
static bool ctune_Controller_init( void ) {
    CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_Controller_init()] cTune initialising..." );
    controller.radio_browser_servers = ctune_ServerList.init();
    controller.ui_config             = ctune_Settings.cfg.getUIConfig();

    if( !ctune_Settings.cfg.isLoaded() ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_Controller_init()] Looks like Settings has not loaded a config file yet." );
    }

    ctune_RadioPlayer.init( ctune_Controller_songChangeEvent,
                            ctune_Controller_volumeChangeEvent );

    if( !ctune_RadioPlayer.loadSoundServerPlugin( ctune_Settings.plugins.getPlugin( CTUNE_PLUGIN_OUT_AUDIO_SERVER ) ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_Controller_init()] Failed to load a sound server plugin." );
        return false; //EARLY RETURN
    };

    if( !ctune_RadioPlayer.loadPlayerPlugin( ctune_Settings.plugins.getPlugin( CTUNE_PLUGIN_IN_STREAM_PLAYER ) ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_Controller_init()] Failed to load a player plugin." );
        return false; //EARLY RETURN
    };

    if( ctune_Settings.plugins.getPlugin( CTUNE_PLUGIN_OUT_AUDIO_RECORDER ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_WARNING, "[ctune_Controller_init()] Failed to load a recorder plugin." );
    }

    return true;
}

/**
 * Loads controller level options and act on these
 * @param opts Options
 */
static void ctune_Controller_load( ctune_ArgOptions_t * opts ) {
    if( opts == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_Controller_load( %p )] Options argument is NULL.", opts );
        return; //EARLY RETURN
    }

    const ctune_RadioStationInfo_t * station      = NULL;
    Vector_t                         station_list = Vector.init( sizeof( ctune_RadioStationInfo_t ), ctune_RadioStationInfo.freeContent );

    if( !String.empty( &opts->playback.init_station_uuid ) ) {
        if( ctune_Controller.search.getStationsBy( RADIOBROWSER_STATION_BY_UUID, opts->playback.init_station_uuid._raw, &station_list ) ) {
            station = ( const ctune_RadioStationInfo_t * ) Vector.at( &station_list, 0 );

            if( ctune_Controller.playback.start( station ) ) {
                CTUNE_LOG( CTUNE_LOG_MSG,
                           "[ctune_Controller_load( %p )] "
                           "Playing 'init' station passed by the options (UUID: %s).",
                           opts, opts->playback.init_station_uuid._raw
                );

                const bool is_favourite = ctune_Settings.favs.isFavourite( ctune_RadioStationInfo.get.stationUUID( station ), CTUNE_STATIONSRC_RADIOBROWSER );

                if( opts->playback.favourite_init && !is_favourite ) {
                    ctune_Settings.favs.addStation( station, CTUNE_STATIONSRC_RADIOBROWSER );
                }

                goto end; //(happy path)
            }

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_Controller_load( %p )] "
                       "Error downloading 'init' station passed by the options.",
                       opts
            );
        }

    } //failed so fallthrough to next options

    if( opts->playback.resume_playback ) {
        if( !Vector.reinit( &station_list ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_Controller_load( %p )] Failed to re-initialise vector.",
                       opts
            );

            goto end;
        }
        ctune_StationSrc_e source = ctune_Settings.cfg.getLastPlayedSrc();

        switch( source ) {
            case CTUNE_STATIONSRC_LOCAL: {
                station = ctune_Settings.favs.getFavourite( ctune_Settings.cfg.getLastPlayedUUID(), source );

                if( station != NULL ) {
                    if( ctune_Controller.playback.start( station ) ) {
                        CTUNE_LOG( CTUNE_LOG_MSG,
                                   "[ctune_Controller_load( %p )] "
                                   "Resuming playback of '%s' station from last session (UUID: %s).",
                                   opts, ctune_StationSrc.str( source ), ctune_Settings.cfg.getLastPlayedUUID()
                        );

                        goto end; //(happy path)
                    }

                } else {
                    CTUNE_LOG( CTUNE_LOG_ERROR,
                               "[ctune_Controller_load( %p )] Error fetching '%s' station saved in configuration.",
                               opts, ctune_StationSrc.str( source )
                    );
                }

            } break;

            case CTUNE_STATIONSRC_RADIOBROWSER: {
                if( ctune_Controller.search.getStationsBy( RADIOBROWSER_STATION_BY_UUID, ctune_Settings.cfg.getLastPlayedUUID(), &station_list ) ) {
                    station = ( const ctune_RadioStationInfo_t * ) Vector.at( &station_list, 0 );

                    if( ctune_Controller.playback.start( station ) ) {
                        CTUNE_LOG( CTUNE_LOG_MSG,
                                   "[ctune_Controller_load( %p )] "
                                   "Resuming playback of '%s' station from last session (UUID: %s).",
                                   opts, ctune_StationSrc.str( source ), ctune_Settings.cfg.getLastPlayedUUID()
                        );

                        goto end; //(happy path)
                    }

                } else {
                    CTUNE_LOG( CTUNE_LOG_ERROR,
                               "[ctune_Controller_load( %p )] Error downloading '%s' station info.",
                               opts, ctune_StationSrc.str( source )
                    );
                }
            } break;

            default: {
                CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_Controller_load( %p )] Station source not implemented!", opts );
            } break;
        }
    }

    end:
        Vector.clear_vector( &station_list );
}

/**
 * Call the resize callback method if set
 */
static void ctune_Controller_resizeUI( void ) {
    if( controller.cb.resize_cb ) {
        controller.cb.resize_cb();
    }
}

/**
 * Shutdown and cleanup cTune
 */
static void ctune_Controller_free() {
    ctune_Controller.cfg.saveUIConfig();
    ctune_Controller.cfg.saveFavourites();
    ctune_ServerList.freeServerList( &controller.radio_browser_servers );
    String.free( &cache.last_played_song );
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_Controller_free()] Controller freed." );
}

/**
 * Stops the playback of the currently playing stream
 */
static void ctune_Controller_playback_stopPlayback() {
    ctune_RadioPlayer.stopPlayback();
};

/**
 * Gets the playback state variable's value
 * @return Playback state value
 */
static bool ctune_Controller_playback_getPlaybackState() {
    return ctune_RadioPlayer.getPlaybackState();
}

/**
 * Starts playback of a radio station
 * @param station Pointer to a RadioStationInfo DTO
 * @return Success
 */
static bool ctune_Controller_playback_startPlayback( const ctune_RadioStationInfo_t * station ) {
    if( station == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_Controller_startPlayback( %p )] Station is NULL.", station );
        return false; //EARLY RETURN
    }

    if( ctune_RadioStationInfo.get.resolvedURL( station ) == NULL && ctune_RadioStationInfo.get.stationURL( station ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_Controller_startPlayback( %p )] Station URL fields are NULL.", station );
        return false; //EARLY RETURN
    }

    if( ctune_RadioPlayer.getPlaybackState() ) {
        ctune_RadioPlayer.stopPlayback();
    }

    { //signal the radio station change to callback and the playback log
        String.free( &cache.last_played_song );

        if( controller.cb.station_change_cb != NULL ) {
            controller.cb.station_change_cb( station );
        }

        ctune_PlaybackLog.writeln( "" );
        ctune_RadioStationInfo.printLite( station, ctune_PlaybackLog.output() );
        ctune_PlaybackLog.writeln( "\n" );
    }

    const char * url = ( ctune_RadioStationInfo.get.resolvedURL( station ) == NULL
                       ? ctune_RadioStationInfo.get.stationURL( station )
                       : ctune_RadioStationInfo.get.resolvedURL( station ) );

    if( !ctune_RadioPlayer.playRadioStream( url, ctune_Settings.cfg.getVolume(), ctune_Settings.cfg.getStreamTimeoutVal() ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_startPlayback( %p )] Failed to start playback." );
    }

    ctune_Settings.cfg.setLastPlayedStation( ctune_RadioStationInfo.get.stationUUID( station ), ctune_RadioStationInfo.get.stationSource( station ) );

    if( ctune_RadioStationInfo.get.stationSource( station ) == CTUNE_STATIONSRC_RADIOBROWSER ) {
        //sends RadioBrowser station click request as per the RadioBrowser API documentation
        ctune_ClickCounter_t click_counter;
        ctune_ClickCounter.init( &click_counter );
        bool ret = ctune_RadioBrowser.stationClickCounter( &controller.radio_browser_servers,
                                                           ctune_Settings.cfg.getNetworkTimeoutVal(),
                                                           ctune_RadioStationInfo.get.stationUUID( station ),
                                                           &click_counter );

        if( ret ) {
            CTUNE_LOG( CTUNE_LOG_MSG,
                       "[ctune_Controller_startPlayback( %p )] Click counter: %s (msg: \"%s\")",
                       station, ( click_counter.ok ? "OK" : "ERR" ), click_counter.message
            );
        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_Controller_startPlayback( %p )] Click counter request failed.",
                       station
            );
        }

        ctune_ClickCounter.freeContent( &click_counter );
    }

    return true;
}

/**
 * Search for all stations matching the criteria in filter
 * @param filter   Filter
 * @param stations RadioStationInfo container
 * @return Success
 */
static bool ctune_Controller_search_getStations( ctune_RadioBrowserFilter_t * filter, Vector_t * stations ) {
    bool ret = ctune_RadioBrowser.downloadStations( &controller.radio_browser_servers,
                                                    ctune_Settings.cfg.getNetworkTimeoutVal(),
                                                    filter,
                                                    stations );

    if( !ret ) {
        if( ctune_err.number() == CTUNE_ERR_NONE )
            ctune_err.set( CTUNE_ERR_ACTION_FETCH );

        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Controller_search_getStations( %p, %p )] Error downloading radio stations.",
                   filter, stations );
    } else {
        CTUNE_LOG( CTUNE_LOG_DEBUG,
                   "[ctune_Controller_search_getStations( %p, %p )] Downloaded %lu radio stations.",
                   filter, stations, Vector.size( stations ) );
    }

    return ( ret && !Vector.empty( stations ) );
}

/**
 * Search for all stations matching the criteria
 * @param category    Category
 * @param search_term Search term (optional)
 * @param stations    Container for stations
 * @return Success
 */
static bool ctune_Controller_search_getStationsBy( const ctune_ByCategory_e category, const char * search_term, Vector_t * stations ) {
    bool ret = ctune_RadioBrowser.downloadStationsBy( &controller.radio_browser_servers,
                                                      ctune_Settings.cfg.getNetworkTimeoutVal(),
                                                      category,
                                                      search_term,
                                                      stations );
    if( !ret ) {
        if( ctune_err.number() == CTUNE_ERR_NONE )
            ctune_err.set( CTUNE_ERR_ACTION_FETCH );

        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Controller_search_getStationsBy( %i, \"%s\", %p )] Error downloading radio stations.",
                   category, ( search_term ? search_term : "" ), stations );
    } else {
        CTUNE_LOG( CTUNE_LOG_TRACE,
                   "[ctune_Controller_search_getStationsBy( %i, \"%s\", %p )] Downloaded %lu radio stations.",
                   category, ( search_term ? search_term : "" ), stations, Vector.size( stations ) );
    }

    return ret;
}

/**
 * Download items within a specified category
 * @param category       Category for which to download items from
 * @param filter         Search filter
 * @param category_items Data-structure to store the category items (`ctune_CategoryItem_t`) into
 */
bool ctune_Controller_search_getCategoryItems( const ctune_ListCategory_e category, const ctune_RadioBrowserFilter_t * filter, Vector_t * categories ) {
    bool ret = ctune_RadioBrowser.downloadCategoryItems( &controller.radio_browser_servers,
                                                         ctune_Settings.cfg.getNetworkTimeoutVal(),
                                                         category,
                                                         filter,
                                                         categories );

    if( !ret ) {
        if( ctune_err.number() == CTUNE_ERR_NONE )
            ctune_err.set( CTUNE_ERR_ACTION_FETCH );

        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Controller_search_getCategoryItems( %i, %p, %p )] Error downloading category items.",
                   category, filter, categories );
    } else {
        CTUNE_LOG( CTUNE_LOG_TRACE,
                   "[ctune_Controller_search_getCategoryItems( %i, %p, %p )] Downloaded %lu items for category %i.",
                   category, filter, categories, Vector.size( categories ), category );
    }

    return ret;
}

/**
 * [THREAD SAFE] Changes playback volume variable
 * @param delta Volume change (+/-)
 */
static void ctune_Controller_playback_modifyVolume( int delta ) {
    ctune_RadioPlayer.modifyVolume( delta );
}

/**
 * Test a stream and get properties
 * @param url     Stream URL
 * @param codec   Pointer to store codec string value into
 * @param bitrate Pointer to store bitrate value into
 * @return Success
 */
static bool ctune_Controller_playback_testStream( const char * url, String_t * codec, ulong * bitrate ) {
    return ctune_RadioPlayer.testStream( url, ctune_Settings.cfg.getStreamTimeoutVal(), codec, bitrate );
}

/**
 * Tests the validity of a URL string
 * @param url URL string
 * @return Valid state
 */
static bool ctune_Controller_playback_validateURL( const char * url ) {
    return ctune_NetworkUtils.validateURL( url );
}

/**
 * Starts recording
 * @param station
 * @return Success
 */
static bool ctune_Controller_recording_start( void ) {
    bool              error_state = false;
    String_t          path        = String.init();
    ctune_FileOut_t * plugin      = ctune_Settings.plugins.getPlugin( CTUNE_PLUGIN_OUT_AUDIO_RECORDER );

    if( plugin == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Controller_recording_start()] Failed to get a file output plugin to use for recording."
        );

        error_state = true;
        goto end;
    }

    String.append_back( &path, ctune_Settings.cfg.outputPath() );

    if( !ctune_timestampLocal( &path, "%F-%H%M%S" ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Controller_recording_start()] Failed to create timestamp."
        );

        ctune_err.set( CTUNE_ERR_FUNC_FAIL );
        error_state = true;
        goto end;
    }

    String.append_back( &path, "." );
    String.append_back( &path, plugin->extension() );

    if( !ctune_RadioPlayer.startRecording( path._raw, plugin ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Controller_recording_start()] Failed to start recording (path=\"%s\").",
                   path._raw
        );
    }

    end:
        String.free( &path );
        return !( error_state );
}

/**
 * Stops recording
 */
static void ctune_Controller_recording_stop( void ) {
    return ctune_RadioPlayer.stopRecording();
}

/**
 * Check if stream is being recorded
 * @return Live recording state
 */
static bool ctune_Controller_recording_isRecording( void ) {
    return ctune_RadioPlayer.getRecordingState();
}

/**
 * Sets the output path directory for recordings
 * @param path Directory path
 * @return Success
 */
static bool ctune_Controller_recording_setPath( const char * path ) {
    return ctune_Settings.cfg.setOutputPath( path );
}

/**
 * Gets the current recording output directory path
 * @return Directory path string
 */
static const char * ctune_Controller_recording_path( void ) {
    return ctune_Settings.cfg.outputPath();
}

/**
 * Add/Remove a station to the list of favourites
 * @param rsi RadioStationInfo_t object
 * @param src  Radio station provenance
 * @return Success
 */
static bool ctune_Controller_cfg_toggleFavourite( ctune_RadioStationInfo_t * rsi, ctune_StationSrc_e src ) {
    bool success = false;

    if( ctune_Settings.favs.isFavourite( ctune_RadioStationInfo.get.stationUUID( rsi ), src ) )
        success = ctune_Settings.favs.removeStation( rsi, src );
    else
        success = ctune_Settings.favs.addStation( rsi, src );

    if( !success ) {
        ctune_err.set( CTUNE_ERR_ACTION_FAV_TOGGLE );
        return false; //EARLY RETURN
    }

    ctune_RadioStationInfo.set.favourite( rsi, !( ctune_RadioStationInfo.get.favourite( rsi ) ) );
    return true;
}

/**
 * Updates a favourite station's information
 * @param rsi RadioStationInfo_t object
 * @param src Radio station origin
 * @return Success
 */
bool ctune_Controller_cfg_updateFavourite( ctune_RadioStationInfo_t * rsi, ctune_StationSrc_e src ) {
    if( ctune_Settings.favs.isFavourite( ctune_RadioStationInfo.get.stationUUID( rsi ), src ) ) {
        bool ret1 = ctune_Settings.favs.removeStation( rsi, src );
        bool ret2 = ctune_Settings.favs.addStation( rsi, src );

        if( ret1 && ret2 ) {
            return true; //EARLY RETURN (happy path)

        } else {
            ctune_err.set( CTUNE_ERR_ACTION_FAV_UPDATE );
            CTUNE_LOG( CTUNE_LOG_FATAL,
                       "[ctune_Controller_updateFavourite( %p, %i )] Failed update (rm=%s, add=%s).",
                       rsi, src, ( ret1 ? "ok" : "fail" ), ( ret2 ? "ok" : "fail" ) );
        }

    } else {
        ctune_err.set( CTUNE_ERR_ACTION_FAV_UPDATE );
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_Controller_updateFavourite( %p, %i )] Station could not be found in '%s' favourites.",
                   rsi, src, ctune_StationSrc.str( src )
        );
    }

    return false;
}

/**
 * Check if a station is in the list of favourites
 * @param rsi RadioStationInfo_t object
 * @param src  Radio station provenance
 * @return Favourite state
 */
static bool ctune_Controller_cfg_isFavourite( const ctune_RadioStationInfo_t * rsi, ctune_StationSrc_e src ) {
    return ctune_Settings.favs.isFavourite( ctune_RadioStationInfo.get.stationUUID( rsi ), src );
}

/**
 * Checks if a UUID is used by the favourite stations
 * @param uuid UUID string
 * @param src  Radio station UUID origin
 * @return In-use state
 */
static bool ctune_Controller_cfg_isFavouriteUUID( const char * uuid, ctune_StationSrc_e src ) {
    return ctune_Settings.favs.isFavourite( uuid, src );
}

/**
 * Saves the favourite stations to file
 * @return Success
 */
static bool ctune_Controller_cfg_saveFavourites() {
    return ctune_Settings.favs.saveFavourites();
}

/**
 * Update the collection of stations so that it contains only the current favourites
 * @param stations Vector of stations
 */
static bool ctune_Controller_cfg_updateFavourites( Vector_t * stations ) {
    return ctune_Settings.favs.refreshView( stations );
}

/**
 * Sets the sorting attribute for the list of favourite stations
 * @param attr ctune_RadioStationInfo_SortBy_e ID
 */
static void ctune_Controller_cfg_setFavouriteSorting( ctune_RadioStationInfo_SortBy_e attr ) {
    ctune_Settings.favs.setSortingAttribute( attr );
}

/**
 * Gets the loaded UI theme
 * @return Colour theme
 */
struct ctune_ColourTheme * ctune_Controller_cfg_getUiTheme( void ) {
    struct ctune_ColourTheme * pallet = ctune_UIConfig.theming.getCurrentThemePallet( &controller.ui_config );

    if( pallet == NULL ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_Controller_getUiTheme()] "
                   "Failed to get a pointer to the active UI colour pallet."
        );
    }

    return pallet;
}

/**
 * Gets a pointer to the internal UIConfig object
 * @return Pointer to ctune_UIConfig_t object
 */
static ctune_UIConfig_t * ctune_Controller_cfg_getUIConfig( void ) {
    return &controller.ui_config;
}

/**
 * Saves the internal UIConfig_t object to the Settings component
 */
static void ctune_Controller_cfg_saveUIConfig( void ) {
    ctune_Settings.cfg.setUIConfig( &controller.ui_config );
}

/**
 * Gets the timeout value in seconds for connecting to and playing a stream
 * @return Timeout value in seconds
 */
static int ctune_Controller_cfg_getStreamTimeout( void ) {
    return ctune_Settings.cfg.getStreamTimeoutVal();
}

/**
 * Sets the timeout value in seconds for connecting to and playing a stream
 * @param val Timeout value in seconds (1-10 inclusive)
 * @return Success
 */
static bool ctune_Controller_cfg_setStreamTimeout( int seconds ) {
    return ctune_Settings.cfg.setStreamTimeoutVal( seconds );
}

/**
 * Changes the selected plugin of a given type
 * @param type Plugin type enum
 * @param id   Plugin ID
 * @return Success
 */
static bool ctune_Controller_plugin_changePlugin( ctune_PluginType_e type, size_t id ) {
    switch( type ) {
        case CTUNE_PLUGIN_IN_STREAM_PLAYER: //fallthrough
        case CTUNE_PLUGIN_OUT_AUDIO_SERVER: {
            if( ctune_RadioPlayer.getPlaybackState() ) {
                ctune_RadioPlayer.stopPlayback();
            }
        } break;

        case CTUNE_PLUGIN_OUT_AUDIO_RECORDER: {
            if( ctune_RadioPlayer.getRecordingState() ) {
                ctune_RadioPlayer.stopRecording();
            }
        } break;

        default: break;
    }

    if( ctune_Settings.plugins.setPlugin( type, id ) ) {
        CTUNE_LOG( CTUNE_LOG_MSG,
                   "[ctune_Controller_plugin_changePlugin( '%s', %lu )] Selected plugin changed.",
                   ctune_PluginType.str( type ), id
        );

        return true;

    } else {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Controller_plugin_changePlugin( '%s', %lu )] Failed to change selected plugin.",
                   ctune_PluginType.str( type ), id
        );

        return false;
    }
}

/**
 * Gets a list of all the loaded plugins of a specified type
 * @param type Plugin type enum
 * @return Pointer to a heap allocated list of ids, names, descriptions and 'selected' flags
 */
static const Vector_t * ctune_Controller_plugin_getPluginList( ctune_PluginType_e type ) {
    return ctune_Settings.plugins.getPluginList( type );
}

/**
 * Sets a callback for resize events
 * @param cb Callback method
 */
static void ctune_Controller_setResizeUIEventCallback( void(* cb)( void ) ) {
    controller.cb.resize_cb = cb;
}

/**
 * [OPTIONAL] Sets a callback for the volume change event
 * @param cb Callback method
 */
static void ctune_Controller_setVolumeChangeEventCallback( void(* cb)( int ) ) {
    controller.cb.volume_change_cb = cb;
}

/**
 * [OPTIONAL] Sets a callback for the song change event
 * @param cb Callback method
 */
static void ctune_Controller_setSongChangeEventCallback( void(* cb)( const char *) ) {
    controller.cb.song_change_cb = cb;
}

/**
 * [OPTIONAL] Sets the callback for the radio station change event
 * @param cb Callback method
 */
static void ctune_Controller_setStationChangeEventCallback( void(* cb)( const ctune_RadioStationInfo_t * ) ) {
    controller.cb.station_change_cb = cb;
}


/**
 * [OPTIONAL] Assigns a function as the playback state change callback
 * @param cb Callback function pointer
 */
static void ctune_Controller_setPlaybackStateChangeEvent_cb( void(* cb)( ctune_PlaybackCtrl_e ) ) {
    ctune_RadioPlayer.setStateChangeCallback( cb );
}

/**
 * [OPTIONAL] Assigns a function as the search state change callback
 * @param cb Callback function pointer
 */
static void ctune_Controller_setSearchStateChangeEvent_cb( void(* cb)( bool ) ) {
    controller.cb.search_state_change_cb = cb;
}


/**
 * Constructor
 */
const struct ctune_Controller_Instance ctune_Controller = {
    .init     = &ctune_Controller_init,
    .load     = &ctune_Controller_load,
    .resizeUI = &ctune_Controller_resizeUI,
    .free     = &ctune_Controller_free,

    .playback = {
        .getPlaybackState    = &ctune_Controller_playback_getPlaybackState,
        .start               = &ctune_Controller_playback_startPlayback,
        .stop                = &ctune_Controller_playback_stopPlayback,
        .modifyVolume        = &ctune_Controller_playback_modifyVolume,
        .testStream          = &ctune_Controller_playback_testStream,
        .validateURL         = &ctune_Controller_playback_validateURL,
    },

    .recording = {
        .start               = &ctune_Controller_recording_start,
        .stop                = &ctune_Controller_recording_stop,
        .isRecording         = &ctune_Controller_recording_isRecording,
        .setPath             = &ctune_Controller_recording_setPath,
        .path                = &ctune_Controller_recording_path,
    },

    .search = {
      .getStations           = &ctune_Controller_search_getStations,
      .getStationsBy         = &ctune_Controller_search_getStationsBy,
      .getCategoryItems      = &ctune_Controller_search_getCategoryItems,
    },

    .cfg = {
        .toggleFavourite     = &ctune_Controller_cfg_toggleFavourite,
        .updateFavourite     = &ctune_Controller_cfg_updateFavourite,
        .isFavourite         = &ctune_Controller_cfg_isFavourite,
        .isFavouriteUUID     = &ctune_Controller_cfg_isFavouriteUUID,
        .saveFavourites      = &ctune_Controller_cfg_saveFavourites,
        .getListOfFavourites = &ctune_Controller_cfg_updateFavourites,
        .setFavouriteSorting = &ctune_Controller_cfg_setFavouriteSorting,
        .getUiTheme          = &ctune_Controller_cfg_getUiTheme,
        .getUIConfig         = &ctune_Controller_cfg_getUIConfig,
        .saveUIConfig        = &ctune_Controller_cfg_saveUIConfig,
        .getStreamTimeout    = &ctune_Controller_cfg_getStreamTimeout,
        .setStreamTimeout    = &ctune_Controller_cfg_setStreamTimeout,
    },

    .plugins = {
        .changePlugin        = &ctune_Controller_plugin_changePlugin,
        .getPluginList       = &ctune_Controller_plugin_getPluginList,
    },

    .setResizeUIEventCallback            = &ctune_Controller_setResizeUIEventCallback,
    .setVolumeChangeEventCallback        = &ctune_Controller_setVolumeChangeEventCallback,
    .setSongChangeEventCallback          = &ctune_Controller_setSongChangeEventCallback,
    .setStationChangeEventCallback       = &ctune_Controller_setStationChangeEventCallback,
    .setPlaybackStateChangeEventCallback = &ctune_Controller_setPlaybackStateChangeEvent_cb,
    .setSearchStateChangeEventCallback   = &ctune_Controller_setSearchStateChangeEvent_cb,
};