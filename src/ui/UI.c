#include "UI.h"

#include <panel.h>
#include <sys/ioctl.h>

#include "../ctune_err.h"
#include "../logger/Logger.h"
#include "../datastructure/Vector.h"
#include "../datastructure/StrList.h"
#include "../dto/RadioBrowserFilter.h"
#include "../network/RadioBrowser.h"
#include "../Controller.h"

#include "Resizer.h"
#include "definitions/KeyBinding.h"
#include "definitions/Theme.h"
#include "dialog/ContextHelp.h"
#include "dialog/RSFind.h"
#include "dialog/RSInfo.h"
#include "dialog/RSEdit.h"
#include "dialog/OptionsMenu.h"
#include "window/RSListWin.h"
#include "window/BrowserWin.h"

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
    CTUNE_UI_INITSTAGE_CTXHELP,
    CTUNE_UI_INITSTAGE_RSFIND,
    CTUNE_UI_INITSTAGE_RSEDIT,
    CTUNE_UI_INITSTAGE_RSINFO,
    CTUNE_UI_INITSTAGE_OPTMENU,
    CTUNE_UI_INITSTAGE_WINDOWS,
    CTUNE_UI_INITSTAGE_COMPLETE,

    CTUNE_UI_INITSTAGE_COUNT
} UIInitStages_e;

/**
 * Internal UI variables
 */
static struct ctune_UI {
    bool                 init_stages  [CTUNE_UI_INITSTAGE_COUNT];
    WINDOW             * panel_windows[CTUNE_UI_PANEL_COUNT];
    PANEL              * panels       [CTUNE_UI_PANEL_COUNT];
    int                  old_cursor;
    ctune_UI_Context_e   curr_ctx;

    struct ctune_UI_WinSizes {
        WindowProperty_t screen;
        WindowProperty_t title_bar;
        WindowProperty_t status_bar1;
        WindowProperty_t status_bar2;
        WindowProperty_t status_bar3;
        WindowProperty_t msg_line;
        WindowProperty_t tab_border;
        WindowProperty_t tab_canvas;
        WindowProperty_t browser_left;
        WindowProperty_t browser_right;

    } size;

    struct {
        ctune_UI_RSListWin_t  favourites;
        ctune_UI_RSListWin_t  search;
        ctune_UI_BrowserWin_t browser;
    } tabs;

    struct { //each dialog object type is recycled after the 1st use
        ctune_UI_RSInfo_t      rsinfo;
        ctune_UI_RSFind_t      rsfind;
        ctune_UI_RSEdit_t      rsedit;
        ctune_UI_OptionsMenu_t optmenu;
    } dialogs;

    struct {
        int(* quietVolChangeCallback)( int );
    } cb;

    struct {
        ctune_UI_PanelID_e           prev_panel;
        ctune_UI_PanelID_e           curr_panel;

        ctune_RadioStationInfo_t   * curr_radio_station;
        uint64_t                     curr_radio_station_hash;
        Vector_t                     favourites;

        //vars for painting back previous state post-resizing
        String_t                     curr_song;
        int                          curr_vol;
        bool                         playback_state;

    } cache;

} ui = {
    .curr_ctx      = CTUNE_UI_CTX_MAIN,
    .init_stages   = { false },
    .panel_windows = { NULL  },
    .panels        = { NULL  },
    .size = {
        .screen        = { 0, 0, 0, 0 },
        .title_bar     = { 0, 0, 0, 0 },
        .status_bar1   = { 0, 0, 0, 0 },
        .status_bar2   = { 0, 0, 0, 0 },
        .status_bar3   = { 0, 0, 0, 0 },
        .msg_line      = { 0, 0, 0, 0 },
        .tab_border    = { 0, 0, 0, 0 },
        .tab_canvas    = { 0, 0, 0, 0 },
        .browser_left  = { 0, 0, 0, 0 },
        .browser_right = { 0, 0, 0, 0 },
    },
    .cache = {
        .curr_radio_station      = NULL,
        .curr_radio_station_hash = 0,
        .curr_song               = { ._raw = NULL, ._length = 0 },
        .curr_vol                = -1,
        .playback_state          = false,
    },
};

/* ============================================================================================== */
/* ========================================== PRIVATE =========================================== */
/* ============================================================================================== */
/**
 * Calculates all the main UI window positions on the screen
 * @param sizes Pointer to ctune_UI_WinSizes struct Object
 */
static void ctune_UI_calculateWinSizes( struct ctune_UI_WinSizes * sizes ) {
    sizes->screen        = (WindowProperty_t) { ( getmaxy( stdscr ) >= UI_MIN_ROWS ? getmaxy( stdscr ) : UI_MIN_ROWS ),
                                                ( getmaxx( stdscr ) >= UI_MIN_COLS ? getmaxx( stdscr ) : UI_MIN_COLS ),
                                                0,
                                                0 };
    sizes->title_bar     = (WindowProperty_t) { 1,
                                                sizes->screen.cols,
                                                0,
                                                0 };
    sizes->status_bar1   = (WindowProperty_t) { 1,
                                                3,
                                                ( sizes->screen.rows - 2 ),
                                                0 };
    sizes->status_bar3   = (WindowProperty_t) { 1,
                                                10,
                                                ( sizes->screen.rows - 2 ),
                                                ( sizes->screen.cols - 10 ) };
    sizes->status_bar2   = (WindowProperty_t) { 1,
                                                ( sizes->screen.cols - sizes->status_bar3.cols - sizes->status_bar1.cols ),
                                                ( sizes->screen.rows - 2 ),
                                                sizes->status_bar1.cols };
    sizes->msg_line      = (WindowProperty_t) { 1,
                                                sizes->screen.cols,
                                                ( sizes->screen.rows - 1 ),
                                                0 };
    sizes->tab_border    = (WindowProperty_t) { ( sizes->screen.rows - sizes->title_bar.rows - sizes->status_bar1.rows - sizes->msg_line.rows ),
                                                sizes->screen.cols,
                                                sizes->title_bar.rows,
                                                0 };
    sizes->tab_canvas    = (WindowProperty_t) { ( sizes->tab_border.rows - 2 ),
                                                ( sizes->tab_border.cols  - 2 ),
                                                ( sizes->tab_border.pos_y + 1 ),
                                                ( sizes->tab_border.pos_x + 1 ) };
    sizes->browser_left  = (WindowProperty_t) { sizes->tab_canvas.rows,
                                                ( ( sizes->tab_canvas.cols / 3 ) > 39 ? 39 : ( sizes->tab_canvas.cols / 3 ) ) + 1,
                                                // the extra +1 is for the scrollbar                                                ^^^
                                                sizes->tab_canvas.pos_y,
                                                sizes->tab_canvas.pos_x };
    const int sep_width = 1;
    sizes->browser_right = (WindowProperty_t) { sizes->tab_canvas.rows,
                                                ( sizes->tab_canvas.cols - sizes->browser_left.cols - sep_width ),
                                                sizes->tab_canvas.pos_y,
                                                ( sizes->browser_left.cols + sep_width + 1 ) };

    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_calculateWinSizes( %p )] Screen         = { %i, %i, %i, %i }", sizes, sizes->screen.rows, sizes->screen.cols, sizes->screen.pos_y, sizes->screen.pos_x );
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_calculateWinSizes( %p )] Title          = { %i, %i, %i, %i }", sizes, sizes->title_bar.rows, sizes->title_bar.cols, sizes->title_bar.pos_y, sizes->title_bar.pos_x );
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_calculateWinSizes( %p )] Status bar #1  = { %i, %i, %i, %i }", sizes, sizes->status_bar1.rows, sizes->status_bar1.cols, sizes->status_bar1.pos_y, sizes->status_bar1.pos_x );
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_calculateWinSizes( %p )] Status bar #2  = { %i, %i, %i, %i }", sizes, sizes->status_bar2.rows, sizes->status_bar2.cols, sizes->status_bar2.pos_y, sizes->status_bar2.pos_x );
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_calculateWinSizes( %p )] Status bar #3  = { %i, %i, %i, %i }", sizes, sizes->status_bar3.rows, sizes->status_bar3.cols, sizes->status_bar3.pos_y, sizes->status_bar3.pos_x );
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_calculateWinSizes( %p )] Message line   = { %i, %i, %i, %i }", sizes, sizes->msg_line.rows, sizes->msg_line.cols, sizes->msg_line.pos_y, sizes->msg_line.pos_x );
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_calculateWinSizes( %p )] Tab border     = { %i, %i, %i, %i }", sizes, sizes->tab_border.rows, sizes->tab_border.cols, sizes->tab_border.pos_y, sizes->tab_border.pos_x );
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_calculateWinSizes( %p )] Tab canvas     = { %i, %i, %i, %i }", sizes, sizes->tab_canvas.rows, sizes->tab_canvas.cols, sizes->tab_canvas.pos_y, sizes->tab_canvas.pos_x );
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_calculateWinSizes( %p )] Browser::left  = { %i, %i, %i, %i }", sizes, sizes->browser_left.rows, sizes->browser_left.cols, sizes->browser_left.pos_y, sizes->browser_left.pos_x );
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_calculateWinSizes( %p )] Browser::right = { %i, %i, %i, %i }", sizes, sizes->browser_right.rows, sizes->browser_right.cols, sizes->browser_right.pos_y, sizes->browser_right.pos_x );

    CTUNE_LOG( CTUNE_LOG_MSG,
               "[ctune_UI_calculateWinSizes( %p )] "
               "Main window section sizes calculated based on screen size (%i x %i).",
               sizes, sizes->screen.rows, sizes->screen.cols
    );
}

/**
 * [PRIVATE] Clears all content from a panel and refresh to screen
 * @param panel Panel ID
 */
static void ctune_UI_clearPanel( enum ctune_UI_PanelID panel ) {
    werase( ui.panel_windows[panel] );
    update_panels();
    doupdate();
}

/**
 * [PRIVATE] Clears the message line of any messages if not used for input
 */
static void ctune_UI_clearMsgLine() {
    ctune_UI_clearPanel( CTUNE_UI_PANEL_MSG_LINE );
}

/**
 * [PRIVATE] Gets the favourite/queued state of a RadioStationInfo_t object
 * @param rsi Pointer to RadioStationInfo_t object
 * @return State
 */
static unsigned ctune_UI_getStationState( const ctune_RadioStationInfo_t * rsi ) {
    unsigned state = 0b0000;

    if( rsi != NULL ) {
        if( ctune_RadioStationInfo.hash( ctune_RadioStationInfo.get.stationUUID( rsi ) ) == ui.cache.curr_radio_station_hash ) {
            if( ctune_RadioStationInfo.sameUUID( rsi, ui.cache.curr_radio_station ) ) //<- in case of Hash clashing occurring and we end up getting the wrong station
                state |= ctune_RadioStationInfo.IS_QUEUED;
        }

        if( ctune_Controller.cfg.isFavourite( rsi, ctune_RadioStationInfo.get.stationSource( rsi ) ) )
            state |= ctune_RadioStationInfo.IS_FAV;
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
 * [PRIVATE] Write text to command line
 * @param str String to replace command line content
 */
static void ctune_UI_writeToMsgLine( const char * str ) {
    werase( ui.panel_windows[CTUNE_UI_PANEL_MSG_LINE] );
    mvwprintw( ui.panel_windows[CTUNE_UI_PANEL_MSG_LINE], 0, 0, "%s", str );
    update_panels();
    doupdate();
}

/**
 * [PRIVATE] Start playback of current selected station on current tab view
 * @param tab PanelID of the current tab
 */
static void ctune_UI_playSelectedStation( ctune_UI_PanelID_e tab ) {
    const ctune_RadioStationInfo_t * rsi   = NULL;
    size_t                           row_i = 0;

    if( tab == CTUNE_UI_PANEL_SEARCH ) {
        if( ctune_UI_RSListWin.isCtrlRowSelected( &ui.tabs.search ) ) {
            ctune_UI_RSListWin.triggerCtrlRow( &ui.tabs.search );
            ctune_UI_RSListWin.show( &ui.tabs.search );
        } else {
            rsi = ctune_UI_RSListWin.getSelected( &ui.tabs.search );
        }

        row_i = ui.tabs.search.row.selected;

    } else if( tab == CTUNE_UI_PANEL_FAVOURITES ) {
        if( !ctune_UI_RSListWin.isCtrlRowSelected( &ui.tabs.favourites ) ) {
            rsi = ctune_UI_RSListWin.getSelected( &ui.tabs.favourites );
        }

        row_i = ui.tabs.favourites.row.selected;

    } else if( tab == CTUNE_UI_PANEL_BROWSER ) {
        if( !ctune_UI_BrowserWin.isCtrlRowSelected( &ui.tabs.browser ) ) {
            rsi = ctune_UI_BrowserWin.getSelectedStation( &ui.tabs.browser);
        }

        row_i = ui.tabs.browser.right_pane.row.selected;
    }

    if( rsi != NULL ) {
        CTUNE_LOG( CTUNE_LOG_DEBUG,
                   "[ctune_UI_runKeyInterfaceLoop()] "
                   "Initiating RSI playback from search results (i=%lu)...",
                   row_i
        );

        if( !ctune_Controller.playback.start( rsi ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_runKeyInterfaceLoop()] Failed playback init (<%s>).",
                       ctune_RadioStationInfo.get.stationUUID( rsi )
            );
        }
    }

    if( tab == CTUNE_UI_PANEL_SEARCH ) {
        ctune_UI_RSListWin.setRedraw( &ui.tabs.search );
        ctune_UI_RSListWin.show( &ui.tabs.search );
    } else if( tab == CTUNE_UI_PANEL_FAVOURITES ) {
        ctune_UI_RSListWin.setRedraw( &ui.tabs.favourites );
        ctune_UI_RSListWin.show( &ui.tabs.favourites );
    } else if( tab == CTUNE_UI_PANEL_BROWSER ) {
        ctune_UI_BrowserWin.setRedraw( &ui.tabs.browser, false );
        ctune_UI_BrowserWin.show( &ui.tabs.browser );
    }
}

/**
 * [PRIVATE] Sorts the on-screen list of radio stations
 * @param tab  PanelID of the current tab
 * @param attr Sorting attribute ID
 */
static void ctune_UI_sortStationList( ctune_UI_PanelID_e tab, ctune_RadioStationInfo_SortBy_e attr ) {
    ctune_UI_OptionsMenu.close( &ui.dialogs.optmenu );

    if( tab == CTUNE_UI_PANEL_FAVOURITES ) {
        ctune_Controller.cfg.setFavouriteSorting( attr );
        ctune_Controller.cfg.getListOfFavourites( &ui.cache.favourites );
        ctune_UI_RSListWin.loadResults( &ui.tabs.favourites, &ui.cache.favourites, NULL );
        ctune_UI_RSListWin.show( &ui.tabs.favourites );
    }
}

/**
 * [PRIVATE] Synchronises the selected favourite station from a remote source to its remote counterpart
 * @param tab PanelID of the current tab
 */
static void ctune_UI_syncRemoteStation( ctune_UI_PanelID_e tab ) {
    ctune_UI_OptionsMenu.close( &ui.dialogs.optmenu );

    const ctune_RadioStationInfo_t * rsi    = NULL;
    Vector_t                         vector = Vector.init( sizeof( ctune_RadioStationInfo_t ), ctune_RadioStationInfo.freeContent );

    if( tab == CTUNE_UI_PANEL_FAVOURITES ) {
        if( !ctune_UI_RSListWin.isCtrlRowSelected( &ui.tabs.favourites ) ) {
            rsi = ctune_UI_RSListWin.getSelected( &ui.tabs.favourites );
        }
    }

    if( rsi != NULL && ctune_RadioStationInfo.get.stationSource( rsi ) != CTUNE_STATIONSRC_LOCAL ) {
        if( ctune_Controller.search.getStationsBy( RADIOBROWSER_STATION_BY_UUID, ctune_RadioStationInfo.get.stationUUID( rsi ), &vector ) ) {

            if( !Vector.empty( &vector ) ) {
                ctune_UI_RSListWin_PageState_t view_state = ctune_UI_RSListWin.getPageState( &ui.tabs.favourites );

                ctune_Controller.cfg.updateFavourite( Vector.at( &vector, 0 ), ctune_RadioStationInfo.get.stationSource( rsi ) );
                ctune_Controller.cfg.getListOfFavourites( &ui.cache.favourites );

                ctune_UI_RSListWin.loadResults( &ui.tabs.favourites, &ui.cache.favourites, NULL );
                ctune_UI_RSListWin.setPageState( &ui.tabs.favourites, view_state );
                ctune_UI_RSListWin.show( &ui.tabs.favourites );

                ctune_UI_writeToMsgLine( ctune_UI_Language.text( CTUNE_UI_TEXT_SYNC_SUCCESS ) );

            } else {
                ctune_UI_writeToMsgLine( ctune_UI_Language.text( CTUNE_UI_TEXT_SYNC_FAIL_FETCH_REMOTE_NOT_FOUND ) );
            }

        } else {
            ctune_UI_writeToMsgLine( ctune_UI_Language.text( CTUNE_UI_TEXT_SYNC_FAIL_FETCH ) );
        }

    } else {
        ctune_UI_writeToMsgLine( ctune_UI_Language.text( CTUNE_UI_TEXT_SYNC_FAIL_LOCAL_STATION )  );
    }

    Vector.clear_vector( &vector );
}

/**
 * [PRIVATE] Sets the current pane's list row size
 * @param tab PanelID of the current tab
 * @param action_flag Action to take (get/set)
 * @return State of the "large row" property or the action flag on error
 */
static bool ctune_UI_setCurrListRowSize( ctune_UI_PanelID_e tab, ctune_Flag_e action_flag ) {
    ctune_UI_OptionsMenu.close( &ui.dialogs.optmenu );

    ctune_UIConfig_t * ui_config = ctune_Controller.cfg.getUIConfig();
    bool               state     = action_flag;

    switch( tab ) {
        case CTUNE_UI_PANEL_FAVOURITES: {
            state = ctune_UIConfig.fav_tab.largeRowSize( ui_config, action_flag );

            if( action_flag != FLAG_GET_VALUE ) {
                ctune_UI_RSListWin.setLargeRow( &ui.tabs.favourites, action_flag );
                ctune_UI_RSListWin.resize( &ui.tabs.favourites );
                ctune_UI.show( CTUNE_UI_PANEL_FAVOURITES ); //force hard redraw
            }
        } break;

        case CTUNE_UI_PANEL_SEARCH: {
            state = ctune_UIConfig.search_tab.largeRowSize( ui_config, action_flag );

            if( action_flag != FLAG_GET_VALUE ) {
                ctune_UI_RSListWin.setLargeRow( &ui.tabs.search, action_flag );
                ctune_UI_RSListWin.resize( &ui.tabs.search );
                ctune_UI.show( CTUNE_UI_PANEL_SEARCH ); //force hard redraw
            }
        } break;

        case CTUNE_UI_PANEL_BROWSER: {
            state = ctune_UIConfig.browse_tab.largeRowSize( ui_config, action_flag );

            if( action_flag != FLAG_GET_VALUE ) {
                ctune_UI_RSListWin.setLargeRow( &ui.tabs.browser.right_pane, action_flag );
                ctune_UI_RSListWin.resize( &ui.tabs.browser.right_pane );
                ctune_UI.show( CTUNE_UI_PANEL_BROWSER ); //force hard redraw
            }
        } break;

        default: {
            CTUNE_LOG( CTUNE_LOG_FATAL,
                       "[ctune_UI_setCurrListRowSize( %d, %d )] Invalid PanelID passed.",
                       tab, action_flag );
        } break;
    }

    return state;
}

/**
 *
 * @param tab
 * @param action_flag
 * @return
 */
static bool ctune_UI_setFavouriteTabTheming( ctune_UI_PanelID_e tab, ctune_Flag_e action_flag ) {
    ctune_UI_OptionsMenu.close( &ui.dialogs.optmenu );

    ctune_UIConfig_t * ui_config = ctune_Controller.cfg.getUIConfig();

    if( tab == CTUNE_UI_PANEL_FAVOURITES ) {
        bool state = ctune_UIConfig.fav_tab.theming( ui_config, action_flag );

        if( action_flag != FLAG_GET_VALUE ) {
            ctune_UI_RSListWin.themeFavourites( &ui.tabs.favourites, (bool) action_flag );
            ctune_UI.show( CTUNE_UI_PANEL_FAVOURITES ); //force hard redraw
        }

        return state;
    }

    return action_flag;
}

/**
 * [PRIVATE] Toggles the favourite state of a selected station in any of the tabs
 * @param tab PanelID of the current tab
 */
static void ctune_UI_toggleFavourite( ctune_UI_PanelID_e tab ) {
    ctune_UI_OptionsMenu.close( &ui.dialogs.optmenu );

    int  ch;
    bool refresh = true;
    const ctune_RadioStationInfo_t * rsi = NULL;

    if( tab == CTUNE_UI_PANEL_FAVOURITES ) {
        rsi = ctune_UI_RSListWin.getSelected( &ui.tabs.favourites );

        if( ctune_Controller.cfg.isFavourite( rsi, ctune_RadioStationInfo.get.stationSource( rsi ) ) ) {
            ctune_UI_writeToMsgLine( ctune_UI_Language.text( CTUNE_UI_TEXT_MSG_CONFIRM_UNFAV ) );

            if( ( ch = getch() ) == 'y' )
                ctune_UI_RSListWin.toggleFav( &ui.tabs.favourites );
            else
                refresh = false;

        } else {
            ctune_UI_RSListWin.toggleFav( &ui.tabs.favourites );
        }

        if( refresh ) {
            ctune_Controller.cfg.getListOfFavourites( &ui.cache.favourites );
            ctune_UI_RSListWin.loadResults( &ui.tabs.favourites, &ui.cache.favourites, NULL );
            ctune_UI_RSListWin.show( &ui.tabs.favourites );
        }

    } else if( tab == CTUNE_UI_PANEL_SEARCH ) {
        rsi = ctune_UI_RSListWin.getSelected( &ui.tabs.search );

        if( ctune_Controller.cfg.isFavourite( rsi, ctune_RadioStationInfo.get.stationSource( rsi ) ) ) {
            ctune_UI_writeToMsgLine( ctune_UI_Language.text( CTUNE_UI_TEXT_MSG_CONFIRM_UNFAV ) );

            if( ( ch = getch() ) == 'y' )
                ctune_UI_RSListWin.toggleFav( &ui.tabs.search );
            else
                refresh = false;

        } else {
            ctune_UI_RSListWin.toggleFav( &ui.tabs.search );
        }

        if( refresh )
            ctune_UI_RSListWin.show( &ui.tabs.search );

    } else if( tab == CTUNE_UI_PANEL_BROWSER && ctune_UI_BrowserWin.isInFocus( &ui.tabs.browser, FOCUS_PANE_RIGHT ) ) {
        rsi = ctune_UI_BrowserWin.getSelectedStation( &ui.tabs.browser );

        if( ctune_Controller.cfg.isFavourite( rsi, ctune_RadioStationInfo.get.stationSource( rsi ) ) ) {
            ctune_UI_writeToMsgLine( ctune_UI_Language.text( CTUNE_UI_TEXT_MSG_CONFIRM_UNFAV ) );

            if( ( ch = getch() ) == 'y' )
                ctune_UI_BrowserWin.toggleFav( &ui.tabs.browser );
            else
                refresh = false;

        } else {
            ctune_UI_BrowserWin.toggleFav( &ui.tabs.browser );
        }

        if( refresh )
            ctune_UI_BrowserWin.show( &ui.tabs.browser );
    }

    ctune_UI_clearMsgLine();
}

/**
 * [PRIVATE] Opens the station information window for the currently selected station
 * @param tab PanelID of the current tab
 */
static void ctune_UI_openSelectedStationInformation( ctune_UI_PanelID_e tab ) {
    const ctune_RadioStationInfo_t * rsi = NULL;

    if( tab == CTUNE_UI_PANEL_FAVOURITES ) {
        rsi = ctune_UI_RSListWin.getSelected( &ui.tabs.favourites );
    } else if( tab == CTUNE_UI_PANEL_SEARCH ) {
        rsi = ctune_UI_RSListWin.getSelected( &ui.tabs.search );
    } else if( tab == CTUNE_UI_PANEL_BROWSER ) {
        rsi = ctune_UI_BrowserWin.getSelectedStation( &ui.tabs.browser );
    }

    if( rsi != NULL ) {
        ctune_UI_RSInfo.show( &ui.dialogs.rsinfo, ctune_UI_Language.text( CTUNE_UI_TEXT_WIN_TITLE_RSINFO_SELECTED ), rsi );
        ctune_UI_RSInfo.captureInput( &ui.dialogs.rsinfo );
    }
}

/**
 * [PRIVATE] Opens the 'find station' dialog window
 */
static void ctune_UI_openFindDialog( void ) {
    ctune_UI_clearMsgLine();
    ctune_UI.show( CTUNE_UI_PANEL_SEARCH );

    if( !ctune_UI_RSFind.show( &ui.dialogs.rsfind ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_openFindDialog()] Failed to show RSFind window." );
        ctune_err.set( CTUNE_ERR_UI );
        return; //EARLY RETURN
    }

    if( ctune_UI_RSFind.captureInput( &ui.dialogs.rsfind ) == CTUNE_UI_FORM_SUBMIT ) {
        Vector_t results = Vector.init( sizeof( ctune_RadioStationInfo_t ), ctune_RadioStationInfo.freeContent );

        bool ret = ctune_Controller.search.getStations( ctune_UI_RSFind.getFilter( &ui.dialogs.rsfind ), &results );

        if( ret && !Vector.empty( &results ) ) {
            ctune_UI_RSListWin.loadResults( &ui.tabs.search, &results, ctune_UI_RSFind.getFilter( &ui.dialogs.rsfind ) );
        } else {
            ctune_UI_RSListWin.loadNothing( &ui.tabs.search );
        }

        ctune_UI_RSListWin.show( &ui.tabs.search );
        Vector.clear_vector( &results );
    }
}

/**
 * [PRIVATE] Opens the 'add/edit station' dialog window for a new station
 * @param tab PanelID of the current tab
 */
static void ctune_UI_openNewStationDialog( ctune_UI_PanelID_e tab ) {
    ctune_UI_clearMsgLine();
    ctune_UI_OptionsMenu.close( &ui.dialogs.optmenu );

    if( tab == CTUNE_UI_PANEL_FAVOURITES ) {
        if( !ctune_UI_RSEdit.newStation( &ui.dialogs.rsedit ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_openNewStationDialog()] Failed to set new station init state." );
            return; //EARLY RETURN
        }

        if( !ctune_UI_RSEdit.show( &ui.dialogs.rsedit ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_openNewStationDialog()] Failed to show RSFind window." );
            ctune_err.set( CTUNE_ERR_UI );
            return; //EARLY RETURN
        }

        if( ctune_UI_RSEdit.captureInput( &ui.dialogs.rsedit ) == CTUNE_UI_FORM_SUBMIT ) {
            ctune_RadioStationInfo_t * station = ctune_UI_RSEdit.getStation( &ui.dialogs.rsedit );
            ctune_Controller.cfg.toggleFavourite( station, ctune_RadioStationInfo.get.stationSource( station ) );
            ctune_Controller.cfg.getListOfFavourites( &ui.cache.favourites );
            ctune_UI_RSListWin.loadResults( &ui.tabs.favourites, &ui.cache.favourites, NULL );
            ctune_UI_RSListWin.show( &ui.tabs.favourites );
        }
    }
}

/**
 * [PRIVATE] Opens the 'add/edit station' dialog window for an existing station
 * @param tab PanelID of the current tab
 */
static void ctune_UI_openEditSelectedStationDialog( ctune_UI_PanelID_e tab ) {
    ctune_UI_clearMsgLine();
    ctune_UI_OptionsMenu.close( &ui.dialogs.optmenu );

    if( tab == CTUNE_UI_PANEL_FAVOURITES ) {
        const ctune_RadioStationInfo_t * rsi = ctune_UI_RSListWin.getSelected( &ui.tabs.favourites );

        if( rsi == NULL ) {
            ctune_UI.printError( ctune_UI_Language.text( CTUNE_UI_TEXT_STATION_NOT_EDITABLE ), CTUNE_ERR_ACTION_UNSUPPORTED );
            return; //EARLY RETURN
        }

        if( ctune_RadioStationInfo.get.stationSource( rsi ) == CTUNE_STATIONSRC_LOCAL ) {
            ctune_UI_RSEdit.loadStation( &ui.dialogs.rsedit, rsi );

            if( !ctune_UI_RSEdit.show( &ui.dialogs.rsedit ) ) {
                CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_openEditSelectedStationDialog()] Failed to show RSFind window." );
                ctune_err.set( CTUNE_ERR_UI );
                return; //EARLY RETURN
            }

            if( ctune_UI_RSEdit.captureInput( &ui.dialogs.rsedit ) == CTUNE_UI_FORM_SUBMIT ) {
                ctune_RadioStationInfo_t * station = ctune_UI_RSEdit.getStation( &ui.dialogs.rsedit );
                ctune_Controller.cfg.updateFavourite( station, ctune_RadioStationInfo.get.stationSource( station ) );
            }

        } else if( ctune_RadioStationInfo.get.stationSource( rsi ) == CTUNE_STATIONSRC_RADIOBROWSER ) {
            ctune_UI_RSEdit.copyStation( &ui.dialogs.rsedit, rsi );

            if( !ctune_UI_RSEdit.show( &ui.dialogs.rsedit ) ) {
                CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_openEditSelectedStationDialog()] Failed to show RSFind window." );
                ctune_err.set( CTUNE_ERR_UI );
                return; //EARLY RETURN
            }

            if( ctune_UI_RSEdit.captureInput( &ui.dialogs.rsedit ) == CTUNE_UI_FORM_SUBMIT ) {
                ctune_RadioStationInfo_t * station = ctune_UI_RSEdit.getStation( &ui.dialogs.rsedit );
                ctune_Controller.cfg.toggleFavourite( station, ctune_RadioStationInfo.get.stationSource( station ) );
            }

        } else {
            ctune_UI.printError( ctune_UI_Language.text( CTUNE_UI_TEXT_STATION_NOT_EDITABLE ), CTUNE_ERR_ACTION_UNSUPPORTED );
            return; //EARLY RETURN
        }

        ctune_Controller.cfg.getListOfFavourites( &ui.cache.favourites );
        ctune_UI_RSListWin.loadResults( &ui.tabs.favourites, &ui.cache.favourites, NULL );
        ctune_UI_RSListWin.show( &ui.tabs.favourites );
    }
}

/**
 * [PRIVATE] Opens the options menu dialog
 * @param tab PanelID of the current tab
 */
static void ctune_UI_openOptionsMenuDialog( ctune_UI_PanelID_e tab ) {
    ctune_UI_clearMsgLine();

    switch( tab ) {
        case CTUNE_UI_PANEL_FAVOURITES: {
            ctune_UI_OptionsMenu.free( &ui.dialogs.optmenu );

            ui.dialogs.optmenu = ctune_UI_OptionsMenu.create( &ui.size.screen, tab, ctune_UI_Language.text );
            ctune_UI_OptionsMenu.cb.setSortStationListCallback( &ui.dialogs.optmenu, ctune_UI_sortStationList );
            ctune_UI_OptionsMenu.cb.setAddNewStationCallback( &ui.dialogs.optmenu, ctune_UI_openNewStationDialog );
            ctune_UI_OptionsMenu.cb.setEditStationCallback( &ui.dialogs.optmenu, ctune_UI_openEditSelectedStationDialog );
            ctune_UI_OptionsMenu.cb.setToggleFavouriteCallback( &ui.dialogs.optmenu, ctune_UI_toggleFavourite );
            ctune_UI_OptionsMenu.cb.setSyncCurrSelectedStationCallback( &ui.dialogs.optmenu, ctune_UI_syncRemoteStation );
            ctune_UI_OptionsMenu.cb.setFavouriteTabThemingCallback( &ui.dialogs.optmenu, ctune_UI_setFavouriteTabTheming );
            ctune_UI_OptionsMenu.cb.setListRowSizeLargeCallback( &ui.dialogs.optmenu, ctune_UI_setCurrListRowSize );

            if( ctune_UI_OptionsMenu.init( &ui.dialogs.optmenu ) ) {
                ctune_UI_OptionsMenu.show( &ui.dialogs.optmenu );
                ctune_UI_OptionsMenu.captureInput( &ui.dialogs.optmenu );

            } else {
                CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_UI_openOptionsMenuDialog( %i )] Could not init OptionsMenu_t dialog.", tab );
                ctune_err.set( CTUNE_ERR_UI );
            }
        } break;

        case CTUNE_UI_PANEL_SEARCH: //fallthrough
        case CTUNE_UI_PANEL_BROWSER: {
            ctune_UI_OptionsMenu.free( &ui.dialogs.optmenu );

            ui.dialogs.optmenu = ctune_UI_OptionsMenu.create( &ui.size.screen, tab, ctune_UI_Language.text );
            ctune_UI_OptionsMenu.cb.setListRowSizeLargeCallback( &ui.dialogs.optmenu, ctune_UI_setCurrListRowSize );

            if( ctune_UI_OptionsMenu.init( &ui.dialogs.optmenu ) ) {
                ctune_UI_OptionsMenu.show( &ui.dialogs.optmenu );
                ctune_UI_OptionsMenu.captureInput( &ui.dialogs.optmenu );

            } else {
                CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_UI_openOptionsMenuDialog( %i )] Could not init OptionsMenu_t dialog.", tab );
                ctune_err.set( CTUNE_ERR_UI );
            }
        } break;

        default: break;
    }
}

/**
 * [PRIVATE] Navigation select for 'ENTER' key
 * @param tab PanelID of the current tab
 */
static void ctune_UI_navEnter( ctune_UI_PanelID_e tab ) {
    ctune_UI_clearMsgLine();

    switch( tab ) {
        case CTUNE_UI_PANEL_FAVOURITES: //fallthrough
        case CTUNE_UI_PANEL_SEARCH: {
            ctune_UI_playSelectedStation( ui.cache.curr_panel );
        } break;

        case CTUNE_UI_PANEL_BROWSER: {
            if( ctune_UI_BrowserWin.isInFocus( &ui.tabs.browser, FOCUS_PANE_LEFT ) ) {
                ctune_UI_BrowserWin.navKeyEnter( &ui.tabs.browser );
                ctune_UI_BrowserWin.show( &ui.tabs.browser );
            } else { //focus: FOCUS_PANE_RIGHT
                ctune_UI_playSelectedStation( ui.cache.curr_panel );
            }
        } break;

        default:
            break;
    }
}

/**
 * [PRIVATE] Navigation select for 'UP' key
 * @param tab PanelID of the current tab
 */
static void ctune_UI_navSelectUp( ctune_UI_PanelID_e tab ) {
    ctune_UI_clearMsgLine();

    if( tab == CTUNE_UI_PANEL_FAVOURITES ) {
        ctune_UI_RSListWin.selectUp( &ui.tabs.favourites );
        ctune_UI_RSListWin.show( &ui.tabs.favourites );
    } else if( tab == CTUNE_UI_PANEL_SEARCH ) {
        ctune_UI_RSListWin.selectUp( &ui.tabs.search );
        ctune_UI_RSListWin.show( &ui.tabs.search );
    } else if( tab == CTUNE_UI_PANEL_BROWSER ) {
        ctune_UI_BrowserWin.navKeyUp( &ui.tabs.browser );
        ctune_UI_BrowserWin.show( &ui.tabs.browser );
    }
}

/**
 * [PRIVATE] Navigation select for 'DOWN' key
 * @param tab PanelID of the current tab
 */
static void ctune_UI_navSelectDown( ctune_UI_PanelID_e tab ) {
    ctune_UI_clearMsgLine();

    if( tab == CTUNE_UI_PANEL_FAVOURITES ) {
        ctune_UI_RSListWin.selectDown( &ui.tabs.favourites );
        ctune_UI_RSListWin.show( &ui.tabs.favourites );
    } else if( tab == CTUNE_UI_PANEL_SEARCH ) {
        ctune_UI_RSListWin.selectDown( &ui.tabs.search );
        ctune_UI_RSListWin.show( &ui.tabs.search );
    } else if( tab == CTUNE_UI_PANEL_BROWSER ) {
        ctune_UI_BrowserWin.navKeyDown( &ui.tabs.browser );
        ctune_UI_BrowserWin.show( &ui.tabs.browser );
    }
}

/**
 * [PRIVATE] Navigation select for 'LEFT' key
 * @param tab PanelID of the current tab
 */
static void ctune_UI_navSelectLeft( ctune_UI_PanelID_e tab ) {
    ctune_UI_clearMsgLine();

    if( tab == CTUNE_UI_PANEL_BROWSER ) {
        ctune_UI_BrowserWin.navKeyLeft( &ui.tabs.browser );
        ctune_UI_BrowserWin.show( &ui.tabs.browser );
    }
}

/**
 * [PRIVATE] Navigation select for 'RIGHT' key
 * @param tab PanelID of the current tab
 */
static void ctune_UI_navSelectRight( ctune_UI_PanelID_e tab ) {
    ctune_UI_clearMsgLine();

    switch( tab ) {
        case CTUNE_UI_PANEL_FAVOURITES: //fallthrough
        case CTUNE_UI_PANEL_SEARCH: {
            ctune_UI_openSelectedStationInformation( ui.cache.curr_panel );
        } break;

        case CTUNE_UI_PANEL_BROWSER: {
            if( ctune_UI_BrowserWin.isInFocus( &ui.tabs.browser, FOCUS_PANE_RIGHT ) ) {
                ctune_UI_openSelectedStationInformation( ui.cache.curr_panel );
            } else {
                ctune_UI_BrowserWin.navKeyRight( &ui.tabs.browser );
                ctune_UI_BrowserWin.show( &ui.tabs.browser );
            }
        } break;

        default:
            break;
    }
}

/**
 * [PRIVATE] Navigation select for 'PPAGE' key
 * @param tab PanelID of the current tab
 */
static void ctune_UI_navSelectPageUp( ctune_UI_PanelID_e tab ) {
    ctune_UI_clearMsgLine();

    if( tab == CTUNE_UI_PANEL_FAVOURITES ) {
        ctune_UI_RSListWin.selectPageUp( &ui.tabs.favourites );
        ctune_UI_RSListWin.show( &ui.tabs.favourites );
    } else if( tab == CTUNE_UI_PANEL_SEARCH ) {
        ctune_UI_RSListWin.selectPageUp( &ui.tabs.search );
        ctune_UI_RSListWin.show( &ui.tabs.search );
    } else if( tab == CTUNE_UI_PANEL_BROWSER ) {
        ctune_UI_BrowserWin.navKeyPageUp( &ui.tabs.browser );
        ctune_UI_BrowserWin.show( &ui.tabs.browser );
    }
}

/**
 * [PRIVATE] Navigation select for 'NPAGE' key
 * @param tab PanelID of the current tab
 */
static void ctune_UI_navSelectPageDown( ctune_UI_PanelID_e tab ) {
    ctune_UI_clearMsgLine();

    if( tab == CTUNE_UI_PANEL_FAVOURITES ) {
        ctune_UI_RSListWin.selectPageDown( &ui.tabs.favourites );
        ctune_UI_RSListWin.show( &ui.tabs.favourites );
    } else if( tab == CTUNE_UI_PANEL_SEARCH ) {
        ctune_UI_RSListWin.selectPageDown( &ui.tabs.search );
        ctune_UI_RSListWin.show( &ui.tabs.search );
    } else if( tab == CTUNE_UI_PANEL_BROWSER ) {
        ctune_UI_BrowserWin.navKeyPageDown( &ui.tabs.browser );
        ctune_UI_BrowserWin.show( &ui.tabs.browser );
    }
}

/**
 * [PRIVATE] Navigation select for 'HOME' key
 * @param tab PanelID of the current tab
 */
static void ctune_UI_navSelectHome( ctune_UI_PanelID_e tab ) {
    ctune_UI_clearMsgLine();

    if( tab == CTUNE_UI_PANEL_FAVOURITES ) {
        ctune_UI_RSListWin.selectFirst( &ui.tabs.favourites );
        ctune_UI_RSListWin.show( &ui.tabs.favourites );
    } else if( tab == CTUNE_UI_PANEL_SEARCH ) {
        ctune_UI_RSListWin.selectFirst( &ui.tabs.search );
        ctune_UI_RSListWin.show( &ui.tabs.search );
    } else if( tab == CTUNE_UI_PANEL_BROWSER ) {
        ctune_UI_BrowserWin.navKeyHome( &ui.tabs.browser );
        ctune_UI_BrowserWin.show( &ui.tabs.browser );
    }
}

/**
 * [PRIVATE] Navigation select for 'END' key
 * @param tab PanelID of the current tab
 */
static void ctune_UI_navSelectEnd( ctune_UI_PanelID_e tab ) {
    ctune_UI_clearMsgLine();

    if( tab == CTUNE_UI_PANEL_FAVOURITES ) {
        ctune_UI_RSListWin.selectLast( &ui.tabs.favourites );
        ctune_UI_RSListWin.show( &ui.tabs.favourites );
    } else if( tab == CTUNE_UI_PANEL_SEARCH ) {
        ctune_UI_RSListWin.selectLast( &ui.tabs.search );
        ctune_UI_RSListWin.show( &ui.tabs.search );
    } else if( tab == CTUNE_UI_PANEL_BROWSER ) {
        ctune_UI_BrowserWin.navKeyEnd( &ui.tabs.browser );
        ctune_UI_BrowserWin.show( &ui.tabs.browser );
    }
}

/**
 * [PRIVATE] Navigation for 'TAB' key
 * @param tab PanelID of the current tab
 */
static void ctune_UI_navSwitchFocus( ctune_UI_PanelID_e tab ) {
    if( tab == CTUNE_UI_PANEL_BROWSER ) {
        ctune_UI_BrowserWin.switchFocus( &ui.tabs.browser );
        ctune_UI_BrowserWin.show( &ui.tabs.browser );
    }
}

/**
 * [PRIVATE] Prints the tab menu for a panel
 * @param curr_panel Panel to print the tab menu for and highlight within
 * @param end_col    Var pointer for storing col width of the tab menu (optional)
 */
static void ctune_UI_printTabMenu( enum ctune_UI_PanelID curr_panel, int * end_col ) {
    const int    tabs[3]   = {
        [0] = CTUNE_UI_PANEL_FAVOURITES,
        [1] = CTUNE_UI_PANEL_SEARCH,
        [2] = CTUNE_UI_PANEL_BROWSER
    };
    const char * titles[3] = {
        [0] = ctune_UI_Language.text( CTUNE_UI_TEXT_WIN_TITLE_FAV ),
        [1] = ctune_UI_Language.text( CTUNE_UI_TEXT_WIN_TITLE_SEARCH ),
        [2] = ctune_UI_Language.text( CTUNE_UI_TEXT_WIN_TITLE_BROWSE )
    };
    const int row = 0; //menu location
    int       col = 1; //starting column

    mvwprintw( ui.panel_windows[curr_panel], 0, (col++), "|" );

    for( int i = 0; i < 3; ++i ) {
        size_t width = strlen( titles[i] ) + 2;;

        if( (int) curr_panel == tabs[i] ) {
            wattron( ui.panel_windows[curr_panel], ctune_UI_Theme.color( CTUNE_UI_ITEM_TAB_CURR ) );
            mvwprintw( ui.panel_windows[curr_panel], row, col, " %s ", titles[i] );
            wattroff( ui.panel_windows[curr_panel], ctune_UI_Theme.color( CTUNE_UI_ITEM_TAB_CURR ) );
        } else {
            mvwprintw( ui.panel_windows[curr_panel], row, col, " %s ", titles[i] );
        }

        col += ( (int) width );

        mvwprintw( ui.panel_windows[curr_panel], row, (col++), "|" );

        if( end_col != NULL && *end_col < col )
            *end_col = col;
    }

    refresh();
}

/**
 * [PRIVATE] Creates all the main UI panels
 */
static void ctune_UI_createPanels( void ) {
    int tab_menu_end_col = 0;

    for( int i = 0; i < CTUNE_UI_PANEL_COUNT; ++i ) {
        switch( i ) {
            case CTUNE_UI_PANEL_TITLE: {
                ui.panel_windows[i] = newwin( ui.size.title_bar.rows,
                                              ui.size.title_bar.cols,
                                              ui.size.title_bar.pos_y,
                                              ui.size.title_bar.pos_x );
                wbkgd( ui.panel_windows[i], ctune_UI_Theme.color( CTUNE_UI_ITEM_TITLE ) );
                mvwprintw( ui.panel_windows[i], 0, 1, ctune_UI_Language.text( CTUNE_UI_TEXT_WIN_TITLE_MAIN ) );
            } break;

            case CTUNE_UI_PANEL_STATUS_1: {
                ui.panel_windows[i] = newwin( ui.size.status_bar1.rows,
                                              ui.size.status_bar1.cols,
                                              ui.size.status_bar1.pos_y,
                                              ui.size.status_bar1.pos_x );
                wbkgd( ui.panel_windows[i], ctune_UI_Theme.color( CTUNE_UI_ITEM_STATUS_BAR1 ) );
            } break;

            case CTUNE_UI_PANEL_STATUS_2: {
                ui.panel_windows[i] = newwin( ui.size.status_bar2.rows,
                                              ui.size.status_bar2.cols,
                                              ui.size.status_bar2.pos_y,
                                              ui.size.status_bar2.pos_x );
                wbkgd( ui.panel_windows[i], ctune_UI_Theme.color( CTUNE_UI_ITEM_STATUS_BAR2 ) );
            } break;

            case CTUNE_UI_PANEL_STATUS_3: {
                ui.panel_windows[i] = newwin( ui.size.status_bar3.rows,
                                              ui.size.status_bar3.cols,
                                              ui.size.status_bar3.pos_y,
                                              ui.size.status_bar3.pos_x );
                wbkgd( ui.panel_windows[i], ctune_UI_Theme.color( CTUNE_UI_ITEM_STATUS_BAR3 ) );
            } break;

            case CTUNE_UI_PANEL_MSG_LINE: {
                ui.panel_windows[i] = newwin( ui.size.msg_line.rows,
                                              ui.size.msg_line.cols,
                                              ui.size.msg_line.pos_y,
                                              ui.size.msg_line.pos_x );
                wbkgd( ui.panel_windows[i], ctune_UI_Theme.color( CTUNE_UI_ITEM_MSG_LINE ) );
            } break;

            case CTUNE_UI_PANEL_FAVOURITES: //fallthrough
            case CTUNE_UI_PANEL_SEARCH    : {
                ui.panel_windows[i] = newwin( ui.size.tab_border.rows,
                                              ui.size.tab_border.cols,
                                              ui.size.tab_border.pos_y,
                                              ui.size.tab_border.pos_x );

                wbkgd( ui.panel_windows[i], ctune_UI_Theme.color( CTUNE_UI_ITEM_TAB_BG ) );
                box( ui.panel_windows[i], 0, 0 );
                ctune_UI_printTabMenu( i, NULL );
            } break;

            case CTUNE_UI_PANEL_BROWSER: {
                ui.panel_windows[i] = newwin( ui.size.tab_border.rows,
                                              ui.size.tab_border.cols,
                                              ui.size.tab_border.pos_y,
                                              ui.size.tab_border.pos_x );

                wbkgd( ui.panel_windows[i], ctune_UI_Theme.color( CTUNE_UI_ITEM_TAB_BG ) );
                box( ui.panel_windows[i], 0, 0 );
                ctune_UI_printTabMenu( i, &tab_menu_end_col );

                { //panel separator line
                    const int pos_x = ( ui.size.browser_left.cols + 1 );

                    if( pos_x >= tab_menu_end_col ) //i.e.: avoid top 'T' of the separator bar if it clashes with the tab menu
                        mvwaddch( ui.panel_windows[i], 0, pos_x, ACS_TTEE );

                    mvwvline( ui.panel_windows[i], 1, pos_x, ACS_VLINE, ( ui.size.tab_canvas.rows ) );
                    mvwaddch( ui.panel_windows[i], ( ui.size.tab_border.rows - 1 ), pos_x, ACS_BTEE );
                }
            } break;

            default: {
                CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_createPanels()] Panel ID (%i) is not recognised - panel case may not have been implemented.", i );
            } break;
        }

        ui.panels[i] = new_panel( ui.panel_windows[i] );
    }

    CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_UI_createPanels()] Main UI panels created." );
}

/**
 * [PRIVATE] De-allocates all main UI panels
 */
static void ctune_UI_destroyPanels( void ) {
    for( int i = 0; i < CTUNE_UI_PANEL_COUNT; ++i ) {
        if( ui.panels[ i ] ) {
            del_panel( ui.panels[ i ] );
            ui.panels[ i ] = NULL;
        }
        if( ui.panel_windows[ i ] ) {
            delwin( ui.panel_windows[ i ] );
            ui.panel_windows[ i ] = NULL;
        }
    }

    CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_UI_destroyPanels()] Main UI panels freed." );
}

/**
 * [PRIVATE] Runs the key interaction interface
 */
static void ctune_UI_runKeyInterfaceLoop() {
    int  character;
    bool exit = false;

    while( !exit ) {
        character = getch();

        switch( ctune_UI_KeyBinding.getAction( ui.curr_ctx, character ) ) {
            case CTUNE_UI_ACTION_RESIZE       : { ctune_UI_Resizer.resize();                                      } break;
            case CTUNE_UI_ACTION_FIND         : { ctune_UI_openFindDialog();                                      } break;
            case CTUNE_UI_ACTION_NEW          : { ctune_UI_openNewStationDialog( ui.cache.curr_panel );           } break;
            case CTUNE_UI_ACTION_EDIT         : { ctune_UI_openEditSelectedStationDialog( ui.cache.curr_panel );  } break;
            case CTUNE_UI_ACTION_RSI_SELECTED : { ctune_UI_openSelectedStationInformation( ui.cache.curr_panel ); } break;
            case CTUNE_UI_ACTION_OPTIONS      : { ctune_UI_openOptionsMenuDialog( ui.cache.curr_panel );          } break;
            case CTUNE_UI_ACTION_GO_RIGHT     : { ctune_UI_navSelectRight( ui.cache.curr_panel );                 } break;
            case CTUNE_UI_ACTION_GO_LEFT      : { ctune_UI_navSelectLeft( ui.cache.curr_panel );                  } break;
            case CTUNE_UI_ACTION_SELECT_PREV  : { ctune_UI_navSelectUp( ui.cache.curr_panel );                    } break;
            case CTUNE_UI_ACTION_SELECT_NEXT  : { ctune_UI_navSelectDown( ui.cache.curr_panel );                  } break;
            case CTUNE_UI_ACTION_PAGE_UP      : { ctune_UI_navSelectPageUp( ui.cache.curr_panel );                } break;
            case CTUNE_UI_ACTION_PAGE_DOWN    : { ctune_UI_navSelectPageDown( ui.cache.curr_panel );              } break;
            case CTUNE_UI_ACTION_SELECT_FIRST : { ctune_UI_navSelectHome( ui.cache.curr_panel );                  } break;
            case CTUNE_UI_ACTION_SELECT_LAST  : { ctune_UI_navSelectEnd( ui.cache.curr_panel );                   } break;
            case CTUNE_UI_ACTION_FOCUS_LEFT   : //fallthrough
            case CTUNE_UI_ACTION_FOCUS_RIGHT  : { ctune_UI_navSwitchFocus( ui.cache.curr_panel );                 } break;

            case CTUNE_UI_ACTION_HELP: {
                ctune_UI_clearMsgLine();
                ctune_UI_ContextHelp.show( ui.curr_ctx );
                ctune_UI_ContextHelp.captureInput();
            } break;

            case CTUNE_UI_ACTION_GO_BACK: {
                ctune_UI_clearMsgLine();
                ctune_UI.show( ui.cache.prev_panel );
            } break;

            case CTUNE_UI_ACTION_VOLUP: {
                ctune_UI_clearMsgLine();
                if( ctune_Controller.playback.getPlaybackState() == true )
                    ctune_Controller.playback.modifyVolume( +5 );
                else
                    ctune_UI.printVolume( ui.cb.quietVolChangeCallback( +5 ) );
            } break;

            case CTUNE_UI_ACTION_VOLDOWN: {
                ctune_UI_clearMsgLine();
                if( ctune_Controller.playback.getPlaybackState() == true )
                    ctune_Controller.playback.modifyVolume( -5 );
                else
                    ctune_UI.printVolume( ui.cb.quietVolChangeCallback( -5 ) );
            } break;

            case CTUNE_UI_ACTION_TRIGGER: { //action on selected
                ctune_UI_clearMsgLine();
                ctune_UI_navEnter( ui.cache.curr_panel );
            } break;

            case CTUNE_UI_ACTION_PLAY: {
                ctune_UI_clearMsgLine();
                ctune_UI_playSelectedStation( ui.cache.curr_panel );
            } break;

            case CTUNE_UI_ACTION_STOP: {
                ctune_UI_clearMsgLine();
                ctune_Controller.playback.stop();
            } break;

            case CTUNE_UI_ACTION_RESUME: { //play currently queued up station in ui.current_radio_station
                ctune_UI_clearMsgLine();
                if( ui.cache.curr_radio_station != NULL ) {
                    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_runKeyInterfaceLoop()] initiating queued RSI playback..." );
                    if( !ctune_Controller.playback.start( ui.cache.curr_radio_station ) ) {
                        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_runKeyInterfaceLoop()] Failed playback init." );
                    }
                } else {
                    ctune_UI.printStatusMsg( ctune_UI_Language.text( CTUNE_UI_TEXT_MSG_EMPTY_QUEUE ) );
                }
            } break;

            case CTUNE_UI_ACTION_QUIT: {
                ctune_UI_writeToMsgLine( ctune_UI_Language.text( CTUNE_UI_TEXT_MSG_CONFIRM_QUIT ) );

                if( ( character = getch() ) == 'y' || character == 'q' )
                    exit = true;

                ctune_UI_clearMsgLine();
            } break;

            case CTUNE_UI_ACTION_TAB1: { //goto "Favourites" tab
                ctune_UI_clearMsgLine();
                ctune_UI.show( CTUNE_UI_PANEL_FAVOURITES );
            } break;

            case CTUNE_UI_ACTION_TAB2: { //goto "Search" tab
                ctune_UI_clearMsgLine();
                ctune_UI.show( CTUNE_UI_PANEL_SEARCH );
            } break;

            case CTUNE_UI_ACTION_TAB3: { //goto "Browser" tab
                ctune_UI_clearMsgLine();
                ctune_UI.show( CTUNE_UI_PANEL_BROWSER );
            } break;

            case CTUNE_UI_ACTION_RSI_QUEUED: { //Opens the station information window for the currently queued/playing station
                ctune_UI_clearMsgLine();
                if( ui.cache.curr_radio_station != NULL ) {
                    ctune_UI_RSInfo.show( &ui.dialogs.rsinfo, ctune_UI_Language.text( CTUNE_UI_TEXT_WIN_TITLE_RSINFO_QUEUED ), ui.cache.curr_radio_station );
                    ctune_UI_RSInfo.captureInput( &ui.dialogs.rsinfo );
                }
            } break;

            case CTUNE_UI_ACTION_FAV: { //Toggle favourite state on currently selected station
                ctune_UI_clearMsgLine();
                ctune_UI_toggleFavourite( ui.cache.curr_panel );
            } break;

            default:
                break;
        }
//        mvprintw( 2, 2, "%s %d", keyname( ch ), ch );
//        refresh();
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
    CTUNE_LOG( CTUNE_LOG_MSG, "[ctune_UI_init( %i )] Initialising the UI.", show_cursor );

    ui.old_cursor = curs_set( 0 );

    if( !ctune_UI_KeyBinding.init() ) {
        CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_UI_setup( %i )] Failed key binding init.", show_cursor );
        ctune_err.set( CTUNE_ERR_UI );
        return false; //EARLY RETURN

    } else {
        ui.init_stages[CTUNE_UI_INITSTAGE_KEYBINDS] = true;
    }

    if( ( stdscr = initscr() ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_UI_setup( %i )] Failed `initscr()`.", show_cursor );
        ctune_err.set( CTUNE_ERR_UI );
        return false; //EARLY RETURN

    } else {
        ui.init_stages[CTUNE_UI_INITSTAGE_STDSCR] = true;
    }

    ctune_UI_calculateWinSizes( &ui.size );
    ctune_UI_Resizer.init();

    if( !getenv("ESCDELAY") ) {
        set_escdelay( 25 );
    }

    if( has_colors() == false ) {
        CTUNE_LOG( CTUNE_LOG_WARNING, "[ctune_UI_init( %i )] Terminal does not support colours.", show_cursor );
        ctune_err.set( CTUNE_ERR_UI );
        return false; //EARLY RETURN

    } else {
        ctune_UI_Theme.init( ctune_Controller.cfg.getUiTheme() );
        ui.init_stages[CTUNE_UI_INITSTAGE_THEME] = true;
    }

    cbreak();
    noecho();
    keypad( stdscr, TRUE );

    // CONTEXTUAL HELP POPUP DIALOG
    if( !ctune_UI_ContextHelp.init( &ui.size.screen, ctune_UI_Language.text ) ) {
        ctune_err.set( CTUNE_ERR_UI );
        CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_UI_init( %i )] Could not init contextual help.", show_cursor );
        return false; //EARLY RETURN

    } else {
        ui.init_stages[CTUNE_UI_INITSTAGE_CTXHELP] = true;
    }

    // FIND RADIO STATION FORM
    ui.dialogs.rsfind = ctune_UI_RSFind.create( &ui.size.screen, ctune_UI_Language.text );
    if( !ctune_UI_RSFind.init( &ui.dialogs.rsfind ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_UI_init( %i )] Could not init RSFind dialog.", show_cursor );
        ctune_err.set( CTUNE_ERR_UI );
        return false; //EARLY RETURN

    } else {
        ui.init_stages[CTUNE_UI_INITSTAGE_RSFIND] = true;
    }

    // CREATE/EDIT LOCAL RADIO STATION FORM
    ui.dialogs.rsedit = ctune_UI_RSEdit.create( &ui.size.screen,
                                                ctune_UI_Language.text,
                                                ctune_UI_generateLocalUUID,
                                                ctune_Controller.playback.testStream,
                                                ctune_Controller.playback.validateURL );
    if( !ctune_UI_RSEdit.init( &ui.dialogs.rsedit ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_UI_init( %i )] Could not init RSEdit dialog.", show_cursor );
        ctune_err.set( CTUNE_ERR_UI );
        return false; //EARLY RETURN

    } else {
        ui.init_stages[CTUNE_UI_INITSTAGE_RSEDIT] = true;
    }

    // RADIO STATION INFO DIALOG
    ui.dialogs.rsinfo = ctune_UI_RSInfo.create( &ui.size.screen, ctune_UI_Language.text, ": " );
    if( !ctune_UI_RSInfo.init( &ui.dialogs.rsinfo ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_UI_init( %i )] Could not init RSInfo_t dialog.", show_cursor );
        ctune_err.set( CTUNE_ERR_UI );
        return false; //EARLY RETURN

    } else {
        ui.init_stages[CTUNE_UI_INITSTAGE_RSINFO] = true;
    }

    // OPTIONS MENU DIALOG (test build)
    ui.dialogs.optmenu = ctune_UI_OptionsMenu.create( &ui.size.screen, ui.cache.curr_panel, ctune_UI_Language.text );
    ctune_UI_OptionsMenu.cb.setSortStationListCallback( &ui.dialogs.optmenu, ctune_UI_sortStationList );
    ctune_UI_OptionsMenu.cb.setAddNewStationCallback( &ui.dialogs.optmenu, ctune_UI_openNewStationDialog );
    ctune_UI_OptionsMenu.cb.setEditStationCallback( &ui.dialogs.optmenu, ctune_UI_openEditSelectedStationDialog );
    ctune_UI_OptionsMenu.cb.setToggleFavouriteCallback( &ui.dialogs.optmenu, ctune_UI_toggleFavourite );
    ctune_UI_OptionsMenu.cb.setSyncCurrSelectedStationCallback( &ui.dialogs.optmenu, ctune_UI_syncRemoteStation );
    ctune_UI_OptionsMenu.cb.setFavouriteTabThemingCallback( &ui.dialogs.optmenu, ctune_UI_setFavouriteTabTheming );
    ctune_UI_OptionsMenu.cb.setListRowSizeLargeCallback( &ui.dialogs.optmenu, ctune_UI_setCurrListRowSize );

    if( !ctune_UI_OptionsMenu.init( &ui.dialogs.optmenu ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_UI_init( %i )] Could not init OptionsMenu_t dialog.", show_cursor );
        ctune_err.set( CTUNE_ERR_UI );
        return false; //EARLY RETURN

    } else {
        ui.init_stages[CTUNE_UI_INITSTAGE_OPTMENU] = true;
    }

    ctune_UI_createPanels();

    ctune_UIConfig_t * ui_config = ctune_Controller.cfg.getUIConfig();

    { //TAB:FAVOURITE
        ui.tabs.favourites = ctune_UI_RSListWin.init( &ui.size.tab_canvas,
                                                      ctune_UI_Language.text,
                                                      NULL, /* no ctrl row so no need for ctrl callback */
                                                      ctune_Controller.cfg.toggleFavourite,
                                                      ctune_UI_getStationState );

        ctune_UI_RSListWin.showCtrlRow( &ui.tabs.favourites, false );
        ctune_UI_RSListWin.setLargeRow( &ui.tabs.favourites, ctune_UIConfig.fav_tab.largeRowSize( ui_config, FLAG_GET_VALUE ) );
        ctune_UI_RSListWin.themeFavourites( &ui.tabs.favourites, ctune_UIConfig.fav_tab.theming( ui_config, FLAG_GET_VALUE ) );

        ui.cache.favourites = Vector.init( sizeof( ctune_RadioStationInfo_t ), ctune_RadioStationInfo.freeContent );
    }

    { //TAB:SEARCH
        ui.tabs.search = ctune_UI_RSListWin.init( &ui.size.tab_canvas,
                                                  ctune_UI_Language.text,
                                                  ctune_Controller.search.getStations,
                                                  ctune_Controller.cfg.toggleFavourite,
                                                  ctune_UI_getStationState );

        ctune_UI_RSListWin.showCtrlRow( &ui.tabs.search, true );
        ctune_UI_RSListWin.setLargeRow( &ui.tabs.search, ctune_UIConfig.search_tab.largeRowSize( ui_config, FLAG_GET_VALUE ) );
        ctune_UI_RSListWin.themeFavourites( &ui.tabs.search, true );
    }

    { //TAB:BROWSER
        ui.tabs.browser = ctune_UI_BrowserWin.init( &ui.size.browser_left,
                                                    &ui.size.browser_right,
                                                    ctune_UI_Language.text,
                                                    ctune_Controller.search.getStations,
                                                    ctune_Controller.search.getCategoryItems,
                                                    ctune_Controller.search.getStationsBy,
                                                    ctune_Controller.cfg.toggleFavourite,
                                                    ctune_UI_getStationState );

        ctune_UI_BrowserWin.showCtrlRow( &ui.tabs.browser, true );
        ctune_UI_BrowserWin.setLargeRow( &ui.tabs.browser, ctune_UIConfig.browse_tab.largeRowSize( ui_config, FLAG_GET_VALUE ) );
        ctune_UI_BrowserWin.themeFavourites( &ui.tabs.browser, true );
        ctune_UI_BrowserWin.populateRootMenu( &ui.tabs.browser );
    }

    ui.init_stages[CTUNE_UI_INITSTAGE_WINDOWS ] = true;
    ui.init_stages[CTUNE_UI_INITSTAGE_COMPLETE] = true;

    ui.cache.prev_panel = ui.cache.curr_panel = CTUNE_UI_PANEL_FAVOURITES;
    ctune_UI.show( CTUNE_UI_PANEL_FAVOURITES );

//    timeout( 0 );
//    raw();
//    nonl();

    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_UI_init( %i )] Init = [%i][%i][%i][%i][%i][%i][%i][%i][%i][%i]",
               show_cursor,
               ui.init_stages[CTUNE_UI_INITSTAGE_KEYBINDS],
               ui.init_stages[CTUNE_UI_INITSTAGE_STDSCR],
               ui.init_stages[CTUNE_UI_INITSTAGE_THEME],
               ui.init_stages[CTUNE_UI_INITSTAGE_CTXHELP],
               ui.init_stages[CTUNE_UI_INITSTAGE_RSFIND],
               ui.init_stages[CTUNE_UI_INITSTAGE_RSEDIT],
               ui.init_stages[CTUNE_UI_INITSTAGE_RSINFO],
               ui.init_stages[CTUNE_UI_INITSTAGE_OPTMENU],
               ui.init_stages[CTUNE_UI_INITSTAGE_WINDOWS],
               ui.init_stages[CTUNE_UI_INITSTAGE_COMPLETE]
    );

    CTUNE_LOG( CTUNE_LOG_MSG, "[ctune_UI_init( %i )] UI Initialised successfully.", show_cursor );

    refresh();
    return true;
}

/**
 * Start the UI loop
 */
static void ctune_UI_start( void ) {
    ctune_UI.show( CTUNE_UI_PANEL_FAVOURITES );
    ctune_UI_Resizer.push( ctune_UI.resize, NULL );
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

    if( ui.init_stages[CTUNE_UI_INITSTAGE_CTXHELP] ) {
        ctune_UI_ContextHelp.free();
        ui.init_stages[CTUNE_UI_INITSTAGE_CTXHELP] = false;
    }


    if( ui.init_stages[CTUNE_UI_INITSTAGE_WINDOWS] ) {
        ctune_UI_RSListWin.free( &ui.tabs.favourites );
        ctune_UI_RSListWin.free( &ui.tabs.search );
        ctune_UI_BrowserWin.free( &ui.tabs.browser );

        ctune_UI_destroyPanels();

        ui.init_stages[CTUNE_UI_INITSTAGE_WINDOWS] = false;
    }

    { //Cache
        if( ui.cache.curr_radio_station != NULL ) {
            ctune_RadioStationInfo.freeContent( ui.cache.curr_radio_station );
            free( ui.cache.curr_radio_station );
            ui.cache.curr_radio_station = NULL;
            ui.cache.curr_radio_station_hash = 0;
        }

        Vector.clear_vector( &ui.cache.favourites );
        String.free( &ui.cache.curr_song );
    }

    ctune_UI_Resizer.free();

    delwin( stdscr );
    endwin();
    curs_set( ui.old_cursor );

    ui.init_stages[CTUNE_UI_INITSTAGE_STDSCR  ] = false;
    ui.init_stages[CTUNE_UI_INITSTAGE_THEME   ] = false;
    ui.init_stages[CTUNE_UI_INITSTAGE_KEYBINDS] = false;

    ui.init_stages[CTUNE_UI_INITSTAGE_COMPLETE] = false;

    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_UI_teardown()] Init = [%i][%i][%i][%i][%i][%i][%i][%i][%i][%i]",
               ui.init_stages[CTUNE_UI_INITSTAGE_KEYBINDS],
               ui.init_stages[CTUNE_UI_INITSTAGE_STDSCR],
               ui.init_stages[CTUNE_UI_INITSTAGE_THEME],
               ui.init_stages[CTUNE_UI_INITSTAGE_CTXHELP],
               ui.init_stages[CTUNE_UI_INITSTAGE_RSFIND],
               ui.init_stages[CTUNE_UI_INITSTAGE_RSEDIT],
               ui.init_stages[CTUNE_UI_INITSTAGE_RSINFO],
               ui.init_stages[CTUNE_UI_INITSTAGE_OPTMENU],
               ui.init_stages[CTUNE_UI_INITSTAGE_WINDOWS],
               ui.init_stages[CTUNE_UI_INITSTAGE_COMPLETE]
    );

    CTUNE_LOG( CTUNE_LOG_MSG, "[ctune_UI_teardown()] UI shutdown." );
}

/**
 * Reconstruct the UI for when window sizes change
 */
static void ctune_UI_resize() {
    CTUNE_LOG( CTUNE_LOG_MSG, "[ctune_UI_resize()] Resize event called." );

    endwin();
    ctune_UI_destroyPanels();
    ctune_UI_calculateWinSizes( &ui.size );
    ctune_UI_createPanels();

    switch( ui.cache.curr_panel ) {
        case CTUNE_UI_PANEL_FAVOURITES: {
            ctune_UI_RSListWin.resize( &ui.tabs.search );
            ctune_UI_BrowserWin.resize( &ui.tabs.browser );
            top_panel( ui.panels[CTUNE_UI_PANEL_FAVOURITES] );
            ctune_UI_RSListWin.resize( &ui.tabs.favourites );
        } break;

        case CTUNE_UI_PANEL_SEARCH: {
            ctune_UI_RSListWin.resize( &ui.tabs.favourites );
            ctune_UI_BrowserWin.resize( &ui.tabs.browser );
            top_panel( ui.panels[CTUNE_UI_PANEL_SEARCH] );
            ctune_UI_RSListWin.resize( &ui.tabs.search );
        } break;

        case CTUNE_UI_PANEL_BROWSER: {
            ctune_UI_RSListWin.resize( &ui.tabs.favourites );
            ctune_UI_RSListWin.resize( &ui.tabs.search );
            top_panel( ui.panels[CTUNE_UI_PANEL_BROWSER] );
            ctune_UI_BrowserWin.resize( &ui.tabs.browser );
        } break;

        default:
            break;
    }

    { //reload field contents
        ctune_UI.printPlaybackState( ui.cache.playback_state );
        ctune_UI.printVolume( ui.cache.curr_vol );

        //note: needs to copy as the `printSongInfo( const char * )` sets `ui.cache.curr_song` and
        //      ends up corrupting that variable otherwise (circular reference because of the pointer)
        if( !String.empty( &ui.cache.curr_song ) ) {
            String_t cp = String.init();
            String.copy( &cp, &ui.cache.curr_song );
            ctune_UI.printSongInfo( cp._raw );
            String.free( &cp );
        }
    }

    wrefresh( ui.panel_windows[ui.cache.curr_panel] );
    update_panels();
    doupdate();
    refresh();
}

/**
 * Move a particular pane to the top
 * @param pane Pane index
 */
static void ctune_UI_show( enum ctune_UI_PanelID panel_index ) {
    if( panel_index == CTUNE_UI_PANEL_COUNT )
        return;

    ui.cache.prev_panel = ui.cache.curr_panel;

    switch( panel_index ) {
        case CTUNE_UI_PANEL_FAVOURITES: {
            top_panel( ui.panels[CTUNE_UI_PANEL_FAVOURITES] );
            ui.cache.curr_panel = panel_index;
            ui.curr_ctx         = CTUNE_UI_CTX_FAV_TAB;
            ctune_Controller.cfg.getListOfFavourites( &ui.cache.favourites );
            ctune_UI_RSListWin.loadResults( &ui.tabs.favourites, &ui.cache.favourites, NULL );
            ctune_UI_RSListWin.show( &ui.tabs.favourites );
        } break;

        case CTUNE_UI_PANEL_SEARCH: {
            top_panel( ui.panels[CTUNE_UI_PANEL_SEARCH] );
            ui.cache.curr_panel = panel_index;
            ui.curr_ctx         = CTUNE_UI_CTX_SEARCH_TAB;
            ctune_UI_RSListWin.setRedraw( &ui.tabs.search );
            ctune_UI_RSListWin.show( &ui.tabs.search );
        } break;

        case CTUNE_UI_PANEL_BROWSER: {
            top_panel( ui.panels[CTUNE_UI_PANEL_BROWSER] );
            ui.cache.curr_panel = panel_index;
            ui.curr_ctx         = CTUNE_UI_CTX_BROWSE_TAB;
            ctune_UI_BrowserWin.setRedraw( &ui.tabs.browser, false );
            ctune_UI_BrowserWin.show( &ui.tabs.browser );
        } break;

        default:
            break;
    }

    wrefresh( ui.panel_windows[panel_index] );
    update_panels();
    doupdate();
    refresh();
}

/**
 * Signals a radio station as 'current' (i.e. as queued or playing)
 * @param rsi Pointer to RadioStationInfo DTO
 */
static void ctune_UI_setCurrStation( const ctune_RadioStationInfo_t * rsi ) {
    if( !ui.init_stages[CTUNE_UI_INITSTAGE_COMPLETE] )
        return; //EARLY RETURN

    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_UI_setCurrStation( %p )] Setting \"%s\" as current radio station (Source: %s, UUID: %s).",
               rsi,
               ( rsi != NULL ? ctune_RadioStationInfo.get.stationName( rsi ) : "NULL" ),
               ( rsi != NULL ? ctune_StationSrc.str( ctune_RadioStationInfo.get.stationSource( rsi ) ) : "N/A" ),
               ( rsi != NULL ? ctune_RadioStationInfo.get.stationUUID( rsi ) : "N/A" )
    );

    if( ui.cache.curr_radio_station == NULL ) {
        if( ( ui.cache.curr_radio_station = malloc( sizeof( ctune_RadioStationInfo_t ) ) ) == NULL ) {
            CTUNE_LOG( CTUNE_LOG_FATAL,
                       "[ctune_UI_setCurrStation( %p )] "
                       "Failed memory allocation of current RadioStationInfo.",
                       rsi );

            return; //EARLY RETURN
        }

        ctune_RadioStationInfo.init( ui.cache.curr_radio_station );
    }

    if( rsi != ui.cache.curr_radio_station ) {
        ctune_RadioStationInfo.copy( rsi, ui.cache.curr_radio_station );
        ui.cache.curr_radio_station_hash = ctune_RadioStationInfo.hash( ctune_RadioStationInfo.get.stationUUID( rsi ) );

        CTUNE_LOG( CTUNE_LOG_TRACE,
                   "[ctune_UI_setCurrStation( %p )] hash = %lu (Source: %s, UUID: %s).",
                   rsi, ui.cache.curr_radio_station_hash,
                   ctune_StationSrc.str( ctune_RadioStationInfo.get.stationSource( rsi ) ), ctune_RadioStationInfo.get.stationUUID( rsi )
        );
    }

    if( rsi != NULL ) {
        ctune_UI_clearPanel( CTUNE_UI_PANEL_STATUS_2 );
        mvwprintw( ui.panel_windows[CTUNE_UI_PANEL_STATUS_2], 0, 0, "%s", ctune_RadioStationInfo.get.stationName( ui.cache.curr_radio_station ) );
        wrefresh( ui.panel_windows[CTUNE_UI_PANEL_STATUS_2] );
        update_panels();
        refresh();
        doupdate();
    }
}

/**
 * Prints a string inside the area reserved for song descriptions
 * @param str String to display on screen
 */
static void ctune_UI_printSongInfo( const char * str ) {
    if( !ui.init_stages[CTUNE_UI_INITSTAGE_COMPLETE] )
        return; //EARLY RETURN

    ctune_UI_clearPanel( CTUNE_UI_PANEL_STATUS_2 );

    const ctune_RadioStationInfo_t * curr_rsi = ui.cache.curr_radio_station;

    if( strlen( ctune_RadioStationInfo.get.stationName( curr_rsi ) ) > INT_MAX ) { //should not happen but you never know...
        CTUNE_LOG( CTUNE_LOG_WARNING,
                   "[ctune_UI_printSongInfo( %s )] station name size > INT_MAX!! Not gonna print this!",
                   str
        );
        mvwprintw( ui.panel_windows[CTUNE_UI_PANEL_STATUS_2], 0, 0, " ... | %s", str );
    } else {
        mvwprintw( ui.panel_windows[CTUNE_UI_PANEL_STATUS_2], 0, 0, "%s | %s", ctune_RadioStationInfo.get.stationName( curr_rsi ), str );
    }

    if( str != NULL ) {
        String.set( &ui.cache.curr_song, str );
    } else {
        String.free( &ui.cache.curr_song );
    }

    update_panels();
    refresh();
    doupdate();
}

/**
 * Prints an integer inside the area reserved to display the current volume
 * @param vol Volume to display on screen
 */
static void ctune_UI_printVolume( const int vol ) {
    if( !ui.init_stages[CTUNE_UI_INITSTAGE_COMPLETE] )
        return; //EARLY RETURN

    ui.cache.curr_vol = vol;
    mvwprintw( ui.panel_windows[CTUNE_UI_PANEL_STATUS_3], 0, 0, "Vol: %3d%%", vol );
    update_panels();
    refresh();
    doupdate();
}

/**
 * Prints the playback state to the screen
 * @param state Playback state
 */
static void ctune_UI_printPlaybackState( const bool state ) {
    if( !ui.init_stages[CTUNE_UI_INITSTAGE_COMPLETE] )
        return; //EARLY RETURN

    ui.cache.playback_state = state;

    wbkgd( ui.panel_windows[CTUNE_UI_PANEL_STATUS_1],
           ( state
             ? ctune_UI_Theme.color( CTUNE_UI_ITEM_PLAYBACK_ON )
             : ctune_UI_Theme.color( CTUNE_UI_ITEM_PLAYBACK_OFF )
           )
    );

    mvwprintw( ui.panel_windows[CTUNE_UI_PANEL_STATUS_1], 0, 1, "%c", ( state ? '>' : '.' ) );
    update_panels();
    refresh();
    doupdate();
}

/**
 * Prints an error description string to the screen
 * @param err_str Error string
 * @param err_no  Error number (cTune specific)
 */
static void ctune_UI_printError( const char * error_str, int err_no ) {
    if( !ui.init_stages[CTUNE_UI_INITSTAGE_COMPLETE] )
        return; //EARLY RETURN

    wclear( ui.panel_windows[ CTUNE_UI_PANEL_MSG_LINE ] );
    if( strlen( error_str ) > 0 ) {
        mvwprintw( ui.panel_windows[ CTUNE_UI_PANEL_MSG_LINE ], 0, 0, "%s%s", ( err_no > CTUNE_ERR_ACTION ? "" : "Error: " ), error_str );
        wrefresh( ui.panel_windows[ CTUNE_UI_PANEL_MSG_LINE ] );
    }
}

/**
 * Prints a message string to the screen
 * @param info_str Information string
 */
static void ctune_UI_printStatusMsg( const char * info_str ) {
    if( !ui.init_stages[CTUNE_UI_INITSTAGE_COMPLETE] )
        return; //EARLY RETURN

    werase( ui.panel_windows[ CTUNE_UI_PANEL_MSG_LINE ] );
    mvwprintw( ui.panel_windows[ CTUNE_UI_PANEL_MSG_LINE ], 0, 0, "%s", info_str );
}

/**
 * Prints the search state to the screen
 * @param state Search state
 */
static void ctune_UI_printSearchingState( const bool state ) {
    if( !ui.init_stages[CTUNE_UI_INITSTAGE_COMPLETE] )
        return; //EARLY RETURN

    static const int row = 0;
    static const int col = 45;
    mvwprintw( ui.panel_windows[CTUNE_UI_PANEL_TITLE], row, col, "%s", ( state ? "Searching" : "Idle" ) );
    refresh();
    doupdate();
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
    .show                      = &ctune_UI_show,
    .setCurrStation            = &ctune_UI_setCurrStation,
    .printSongInfo             = &ctune_UI_printSongInfo,
    .printVolume               = &ctune_UI_printVolume,
    .printPlaybackState        = &ctune_UI_printPlaybackState,
    .printSearchingState       = &ctune_UI_printSearchingState,
    .printError                = &ctune_UI_printError,
    .printStatusMsg            = &ctune_UI_printStatusMsg,
    .setQuietVolChangeCallback = &ctune_UI_setQuietVolChangeCallback,
};