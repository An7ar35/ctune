#include "UI.h"

#include <panel.h>
#include <sys/ioctl.h>

#include "../ctune_err.h"
#include "logger/src/Logger.h"
#include "../datastructure/Vector.h"
#include "../datastructure/CircularBuffer.h"
#include "../datastructure/StrList.h"
#include "../dto/RadioBrowserFilter.h"
#include "../enum/PlaybackCtrl.h"
#include "../network/RadioBrowser.h"
#include "../Controller.h"

#include "EventQueue.h"
#include "Resizer.h"
#include "definitions/KeyBinding.h"
#include "definitions/Theme.h"
#include "definitions/Icons.h"
#include "dialog/ContextHelp.h"
#include "dialog/RSFind.h"
#include "dialog/RSInfo.h"
#include "dialog/RSEdit.h"
#include "dialog/OptionsMenu.h"
#include "dialog/SetOutputDir.h"
#include "window/RSListWin.h"
#include "window/MainWin.h"

/* Smallest supported terminal screen size */
#define UI_MIN_COLS 40 //(as reference, default standard terminal width is 110)
#define UI_MIN_ROWS 10 //(as reference, default standard terminal height is 28)

/**
 * Initialisation stages (to help with cleanup on fail)
 */
typedef enum ctune_UI_InitStages {
    CTUNE_UI_INITSTAGE_KEYBINDS = 0,
    CTUNE_UI_INITSTAGE_STDSCR,
    CTUNE_UI_INITSTAGE_THEME,
    CTUNE_UI_INITSTAGE_MAINWIN,
    CTUNE_UI_INITSTAGE_EVENTQUEUE,
    CTUNE_UI_INITSTAGE_CTXHELP,
    CTUNE_UI_INITSTAGE_RSFIND,
    CTUNE_UI_INITSTAGE_RSEDIT,
    CTUNE_UI_INITSTAGE_RSINFO,
    CTUNE_UI_INITSTAGE_OPTMENU,
    CTUNE_UI_INITSTAGE_SETOUTDIR,
    CTUNE_UI_INITSTAGE_COMPLETE,

    CTUNE_UI_INITSTAGE_COUNT
} UIInitStages_e;

static const int ACTION_CANCELED  = 0b00;
static const int ACTION_REQUEST   = 0b01;
static const int ACTION_CONFIRMED = 0b10;

/**
 * Internal UI variables
 */
static struct ctune_UI {
    WindowProperty_t     screen_size;
    ctune_UI_MainWin_t   main_win;

    bool                 init_stages  [CTUNE_UI_INITSTAGE_COUNT];
    int                  old_cursor;
    MEVENT               mouse_event;

    struct { //each dialog object type is recycled after the 1st use
        ctune_UI_RSInfo_t       rsinfo;
        ctune_UI_RSFind_t       rsfind;
        ctune_UI_RSEdit_t       rsedit;
        ctune_UI_OptionsMenu_t  optmenu;
        ctune_UI_SetOutputDir_t setrecdir;
    } dialogs;

    struct {
        int(* quietVolChangeCallback)( int );
    } cb;

} ui = {
    .screen_size   = { 0, 0, 0, 0 },
    .init_stages   = { false },
};

/* ============================================================================================== */
/* ========================================== PRIVATE =========================================== */
/* ============================================================================================== */
/**
 * [PRIVATE] Gets the terminal's current size from the environment variables ("COLUMNS", "LINES")
 * @return Terminal size packaged in WindowProperty_t
 */
static WindowProperty_t ctune_UI_getScreenSize( void ) {
    struct winsize w1;
    ioctl( 0, TIOCGWINSZ, &w1 );

    CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_UI_getScreenSize()] Screen size from ioctl = { w: %d, h: %d }", w1.ws_col, w1.ws_row );

    return (WindowProperty_t) { ( w1.ws_row >= UI_MIN_ROWS ? w1.ws_row : UI_MIN_ROWS ),
                                ( w1.ws_col >= UI_MIN_COLS ? w1.ws_col : UI_MIN_COLS ),
                                0,
                                0 };
}

/**
 * [PRIVATE] Gets the number of raised flags in the init stage array
 * @return Successful init count
 */
static size_t ctune_UI_getInitCount() {
    size_t count = 0;

    for( size_t i = 0; i < CTUNE_UI_INITSTAGE_COUNT; i++ ) {
        if( ui.init_stages[ i ] ) {
            ++count;
        }
    }

    return count;
}

/**
 * [PRIVATE] Gets the favourite/queued state of a RadioStationInfo_t object
 * @param rsi Pointer to RadioStationInfo_t object
 * @return State
 */
static unsigned ctune_UI_getStationState( const ctune_RadioStationInfo_t * rsi ) {
    unsigned state = 0b0000;

    if( rsi != NULL ) {
        if( ctune_RadioStationInfo.hash( ctune_RadioStationInfo.get.stationUUID( rsi ) ) == ctune_UI_MainWin.getCurrStationHash( &ui.main_win ) ) {
            if( ctune_RadioStationInfo.sameUUID( rsi, ctune_UI_MainWin.getCurrStation( &ui.main_win ) ) ) {
                //^^^ in case of Hash clashing occurring and we end up getting the wrong station
                state |= ctune_RadioStationInfo.IS_QUEUED;
            }
        }

        if( ctune_Controller.cfg.isFavourite( rsi, ctune_RadioStationInfo.get.stationSource( rsi ) ) ) {
            state |= ctune_RadioStationInfo.IS_FAV;
        }
    }

    return state;
}

/**
 * [PRIVATE] Helper method to check if RSI is already a LOCAL favourite
 * @param rsi Pointer to RadioStationInfo_t object
 * @return Local favourite state
 */
static bool ctune_UI_generateLocalUUID( String_t * uuid ) {
    const int max_retries   = 5;
    int       attempt_count = 0;

    while( attempt_count < max_retries ) {
        if( ctune_generateUUID( uuid ) ) {
            if( !ctune_Controller.cfg.isFavouriteUUID( uuid->_raw, CTUNE_STATIONSRC_LOCAL ) ) {
                CTUNE_LOG( CTUNE_LOG_DEBUG,
                           "[ctune_UI_generateLocalUUID( %p )] Successfully generated UUID: <%s>.",
                           uuid, uuid->_raw, ( attempt_count + 1 )
                );

                return true; //EARLY RETURN  (happy path)

            } else {
                CTUNE_LOG( CTUNE_LOG_WARNING,
                           "[ctune_UI_generateLocalUUID( %p )] UUID <%s> already in use (attempt #%d).",
                           uuid, uuid->_raw, ( attempt_count + 1 )
                );
            }

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_generateLocalUUID( %p )] Failed to generate UUID (attempt #%d).",
                       uuid, ( attempt_count + 1 )
            );
        }

        ++attempt_count;
    }

    CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_generateLocalUUID( %p )] Failed to generate a UUID: max attempt reached.", uuid );
    return false;
}

/**
 * [PRIVATE] Start playback of currently queued radio station
 */
static void ctune_UI_startQueuedStationPlayback( void ) {
    const ctune_RadioStationInfo_t * rsi = ctune_UI_MainWin.getCurrStation( &ui.main_win );

    if( rsi != NULL ) {
        CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_startQueuedStationPlayback()] initiating queued RSI playback..." );

        if( !ctune_Controller.playback.start( rsi ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_startQueuedStationPlayback()] Failed playback init." );
        }

    } else {
        ctune_UI_MainWin.print.statusMsg( &ui.main_win, ctune_UI_Language.text( CTUNE_UI_TEXT_MSG_EMPTY_QUEUE ) );
    }
}

/**
 * [PRIVATE] Toggles recording of currently playing stream
 */
static void ctune_UI_recordPlayingStream() {
    if( ctune_Controller.recording.isRecording() ) {
        ctune_Controller.recording.stop();
    } else {
        ctune_Controller.recording.start();
    }
}

/**
 * [PRIVATE] Sorts the on-screen list of radio stations
 * @param tab  PanelID of the current tab
 * @param attr Sorting attribute ID
 * @return Success (unused)
 */
static int ctune_UI_sortStationList( ctune_UI_PanelID_e tab, int sort_by ) {
    ctune_UI_OptionsMenu.close( &ui.dialogs.optmenu );

    if( tab == CTUNE_UI_PANEL_FAVOURITES ) {
        ctune_Controller.cfg.setFavouriteSorting( sort_by );
        ctune_UI_MainWin.ctrl.updateFavourites( &ui.main_win, ctune_Controller.cfg.getListOfFavourites );
        ctune_UI_MainWin.show( &ui.main_win, CTUNE_UI_PANEL_FAVOURITES );
    }

    return 1;
}

/**
 * [PRIVATE] Synchronises the selected favourite station from a remote source to its remote counterpart
 * @param tab PanelID of the current tab
 * @param arg (unused)
 * @return 1 (unused)
 */
static int ctune_UI_syncRemoteStation( ctune_UI_PanelID_e tab, int arg ) {
    ctune_UI_OptionsMenu.close( &ui.dialogs.optmenu );

    const ctune_RadioStationInfo_t * rsi    = NULL;
    Vector_t                         vector = Vector.init( sizeof( ctune_RadioStationInfo_t ), ctune_RadioStationInfo.freeContent );

    if( tab == CTUNE_UI_PANEL_FAVOURITES ) {
        if( !ctune_UI_MainWin.isCtrlRowSelected( &ui.main_win, CTUNE_UI_PANEL_FAVOURITES ) ) {
            rsi = ctune_UI_MainWin.getSelectedStation( &ui.main_win, CTUNE_UI_PANEL_FAVOURITES );
        }
    }

    if( rsi != NULL && ctune_RadioStationInfo.get.stationSource( rsi ) != CTUNE_STATIONSRC_LOCAL ) {
        if( ctune_Controller.search.getStationsBy( RADIOBROWSER_STATION_BY_UUID, ctune_RadioStationInfo.get.stationUUID( rsi ), &vector ) ) {

            if( !Vector.empty( &vector ) ) {
                ctune_UI_RSListWin_PageState_t view_state = ctune_UI_MainWin.getViewState( &ui.main_win, CTUNE_UI_PANEL_FAVOURITES );

                ctune_Controller.cfg.updateFavourite( Vector.at( &vector, 0 ), ctune_RadioStationInfo.get.stationSource( rsi ) );
                ctune_UI_MainWin.ctrl.updateFavourites( &ui.main_win, ctune_Controller.cfg.getListOfFavourites );

                ctune_UI_MainWin.setViewState( &ui.main_win, CTUNE_UI_PANEL_FAVOURITES, view_state );

                ctune_UI_MainWin.show( &ui.main_win, CTUNE_UI_PANEL_FAVOURITES );
                ctune_UI_MainWin.print.statusMsg( &ui.main_win, ctune_UI_Language.text( CTUNE_UI_TEXT_SYNC_SUCCESS ) );

            } else {
                ctune_UI_MainWin.print.statusMsg( &ui.main_win, ctune_UI_Language.text( CTUNE_UI_TEXT_SYNC_FAIL_FETCH_REMOTE_NOT_FOUND ) );
            }

        } else {
            ctune_UI_MainWin.print.statusMsg( &ui.main_win, ctune_UI_Language.text( CTUNE_UI_TEXT_SYNC_FAIL_FETCH ) );
        }

    } else {
        ctune_UI_MainWin.print.statusMsg( &ui.main_win, ctune_UI_Language.text( CTUNE_UI_TEXT_SYNC_FAIL_LOCAL_STATION ) );
    }

    update_panels();
    doupdate();

    Vector.clear_vector( &vector );
    return 1;
}

/**
 * [PRIVATE] Sets the current pane's list row size
 * @param tab           PanelID of the current tab
 * @param action_flag_e Action to take (get/set)
 * @return State of the "large row" property or the action flag on error
 */
static int ctune_UI_setCurrListRowSize( ctune_UI_PanelID_e tab, int action_flag_e ) {
    ctune_UI_OptionsMenu.close( &ui.dialogs.optmenu );

    ctune_UIConfig_t * ui_config = ctune_Controller.cfg.getUIConfig();
    bool               state     = action_flag_e;

    switch( tab ) {
        case CTUNE_UI_PANEL_FAVOURITES: {
            state = ctune_UIConfig.fav_tab.largeRowSize( ui_config, action_flag_e );

            if( action_flag_e != FLAG_GET_VALUE ) {
                ctune_UI_MainWin.ctrl.setLargeRow( &ui.main_win, CTUNE_UI_PANEL_FAVOURITES, action_flag_e );
                ctune_UI_MainWin.show( &ui.main_win, CTUNE_UI_PANEL_FAVOURITES ); //force hard redraw
            }
        } break;

        case CTUNE_UI_PANEL_SEARCH: {
            state = ctune_UIConfig.search_tab.largeRowSize( ui_config, action_flag_e );

            if( action_flag_e != FLAG_GET_VALUE ) {
                ctune_UI_MainWin.ctrl.setLargeRow( &ui.main_win, CTUNE_UI_PANEL_SEARCH, action_flag_e );
                ctune_UI_MainWin.show( &ui.main_win, CTUNE_UI_PANEL_SEARCH ); //force hard redraw
            }
        } break;

        case CTUNE_UI_PANEL_BROWSER: {
            state = ctune_UIConfig.browse_tab.largeRowSize( ui_config, action_flag_e );

            if( action_flag_e != FLAG_GET_VALUE ) {
                ctune_UI_MainWin.ctrl.setLargeRow( &ui.main_win, CTUNE_UI_PANEL_BROWSER, action_flag_e );
                ctune_UI_MainWin.show( &ui.main_win, CTUNE_UI_PANEL_BROWSER ); //force hard redraw
            }
        } break;

        default: {
            CTUNE_LOG( CTUNE_LOG_FATAL,
                       "[ctune_UI_setCurrListRowSize( '%s', '%s' )] Invalid PanelID passed.",
                       ctune_UI_PanelID.str( tab ), ctune_Flag.str( action_flag_e ) );
        } break;
    }

    return state;
}

/**
 * [PRIVATE] Sets the 'favourite' tab's theming
 * @param tab           PanelID of the current tab
 * @param action_flag_e Action to take (get/set)
 * @return State of the "fav theming" property
 */
static int ctune_UI_setFavouriteTabTheming( ctune_UI_PanelID_e tab, int action_flag_e ) {
    ctune_UI_OptionsMenu.close( &ui.dialogs.optmenu );

    ctune_UIConfig_t * ui_config = ctune_Controller.cfg.getUIConfig();

    if( tab == CTUNE_UI_PANEL_FAVOURITES ) {
        const bool state = ctune_UIConfig.fav_tab.theming( ui_config, action_flag_e );

        if( action_flag_e != FLAG_GET_VALUE ) {
            ctune_UI_MainWin.ctrl.themeFavourites( &ui.main_win, (bool) action_flag_e );
            ctune_UI_MainWin.show( &ui.main_win, CTUNE_UI_PANEL_FAVOURITES ); //force hard redraw
        }

        return state;
    }

    return action_flag_e;
}

/**
 * [PRIVATE] Sets the 'favourite' tab's custom theming based on station location
 * @param tab           PanelID of the current tab
 * @param action_flag_e Action to take (get/set)
 * @return Active state (bit 1: Custom fav theming, bit 2: UI 'custom' preset)
 */
static int ctune_UI_favouriteTabCustomTheming( ctune_UI_PanelID_e tab, int action_flag_e ) {
    ctune_UI_OptionsMenu.close( &ui.dialogs.optmenu );

    if( tab == CTUNE_UI_PANEL_FAVOURITES ) {
        ctune_UIConfig_t * ui_config                = ctune_Controller.cfg.getUIConfig();
        const bool         using_fav_custom_theming = ctune_UIConfig.fav_tab.customTheming( ui_config, action_flag_e );
        const bool         using_ui_custom_preset   = ( ctune_UIConfig.theming.currentPreset( ui_config ) == CTUNE_UIPRESET_CUSTOM );
        const int          active_states            = ( using_ui_custom_preset << 1 ) | using_fav_custom_theming;

        if( action_flag_e != FLAG_GET_VALUE ) {
            const ctune_UIPreset_e curr_ui_preset = ctune_UIConfig.theming.currentPreset( ui_config );

            if( !ctune_UIConfig.theming.setPreset( ui_config, curr_ui_preset ) ) {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_UI_favouriteTabCustomTheming( '%s', '%s' )] Failed to re-set UI colour pallet preset '%s'.",
                           ctune_UI_PanelID.str( tab ), ctune_Flag.str( action_flag_e ), ctune_UIPreset.str( curr_ui_preset )
                );
            }

            ctune_UI_Theme.init( ctune_UIConfig.theming.getCurrentThemePallet( ui_config ) );
            ctune_UI_MainWin.show( &ui.main_win, CTUNE_UI_PANEL_FAVOURITES ); //force hard redraw
        }

        return active_states;
    }

    return action_flag_e;
}

/**
 * [PRIVATE] Sets the mouse support
 * @param tab           PanelID of the current tab
 * @param action_flag_e Action to take (get/set)
 * @return State of the mouse support
 */
static int ctune_UI_setMouseSupport( ctune_UI_PanelID_e tab, int action_flag_e ) {
    ctune_UI_OptionsMenu.close( &ui.dialogs.optmenu );

    ctune_UIConfig_t * ui_config = ctune_Controller.cfg.getUIConfig();

    const bool old_state = ctune_UIConfig.mouse.enabled( ui_config, FLAG_GET_VALUE );

    if( action_flag_e != FLAG_GET_VALUE ) {
        const bool new_state = ctune_UIConfig.mouse.enabled( ui_config, action_flag_e );

        if( old_state != new_state ) {
            if( new_state ) { //Turn ON
                if( mousemask( ALL_MOUSE_EVENTS, NULL ) != 0 ) {
                    CTUNE_LOG( CTUNE_LOG_MSG,
                               "[ctune_UI_setMouseSupport( '%s', '%s' )] Mouse navigation enabled.",
                               ctune_UI_PanelID.str( tab ), ctune_Flag.str( action_flag_e ) );

                    ctune_UI_Resizer.resize();

                } else {
                    ctune_err.set( CTUNE_ERR_IO_MOUSE_ENABLE_FAIL );
                    goto fail;
                }

            } else { //Turn OFF
                if( mousemask( 0, NULL ) == 0 ) {
                    CTUNE_LOG( CTUNE_LOG_MSG,
                               "[ctune_UI_setMouseSupport( '%s', '%s' )] Mouse navigation disabled.",
                               ctune_UI_PanelID.str( tab ), ctune_Flag.str( action_flag_e ) );

                    ctune_UI_Resizer.resize();

                } else {
                    ctune_err.set( CTUNE_ERR_IO_MOUSE_DISABLE_FAIL );
                    goto fail;
                }
            }
        }

        ctune_UI_ContextHelp.setMouseCtrl( new_state );
        ctune_UI_RSInfo.setMouseCtrl( &ui.dialogs.rsinfo, new_state );
        ctune_UI_RSEdit.setMouseCtrl( &ui.dialogs.rsedit, new_state );
        ctune_UI_RSFind.setMouseCtrl( &ui.dialogs.rsfind, new_state );
        ctune_UI_MainWin.setMouseCtrl( &ui.main_win, new_state );

        return new_state;
    }

    fail:
        return old_state;
}

/**
 * [PRIVATE] Sets the current mouse interval resolution
 * @param tab      PanelID of the current tab
 * @param preset_e MouseResolution preset to apply
 * @return Success
 */
static int ctune_UI_setMouseIntervalResolution( ctune_UI_PanelID_e tab, int preset_e ) {
    ctune_UI_OptionsMenu.close( &ui.dialogs.optmenu );

    ctune_UIConfig_t * ui_config = ctune_Controller.cfg.getUIConfig();

    if( !ctune_UIConfig.mouse.setResolutionPreset( ui_config, preset_e ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_setMouseIntervalResolution( '%s', %d )] Failed to set the mouse click interval resolution ('%s').",
                   ctune_UI_PanelID.str( tab ), preset_e, ctune_MouseInterval.str( preset_e )
        );

        return 0; //EARLY RETURN
    }

    mouseinterval( ctune_UIConfig.mouse.clickIntervalResolution( ui_config ) );

    CTUNE_LOG( CTUNE_LOG_ERROR,
               "[ctune_UI_setMouseIntervalResolution( '%s', %d )] Mouse click interval resolution preset '%s' set.",
               ctune_UI_PanelID.str( tab ), preset_e, ctune_MouseInterval.str( preset_e )
    );

    return 1;
}

/**
 * [PRIVATE] Sets the current colour pallet theme for the UI
 * @param tab            PanelID of the current tab
 * @param ui_preset_enum ctune_UIPreset_e value to set
 * @return Success
 */
static int ctune_UI_setUITheme( ctune_UI_PanelID_e tab, int ui_preset_enum ) {
    ctune_UI_OptionsMenu.close( &ui.dialogs.optmenu );

    ctune_UIConfig_t * ui_config = ctune_Controller.cfg.getUIConfig();

    if( !ctune_UIConfig.theming.setPreset( ui_config, ui_preset_enum ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_setUITheme( '%s', %d )] Failed to set UI colour pallet preset '%s'.",
                   ctune_UI_PanelID.str( tab ), ui_preset_enum, ctune_UIPreset.str( ui_preset_enum )
        );

        return 0; //EARLY RETURN
    }

    CTUNE_LOG( CTUNE_LOG_MSG,
               "[ctune_UI_setUITheme( '%s', %d )] UI colour pallet preset '%s' set.",
               ctune_UI_PanelID.str( tab ), ui_preset_enum, ctune_UIPreset.str( ui_preset_enum )
    );

    ctune_UI_Theme.init( ctune_UIConfig.theming.getCurrentThemePallet( ui_config ) );
    ctune_UI_Resizer.resize();

    return 1;
}

/**
 * [PRIVATE] Sets the unicode icon state
 * @param tab PanelID of the current tab
 * @param action_flag_e Action to take (get/set)
 * @return State of the unicode icon usage
 */
static int ctune_UI_setUnicodeIcons( ctune_UI_PanelID_e tab, int action_flag_e ) {
    ctune_UI_OptionsMenu.close( &ui.dialogs.optmenu );

    ctune_UIConfig_t * ui_config = ctune_Controller.cfg.getUIConfig();
    const bool         state     = ctune_UIConfig.unicodeIcons( ui_config, action_flag_e );

    if( action_flag_e != FLAG_GET_VALUE ) {
        ctune_UI_Icons.setUnicode( state );
        ctune_UI_Resizer.resize();
    }

    return state;
}

/**
 * [PRIVATE] Sets a player plugin
 * @param tab       PanelID of the current tab
 * @param plugin_id Plugin ID
 * @return Success
 */
static int ctune_UI_setPlayerPlugin( ctune_UI_PanelID_e tab, int plugin_id ) {
    ctune_UI_OptionsMenu.close( &ui.dialogs.optmenu );

    if( !ctune_Controller.plugins.changePlugin( CTUNE_PLUGIN_IN_STREAM_PLAYER, plugin_id ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_setPlayerPlugin( '%s', %d )] Failed to change plugin.",
                   ctune_UI_PanelID.str( tab ), plugin_id
        );

        return 0; //EARLY RETURN
    }

    return 1;
}

/**
 * [PRIVATE] Sets the recording directory path
 * @param tab (unused)
 * @param arg (unused)
 * @return Success
 */
static int ctune_UI_openRecordingOutputPathDialog( ctune_UI_PanelID_e tab, int arg ) {
    //omitting closing options menu is not a mistake
    if( !ctune_UI_SetOutputDir.show( &ui.dialogs.setrecdir ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_openRecordingOutputPathDialog( '%s', %d )] Failed to show SetOutputDir window.",
                   ctune_UI_PanelID.str( tab ), arg
        );

        ctune_err.set( CTUNE_ERR_UI );
        return 0; //EARLY RETURN
    }

    ctune_UI_SetOutputDir.captureInput( &ui.dialogs.setrecdir ); //ignore return since the callbacks do the job

    return 1;
}

/**
 * [PRIVATE] Sets the stream timeout value
 * @param tab   PanelID of the current tab
 * @param value Value in seconds (<0 will just return the current value without setting anything)
 * @return Current value
 */
static int ctune_UI_setStreamTimeOut( ctune_UI_PanelID_e tab, int value ) {
    ctune_UI_OptionsMenu.close( &ui.dialogs.optmenu );

    const int curr = ctune_Controller.cfg.getStreamTimeout();

    if( value >= 0 && ctune_Controller.cfg.setStreamTimeout( value ) ) {
        return value;
    }

    return curr;
}

/**
 * [PRIVATE] Sets a sound server plugin
 * @param tab       PanelID of the current tab
 * @param plugin_id Plugin ID
 * @return Success
 */
static int ctune_UI_setSoundServerPlugin( ctune_UI_PanelID_e tab, int plugin_id ) {
    ctune_UI_OptionsMenu.close( &ui.dialogs.optmenu );

    if( !ctune_Controller.plugins.changePlugin( CTUNE_PLUGIN_OUT_AUDIO_SERVER, plugin_id ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_setSoundServerPlugin( '%s', %d )] Failed to change plugin.",
                   ctune_UI_PanelID.str( tab ), plugin_id
        );

        return 0; //EARLY RETURN
    }

    return 1;
}

/**
 * [PRIVATE] Sets a recorder plugin
 * @param tab       PanelID of the current tab
 * @param plugin_id Plugin ID
 * @return Success
 */
static int ctune_UI_setRecorderPlugin( ctune_UI_PanelID_e tab, int plugin_id ) {
    ctune_UI_OptionsMenu.close( &ui.dialogs.optmenu );

    if( !ctune_Controller.plugins.changePlugin( CTUNE_PLUGIN_OUT_AUDIO_RECORDER, plugin_id ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_setRecorderPlugin( '%s', %d )] Failed to change plugin.",
                   ctune_UI_PanelID.str( tab ), plugin_id
        );

        return 0; //EARLY RETURN
    }

    return 1;
}

/**
 * [PRIVATE] Toggles the favourite state of a selected station in any of the tabs
 * @param tab PanelID of the current tab
 * @param arg Pending state
 * @return 1 (unused)
 */
static int ctune_UI_toggleFavourite( ctune_UI_PanelID_e tab, int pending_state ) {
    ctune_UI_OptionsMenu.close( &ui.dialogs.optmenu );

    int  ch;
    bool refresh = false;

    const ctune_RadioStationInfo_t * rsi = ctune_UI_MainWin.getSelectedStation( &ui.main_win, tab );

    CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_UI_toggleFavourite( %s, %d )]", ctune_UI_PanelID.str( tab ), pending_state );

    if( tab == CTUNE_UI_PANEL_FAVOURITES ) {
        if( ctune_Controller.cfg.isFavourite( rsi, ctune_RadioStationInfo.get.stationSource( rsi ) ) ) {
            if( pending_state & ACTION_CONFIRMED ) {
                ctune_UI_MainWin.ctrl.toggleFavourite( &ui.main_win, CTUNE_UI_PANEL_FAVOURITES );
                refresh = true;

            } else if( pending_state & ACTION_REQUEST ) {
                ctune_UI_MainWin.print.statusMsg( &ui.main_win, ctune_UI_Language.text( CTUNE_UI_TEXT_MSG_CONFIRM_UNFAV ) );
                update_panels();
                doupdate();
            }

        } else {
            ctune_UI_MainWin.ctrl.toggleFavourite( &ui.main_win, CTUNE_UI_PANEL_FAVOURITES );
            pending_state = ACTION_CONFIRMED;
            refresh = true;
        }

        if( refresh ) {
            ctune_UI_MainWin.ctrl.updateFavourites( &ui.main_win, ctune_Controller.cfg.getListOfFavourites );
            ctune_UI_MainWin.show( &ui.main_win, CTUNE_UI_PANEL_FAVOURITES );
        }

    } else if( tab == CTUNE_UI_PANEL_SEARCH ) {
        if( ctune_Controller.cfg.isFavourite( rsi, ctune_RadioStationInfo.get.stationSource( rsi ) ) ) {
            if( pending_state & ACTION_CONFIRMED ) {
                ctune_UI_MainWin.ctrl.toggleFavourite( &ui.main_win, CTUNE_UI_PANEL_SEARCH );
                refresh = true;

            } else if( pending_state & ACTION_REQUEST ) {
                ctune_UI_MainWin.print.statusMsg( &ui.main_win, ctune_UI_Language.text( CTUNE_UI_TEXT_MSG_CONFIRM_UNFAV ) );
                update_panels();
                doupdate();
            }

        } else {
            ctune_UI_MainWin.ctrl.toggleFavourite( &ui.main_win, CTUNE_UI_PANEL_SEARCH );
            pending_state = ACTION_CONFIRMED;
            refresh = true;
        }

        if( refresh ) {
            ctune_UI_MainWin.show( &ui.main_win, CTUNE_UI_PANEL_SEARCH );
        }

    } else if( tab == CTUNE_UI_PANEL_BROWSER && ctune_UI_MainWin.browserPaneIsInFocus( &ui.main_win, FOCUS_PANE_RIGHT ) ) {
        if( ctune_Controller.cfg.isFavourite( rsi, ctune_RadioStationInfo.get.stationSource( rsi ) ) ) {
            if( pending_state & ACTION_CONFIRMED ) {
                ctune_UI_MainWin.ctrl.toggleFavourite( &ui.main_win, CTUNE_UI_PANEL_BROWSER );
                refresh = true;

            } else if( pending_state & ACTION_REQUEST ) {
                ctune_UI_MainWin.print.statusMsg( &ui.main_win, ctune_UI_Language.text( CTUNE_UI_TEXT_MSG_CONFIRM_UNFAV ) );
                update_panels();
                doupdate();
            }

        } else {
            ctune_UI_MainWin.ctrl.toggleFavourite( &ui.main_win, CTUNE_UI_PANEL_BROWSER );
            pending_state = ACTION_CONFIRMED;
            refresh = true;
        }

        if( refresh ) {
            ctune_UI_MainWin.show( &ui.main_win, CTUNE_UI_PANEL_BROWSER );
        }
    }

    return pending_state;
}

/**
 * [PRIVATE] Opens the station information window for a selected station
 * @param rsi Pointer to selected RadioStationInfo
 */
static void ctune_UI_openSelectedStationInformationDialog( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi != NULL ) {
        ctune_UI_RSInfo.show( &ui.dialogs.rsinfo, ctune_UI_Language.text( CTUNE_UI_TEXT_WIN_TITLE_RSINFO_SELECTED ), rsi );
        ctune_UI_RSInfo.captureInput( &ui.dialogs.rsinfo );
    }
}

/**
 * [PRIVATE] Opens the 'find station' dialog window
 */
static void ctune_UI_openFindDialog( void ) {
    ctune_UI_MainWin.print.clearMsgLine( &ui.main_win );
    ctune_UI_MainWin.show( &ui.main_win, CTUNE_UI_PANEL_SEARCH );

    if( !ctune_UI_RSFind.show( &ui.dialogs.rsfind ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_openFindDialog()] Failed to show RSFind window." );
        ctune_err.set( CTUNE_ERR_UI );
        return; //EARLY RETURN
    }

    if( ctune_UI_RSFind.captureInput( &ui.dialogs.rsfind ) == CTUNE_UI_FORM_SUBMIT ) {
        Vector_t results = Vector.init( sizeof( ctune_RadioStationInfo_t ), ctune_RadioStationInfo.freeContent );

        bool ret = ctune_Controller.search.getStations( ctune_UI_RSFind.getFilter( &ui.dialogs.rsfind ), &results );

        if( ret && !Vector.empty( &results ) ) {
            ctune_UI_MainWin.ctrl.loadSearchResults( &ui.main_win, &results, ctune_UI_RSFind.getFilter( &ui.dialogs.rsfind ) );
        } else {
            ctune_UI_MainWin.ctrl.clearSearchResults( &ui.main_win );
        }

        ctune_UI_MainWin.show( &ui.main_win, CTUNE_UI_PANEL_SEARCH );
        Vector.clear_vector( &results );
    }
}

/**
 * [PRIVATE] Opens the 'add/edit station' dialog window for a new station
 * @param tab PanelID of the current tab
 * @param arg (unused)
 * @return Success
 */
static int ctune_UI_openNewStationDialog( ctune_UI_PanelID_e tab, int arg /*unused*/ ) {
    ctune_UI_MainWin.print.clearMsgLine( &ui.main_win );
    ctune_UI_OptionsMenu.close( &ui.dialogs.optmenu );

    if( tab == CTUNE_UI_PANEL_FAVOURITES ) {
        if( !ctune_UI_RSEdit.newStation( &ui.dialogs.rsedit ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_openNewStationDialog( '%s', %d )] Failed to set new station init state.",
                       ctune_UI_PanelID.str( tab ), arg
            );

            return 0; //EARLY RETURN
        }

        if( !ctune_UI_RSEdit.show( &ui.dialogs.rsedit ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_openNewStationDialog( '%s', %d )] Failed to show RSFind window.",
                       ctune_UI_PanelID.str( tab ), arg
            );

            ctune_err.set( CTUNE_ERR_UI );
            return 0; //EARLY RETURN
        }

        if( ctune_UI_RSEdit.captureInput( &ui.dialogs.rsedit ) == CTUNE_UI_FORM_SUBMIT ) {
            ctune_RadioStationInfo_t * station = ctune_UI_RSEdit.getStation( &ui.dialogs.rsedit );
            ctune_Controller.cfg.toggleFavourite( station, ctune_RadioStationInfo.get.stationSource( station ) );
            ctune_UI_MainWin.ctrl.updateFavourites( &ui.main_win, ctune_Controller.cfg.getListOfFavourites );
            ctune_UI_MainWin.show( &ui.main_win, CTUNE_UI_PANEL_FAVOURITES );
        }
    }

    return 1;
}

/**
 * [PRIVATE] Opens the 'add/edit station' dialog window for an existing station
 * @param tab PanelID of the current tab
 * @param arg (unused)
 * @return Success
 */
static int ctune_UI_openEditSelectedStationDialog( ctune_UI_PanelID_e tab, int arg /*unused*/ ) {
    ctune_UI_MainWin.print.clearMsgLine( &ui.main_win );
    ctune_UI_OptionsMenu.close( &ui.dialogs.optmenu );

    if( tab == CTUNE_UI_PANEL_FAVOURITES ) {
        const ctune_RadioStationInfo_t * rsi = ctune_UI_MainWin.getSelectedStation( &ui.main_win, tab );

        if( rsi == NULL ) {
            ctune_UI.printError( ctune_UI_Language.text( CTUNE_UI_TEXT_STATION_NOT_EDITABLE ), CTUNE_ERR_ACTION_UNSUPPORTED );
            return 1; //EARLY RETURN
        }

        if( ctune_RadioStationInfo.get.stationSource( rsi ) == CTUNE_STATIONSRC_LOCAL ) {
            ctune_UI_RSEdit.loadStation( &ui.dialogs.rsedit, rsi );

            if( !ctune_UI_RSEdit.show( &ui.dialogs.rsedit ) ) {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_UI_openEditSelectedStationDialog( '%s', %d )] Failed to show RSFind window.",
                           ctune_UI_PanelID.str( tab ), arg
                );

                ctune_err.set( CTUNE_ERR_UI );
                return 0; //EARLY RETURN
            }

            if( ctune_UI_RSEdit.captureInput( &ui.dialogs.rsedit ) == CTUNE_UI_FORM_SUBMIT ) {
                ctune_RadioStationInfo_t * station = ctune_UI_RSEdit.getStation( &ui.dialogs.rsedit );
                ctune_Controller.cfg.updateFavourite( station, ctune_RadioStationInfo.get.stationSource( station ) );
            }

        } else if( ctune_RadioStationInfo.get.stationSource( rsi ) == CTUNE_STATIONSRC_RADIOBROWSER ) {
            ctune_UI_RSEdit.copyStation( &ui.dialogs.rsedit, rsi );

            if( !ctune_UI_RSEdit.show( &ui.dialogs.rsedit ) ) {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_UI_openEditSelectedStationDialog( '%s', %d )] Failed to show RSFind window.",
                           ctune_UI_PanelID.str( tab ), arg
                );

                ctune_err.set( CTUNE_ERR_UI );
                return 0; //EARLY RETURN
            }

            if( ctune_UI_RSEdit.captureInput( &ui.dialogs.rsedit ) == CTUNE_UI_FORM_SUBMIT ) {
                ctune_RadioStationInfo_t * station = ctune_UI_RSEdit.getStation( &ui.dialogs.rsedit );
                ctune_Controller.cfg.toggleFavourite( station, ctune_RadioStationInfo.get.stationSource( station ) );
            }

        } else {
            ctune_UI.printError( ctune_UI_Language.text( CTUNE_UI_TEXT_STATION_NOT_EDITABLE ), CTUNE_ERR_ACTION_UNSUPPORTED );
            return 1; //EARLY RETURN
        }

        ctune_UI_MainWin.ctrl.updateFavourites( &ui.main_win, ctune_Controller.cfg.getListOfFavourites );
        ctune_UI_MainWin.show( &ui.main_win, CTUNE_UI_PANEL_FAVOURITES );
    }

    return 1;
}

/**
 * [PRIVATE] Opens the options menu dialog
 * @param tab PanelID of the current tab
 */
static void ctune_UI_openOptionsMenuDialog( ctune_UI_PanelID_e tab ) {
    ctune_UI_MainWin.print.clearMsgLine( &ui.main_win );

    ctune_UIConfig_t * ui_config = ctune_Controller.cfg.getUIConfig();

    switch( tab ) {
        case CTUNE_UI_PANEL_FAVOURITES: {
            ctune_UI_OptionsMenu.free( &ui.dialogs.optmenu );

            ui.dialogs.optmenu = ctune_UI_OptionsMenu.create( &ui.screen_size, tab, ctune_UI_Language.text );
            ctune_UI_OptionsMenu.cb.setSortStationListCallback( &ui.dialogs.optmenu, ctune_UI_sortStationList );
            ctune_UI_OptionsMenu.cb.setAddNewStationCallback( &ui.dialogs.optmenu, ctune_UI_openNewStationDialog );
            ctune_UI_OptionsMenu.cb.setEditStationCallback( &ui.dialogs.optmenu, ctune_UI_openEditSelectedStationDialog );
            ctune_UI_OptionsMenu.cb.setToggleFavouriteCallback( &ui.dialogs.optmenu, ctune_UI_toggleFavourite );
            ctune_UI_OptionsMenu.cb.setSyncCurrSelectedStationCallback( &ui.dialogs.optmenu, ctune_UI_syncRemoteStation );
            ctune_UI_OptionsMenu.cb.setFavouriteTabThemingCallback( &ui.dialogs.optmenu, ctune_UI_setFavouriteTabTheming );
            ctune_UI_OptionsMenu.cb.setListRowSizeLargeCallback( &ui.dialogs.optmenu, ctune_UI_setCurrListRowSize );
            ctune_UI_OptionsMenu.cb.setGetUIConfigCallback( &ui.dialogs.optmenu, ctune_Controller.cfg.getUIConfig );
            ctune_UI_OptionsMenu.cb.setSetUIPresetCallback( &ui.dialogs.optmenu, ctune_UI_setUITheme );
            ctune_UI_OptionsMenu.cb.setFavTabCustomThemingCallback( &ui.dialogs.optmenu, ctune_UI_favouriteTabCustomTheming );
            ctune_UI_OptionsMenu.cb.setMouseSupportCallback( &ui.dialogs.optmenu, ctune_UI_setMouseSupport );
            ctune_UI_OptionsMenu.cb.setMouseResolutionCallback( &ui.dialogs.optmenu, ctune_UI_setMouseIntervalResolution );
            ctune_UI_OptionsMenu.cb.setUnicodeIconsCallback( &ui.dialogs.optmenu, ctune_UI_setUnicodeIcons );
            ctune_UI_OptionsMenu.cb.setStreamTimeoutValueCallback( &ui.dialogs.optmenu, ctune_UI_setStreamTimeOut );
            ctune_UI_OptionsMenu.cb.setPluginListCallback( &ui.dialogs.optmenu, ctune_Controller.plugins.getPluginList );
            ctune_UI_OptionsMenu.cb.setPluginSetterCallbacks( &ui.dialogs.optmenu, ctune_UI_setPlayerPlugin, ctune_UI_setSoundServerPlugin, ctune_UI_setRecorderPlugin );
            ctune_UI_OptionsMenu.cb.setRecordingDirPathCallback( &ui.dialogs.optmenu, ctune_UI_openRecordingOutputPathDialog );

            if( ctune_UI_OptionsMenu.init( &ui.dialogs.optmenu, ctune_UIConfig.mouse.enabled( ui_config, FLAG_GET_VALUE ) ) ) {
                ctune_UI_OptionsMenu.show( &ui.dialogs.optmenu );
                ctune_UI_OptionsMenu.captureInput( &ui.dialogs.optmenu );

            } else {
                CTUNE_LOG( CTUNE_LOG_FATAL,
                           "[ctune_UI_openOptionsMenuDialog( '%s' )] Could not init OptionsMenu_t dialog.",
                           ctune_UI_PanelID.str( tab )
                );

                ctune_err.set( CTUNE_ERR_UI );
            }
        } break;

        case CTUNE_UI_PANEL_SEARCH: //fallthrough
        case CTUNE_UI_PANEL_BROWSER: {
            ctune_UI_OptionsMenu.free( &ui.dialogs.optmenu );

            ui.dialogs.optmenu = ctune_UI_OptionsMenu.create( &ui.screen_size, tab, ctune_UI_Language.text );
            ctune_UI_OptionsMenu.cb.setListRowSizeLargeCallback( &ui.dialogs.optmenu, ctune_UI_setCurrListRowSize );
            ctune_UI_OptionsMenu.cb.setGetUIConfigCallback( &ui.dialogs.optmenu, ctune_Controller.cfg.getUIConfig );
            ctune_UI_OptionsMenu.cb.setSetUIPresetCallback( &ui.dialogs.optmenu, ctune_UI_setUITheme );
            ctune_UI_OptionsMenu.cb.setMouseSupportCallback( &ui.dialogs.optmenu, ctune_UI_setMouseSupport );
            ctune_UI_OptionsMenu.cb.setMouseResolutionCallback( &ui.dialogs.optmenu, ctune_UI_setMouseIntervalResolution );
            ctune_UI_OptionsMenu.cb.setUnicodeIconsCallback( &ui.dialogs.optmenu, ctune_UI_setUnicodeIcons );
            ctune_UI_OptionsMenu.cb.setStreamTimeoutValueCallback( &ui.dialogs.optmenu, ctune_UI_setStreamTimeOut );
            ctune_UI_OptionsMenu.cb.setPluginListCallback( &ui.dialogs.optmenu, ctune_Controller.plugins.getPluginList );
            ctune_UI_OptionsMenu.cb.setPluginSetterCallbacks( &ui.dialogs.optmenu, ctune_UI_setPlayerPlugin, ctune_UI_setSoundServerPlugin, ctune_UI_setRecorderPlugin );
            ctune_UI_OptionsMenu.cb.setRecordingDirPathCallback( &ui.dialogs.optmenu, ctune_UI_openRecordingOutputPathDialog );

            if( ctune_UI_OptionsMenu.init( &ui.dialogs.optmenu, ctune_UIConfig.mouse.enabled( ui_config, FLAG_GET_VALUE ) ) ) {
                ctune_UI_OptionsMenu.show( &ui.dialogs.optmenu );
                ctune_UI_OptionsMenu.captureInput( &ui.dialogs.optmenu );

            } else {
                CTUNE_LOG( CTUNE_LOG_FATAL,
                           "[ctune_UI_openOptionsMenuDialog( '%s' )] Could not init OptionsMenu_t dialog.",
                           ctune_UI_PanelID.str( tab )
                );

                ctune_err.set( CTUNE_ERR_UI );
            }
        } break;

        default: break;
    }
}

/**
 * [PRIVATE] Processes any event queued in the buffer
 */
static void ctune_UI_processEvent( ctune_UI_Event_t * event ) {
    switch( event->type ) {
        case EVENT_SONG_CHANGE: {
            CTUNE_LOG( CTUNE_LOG_DEBUG,
                       "[ctune_UI_processEvents( %p )] Dequeued song change event.",
                       event
            );

            char * str = event->data.pointer;
            ctune_UI_MainWin.print.songInfo( &ui.main_win, str );
            free( str );
        } break;

        case EVENT_VOLUME_CHANGE: {
            CTUNE_LOG( CTUNE_LOG_DEBUG,
                       "[ctune_UI_processEvents( %p )] Dequeued volume change event: %d",
                       event, event->data.integer
            );

            ctune_UI_MainWin.print.volume( &ui.main_win, event->data.integer );
        } break;

        case EVENT_PLAYBACK_STATE_CHANGE: {
            CTUNE_LOG( CTUNE_LOG_DEBUG,
                       "[ctune_UI_processEvents( %p )] Dequeued playback state change event: %s",
                       event, ctune_PlaybackCtrl.str( event->data.playback_ctrl )
            );

            ctune_UI_MainWin.print.playbackState( &ui.main_win, event->data.playback_ctrl );
        } break;

        case EVENT_SEARCH_STATE_CHANGE: {
            CTUNE_LOG( CTUNE_LOG_DEBUG,
                       "[ctune_UI_processEvents( %p )] Dequeued search state change event.",
                       event
            );

            ctune_UI_MainWin.print.searchingState( &ui.main_win, (bool) event->data.integer );
        } break;

        case EVENT_ERROR_MSG: {
            CTUNE_LOG( CTUNE_LOG_DEBUG,
                       "[ctune_UI_processEvents( %p )] Dequeued error message event: %s",
                       event, event->data.pointer
            );

            ctune_UI_MainWin.print.error( &ui.main_win, event->data.pointer );
        } break;

        case EVENT_STATUS_MSG: {
            CTUNE_LOG( CTUNE_LOG_DEBUG,
                       "[ctune_UI_processEvents( %p )] Dequeued status message event: %s",
                       event, event->data.pointer
            );

            ctune_UI_MainWin.print.statusMsg( &ui.main_win, event->data.pointer );
        } break;

        case EVENT_STATION_CHANGE: {
            CTUNE_LOG( CTUNE_LOG_DEBUG,
                       "[ctune_UI_processEvents( %p )] Dequeued station change event.",
                       event
            );

            ctune_UI_MainWin.ctrl.setCurrStation( &ui.main_win, event->data.pointer );
        } break;
    }
}

/**
 * [PRIVATE] Runs the key interaction interface
 */
static void ctune_UI_runKeyInterfaceLoop() {
    int                 character;
    ctune_UI_ActionID_e current_action;
    ctune_UI_ActionID_e pending_action = CTUNE_UI_ACTION_NONE;
    int                 pending_state  = ACTION_CANCELED;

    while( !( pending_action == CTUNE_UI_ACTION_QUIT && ( pending_state & ACTION_CONFIRMED ) ) ) {
        character      = getch();
        current_action = ctune_UI_KeyBinding.getAction( ctune_UI_MainWin.currentContext( &ui.main_win ), character );

        if( pending_action == CTUNE_UI_ACTION_QUIT && pending_state & ACTION_REQUEST && current_action != CTUNE_UI_ACTION_ERR ) {
            if( character == 'y' || character == 'q' ) {
                pending_state |= ACTION_CONFIRMED;
                continue;
            } else {
                pending_state  = ACTION_CANCELED;
                pending_action = CTUNE_UI_ACTION_NONE;
                current_action = CTUNE_UI_ACTION_ERR; //avoids initiating any actions if one is linked to the character
                ctune_UI_MainWin.print.clearMsgLine( &ui.main_win );
            }
        }

        if( pending_action == CTUNE_UI_ACTION_FAV && pending_state & ACTION_REQUEST && current_action != CTUNE_UI_ACTION_ERR ) {
            if( character == 'y' ) {
                pending_state |= ACTION_CONFIRMED;
                pending_state  = ctune_UI_toggleFavourite( ctune_UI_MainWin.currentPanelID( &ui.main_win ), pending_state );
            }

            pending_state  = ACTION_CANCELED;
            pending_action = CTUNE_UI_ACTION_NONE;
            current_action = CTUNE_UI_ACTION_ERR; //avoids initiating any actions if one is linked to the character
            ctune_UI_MainWin.print.clearMsgLine( &ui.main_win );
        }

        if( current_action == CTUNE_UI_ACTION_MOUSE_EVENT && getmouse( &ui.mouse_event ) == OK ) {
            current_action = ctune_UI_MainWin.handleMouseEvent( &ui.main_win, &ui.mouse_event );
        }

        switch( current_action ) {
            case CTUNE_UI_ACTION_FIND: {
                ctune_UI_openFindDialog();
            } break;

            case CTUNE_UI_ACTION_NEW: {
                ctune_UI_openNewStationDialog( ctune_UI_MainWin.currentPanelID( &ui.main_win ), 0 );
            } break;

            case CTUNE_UI_ACTION_EDIT: {
                ctune_UI_openEditSelectedStationDialog( ctune_UI_MainWin.currentPanelID( &ui.main_win ), 0 );
            } break;

            case CTUNE_UI_ACTION_RSI_SELECTED: {
                const ctune_UI_PanelID_e         tab = ctune_UI_MainWin.currentPanelID( &ui.main_win );
                const ctune_RadioStationInfo_t * rsi = ctune_UI_MainWin.getSelectedStation( &ui.main_win, tab );
                ctune_UI_openSelectedStationInformationDialog( rsi );
            } break;

            case CTUNE_UI_ACTION_OPTIONS: {
                ctune_UI_openOptionsMenuDialog( ctune_UI_MainWin.currentPanelID( &ui.main_win ) );
            } break;

            case CTUNE_UI_ACTION_GO_RIGHT     : { ctune_UI_MainWin.nav.selectRight( &ui.main_win );    } break;
            case CTUNE_UI_ACTION_GO_LEFT      : { ctune_UI_MainWin.nav.selectLeft( &ui.main_win );     } break;
            case CTUNE_UI_ACTION_SELECT_PREV  : { ctune_UI_MainWin.nav.selectUp( &ui.main_win );       } break;
            case CTUNE_UI_ACTION_SELECT_NEXT  : { ctune_UI_MainWin.nav.selectDown( &ui.main_win );     } break;
            case CTUNE_UI_ACTION_PAGE_UP      : { ctune_UI_MainWin.nav.selectPageUp( &ui.main_win );   } break;
            case CTUNE_UI_ACTION_PAGE_DOWN    : { ctune_UI_MainWin.nav.selectPageDown( &ui.main_win ); } break;
            case CTUNE_UI_ACTION_SELECT_FIRST : { ctune_UI_MainWin.nav.selectHome( &ui.main_win );     } break;
            case CTUNE_UI_ACTION_SELECT_LAST  : { ctune_UI_MainWin.nav.selectEnd( &ui.main_win );      } break;
            case CTUNE_UI_ACTION_FOCUS_LEFT   : //fallthrough
            case CTUNE_UI_ACTION_FOCUS_RIGHT  : { ctune_UI_MainWin.nav.switchFocus( &ui.main_win );    } break;

            case CTUNE_UI_ACTION_HELP: {
                ctune_UI_MainWin.print.clearMsgLine( &ui.main_win );
                ctune_UI_ContextHelp.show( ctune_UI_MainWin.currentContext( &ui.main_win ) );
                ctune_UI_ContextHelp.captureInput();
            } break;

            case CTUNE_UI_ACTION_GO_BACK: {
                ctune_UI_MainWin.print.clearMsgLine( &ui.main_win );
                ctune_UI_MainWin.show( &ui.main_win, ctune_UI_MainWin.previousPanelID( &ui.main_win ) );
            } break;

            case CTUNE_UI_ACTION_VOLUP_5: {
                ctune_UI_MainWin.print.clearMsgLine( &ui.main_win );
                if( ctune_Controller.playback.getPlaybackState() == true ) {
                    ctune_Controller.playback.modifyVolume( +5 );
                } else {
                    ctune_UI_MainWin.print.volume( &ui.main_win, ui.cb.quietVolChangeCallback( +5 ) );
                }
            } break;

            case CTUNE_UI_ACTION_VOLUP_10: {
                ctune_UI_MainWin.print.clearMsgLine( &ui.main_win );
                if( ctune_Controller.playback.getPlaybackState() == true ) {
                    ctune_Controller.playback.modifyVolume( +10 );
                } else {
                    ctune_UI_MainWin.print.volume( &ui.main_win, ui.cb.quietVolChangeCallback( +10 ) );
                }
            } break;

            case CTUNE_UI_ACTION_VOLDOWN_5: {
                ctune_UI_MainWin.print.clearMsgLine( &ui.main_win );
                if( ctune_Controller.playback.getPlaybackState() == true ) {
                    ctune_Controller.playback.modifyVolume( -5 );
                } else {
                    ctune_UI_MainWin.print.volume( &ui.main_win, ui.cb.quietVolChangeCallback( -5 ) );
                }
            } break;

            case CTUNE_UI_ACTION_VOLDOWN_10: {
                ctune_UI_MainWin.print.clearMsgLine( &ui.main_win );
                if( ctune_Controller.playback.getPlaybackState() == true ) {
                    ctune_Controller.playback.modifyVolume( -10 );
                } else {
                    ctune_UI_MainWin.print.volume( &ui.main_win, ui.cb.quietVolChangeCallback( -10 ) );
                }
            } break;

            case CTUNE_UI_ACTION_TRIGGER: { //current_action on selected
                ctune_UI_MainWin.print.clearMsgLine( &ui.main_win );
                ctune_UI_MainWin.nav.enter( &ui.main_win );
            } break;

            case CTUNE_UI_ACTION_PLAY: {
                ctune_UI_MainWin.print.clearMsgLine( &ui.main_win );
                ctune_UI_MainWin.ctrl.playSelectedStation( &ui.main_win );
            } break;

            case CTUNE_UI_ACTION_TOGGLE_PLAYBACK: {
                if( ctune_Controller.playback.getPlaybackState() ) {
                    ctune_Controller.playback.stop();
                } else {
                    ctune_UI_startQueuedStationPlayback();
                }
            } break;

            case CTUNE_UI_ACTION_RECORD: {
                ctune_UI_MainWin.print.clearMsgLine( &ui.main_win );
                ctune_UI_recordPlayingStream();
            } break;

            case CTUNE_UI_ACTION_STOP: {
                ctune_UI_MainWin.print.clearMsgLine( &ui.main_win );
                ctune_Controller.playback.stop();
            } break;

            case CTUNE_UI_ACTION_RESUME: { //play currently queued up station in ui.current_radio_station
                ctune_UI_MainWin.print.clearMsgLine( &ui.main_win );
                ctune_UI_startQueuedStationPlayback();
            } break;

            case CTUNE_UI_ACTION_QUIT: {
                pending_action = CTUNE_UI_ACTION_QUIT;
                pending_state |= ACTION_REQUEST;
                ctune_UI_MainWin.print.statusMsg( &ui.main_win, ctune_UI_Language.text( CTUNE_UI_TEXT_MSG_CONFIRM_QUIT ) );
                update_panels();
            } break;

            case CTUNE_UI_ACTION_TAB1: { //goto "Favourites" tab
                ctune_UI_MainWin.print.clearMsgLine( &ui.main_win );
                ctune_UI_MainWin.ctrl.updateFavourites( &ui.main_win, ctune_Controller.cfg.getListOfFavourites );
                ctune_UI_MainWin.show( &ui.main_win, CTUNE_UI_PANEL_FAVOURITES );
            } break;

            case CTUNE_UI_ACTION_TAB2: { //goto "Search" tab
                ctune_UI_MainWin.print.clearMsgLine( &ui.main_win );
                ctune_UI_MainWin.show( &ui.main_win, CTUNE_UI_PANEL_SEARCH );
            } break;

            case CTUNE_UI_ACTION_TAB3: { //goto "Browser" tab
                ctune_UI_MainWin.print.clearMsgLine( &ui.main_win );
                ctune_UI_MainWin.show( &ui.main_win, CTUNE_UI_PANEL_BROWSER );
            } break;

            case CTUNE_UI_ACTION_RSI_QUEUED: { //Opens the station information window for the currently queued/playing station
                ctune_UI_MainWin.print.clearMsgLine( &ui.main_win );

                const ctune_RadioStationInfo_t * rsi = ctune_UI_MainWin.getCurrStation( &ui.main_win );

                if( rsi != NULL ) {
                    ctune_UI_RSInfo.show( &ui.dialogs.rsinfo, ctune_UI_Language.text( CTUNE_UI_TEXT_WIN_TITLE_RSINFO_QUEUED ), rsi );
                    ctune_UI_RSInfo.captureInput( &ui.dialogs.rsinfo );
                }
            } break;

            case CTUNE_UI_ACTION_FAV: { //Toggle favourite state on currently selected station
                pending_action = CTUNE_UI_ACTION_FAV;

                if( pending_state == ACTION_CANCELED ) {
                    pending_state |= ACTION_REQUEST;
                }

                pending_state = ctune_UI_toggleFavourite( ctune_UI_MainWin.currentPanelID( &ui.main_win ), pending_state );

                if( pending_state == ACTION_CONFIRMED ) { //for actions that don't require confirmations
                    pending_state  = ACTION_CANCELED;
                    pending_action = CTUNE_UI_ACTION_NONE;
                    current_action = CTUNE_UI_ACTION_ERR; //avoids initiating any actions if one is linked to the character
                }
            } break;

            case CTUNE_UI_ACTION_ERR: {
                if( ctune_UI_Resizer.resizingRequested() ) {
                    ctune_UI_Resizer.resize();
                }

                if( !ctune_UI_EventQueue.empty() ) {
                    ctune_UI_EventQueue.flush();
                }
            } break;

            default: break;
        }

        doupdate();
    }
}

/* ============================================================================================== */
/* ========================================== PUBLIC ============================================ */
/* ============================================================================================== */
/**
 * Initialises the UI and its internal variables
 * @param show_cursor Flag to show the UI cursor
 * @return Success
 */
static bool ctune_UI_setup( bool show_cursor ) {
    CTUNE_LOG( CTUNE_LOG_MSG, "[ctune_UI_setup( %i )] Initialising the UI.", show_cursor );

    ui.old_cursor = curs_set( 0 );

    if( !ctune_UI_KeyBinding.init() ) {
        CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_UI_setup( %i )] Failed key binding init.", show_cursor );
        ctune_err.set( CTUNE_ERR_UI );
        return false; //EARLY RETURN

    } else {
        ui.init_stages[CTUNE_UI_INITSTAGE_KEYBINDS] = true;
    }

    ctune_UI_Icons.setUnicode( ctune_UIConfig.unicodeIcons( ctune_Controller.cfg.getUIConfig(), FLAG_GET_VALUE ) );

    if( ( stdscr = initscr() ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_UI_setup( %i )] Failed `initscr()`.", show_cursor );
        ctune_err.set( CTUNE_ERR_UI );
        return false; //EARLY RETURN

    } else {
        ui.init_stages[CTUNE_UI_INITSTAGE_STDSCR] = true;
    }

    ui.screen_size = ctune_UI_getScreenSize();
    ui.main_win    = ctune_UI_MainWin.create( ctune_UI_Language.text, &ui.screen_size );

    ctune_UI_MainWin.cb.setStationStateGetterCallback( &ui.main_win, ctune_UI_getStationState );
    ctune_UI_MainWin.cb.setOpenInfoDialogCallback( &ui.main_win, ctune_UI_openSelectedStationInformationDialog );
    ctune_UI_MainWin.cb.setPlayStationCallback( &ui.main_win, ctune_Controller.playback.start );
    ctune_UI_MainWin.cb.setOpenInfoDialogCallback( &ui.main_win, ctune_UI_openSelectedStationInformationDialog );

    if( !getenv("ESCDELAY") ) {
        set_escdelay( 25 );
    }

    if( has_colors() == false ) {
        CTUNE_LOG( CTUNE_LOG_WARNING, "[ctune_UI_setup( %i )] Terminal does not support colours.", show_cursor );
        ctune_err.set( CTUNE_ERR_UI );
        return false; //EARLY RETURN

    } else {
        ctune_UI_Theme.init( ctune_Controller.cfg.getUiTheme() );
        ui.init_stages[CTUNE_UI_INITSTAGE_THEME] = true;
    }

    ctune_UIConfig_t * ui_config = ctune_Controller.cfg.getUIConfig();

    const bool main_win_is_init = ctune_UI_MainWin.init( &ui.main_win,
                                                         ui_config,
                                                         ctune_Controller.search.getStations,
                                                         ctune_Controller.search.getCategoryItems,
                                                         ctune_Controller.search.getStationsBy,
                                                         ctune_Controller.cfg.toggleFavourite );

    if( main_win_is_init ) {
        ui.init_stages[CTUNE_UI_INITSTAGE_MAINWIN] = true;
    }

    ctune_UI_Resizer.init();

    if( ctune_UI_EventQueue.init( ctune_UI_processEvent ) ) {
        ui.init_stages[CTUNE_UI_INITSTAGE_EVENTQUEUE] = true;
    }

    cbreak();
    noecho();
    keypad( stdscr, TRUE );
    halfdelay( 1 );

    bool mouse_nav = ctune_UIConfig.mouse.enabled( ui_config, FLAG_GET_VALUE );

    // MOUSE NAVIGATION
    if( mouse_nav ) {
        if( mousemask( ALL_MOUSE_EVENTS, NULL ) != 0 ) {
            const ctune_MouseInterval_e resolution_preset = ctune_UIConfig.mouse.clickIntervalPreset( ui_config );
            const int                   resolution        = ctune_UIConfig.mouse.clickIntervalResolution( ui_config );

            mouseinterval( resolution );

            CTUNE_LOG( CTUNE_LOG_MSG,
                       "[ctune_UI_setup( %i )] Mouse navigation enabled (resolution '%s': %ims).",
                       show_cursor, ctune_MouseInterval.str( resolution_preset ), resolution
            );

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_setup( %i, %i )] Failed to enable mouse navigation.", show_cursor );
            ctune_err.set( CTUNE_ERR_IO_MOUSE_ENABLE_FAIL );
            ctune_UIConfig.mouse.enabled( ui_config, FLAG_SET_OFF );
            mouse_nav = false;
        }
    }

    // CONTEXTUAL HELP POPUP DIALOG
    if( !ctune_UI_ContextHelp.init( &ui.screen_size, ctune_UI_Language.text ) ) {
        ctune_err.set( CTUNE_ERR_UI );
        CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_UI_setup( %i )] Could not init contextual help.", show_cursor );
        return false; //EARLY RETURN

    } else {
        ctune_UI_ContextHelp.setMouseCtrl( mouse_nav );
        ui.init_stages[CTUNE_UI_INITSTAGE_CTXHELP] = true;
    }

    // FIND RADIO STATION FORM
    ui.dialogs.rsfind = ctune_UI_RSFind.create( &ui.screen_size, ctune_UI_Language.text );
    if( !ctune_UI_RSFind.init( &ui.dialogs.rsfind, mouse_nav ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_UI_setup( %i )] Could not init RSFind dialog.", show_cursor );
        ctune_err.set( CTUNE_ERR_UI );
        return false; //EARLY RETURN

    } else {
        ui.init_stages[CTUNE_UI_INITSTAGE_RSFIND] = true;
    }

    // CREATE/EDIT LOCAL RADIO STATION FORM
    ui.dialogs.rsedit = ctune_UI_RSEdit.create( &ui.screen_size,
                                                ctune_UI_Language.text,
                                                ctune_UI_generateLocalUUID,
                                                ctune_Controller.playback.testStream,
                                                ctune_Controller.playback.validateURL );
    if( !ctune_UI_RSEdit.init( &ui.dialogs.rsedit, mouse_nav ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_UI_setup( %i )] Could not init RSEdit dialog.", show_cursor );
        ctune_err.set( CTUNE_ERR_UI );
        return false; //EARLY RETURN

    } else {
        ui.init_stages[CTUNE_UI_INITSTAGE_RSEDIT] = true;
    }

    // RADIO STATION INFO DIALOG
    ui.dialogs.rsinfo = ctune_UI_RSInfo.create( &ui.screen_size, ctune_UI_Language.text, ": " );
    if( !ctune_UI_RSInfo.init( &ui.dialogs.rsinfo, mouse_nav ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_UI_setup( %i )] Could not init RSInfo_t dialog.", show_cursor );
        ctune_err.set( CTUNE_ERR_UI );
        return false; //EARLY RETURN

    } else {
        ui.init_stages[CTUNE_UI_INITSTAGE_RSINFO] = true;
    }

    // OPTIONS MENU DIALOG (test build)
    ui.dialogs.optmenu = ctune_UI_OptionsMenu.create( &ui.screen_size,
                                                      ctune_UI_MainWin.currentPanelID( &ui.main_win ),
                                                      ctune_UI_Language.text );
    if( !ctune_UI_OptionsMenu.init( &ui.dialogs.optmenu, mouse_nav ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_UI_setup( %i )] Could not init OptionsMenu_t dialog.", show_cursor );
        ctune_err.set( CTUNE_ERR_UI );
        return false; //EARLY RETURN

    } else {
        ui.init_stages[CTUNE_UI_INITSTAGE_OPTMENU] = true;
    }

    // SET OUTPUT PATH DIALOG
    ui.dialogs.setrecdir = ctune_UI_SetOutputDir.create( &ui.screen_size,
                                                         ctune_UI_Language.text,
                                                         ctune_Controller.recording.setPath,
                                                         ctune_Controller.recording.path );
    if( !ctune_UI_SetOutputDir.init( &ui.dialogs.setrecdir, mouse_nav ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_UI_setup( %i )] Could not init SetOutputDir_t dialog.", show_cursor );
        ctune_err.set( CTUNE_ERR_UI );
        return false; //EARLY RETURN

    } else {
        ui.init_stages[CTUNE_UI_INITSTAGE_SETOUTDIR] = true;
    }

    ui.init_stages[CTUNE_UI_INITSTAGE_COMPLETE] = true;

    ctune_UI_MainWin.ctrl.updateFavourites( &ui.main_win, ctune_Controller.cfg.getListOfFavourites );
    ctune_UI_MainWin.show( &ui.main_win, CTUNE_UI_PANEL_FAVOURITES );

    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_UI_setup( %i )] Init = [%i][%i][%i][%i][%i][%i][%i][%i][%i][%i][%i][%i]",
               show_cursor,
               ui.init_stages[CTUNE_UI_INITSTAGE_KEYBINDS],
               ui.init_stages[CTUNE_UI_INITSTAGE_STDSCR],
               ui.init_stages[CTUNE_UI_INITSTAGE_THEME],
               ui.init_stages[CTUNE_UI_INITSTAGE_MAINWIN],
               ui.init_stages[CTUNE_UI_INITSTAGE_EVENTQUEUE],
               ui.init_stages[CTUNE_UI_INITSTAGE_CTXHELP],
               ui.init_stages[CTUNE_UI_INITSTAGE_RSFIND],
               ui.init_stages[CTUNE_UI_INITSTAGE_RSEDIT],
               ui.init_stages[CTUNE_UI_INITSTAGE_RSINFO],
               ui.init_stages[CTUNE_UI_INITSTAGE_OPTMENU],
               ui.init_stages[CTUNE_UI_INITSTAGE_SETOUTDIR],
               ui.init_stages[CTUNE_UI_INITSTAGE_COMPLETE]
    );


    CTUNE_LOG( CTUNE_LOG_MSG,
               "[ctune_UI_setup( %i )] UI Initialised (%d/%d).",
               show_cursor, ctune_UI_getInitCount(), CTUNE_UI_INITSTAGE_COUNT
    );

    refresh();
    return true;
}

/**
 * Start the UI loop
 */
static void ctune_UI_start( void ) {
    ctune_UI_MainWin.show( &ui.main_win, CTUNE_UI_PANEL_FAVOURITES );
    ctune_UI_Resizer.push( ctune_UI.resize, NULL );
    ctune_UI_Resizer.push( ctune_UI_MainWin.resize, &ui.main_win );
    ctune_UI_runKeyInterfaceLoop();
}

/**
 * Terminates the UI
 */
static void ctune_UI_teardown() {
    CTUNE_LOG( CTUNE_LOG_MSG, "[ctune_UI_teardown()] Shutting down the UI..." );

    if( ui.init_stages[CTUNE_UI_INITSTAGE_RSFIND] && ctune_UI_RSFind.isInitialised( &ui.dialogs.rsfind ) ) {
        ctune_UI_RSFind.free( &ui.dialogs.rsfind );
        ui.init_stages[CTUNE_UI_INITSTAGE_RSFIND] = false;
    }

    if( ui.init_stages[CTUNE_UI_INITSTAGE_RSINFO] && ctune_UI_RSInfo.isInitialised( &ui.dialogs.rsinfo ) ) {
        ctune_UI_RSInfo.free( &ui.dialogs.rsinfo );
        ui.init_stages[CTUNE_UI_INITSTAGE_RSINFO] = false;
    }

    if( ui.init_stages[CTUNE_UI_INITSTAGE_RSEDIT] && ctune_UI_RSEdit.isInitialised( &ui.dialogs.rsedit ) ) {
        ctune_UI_RSEdit.free( &ui.dialogs.rsedit );
        ui.init_stages[CTUNE_UI_INITSTAGE_RSEDIT] = false;
    }

    if( ui.init_stages[CTUNE_UI_INITSTAGE_OPTMENU] ) {
        ctune_UI_OptionsMenu.free( &ui.dialogs.optmenu );
        ui.init_stages[CTUNE_UI_INITSTAGE_OPTMENU] = false;
    }

    if( ui.init_stages[CTUNE_UI_INITSTAGE_SETOUTDIR] ) {
        ctune_UI_SetOutputDir.free( &ui.dialogs.setrecdir );
        ui.init_stages[CTUNE_UI_INITSTAGE_SETOUTDIR] = false;
    }

    if( ui.init_stages[CTUNE_UI_INITSTAGE_CTXHELP] ) {
        ctune_UI_ContextHelp.free();
        ui.init_stages[CTUNE_UI_INITSTAGE_CTXHELP] = false;
    }


    if( ui.init_stages[CTUNE_UI_INITSTAGE_MAINWIN] ) {
        ctune_UI_MainWin.free( &ui.main_win );
        ui.init_stages[CTUNE_UI_INITSTAGE_MAINWIN] = false;
    }

    ctune_UI_Resizer.pop(); //ctune_UI_MainWin.resize
    ctune_UI_Resizer.pop(); //ctune_UI.resize
    ctune_UI_Resizer.free();

    if( ui.init_stages[CTUNE_UI_INITSTAGE_EVENTQUEUE] ) {
        ctune_UI_EventQueue.free();
        ui.init_stages[CTUNE_UI_INITSTAGE_EVENTQUEUE] = false;
    }

    delwin( stdscr );
    endwin();
    curs_set( ui.old_cursor );

    ui.init_stages[CTUNE_UI_INITSTAGE_STDSCR  ] = false;
    ui.init_stages[CTUNE_UI_INITSTAGE_THEME   ] = false;
    ui.init_stages[CTUNE_UI_INITSTAGE_KEYBINDS] = false;
    ui.init_stages[CTUNE_UI_INITSTAGE_COMPLETE] = false;

    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_UI_teardown()] Init = [%i][%i][%i][%i][%i][%i][%i][%i][%i][%i][%i][%i]",
               ui.init_stages[CTUNE_UI_INITSTAGE_KEYBINDS],
               ui.init_stages[CTUNE_UI_INITSTAGE_STDSCR],
               ui.init_stages[CTUNE_UI_INITSTAGE_THEME],
               ui.init_stages[CTUNE_UI_INITSTAGE_MAINWIN],
               ui.init_stages[CTUNE_UI_INITSTAGE_EVENTQUEUE],
               ui.init_stages[CTUNE_UI_INITSTAGE_CTXHELP],
               ui.init_stages[CTUNE_UI_INITSTAGE_RSFIND],
               ui.init_stages[CTUNE_UI_INITSTAGE_RSEDIT],
               ui.init_stages[CTUNE_UI_INITSTAGE_RSINFO],
               ui.init_stages[CTUNE_UI_INITSTAGE_OPTMENU],
               ui.init_stages[CTUNE_UI_INITSTAGE_SETOUTDIR],
               ui.init_stages[CTUNE_UI_INITSTAGE_COMPLETE]
    );

    CTUNE_LOG( CTUNE_LOG_MSG, "[ctune_UI_teardown()] UI shutdown." );
}

/**
 * Reconstruct the UI for when window sizes change
 */
static void ctune_UI_resize() {
    CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_UI_resize()] Resize event called." );
    ui.screen_size = ctune_UI_getScreenSize();
}

/**
 * Signals a radio station as 'current' (i.e. as queued or playing)
 * @param rsi Pointer to RadioStationInfo DTO
 */
static void ctune_UI_setCurrStation( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi ) {
        ctune_RadioStationInfo_t * copy = NULL;

        if( ( copy = malloc( sizeof( ctune_RadioStationInfo_t ) ) ) == NULL )  {
            CTUNE_LOG( CTUNE_LOG_FATAL,
                       "[ctune_UI_setCurrStation( %p )] "
                       "Failed memory allocation of RadioStationInfo copy.",
                       rsi );

            return; //EARLY RETURN
        }

        ctune_RadioStationInfo.init( copy );
        ctune_RadioStationInfo.copy( rsi, copy );

        ctune_UI_Event_t event = (ctune_UI_Event_t) { .type = EVENT_STATION_CHANGE, .data.pointer = copy };
        ctune_UI_EventQueue.add( &event );
    }
}

/**
 * Prints a string inside the area reserved for song descriptions
 * @param str String to display on screen
 */
static void ctune_UI_printSongInfo( const char * str ) {
    String_t song_title = String.init();
    String.set( &song_title, str );
    ctune_UI_Event_t event = (ctune_UI_Event_t) { .type = EVENT_SONG_CHANGE, .data.pointer = song_title._raw };
    ctune_UI_EventQueue.add( &event );
}

/**
 * Prints an integer inside the area reserved to display the current volume
 * @param vol Volume to display on screen
 */
static void ctune_UI_printVolume( const int vol ) {
    ctune_UI_Event_t event = (ctune_UI_Event_t) { .type = EVENT_VOLUME_CHANGE, .data.integer = vol };
    ctune_UI_EventQueue.add( &event );
}

/**
 * Prints the playback state to the screen
 * @param state Playback state
 */
static void ctune_UI_printPlaybackState( const ctune_PlaybackCtrl_e state ) {
    ctune_UI_Event_t event = (ctune_UI_Event_t) { .type = EVENT_PLAYBACK_STATE_CHANGE, .data.playback_ctrl = state };
    ctune_UI_EventQueue.add( &event );
}

/**
 * Prints the search state to the screen
 * @param state Search state
 */
static void ctune_UI_printSearchingState( const bool state ) {
    ctune_UI_Event_t event = (ctune_UI_Event_t) { .type = EVENT_SEARCH_STATE_CHANGE, .data.integer = state };
    ctune_UI_EventQueue.add( &event );
}

/**
 * Prints an error description string to the screen
 * @param err_str Error string
 * @param err_no  Error number (cTune specific)
 */
static void ctune_UI_printError( const char * err_str, int err_no ) {
    if( err_no > CTUNE_ERR_ACTION ) {
        ctune_UI.printStatusMsg( err_str );

    } else if( strlen( err_str ) > 0 ) {
        String_t string = String.init();
        String.set( &string, err_str );
        ctune_UI_Event_t event = (ctune_UI_Event_t) { .type = EVENT_ERROR_MSG, string._raw };
        ctune_UI_EventQueue.add( &event );
    }
}

/**
 * Prints a message string to the screen
 * @param info_str Information string
 */
static void ctune_UI_printStatusMsg( const char * info_str ) {
    String_t string = String.init();
    String.set( &string, info_str );
    ctune_UI_Event_t event = (ctune_UI_Event_t) { .type = EVENT_STATUS_MSG, string._raw };
    ctune_UI_EventQueue.add( &event );
}

/**
 * Sets the callback to use when a volume change occurs without anything playing
 * @param cb Callback method
 */
static void ctune_UI_setQuietVolChangeCallback( int(* cb)( int ) ) {
    ui.cb.quietVolChangeCallback = cb;
}

/**
 * Constructor
 */
const struct ctune_UI_Instance ctune_UI = {
    .setup                     = &ctune_UI_setup,
    .start                     = &ctune_UI_start,
    .teardown                  = &ctune_UI_teardown,
    .resize                    = &ctune_UI_resize,
    .setCurrStation            = &ctune_UI_setCurrStation,
    .printSongInfo             = &ctune_UI_printSongInfo,
    .printVolume               = &ctune_UI_printVolume,
    .printPlaybackState        = &ctune_UI_printPlaybackState,
    .printSearchingState       = &ctune_UI_printSearchingState,
    .printError                = &ctune_UI_printError,
    .printStatusMsg            = &ctune_UI_printStatusMsg,
    .setQuietVolChangeCallback = &ctune_UI_setQuietVolChangeCallback,
};