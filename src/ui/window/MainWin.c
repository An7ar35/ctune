#include "MainWin.h"

#ifdef NO_NCURSESW
    #include <ncurses.h>
    #include <panel.h>
#else
    #include <ncursesw/ncurses.h>
    #include <ncursesw/panel.h>
#endif

#include "../Resizer.h"
#include "../definitions/Language.h"
#include "../definitions/Theme.h"
#include "../definitions/Icons.h"

/**
 * [PRIVATE] Calculates all the main UI window positions on the screen
 * @param screen_size Pointer to object containing screen dimensions
 * @param sizes       Pointer to struct containing all main window sizes to calculate
 */
static void ctune_UI_MainWin_calculateWinSizes( const WindowProperty_t * screen_size, struct ctune_UI_MainWinSizes * sizes ) {
    sizes->title_bar     = (WindowProperty_t) { 1,
                                                screen_size->cols,
                                                0,
                                                0 };
    sizes->status_bar1   = (WindowProperty_t) { 1,
                                                3,
                                                ( screen_size->rows - 2 ),
                                                0 };
    sizes->status_bar3   = (WindowProperty_t) { 1,
                                                10,
                                                ( screen_size->rows - 2 ),
                                                ( screen_size->cols - 10 ) };
    sizes->status_bar2   = (WindowProperty_t) { 1,
                                                ( screen_size->cols - sizes->status_bar3.cols - sizes->status_bar1.cols ),
                                                ( screen_size->rows - 2 ),
                                                sizes->status_bar1.cols };
    sizes->msg_line      = (WindowProperty_t) { 1,
                                                screen_size->cols,
                                                ( screen_size->rows - 1 ),
                                                0 };
    sizes->tab_border    = (WindowProperty_t) { ( screen_size->rows - sizes->title_bar.rows - sizes->status_bar1.rows - sizes->msg_line.rows ),
                                                screen_size->cols,
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

    sizes->browser_line  = (WindowProperty_t) { sizes->tab_canvas.rows,
                                                1,
                                                sizes->tab_canvas.pos_y,
                                                ( sizes->browser_left.cols + 1 ) };

    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_UI_MainWin_calculateWinSizes( %p, %p )] Title          = { %i, %i, %i, %i }",
               screen_size, sizes, sizes->title_bar.rows, sizes->title_bar.cols, sizes->title_bar.pos_y, sizes->title_bar.pos_x );
    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_UI_MainWin_calculateWinSizes( %p, %p )] Status bar #1  = { %i, %i, %i, %i }",
               screen_size, sizes, sizes->status_bar1.rows, sizes->status_bar1.cols, sizes->status_bar1.pos_y, sizes->status_bar1.pos_x );
    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_UI_MainWin_calculateWinSizes( %p, %p )] Status bar #2  = { %i, %i, %i, %i }",
               screen_size, sizes, sizes->status_bar2.rows, sizes->status_bar2.cols, sizes->status_bar2.pos_y, sizes->status_bar2.pos_x );
    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_UI_MainWin_calculateWinSizes( %p, %p )] Status bar #3  = { %i, %i, %i, %i }",
               screen_size, sizes, sizes->status_bar3.rows, sizes->status_bar3.cols, sizes->status_bar3.pos_y, sizes->status_bar3.pos_x );
    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_UI_MainWin_calculateWinSizes( %p, %p )] Message line   = { %i, %i, %i, %i }",
               screen_size, sizes, sizes->msg_line.rows, sizes->msg_line.cols, sizes->msg_line.pos_y, sizes->msg_line.pos_x );
    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_UI_MainWin_calculateWinSizes( %p, %p )] Tab border     = { %i, %i, %i, %i }",
               screen_size, sizes, sizes->tab_border.rows, sizes->tab_border.cols, sizes->tab_border.pos_y, sizes->tab_border.pos_x );
    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_UI_MainWin_calculateWinSizes( %p, %p )] Tab canvas     = { %i, %i, %i, %i }",
               screen_size, sizes, sizes->tab_canvas.rows, sizes->tab_canvas.cols, sizes->tab_canvas.pos_y, sizes->tab_canvas.pos_x );
    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_UI_MainWin_calculateWinSizes( %p, %p )] Browser::left  = { %i, %i, %i, %i }",
               screen_size, sizes, sizes->browser_left.rows, sizes->browser_left.cols, sizes->browser_left.pos_y, sizes->browser_left.pos_x );
    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_UI_MainWin_calculateWinSizes( %p, %p )] Browser::right = { %i, %i, %i, %i }",
               screen_size, sizes, sizes->browser_right.rows, sizes->browser_right.cols, sizes->browser_right.pos_y, sizes->browser_right.pos_x );
    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_UI_MainWin_calculateWinSizes( %p, %p )] Browser::line  = { %i, %i, %i, %i }",
               screen_size, sizes, sizes->browser_line.rows, sizes->browser_line.cols, sizes->browser_line.pos_y, sizes->browser_line.pos_x );

    CTUNE_LOG( CTUNE_LOG_MSG,
               "[ctune_UI_MainWin_calculateWinSizes( %p, %p )] "
               "Main window section sizes calculated based on screen size (%i x %i).",
               screen_size, sizes, screen_size->rows, screen_size->cols
    );
}

/**
 * [PRIVATE] Prints the tab menu for a panel
 * @param main       Pointer to MainWin
 * @param curr_panel Panel to print the tab menu for and highlight within
 * @param end_col    Var pointer for storing col width of the tab menu (optional)
 */
static void ctune_UI_MainWin_printTabMenu( ctune_UI_MainWin_t * main, ctune_UI_PanelID_e curr_panel, int * end_col ) {
    struct Tabs {
        ctune_UI_PanelID_e id;
        const char       * title;
        WindowProperty_t * selector_properties;
    };

    const struct Tabs tabs[3] = {
        { CTUNE_UI_PANEL_FAVOURITES, main->cb.getDisplayText( CTUNE_UI_TEXT_WIN_TITLE_FAV ),    &main->tabs.favourites_tab_selector },
        { CTUNE_UI_PANEL_SEARCH,     main->cb.getDisplayText( CTUNE_UI_TEXT_WIN_TITLE_SEARCH ), &main->tabs.search_tab_selector },
        { CTUNE_UI_PANEL_BROWSER,    main->cb.getDisplayText( CTUNE_UI_TEXT_WIN_TITLE_BROWSE ), &main->tabs.browser_tab_selector },
    };

    const int row = 0; //menu location
    int       col = 1; //starting column

    mvwprintw( main->panel_windows[curr_panel], 0, (col++), "|" );

    for( size_t i = 0; i < 3; ++i ) {
        const size_t width = strlen( tabs[i].title ) + 2;

        *tabs[i].selector_properties = (WindowProperty_t) {
            .rows  = 1,
            .cols  = (int) width,
            .pos_y = main->size.tab_border.pos_y,
            .pos_x = main->size.tab_border.pos_x + col,
        };

        if( curr_panel == tabs[i].id ) {
            wattron( main->panel_windows[curr_panel], ctune_UI_Theme.color( CTUNE_UI_ITEM_TAB_CURR ) );
            mvwprintw( main->panel_windows[curr_panel], row, col, " %s ", tabs[i].title );
            wattroff( main->panel_windows[curr_panel], ctune_UI_Theme.color( CTUNE_UI_ITEM_TAB_CURR ) );
        } else {
            mvwprintw( main->panel_windows[curr_panel], row, col, " %s ", tabs[i].title );
        }

        col += ( (int) width );

        mvwprintw( main->panel_windows[curr_panel], row, (col++), "|" );

        if( end_col != NULL && *end_col < col )
            *end_col = col;
    }
}

/**
 * [PRIVATE] Creates all the main UI panels
 * @param main Pointer to MainWin
 */
static void ctune_UI_MainWin_createPanels( ctune_UI_MainWin_t * main ) {
    int tab_menu_end_col = 0;

    for( int i = 0; i < CTUNE_UI_PANEL_COUNT; ++i ) {
        switch( i ) {
            case CTUNE_UI_PANEL_TITLE: {
                main->panel_windows[i] = newwin( main->size.title_bar.rows,
                                                 main->size.title_bar.cols,
                                                 main->size.title_bar.pos_y,
                                                 main->size.title_bar.pos_x );
                wbkgd( main->panel_windows[i], ctune_UI_Theme.color( CTUNE_UI_ITEM_TITLE ) );
                mvwprintw( main->panel_windows[i], 0, 1, "%s", main->cb.getDisplayText( CTUNE_UI_TEXT_WIN_TITLE_MAIN ) );
            } break;

            case CTUNE_UI_PANEL_STATUS_1: {
                main->panel_windows[i] = newwin( main->size.status_bar1.rows,
                                                 main->size.status_bar1.cols,
                                                 main->size.status_bar1.pos_y,
                                                 main->size.status_bar1.pos_x );
                wbkgd( main->panel_windows[i], ctune_UI_Theme.color( CTUNE_UI_ITEM_STATUS_BAR1 ) );
            } break;

            case CTUNE_UI_PANEL_STATUS_2: {
                main->panel_windows[i] = newwin( main->size.status_bar2.rows,
                                                 main->size.status_bar2.cols,
                                                 main->size.status_bar2.pos_y,
                                                 main->size.status_bar2.pos_x );
                wbkgd( main->panel_windows[i], ctune_UI_Theme.color( CTUNE_UI_ITEM_STATUS_BAR2 ) );
            } break;

            case CTUNE_UI_PANEL_STATUS_3: {
                main->panel_windows[i] = newwin( main->size.status_bar3.rows,
                                                 main->size.status_bar3.cols,
                                                 main->size.status_bar3.pos_y,
                                                 main->size.status_bar3.pos_x );
                wbkgd( main->panel_windows[i], ctune_UI_Theme.color( CTUNE_UI_ITEM_STATUS_BAR3 ) );
            } break;

            case CTUNE_UI_PANEL_MSG_LINE: {
                main->panel_windows[i] = newwin( main->size.msg_line.rows,
                                                 main->size.msg_line.cols,
                                                 main->size.msg_line.pos_y,
                                                 main->size.msg_line.pos_x );
                wbkgd( main->panel_windows[i], ctune_UI_Theme.color( CTUNE_UI_ITEM_MSG_LINE ) );
            } break;

            case CTUNE_UI_PANEL_FAVOURITES: //fallthrough
            case CTUNE_UI_PANEL_SEARCH    : {
                main->panel_windows[i] = newwin( main->size.tab_border.rows,
                                                 main->size.tab_border.cols,
                                                 main->size.tab_border.pos_y,
                                                 main->size.tab_border.pos_x );

                wbkgd( main->panel_windows[i], ctune_UI_Theme.color( CTUNE_UI_ITEM_TAB_BG ) );
                box( main->panel_windows[i], 0, 0 );
                ctune_UI_MainWin_printTabMenu( main, i, NULL );
            } break;

            case CTUNE_UI_PANEL_BROWSER: {
                main->panel_windows[i] = newwin( main->size.tab_border.rows,
                                              main->size.tab_border.cols,
                                              main->size.tab_border.pos_y,
                                              main->size.tab_border.pos_x );

                wbkgd( main->panel_windows[i], ctune_UI_Theme.color( CTUNE_UI_ITEM_TAB_BG ) );
                box( main->panel_windows[i], 0, 0 );
                ctune_UI_MainWin_printTabMenu( main, i, &tab_menu_end_col );

                { //panel separator line
                    if( main->size.browser_line.pos_x >= tab_menu_end_col ) { //i.e.: avoid top 'T' of the separator bar if it clashes with the tab menu
                        mvwaddch( main->panel_windows[ i ], 0, main->size.browser_line.pos_x, ACS_TTEE );
                    }

                    mvwvline( main->panel_windows[i], 1, main->size.browser_line.pos_x, ACS_VLINE, ( main->size.browser_line.rows ) );
                    mvwaddch( main->panel_windows[i], ( main->size.tab_border.rows - 1 ), main->size.browser_line.pos_x, ACS_BTEE );
                }
            } break;

            default: {
                CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_MainWin_createPanels()] Panel ID (%i) is not recognised - panel case may not have been implemented.", i );
            } break;
        }

        main->panels[i] = new_panel( main->panel_windows[i] );
    }

    CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_UI_MainWin_createPanels()] Main UI panels created." );
}

/**
 * [PRIVATE] De-allocates all main UI panels
 * @param main Pointer to MainWin
 */
static void ctune_UI_MainWin_destroyPanels( ctune_UI_MainWin_t * main ) {
    for( int i = 0; i < CTUNE_UI_PANEL_COUNT; ++i ) {
        if( main->panels[ i ] ) {
            del_panel( main->panels[ i ] );
            main->panels[ i ] = NULL;
        }
        if( main->panel_windows[ i ] ) {
            delwin( main->panel_windows[ i ] );
            main->panel_windows[ i ] = NULL;
        }
    }

    CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_UI_MainWin_destroyPanels()] Main UI panels freed." );
}

/**
 * [PRIVATE] Clears all content from a panel and refresh to screen
 * @param main  Pointer to MainWin
 * @param panel Panel ID
 */
static void ctune_UI_MainWin_clearPanel( ctune_UI_MainWin_t * main, ctune_UI_PanelID_e panel ) {
    werase( main->panel_windows[panel] );
    update_panels();
    doupdate();
}

/**
 * Creates a MainWin
 * @param getDisplayText Callback method to get UI text strings
 * @param screen_size    Pointer to the screen size property
 * @return MainWin object
 */
static ctune_UI_MainWin_t ctune_UI_MainWin_create( const char * (* getDisplayText)( ctune_UI_TextID_e ), WindowProperty_t * screen_size ) {
    return (ctune_UI_MainWin_t) {
        .screen_property = screen_size,
        .mouse_ctrl      = false,
        .curr_ctx        = CTUNE_UI_CTX_MAIN,
        .panel_windows   = { NULL },
        .panels          = { NULL },

        .size = {
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
        .cb = {
            .getDisplayText         = getDisplayText,
            .playStation            = NULL,
            .getStationState        = NULL,
            .openInfoDialog         = NULL,
        },
        .cache = {
            .prev_panel              = CTUNE_UI_PANEL_FAVOURITES,
            .curr_panel              = CTUNE_UI_PANEL_FAVOURITES,
            .curr_radio_station      = NULL,
            .curr_radio_station_hash = 0,
            .curr_song               = { ._raw = NULL, ._length = 0 },
            .curr_vol                = -1,
            .playback_state          = CTUNE_PLAYBACK_CTRL_OFF,
        },
    };
}

/**
 * Initialises main window
 * @param main            MainWin object
 * @param ui_config       Pointer to the UI configuration
 * @param getStations     Callback method to fetch more stations
 * @param getCatItems     Callback method to fetch station search category items
 * @param getStationsBy   Callback method to fetch stations based on a sub-category
 * @param toggleFavourite Callback method to toggle a station's "favourite" status
 * @return Success
 */
static bool ctune_UI_MainWin_init( ctune_UI_MainWin_t * main,
                                   ctune_UIConfig_t   * ui_config,
                                   bool (* getStations)( ctune_RadioBrowserFilter_t *, Vector_t * ),
                                   bool (* getCatItems)( const ctune_ListCategory_e, const ctune_RadioBrowserFilter_t *, Vector_t * ),
                                   bool (* getStationsBy)( const ctune_ByCategory_e, const char *, Vector_t * ),
                                   bool (* toggleFavourite)( ctune_RadioStationInfo_t *, ctune_StationSrc_e ) )
{
    main->mouse_ctrl = ctune_UIConfig.mouse.enabled( ui_config, FLAG_GET_VALUE );
    ctune_UI_MainWin_calculateWinSizes( main->screen_property, &main->size );
    ctune_UI_MainWin_createPanels( main );

    { //TAB:FAVOURITE
        main->tabs.favourites = ctune_UI_RSListWin.init( &main->size.tab_canvas,
                                                         main->cb.getDisplayText,
                                                         NULL, /* no ctrl row so no need for ctrl callback */
                                                         toggleFavourite,
                                                         main->cb.getStationState );

        ctune_UI_RSListWin.setMouseCtrl( &main->tabs.favourites, main->mouse_ctrl );
        ctune_UI_RSListWin.showCtrlRow( &main->tabs.favourites, false );
        ctune_UI_RSListWin.setLargeRow( &main->tabs.favourites, ctune_UIConfig.fav_tab.largeRowSize( ui_config, FLAG_GET_VALUE ) );
        ctune_UI_RSListWin.themeFavourites( &main->tabs.favourites, ctune_UIConfig.fav_tab.theming( ui_config, FLAG_GET_VALUE ) );

        main->cache.favourites = Vector.init( sizeof( ctune_RadioStationInfo_t ), ctune_RadioStationInfo.freeContent );
    }

    { //TAB:SEARCH
        main->tabs.search = ctune_UI_RSListWin.init( &main->size.tab_canvas,
                                                     main->cb.getDisplayText,
                                                     getStations,
                                                     toggleFavourite,
                                                     main->cb.getStationState );

        ctune_UI_RSListWin.setMouseCtrl( &main->tabs.search, main->mouse_ctrl );
        ctune_UI_RSListWin.showCtrlRow( &main->tabs.search, true );
        ctune_UI_RSListWin.setLargeRow( &main->tabs.search, ctune_UIConfig.search_tab.largeRowSize( ui_config, FLAG_GET_VALUE ) );
        ctune_UI_RSListWin.themeFavourites( &main->tabs.search, true );
    }

    { //TAB:BROWSER
        main->tabs.browser = ctune_UI_BrowserWin.init( &main->size.browser_left,
                                                       &main->size.browser_right,
                                                       main->cb.getDisplayText,
                                                       getStations,
                                                       getCatItems,
                                                       getStationsBy,
                                                       toggleFavourite,
                                                       main->cb.getStationState );

        ctune_UI_BrowserWin.setMouseCtrl( &main->tabs.browser, main->mouse_ctrl );
        ctune_UI_BrowserWin.showCtrlRow( &main->tabs.browser, true );
        ctune_UI_BrowserWin.setLargeRow( &main->tabs.browser, ctune_UIConfig.browse_tab.largeRowSize( ui_config, FLAG_GET_VALUE ) );
        ctune_UI_BrowserWin.themeFavourites( &main->tabs.browser, true );
        ctune_UI_BrowserWin.populateRootMenu( &main->tabs.browser );
    }
}

/**
 * Switch mouse control UI on/off
 * @param main            Pointer to MainWin
 * @param mouse_ctrl_flag Flag to turn feature on/off
 */
static void ctune_UI_MainWin_setMouseCtrl( ctune_UI_MainWin_t * main, bool mouse_ctrl_flag ) {
    main->mouse_ctrl = mouse_ctrl_flag;
}

/**
 * Handles a mouse event
 * @param main  Pointer to MainWin
 * @param event Mouse event
 * @return ActionID associated with mouse event in MainWin
 */
static ctune_UI_ActionID_e ctune_UI_MainWin_handleMouseEvent( ctune_UI_MainWin_t * main, MEVENT * event ) {
    ctune_UI_ActionID_e action = CTUNE_UI_ACTION_NONE;
    ctune_UI_PanelID_e  panel  = CTUNE_UI_PANEL_COUNT;

    if( wenclose( main->panel_windows[CTUNE_UI_PANEL_TITLE], event->y, event->y ) ) {
        panel = CTUNE_UI_PANEL_TITLE;

    } else if( event->y == main->size.tab_border.pos_y ) { //i.e.: on the tab menu line
        if( event->x >= main->tabs.favourites_tab_selector.pos_x &&
            event->x <= ( main->tabs.favourites_tab_selector.pos_x + main->tabs.favourites_tab_selector.cols ) )
        {
            if( event->bstate & BUTTON1_CLICKED || event->bstate & BUTTON1_DOUBLE_CLICKED ) {
                action = CTUNE_UI_ACTION_TAB1;
                ctune_UI_MainWin.print.clearMsgLine( main );
            }

        } else if( event->x >= main->tabs.search_tab_selector.pos_x &&
                   event->x <= ( main->tabs.search_tab_selector.pos_x + main->tabs.search_tab_selector.cols ) )
        {
            if( event->bstate & BUTTON1_CLICKED || event->bstate & BUTTON1_DOUBLE_CLICKED ) {
                action = CTUNE_UI_ACTION_TAB2;
                ctune_UI_MainWin.print.clearMsgLine( main );
            }

        } else if( event->x >= main->tabs.browser_tab_selector.pos_x &&
                   event->x <= ( main->tabs.browser_tab_selector.pos_x + main->tabs.browser_tab_selector.cols ) )
        {
            if( event->bstate & BUTTON1_CLICKED || event->bstate & BUTTON1_DOUBLE_CLICKED ) {
                action = CTUNE_UI_ACTION_TAB3;
                ctune_UI_MainWin.print.clearMsgLine( main );
            }
        }

    } else if( wenclose( main->panel_windows[ main->cache.curr_panel ], event->y, event->x ) ) {
        panel = main->cache.curr_panel;

    } else if( event->y == main->size.status_bar1.pos_y ) { //i.e. on the status bar line
        if( event->x >= main->size.status_bar1.pos_x && event->x <= ( main->size.status_bar1.pos_x + main->size.status_bar1.cols ) ) {
            panel = CTUNE_UI_PANEL_STATUS_1;

        } else if( event->x >= main->size.status_bar2.pos_x && event->x <= ( main->size.status_bar2.pos_x + main->size.status_bar2.cols ) ) {
            panel = CTUNE_UI_PANEL_STATUS_2;

        } else if( event->x >= main->size.status_bar3.pos_x && event->x <= ( main->size.status_bar3.pos_x + main->size.status_bar3.cols ) ) {
            panel = CTUNE_UI_PANEL_STATUS_3;
        }
    }

    if( action != CTUNE_UI_ACTION_NONE ) {
        return action; //EARLY RETURN
    }

    //=============== Trigger the appropriate event ===============

    switch( panel ) {
        case CTUNE_UI_PANEL_TITLE: {
            if( event->bstate & BUTTON2_CLICKED ) {
                action = CTUNE_UI_ACTION_OPTIONS;
            }
        } break;

        case CTUNE_UI_PANEL_STATUS_1: { //Play/Stop icon for currently queued radio station
            if( ( event->bstate & BUTTON1_CLICKED || event->bstate & BUTTON1_DOUBLE_CLICKED ) && main->cache.curr_radio_station != NULL ) {
                action = CTUNE_UI_ACTION_TOGGLE_PLAYBACK;
                ctune_UI_MainWin.print.clearMsgLine( main );
            }
        } break;

        case CTUNE_UI_PANEL_STATUS_2: { //Currently queued radio station
            if( event->bstate & BUTTON1_CLICKED || event->bstate & BUTTON3_CLICKED ) {
                ctune_UI_MainWin.print.clearMsgLine( main );

                if( main->cache.curr_radio_station != NULL ) {
                    action = CTUNE_UI_ACTION_RSI_QUEUED;
                }

            } else if( event->bstate & BUTTON1_DOUBLE_CLICKED && main->cache.curr_radio_station != NULL ) {
                action = CTUNE_UI_ACTION_TOGGLE_PLAYBACK;
                ctune_UI_MainWin.print.clearMsgLine( main );
            }
        } break;

        case CTUNE_UI_PANEL_STATUS_3: { //Volume
            if( event->bstate & BUTTON1_CLICKED ) {
                ctune_UI_MainWin.print.clearMsgLine( main );
                action = CTUNE_UI_ACTION_VOLUP_5;
            } else if( event->bstate & BUTTON1_DOUBLE_CLICKED ) {
                ctune_UI_MainWin.print.clearMsgLine( main );
                action = CTUNE_UI_ACTION_VOLUP_10;
            } else if( event->bstate & BUTTON3_CLICKED ) {
                ctune_UI_MainWin.print.clearMsgLine( main );
                action = CTUNE_UI_ACTION_VOLDOWN_5;
            } else if( event->bstate & BUTTON3_DOUBLE_CLICKED ) {
                ctune_UI_MainWin.print.clearMsgLine( main );
                action = CTUNE_UI_ACTION_VOLDOWN_10;
            }
        } break;

        case CTUNE_UI_PANEL_MSG_LINE: break;

        case CTUNE_UI_PANEL_FAVOURITES: //fallthrough
        case CTUNE_UI_PANEL_SEARCH    : {
            static const long      valid_buttons = ( BUTTON1_CLICKED | BUTTON1_DOUBLE_CLICKED | BUTTON3_CLICKED );
            ctune_UI_RSListWin_t * list_win      = ( panel == CTUNE_UI_PANEL_FAVOURITES ? &main->tabs.favourites : &main->tabs.search );

            if( event->bstate & valid_buttons ) {
                ctune_UI_MainWin.print.clearMsgLine( main );

                const ctune_UI_ScrollMask_m scroll = ctune_UI_RSListWin.isScrollButton( list_win, event->y, event->x );

                if( event->bstate & BUTTON1_CLICKED ) {
                    if( scroll & CTUNE_UI_SCROLL_UP ) {
                        ctune_UI_RSListWin.selectPageUp( list_win );
                    } else if( scroll & CTUNE_UI_SCROLL_DOWN ) {
                        ctune_UI_RSListWin.selectPageDown( list_win );
                    } else {
                        ctune_UI_RSListWin.selectAt( list_win, event->y, event->x );
                    }

                    ctune_UI_RSListWin.show( list_win );

                } else if( event->bstate & BUTTON1_DOUBLE_CLICKED && scroll == 0 ) {
                    ctune_UI_RSListWin.selectAt( list_win, event->y, event->x );
                    ctune_UI_RSListWin.show( list_win );
                    action = CTUNE_UI_ACTION_TRIGGER;

                } else if( event->bstate & BUTTON3_CLICKED ) {
                    if( scroll & CTUNE_UI_SCROLL_UP ) {
                        ctune_UI_RSListWin.selectFirst( list_win );
                        ctune_UI_RSListWin.show( list_win );

                    } else if( scroll & CTUNE_UI_SCROLL_DOWN ) {
                        ctune_UI_RSListWin.selectLast( list_win );
                        ctune_UI_RSListWin.show( list_win );

                    } else {
                        ctune_UI_RSListWin.selectAt( list_win, event->y, event->x );
                        ctune_UI_RSListWin.show( list_win );
                        action = CTUNE_UI_ACTION_RSI_SELECTED;
                    }
                }
            }
        } break;

        case CTUNE_UI_PANEL_BROWSER: {
            static const long valid_buttons = ( BUTTON1_CLICKED | BUTTON1_DOUBLE_CLICKED | BUTTON3_CLICKED );

            if( event->bstate & valid_buttons ) {
                ctune_UI_MainWin.print.clearMsgLine( main );

                const ctune_UI_ScrollMask_m scroll = ctune_UI_BrowserWin.isScrollButton( &main->tabs.browser, event->y, event->x );

                if( event->bstate & BUTTON1_CLICKED ) {
                    if( scroll & CTUNE_UI_SCROLL_UP ) {
                        ctune_UI_BrowserWin.navKeyPageUp( &main->tabs.browser );
                    } else if( scroll & CTUNE_UI_SCROLL_DOWN ) {
                        ctune_UI_BrowserWin.navKeyPageDown( &main->tabs.browser );
                    } else {
                        ctune_UI_BrowserWin.selectAt( &main->tabs.browser, event->y, event->x );
                    }

                    ctune_UI_BrowserWin.show( &main->tabs.browser );

                } else if( event->bstate & BUTTON1_DOUBLE_CLICKED && scroll == 0 ) {
                    ctune_UI_BrowserWin.selectAt( &main->tabs.browser, event->y, event->x );
                    ctune_UI_BrowserWin.show( &main->tabs.browser );
                    action = CTUNE_UI_ACTION_TRIGGER;

                } else if( event->bstate & BUTTON3_CLICKED ) {
                    if( scroll & CTUNE_UI_SCROLL_UP ) {
                        ctune_UI_BrowserWin.navKeyHome( &main->tabs.browser );
                        ctune_UI_BrowserWin.show( &main->tabs.browser );

                    } else if( scroll & CTUNE_UI_SCROLL_DOWN ) {
                        ctune_UI_BrowserWin.navKeyEnd( &main->tabs.browser );
                        ctune_UI_BrowserWin.show( &main->tabs.browser );

                    } else {
                        ctune_UI_BrowserWin.selectAt( &main->tabs.browser, event->y, event->x );
                        ctune_UI_BrowserWin.show( &main->tabs.browser );
                        action = CTUNE_UI_ACTION_GO_RIGHT;
                    }
                }
            }
        } break;

        default: break;
    }

    return action;
}

/**
 * De-allocates resources used by MainWin
 * @param main MainWin object
 */
static void ctune_UI_MainWin_free( ctune_UI_MainWin_t * main ) {
    ctune_UI_RSListWin.free( &main->tabs.favourites );
    ctune_UI_RSListWin.free( &main->tabs.search );
    ctune_UI_BrowserWin.free( &main->tabs.browser );

    ctune_UI_MainWin_destroyPanels( main );

    if( main->cache.curr_radio_station != NULL ) {
        ctune_RadioStationInfo.freeContent( main->cache.curr_radio_station );
        free( main->cache.curr_radio_station );
        main->cache.curr_radio_station      = NULL;
        main->cache.curr_radio_station_hash = 0;
    }

    Vector.clear_vector( &main->cache.favourites );
    String.free( &main->cache.curr_song );
}

/**
 * Reconstruct the UI for when window sizes change
 * @param main_win Pointer to MainWin
 */
static void ctune_UI_MainWin_resize( void * main_win ) {
    ctune_UI_MainWin_t * main = main_win;

    CTUNE_LOG( CTUNE_LOG_MSG, "[ctune_UI_MainWin_resize( %p )] Resize event called (mouse: %s).", main_win, ( main->mouse_ctrl ? "ON" : "OFF" ) );

    endwin();
    ctune_UI_MainWin_destroyPanels( main );
    ctune_UI_MainWin_calculateWinSizes( main->screen_property, &main->size );

    ctune_UI_RSListWin.setMouseCtrl( &main->tabs.favourites, main->mouse_ctrl );
    ctune_UI_RSListWin.setMouseCtrl( &main->tabs.search, main->mouse_ctrl );
    ctune_UI_BrowserWin.setMouseCtrl( &main->tabs.browser, main->mouse_ctrl );

    switch( main->cache.curr_panel ) {
        case CTUNE_UI_PANEL_FAVOURITES: {
            ctune_UI_RSListWin.resize( &main->tabs.search );
            ctune_UI_BrowserWin.resize( &main->tabs.browser );
            ctune_UI_MainWin_createPanels( main );
            top_panel( main->panels[main->cache.curr_panel] );
            ctune_UI_RSListWin.resize( &main->tabs.favourites );
        } break;

        case CTUNE_UI_PANEL_SEARCH: {
            ctune_UI_RSListWin.resize( &main->tabs.favourites );
            ctune_UI_BrowserWin.resize( &main->tabs.browser );
            ctune_UI_MainWin_createPanels( main );
            top_panel( main->panels[main->cache.curr_panel] );
            ctune_UI_RSListWin.resize( &main->tabs.search );
        } break;

        case CTUNE_UI_PANEL_BROWSER: {
            ctune_UI_RSListWin.resize( &main->tabs.favourites );
            ctune_UI_RSListWin.resize( &main->tabs.search );
            ctune_UI_MainWin_createPanels( main );
            top_panel( main->panels[main->cache.curr_panel] );
            ctune_UI_BrowserWin.resize( &main->tabs.browser );
        } break;

        default:
            break;
    }

    { //reload field contents
        ctune_UI_MainWin.print.playbackState( main, main->cache.playback_state );
        ctune_UI_MainWin.print.volume( main, main->cache.curr_vol );

        //note: needs to copy as the `printSongInfo( const char * )` sets `main->cache.curr_song` and
        //      ends up corrupting that variable otherwise (circular reference because of the pointer)
        if( !String.empty( &main->cache.curr_song ) ) {
            String_t cp = String.init();
            String.copy( &cp, &main->cache.curr_song );
            ctune_UI_MainWin.print.songInfo( main, cp._raw );
            String.free( &cp );
        }
    }

    curs_set( 0 );
    update_panels();
    doupdate();
}

/**
 * Gets the current context
 * @param main Pointer to MainWin
 * @return Context enum
 */
static ctune_UI_Context_e ctune_UI_MainWin_currentContext( ctune_UI_MainWin_t * main ) {
    return main->curr_ctx;
}

/**
 * Gets the current panel ID selected
 * @param main Pointer to MainWin
 * @return PanelID enum
 */
static ctune_UI_PanelID_e ctune_UI_MainWin_currentPanelID( ctune_UI_MainWin_t * main ) {
    return main->cache.curr_panel;
}

/**
 * Gets the previously selected panel ID
 * @param main Pointer to MainWin
 * @return PanelID enum
 */
static ctune_UI_PanelID_e ctune_UI_MainWin_previousPanelID( ctune_UI_MainWin_t * main ) {
    return main->cache.prev_panel;
}

/**
 * Move a particular pane to the top
 * @param main     Pointer to MainWin
 * @param panel_id PanelID
 */
static void ctune_UI_MainWin_show( ctune_UI_MainWin_t * main, ctune_UI_PanelID_e panel_id ) {
    main->cache.prev_panel = main->cache.curr_panel;

    switch( panel_id ) {
        case CTUNE_UI_PANEL_FAVOURITES: {
            top_panel( main->panels[CTUNE_UI_PANEL_FAVOURITES] );
            main->cache.curr_panel = panel_id;
            main->curr_ctx         = CTUNE_UI_CTX_FAV_TAB;
            ctune_UI_RSListWin.show( &main->tabs.favourites );
        } break;

        case CTUNE_UI_PANEL_SEARCH: {
            top_panel( main->panels[CTUNE_UI_PANEL_SEARCH] );
            main->cache.curr_panel = panel_id;
            main->curr_ctx         = CTUNE_UI_CTX_SEARCH_TAB;
            ctune_UI_RSListWin.setRedraw( &main->tabs.search );
            ctune_UI_RSListWin.show( &main->tabs.search );
        } break;

        case CTUNE_UI_PANEL_BROWSER: {
            top_panel( main->panels[CTUNE_UI_PANEL_BROWSER] );
            main->cache.curr_panel = panel_id;
            main->curr_ctx         = CTUNE_UI_CTX_BROWSE_TAB;
            ctune_UI_BrowserWin.setRedraw( &main->tabs.browser, false );
            ctune_UI_BrowserWin.show( &main->tabs.browser );
        } break;

        default:
            break;
    }

    wrefresh( main->panel_windows[panel_id] );
    update_panels();
    doupdate();
    refresh();
}

/**
 * Gets the current radio station
 * @param main Pointer to MainWin
 * @return Current radio station
 */
static const ctune_RadioStationInfo_t * ctune_UI_MainWin_getCurrStation( ctune_UI_MainWin_t * main ) {
    return main->cache.curr_radio_station;
}

/**
 * Gets the current radio station's hash
 * @param main Pointer to MainWin
 * @return Current radio station's hash value
 */
static uint64_t ctune_UI_MainWin_getCurrStationHash( ctune_UI_MainWin_t * main ) {
    return main->cache.curr_radio_station_hash;
}

/**
 * Gets the currently selected station
 * @param main Pointer to MainWin
 * @param tab  PanelID of tab to get selected from
 * @return Pointer to selected radio station or NULL
 */
static const ctune_RadioStationInfo_t * ctune_UI_MainWin_getSelectedStation( ctune_UI_MainWin_t * main, ctune_UI_PanelID_e tab ) {
    const ctune_RadioStationInfo_t * rsi = NULL;

    if( tab == CTUNE_UI_PANEL_FAVOURITES ) {
        rsi = ctune_UI_RSListWin.getSelected( &main->tabs.favourites );
    } else if( tab == CTUNE_UI_PANEL_SEARCH ) {
        rsi = ctune_UI_RSListWin.getSelected( &main->tabs.search );
    } else if( tab == CTUNE_UI_PANEL_BROWSER ) {
        rsi = ctune_UI_BrowserWin.getSelectedStation( &main->tabs.browser );
    }

    return rsi;
}

/**
 * Check if the control row is selected in a given tab's listing
 * @param main Pointer to MainWin
 * @param tab  PanelID enum
 * @return Selected ctrl row state
 */
static bool ctune_UI_MainWin_isCtrlRowSelected( ctune_UI_MainWin_t * main, ctune_UI_PanelID_e tab ) {
    switch( tab ) {
        case CTUNE_UI_PANEL_FAVOURITES: { return ctune_UI_RSListWin.isCtrlRowSelected( &main->tabs.favourites ); } break;
        case CTUNE_UI_PANEL_SEARCH    : { return ctune_UI_RSListWin.isCtrlRowSelected( &main->tabs.search );     } break;
        case CTUNE_UI_PANEL_BROWSER   : { return ctune_UI_BrowserWin.isCtrlRowSelected( &main->tabs.browser );   } break;
        default                       : break;
    }
}

/**
 * Gets a view state from a list
 * @param main Pointer to MainWin
 * @param tab  Tab of the list targeted
 * @return State
 */
static ctune_UI_RSListWin_PageState_t ctune_UI_MainWin_getViewState( ctune_UI_MainWin_t * main, ctune_UI_PanelID_e tab ) {
    switch( tab ) {
        case CTUNE_UI_PANEL_FAVOURITES: { return ctune_UI_RSListWin.getPageState( &main->tabs.favourites ); } break;
        case CTUNE_UI_PANEL_SEARCH    : { return ctune_UI_RSListWin.getPageState( &main->tabs.search );     } break;
        default                       : break;
    }

    return (ctune_UI_RSListWin_PageState_t) { 0, 0, 0 };
}

/**
 * Sets a view state for a list
 * @param main  Pointer to MainWin
 * @param tab   Tab of the list targeted
 * @param state State to load
 */
static void ctune_UI_MainWin_setViewState( ctune_UI_MainWin_t * main, ctune_UI_PanelID_e tab, ctune_UI_RSListWin_PageState_t state ) {
    switch( tab ) {
        case CTUNE_UI_PANEL_FAVOURITES: { ctune_UI_RSListWin.setPageState( &main->tabs.favourites, state ); } break;
        case CTUNE_UI_PANEL_SEARCH    : { ctune_UI_RSListWin.setPageState( &main->tabs.search, state );     } break;
        default                       : break;
    }
}

/**
 * Checks if given browser pane is currently in focus
 * @param main Pointer to MainWin
 * @param pane PaneFocus_e enum ID
 * @return Focussed state
 */
static bool ctune_UI_MainWin_browserPaneisInFocus( ctune_UI_MainWin_t * main, ctune_UI_BrowserWin_PaneFocus_e pane ) {
    return ctune_UI_BrowserWin.isInFocus( &main->tabs.browser, pane );
}

/**
 * Update the cached favourites
 * @param main Pointer to MainWin
 * @param cb   Callback to update collection of favourite stations
 */
static void ctune_UI_MainWin_ctrl_updateFavourites( ctune_UI_MainWin_t * main, bool (* cb)( Vector_t * ) ) {
    if( cb ) {
        cb( &main->cache.favourites );
        ctune_UI_RSListWin.loadResults( &main->tabs.favourites, &main->cache.favourites, NULL );
    }
}

/**
 * Toggles the favourite state of a selected station
 * @param main Pointer to MainWin
 * @param tab  Panel ID of the tab where to toggle the fav flag on the selected station
 */
static void ctune_UI_MainWin_ctrl_toggleFavourite( ctune_UI_MainWin_t * main, ctune_UI_PanelID_e tab ) {
    switch( tab ) {
        case CTUNE_UI_PANEL_FAVOURITES: { ctune_UI_RSListWin.toggleFav( &main->tabs.favourites ); } break;
        case CTUNE_UI_PANEL_SEARCH    : { ctune_UI_RSListWin.toggleFav( &main->tabs.search );     } break;
        case CTUNE_UI_PANEL_BROWSER   : { ctune_UI_BrowserWin.toggleFav( &main->tabs.browser );   } break;
        default                       : break;
    }
}

/**
 * Sets the theming of 'favourite' stations on/off
 * @param main       Pointer to MainWin
 * @param theme_flag Switch to turn on/off theming for rows of favourite stations
 */
static void ctune_UI_MainWin_ctrl_themeFavourites( ctune_UI_MainWin_t * main, bool theme_flag ) {
    ctune_UI_RSListWin.themeFavourites( &main->tabs.favourites, theme_flag );
}

/**
 * Loads search results into search tab
 * @param main    Pointer to MainWin
 * @param results Collection of stations
 * @param filter  Filter used during search
 */
static void ctune_UI_MainWin_ctrl_loadSearchResults( ctune_UI_MainWin_t * main, Vector_t * results, ctune_RadioBrowserFilter_t * filter ) {
    ctune_UI_RSListWin.loadResults( &main->tabs.search, results, filter );
}

/**
 * Clears results from search tab
 * @param main Pointer to MainWin
 */
static void ctune_UI_MainWin_ctrl_clearSearchResults( ctune_UI_MainWin_t * main ) {
    ctune_UI_RSListWin.loadNothing( &main->tabs.search );
}

/**
 * Sets big row displaying on/off (false: 1 x line, true: 2 x lines + line delimiter)
 * @param main       Pointer to MainWin
 * @param tab        PanelID enum
 * @param large_flag Flag to turn feature on/off
 */
static void ctune_UI_MainWin_ctrl_setLargeRow( ctune_UI_MainWin_t * main, ctune_UI_PanelID_e tab, bool large_flag ) {
    switch( tab ) {
        case CTUNE_UI_PANEL_FAVOURITES: {
            ctune_UI_RSListWin.setLargeRow( &main->tabs.favourites, large_flag );
            ctune_UI_RSListWin.resize( &main->tabs.favourites );
        } break;

        case CTUNE_UI_PANEL_SEARCH: {
            ctune_UI_RSListWin.setLargeRow( &main->tabs.search, large_flag );
            ctune_UI_RSListWin.resize( &main->tabs.search );
        } break;

        case CTUNE_UI_PANEL_BROWSER: {
            ctune_UI_BrowserWin.setLargeRow( &main->tabs.browser, large_flag );
            ctune_UI_BrowserWin.resize( &main->tabs.browser );
        } break;

        default: break;
    }
}

/**
 * Signals a radio station as 'current' (i.e. as queued or playing)
 * @param main Pointer to MainWin
 * @param rsi  Pointer to RadioStationInfo DTO
 */
static void ctune_UI_MainWin_ctrl_setCurrStation( ctune_UI_MainWin_t * main, ctune_RadioStationInfo_t * rsi ) {
    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_UI_MainWin_setCurrStation( %p, %p )] Setting \"%s\" as current radio station (Source: %s, UUID: %s).",
               main, rsi,
               ( rsi != NULL ? ctune_RadioStationInfo.get.stationName( rsi ) : "NULL" ),
               ( rsi != NULL ? ctune_StationSrc.str( ctune_RadioStationInfo.get.stationSource( rsi ) ) : "N/A" ),
               ( rsi != NULL ? ctune_RadioStationInfo.get.stationUUID( rsi ) : "N/A" )
    );

    if( rsi != main->cache.curr_radio_station ) {
        if( rsi ) {
            ctune_RadioStationInfo.freeContent( main->cache.curr_radio_station );
            free( main->cache.curr_radio_station );
            main->cache.curr_radio_station = NULL;
        }

        main->cache.curr_radio_station      = rsi;
        main->cache.curr_radio_station_hash = ctune_RadioStationInfo.hash( ctune_RadioStationInfo.get.stationUUID( rsi ) );

        CTUNE_LOG( CTUNE_LOG_TRACE,
                   "[ctune_UI_MainWin_setCurrStation( %p, %p )] hash = %lu (Source: %s, UUID: %s).",
                   main, rsi, main->cache.curr_radio_station_hash,
                   ctune_StationSrc.str( ctune_RadioStationInfo.get.stationSource( rsi ) ), ctune_RadioStationInfo.get.stationUUID( rsi )
        );
    }

    if( rsi != NULL ) {
        ctune_UI_MainWin_clearPanel( main, CTUNE_UI_PANEL_STATUS_2 );
        mvwprintw( main->panel_windows[CTUNE_UI_PANEL_STATUS_2], 0, 0, "%s", ctune_RadioStationInfo.get.stationName( main->cache.curr_radio_station ) );
        wrefresh( main->panel_windows[CTUNE_UI_PANEL_STATUS_2] );
        update_panels();
        refresh();
        doupdate();
    }
}

/**
 * Start playback of current selected station on current tab view
 * @param main Pointer to MainWin
 */
static void ctune_UI_MainWin_ctrl_playSelectedStation( ctune_UI_MainWin_t * main ) {
    const ctune_RadioStationInfo_t * rsi   = NULL;
    size_t                           row_i = 0;

    if( main->cache.curr_panel == CTUNE_UI_PANEL_SEARCH ) {
        rsi = ctune_UI_RSListWin.getSelected( &main->tabs.search );

        if( ctune_UI_RSListWin.isCtrlRowSelected( &main->tabs.search ) ) {
            ctune_UI_RSListWin.triggerCtrlRow( &main->tabs.search );
            ctune_UI_RSListWin.show( &main->tabs.search );
        } else {
            rsi = ctune_UI_RSListWin.getSelected( &main->tabs.search );
        }

        row_i = main->tabs.search.row.selected;

    } else if( main->cache.curr_panel == CTUNE_UI_PANEL_FAVOURITES ) {
        rsi = ctune_UI_RSListWin.getSelected( &main->tabs.favourites );

        if( !ctune_UI_RSListWin.isCtrlRowSelected( &main->tabs.favourites ) ) {
            rsi = ctune_UI_RSListWin.getSelected( &main->tabs.favourites );
        }

        row_i = main->tabs.favourites.row.selected;

    } else if( main->cache.curr_panel == CTUNE_UI_PANEL_BROWSER ) {
        rsi = ctune_UI_BrowserWin.getSelectedStation( &main->tabs.browser );

        if( !ctune_UI_BrowserWin.isCtrlRowSelected( &main->tabs.browser ) ) {
            rsi = ctune_UI_BrowserWin.getSelectedStation( &main->tabs.browser );
        }

        row_i = main->tabs.browser.right_pane.row.selected;
    }

    if( rsi != NULL ) {
        CTUNE_LOG( CTUNE_LOG_DEBUG,
                   "[ctune_UI_MainWin_ctrl_playSelectedStation()] "
                   "Initiating RSI playback from search results (i=%lu)...",
                   row_i
        );

        if( !main->cb.playStation( rsi ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_MainWin_ctrl_playSelectedStation()] Failed playback init (<%s>).",
                       ctune_RadioStationInfo.get.stationUUID( rsi )
            );
        }
    }

    if( main->cache.curr_panel == CTUNE_UI_PANEL_SEARCH ) {
        ctune_UI_RSListWin.setRedraw( &main->tabs.search );
        ctune_UI_RSListWin.show( &main->tabs.search );
    } else if( main->cache.curr_panel == CTUNE_UI_PANEL_FAVOURITES ) {
        ctune_UI_RSListWin.setRedraw( &main->tabs.favourites );
        ctune_UI_RSListWin.show( &main->tabs.favourites );
    } else if( main->cache.curr_panel == CTUNE_UI_PANEL_BROWSER ) {
        ctune_UI_BrowserWin.setRedraw( &main->tabs.browser, false );
        ctune_UI_BrowserWin.show( &main->tabs.browser );
    }
}

/**
 * Navigation select for 'ENTER' key
 * @param main Pointer to MainWin
 */
static void ctune_UI_MainWin_nav_enter( ctune_UI_MainWin_t * main ) {
    ctune_UI_MainWin.print.clearMsgLine( main );

    switch( main->cache.curr_panel ) {
        case CTUNE_UI_PANEL_FAVOURITES: //fallthrough
        case CTUNE_UI_PANEL_SEARCH: {
            ctune_UI_MainWin_ctrl_playSelectedStation( main );
        } break;

        case CTUNE_UI_PANEL_BROWSER: {
            if( ctune_UI_BrowserWin.isInFocus( &main->tabs.browser, FOCUS_PANE_LEFT ) ) {
                ctune_UI_BrowserWin.navKeyEnter( &main->tabs.browser );
                ctune_UI_BrowserWin.show( &main->tabs.browser );
            } else { //focus: FOCUS_PANE_RIGHT
                ctune_UI_MainWin_ctrl_playSelectedStation( main );
            }
        } break;

        default:
            break;
    }
}

/**
 * Navigation select for 'UP' key
 * @param main Pointer to MainWin
 */
static void ctune_UI_MainWin_nav_selectUp( ctune_UI_MainWin_t * main ) {
    ctune_UI_MainWin.print.clearMsgLine( main );

    if( main->cache.curr_panel == CTUNE_UI_PANEL_FAVOURITES ) {
        ctune_UI_RSListWin.selectUp( &main->tabs.favourites );
        ctune_UI_RSListWin.show( &main->tabs.favourites );
    } else if( main->cache.curr_panel == CTUNE_UI_PANEL_SEARCH ) {
        ctune_UI_RSListWin.selectUp( &main->tabs.search );
        ctune_UI_RSListWin.show( &main->tabs.search );
    } else if( main->cache.curr_panel == CTUNE_UI_PANEL_BROWSER ) {
        ctune_UI_BrowserWin.navKeyUp( &main->tabs.browser );
        ctune_UI_BrowserWin.show( &main->tabs.browser );
    }
}

/**
 * Navigation select for 'DOWN' key
 * @param main Pointer to MainWin
 */
static void ctune_UI_MainWin_nav_selectDown( ctune_UI_MainWin_t * main ) {
    ctune_UI_MainWin.print.clearMsgLine( main );

    if( main->cache.curr_panel == CTUNE_UI_PANEL_FAVOURITES ) {
        ctune_UI_RSListWin.selectDown( &main->tabs.favourites );
        ctune_UI_RSListWin.show( &main->tabs.favourites );
    } else if( main->cache.curr_panel == CTUNE_UI_PANEL_SEARCH ) {
        ctune_UI_RSListWin.selectDown( &main->tabs.search );
        ctune_UI_RSListWin.show( &main->tabs.search );
    } else if( main->cache.curr_panel == CTUNE_UI_PANEL_BROWSER ) {
        ctune_UI_BrowserWin.navKeyDown( &main->tabs.browser );
        ctune_UI_BrowserWin.show( &main->tabs.browser );
    }
}

/**
 * Navigation select for 'LEFT' key
 * @param main Pointer to MainWin
 */
static void ctune_UI_MainWin_nav_selectLeft( ctune_UI_MainWin_t * main ) {
    ctune_UI_MainWin.print.clearMsgLine( main );

    if( main->cache.curr_panel == CTUNE_UI_PANEL_BROWSER ) {
        ctune_UI_BrowserWin.navKeyLeft( &main->tabs.browser );
        ctune_UI_BrowserWin.show( &main->tabs.browser );
    }
}

/**
 * Navigation select for 'RIGHT' key
 * @param main Pointer to MainWin
 */
static void ctune_UI_MainWin_nav_selectRight( ctune_UI_MainWin_t * main ) {
    ctune_UI_MainWin.print.clearMsgLine( main );

    switch( main->cache.curr_panel ) {
        case CTUNE_UI_PANEL_FAVOURITES: //fallthrough
        case CTUNE_UI_PANEL_SEARCH: {
            main->cb.openInfoDialog( ctune_UI_MainWin.getSelectedStation( main, main->cache.curr_panel ) );
        } break;

        case CTUNE_UI_PANEL_BROWSER: {
            if( ctune_UI_BrowserWin.isInFocus( &main->tabs.browser, FOCUS_PANE_RIGHT ) ) {
                main->cb.openInfoDialog( ctune_UI_MainWin.getSelectedStation( main, main->cache.curr_panel ) );
            } else {
                ctune_UI_BrowserWin.navKeyRight( &main->tabs.browser );
                ctune_UI_BrowserWin.show( &main->tabs.browser );
            }
        } break;

        default:
            break;
    }
}

/**
 * Navigation select for 'PPAGE' key
 * @param main Pointer to MainWin
 * @param tab  PanelID of the current tab
 */
static void ctune_UI_MainWin_nav_selectPageUp( ctune_UI_MainWin_t * main ) {
    ctune_UI_MainWin.print.clearMsgLine( main );

    if( main->cache.curr_panel == CTUNE_UI_PANEL_FAVOURITES ) {
        ctune_UI_RSListWin.selectPageUp( &main->tabs.favourites );
        ctune_UI_RSListWin.show( &main->tabs.favourites );
    } else if( main->cache.curr_panel == CTUNE_UI_PANEL_SEARCH ) {
        ctune_UI_RSListWin.selectPageUp( &main->tabs.search );
        ctune_UI_RSListWin.show( &main->tabs.search );
    } else if( main->cache.curr_panel == CTUNE_UI_PANEL_BROWSER ) {
        ctune_UI_BrowserWin.navKeyPageUp( &main->tabs.browser );
        ctune_UI_BrowserWin.show( &main->tabs.browser );
    }
}

/**
 * Navigation select for 'NPAGE' key
 * @param main Pointer to MainWin
 */
static void ctune_UI_MainWin_nav_selectPageDown( ctune_UI_MainWin_t * main ) {
    ctune_UI_MainWin.print.clearMsgLine( main );

    if( main->cache.curr_panel == CTUNE_UI_PANEL_FAVOURITES ) {
        ctune_UI_RSListWin.selectPageDown( &main->tabs.favourites );
        ctune_UI_RSListWin.show( &main->tabs.favourites );
    } else if( main->cache.curr_panel == CTUNE_UI_PANEL_SEARCH ) {
        ctune_UI_RSListWin.selectPageDown( &main->tabs.search );
        ctune_UI_RSListWin.show( &main->tabs.search );
    } else if( main->cache.curr_panel == CTUNE_UI_PANEL_BROWSER ) {
        ctune_UI_BrowserWin.navKeyPageDown( &main->tabs.browser );
        ctune_UI_BrowserWin.show( &main->tabs.browser );
    }
}

/**
 * Navigation select for 'HOME' key
 * @param main Pointer to MainWin
 */
static void ctune_UI_MainWin_nav_selectHome( ctune_UI_MainWin_t * main ) {
    ctune_UI_MainWin.print.clearMsgLine( main );

    if( main->cache.curr_panel == CTUNE_UI_PANEL_FAVOURITES ) {
        ctune_UI_RSListWin.selectFirst( &main->tabs.favourites );
        ctune_UI_RSListWin.show( &main->tabs.favourites );
    } else if( main->cache.curr_panel == CTUNE_UI_PANEL_SEARCH ) {
        ctune_UI_RSListWin.selectFirst( &main->tabs.search );
        ctune_UI_RSListWin.show( &main->tabs.search );
    } else if( main->cache.curr_panel == CTUNE_UI_PANEL_BROWSER ) {
        ctune_UI_BrowserWin.navKeyHome( &main->tabs.browser );
        ctune_UI_BrowserWin.show( &main->tabs.browser );
    }
}

/**
 * Navigation select for 'END' key
 * @param main Pointer to MainWin
 */
static void ctune_UI_MainWin_nav_selectEnd( ctune_UI_MainWin_t * main ) {
    ctune_UI_MainWin.print.clearMsgLine( main );

    if( main->cache.curr_panel == CTUNE_UI_PANEL_FAVOURITES ) {
        ctune_UI_RSListWin.selectLast( &main->tabs.favourites );
        ctune_UI_RSListWin.show( &main->tabs.favourites );
    } else if( main->cache.curr_panel == CTUNE_UI_PANEL_SEARCH ) {
        ctune_UI_RSListWin.selectLast( &main->tabs.search );
        ctune_UI_RSListWin.show( &main->tabs.search );
    } else if( main->cache.curr_panel == CTUNE_UI_PANEL_BROWSER ) {
        ctune_UI_BrowserWin.navKeyEnd( &main->tabs.browser );
        ctune_UI_BrowserWin.show( &main->tabs.browser );
    }
}

/**
 * Navigation for 'TAB' key
 * @param main Pointer to MainWin
 */
static void ctune_UI_MainWin_nav_switchFocus( ctune_UI_MainWin_t * main ) {
    if( main->cache.curr_panel == CTUNE_UI_PANEL_BROWSER ) {
        ctune_UI_BrowserWin.switchFocus( &main->tabs.browser );
        ctune_UI_BrowserWin.show( &main->tabs.browser );
    }
}

/**
 * Prints a string inside the area reserved for song descriptions
 * @param main MainWin object
 * @param str  String to display on screen
 */
static void ctune_UI_MainWin_print_songInfo( ctune_UI_MainWin_t * main, const char * str ) {
    ctune_UI_MainWin_clearPanel( main, CTUNE_UI_PANEL_STATUS_2 );

    const ctune_RadioStationInfo_t * curr_rsi = main->cache.curr_radio_station;

    if( strlen( ctune_RadioStationInfo.get.stationName( curr_rsi ) ) > INT_MAX ) { //should not happen but you never know...
        CTUNE_LOG( CTUNE_LOG_WARNING,
                   "[ctune_UI_MainWin_print_songInfo( %s )] station name size > INT_MAX!! Not gonna print this!",
                   str
        );
        mvwprintw( main->panel_windows[CTUNE_UI_PANEL_STATUS_2], 0, 0, " ... | %s", str );
    } else {
        mvwprintw( main->panel_windows[CTUNE_UI_PANEL_STATUS_2], 0, 0, "%s | %s", ctune_RadioStationInfo.get.stationName( curr_rsi ), str );
    }

    if( str != NULL ) {
        String.set( &main->cache.curr_song, str );
    } else {
        String.free( &main->cache.curr_song );
    }

    update_panels();
    refresh();
    doupdate();
}

/**
 * Prints an integer inside the area reserved to display the current volume
 * @param main MainWin object
 * @param vol  Volume to display on screen
 */
static void ctune_UI_MainWin_print_volume( ctune_UI_MainWin_t * main, int vol ) {
    main->cache.curr_vol = vol;

    mvwprintw( main->panel_windows[ CTUNE_UI_PANEL_STATUS_3 ], 0, 0,
               "%s %3d%%",
               ctune_UI_Icons.icon( CTUNE_UI_ICON_VOLUME ), vol );

    update_panels();
    refresh();
    doupdate();
}

/**
 * Prints the playback state to the screen
 * @param main  MainWin object
 * @param state Playback state
 */
static void ctune_UI_MainWin_print_playbackState( ctune_UI_MainWin_t * main, ctune_PlaybackCtrl_e state ) {
    main->cache.playback_state = state;

    if( ctune_PlaybackCtrl.isRecording( state ) ) { //recording + playing
        wbkgd( main->panel_windows[CTUNE_UI_PANEL_STATUS_1], ctune_UI_Theme.color( CTUNE_UI_ITEM_PLAYBACK_REC ) );
        mvwprintw( main->panel_windows[CTUNE_UI_PANEL_STATUS_1], 0, 1, "%s", ctune_UI_Icons.icon( CTUNE_UI_ICON_RECORDING ) );

    } else if( ctune_PlaybackCtrl.isOn( state ) ) { //playing
        wbkgd( main->panel_windows[CTUNE_UI_PANEL_STATUS_1], ctune_UI_Theme.color( CTUNE_UI_ITEM_PLAYBACK_ON ) );
        mvwprintw( main->panel_windows[CTUNE_UI_PANEL_STATUS_1], 0, 1, "%s", ctune_UI_Icons.icon( CTUNE_UI_ICON_PLAYING ) );

    } else if( ctune_PlaybackCtrl.isOff( state ) ) { //off
        wbkgd( main->panel_windows[CTUNE_UI_PANEL_STATUS_1], ctune_UI_Theme.color( CTUNE_UI_ITEM_PLAYBACK_OFF ) );
        mvwprintw( main->panel_windows[CTUNE_UI_PANEL_STATUS_1], 0, 1, "%s", ctune_UI_Icons.icon( CTUNE_UI_ICON_STOPPED ) );
    }

    update_panels();
    refresh();
    doupdate();
}

/**
 * Prints the search state to the screen
 * @param main  MainWin object
 * @param state Search state
 */
static void ctune_UI_MainWin_print_searchingState( ctune_UI_MainWin_t * main, bool state ) {
    static const int row = 0;
    static const int col = 45;
    mvwprintw( main->panel_windows[CTUNE_UI_PANEL_TITLE], row, col, "%s", ( state ? "Searching" : "Idle" ) );
    refresh();
    doupdate();
}

/**
 * Prints an error description string to the screen
 * @param main    MainWin object
 * @param err_str Error string
 */
static void ctune_UI_MainWin_print_error( ctune_UI_MainWin_t * main, const char * err_str ) {
    wclear( main->panel_windows[ CTUNE_UI_PANEL_MSG_LINE ] );
    if( strlen( err_str ) > 0 ) {
        mvwprintw( main->panel_windows[ CTUNE_UI_PANEL_MSG_LINE ], 0, 0, "Error: %s", err_str );
        wrefresh( main->panel_windows[ CTUNE_UI_PANEL_MSG_LINE ] );
    }
}

/**
 * Prints a message string to the screen
 * @param main     MainWin object
 * @param info_str Information string
 */
static void ctune_UI_MainWin_print_statusMsg( ctune_UI_MainWin_t * main, const char * info_str ) {
    werase( main->panel_windows[ CTUNE_UI_PANEL_MSG_LINE ] );
    mvwprintw( main->panel_windows[ CTUNE_UI_PANEL_MSG_LINE ], 0, 0, "%s", info_str );
}

/**
 * Clears the message line of any messages if not used for input
 * @param main Pointer to MainWin
 */
static void ctune_UI_MainWin_print_clearMsgLine( ctune_UI_MainWin_t * main ) {
    ctune_UI_MainWin_clearPanel( main, CTUNE_UI_PANEL_MSG_LINE );
}

/**
 * Sets the callback to use to get a station's state
 * @param main Pointer to MainWin
 * @param cb   Callback method
 */
static void ctune_UI_MainWin_cb_setStationStateGetterCallback( ctune_UI_MainWin_t * main, unsigned (* cb)( const ctune_RadioStationInfo_t * ) ) {
    main->cb.getStationState = cb;
}

/**
 * Sets the callback to use to trigger playback of a station
 * @param main Pointer to MainWin
 * @param cb   Callback method
 */
static void ctune_UI_MainWin_cb_setPlayStationCallback( ctune_UI_MainWin_t * main, bool (* cb)( const ctune_RadioStationInfo_t * ) ) {
    main->cb.playStation = cb;
}

/**
 * Sets the callback to use to open the station information dialog
 * @param main Pointer to MainWin
 * @param cb   Callback method
 */
static void ctune_UI_MainWin_cb_setOpenInfoDialogCallback( ctune_UI_MainWin_t * main, void (* cb)( const ctune_RadioStationInfo_t * ) ) {
    main->cb.openInfoDialog = cb;
}


const struct ctune_UI_MainWinClass ctune_UI_MainWin = {
    .create                               = &ctune_UI_MainWin_create,
    .init                                 = &ctune_UI_MainWin_init,
    .setMouseCtrl                         = &ctune_UI_MainWin_setMouseCtrl,
    .handleMouseEvent                     = &ctune_UI_MainWin_handleMouseEvent,
    .free                                 = &ctune_UI_MainWin_free,

    .resize                               = &ctune_UI_MainWin_resize,
    .show                                 = &ctune_UI_MainWin_show,

    .currentContext                       = &ctune_UI_MainWin_currentContext,
    .currentPanelID                       = &ctune_UI_MainWin_currentPanelID,
    .previousPanelID                      = &ctune_UI_MainWin_previousPanelID,
    .getCurrStation                       = &ctune_UI_MainWin_getCurrStation,
    .getCurrStationHash                   = &ctune_UI_MainWin_getCurrStationHash,
    .getSelectedStation                   = &ctune_UI_MainWin_getSelectedStation,
    .isCtrlRowSelected                    = &ctune_UI_MainWin_isCtrlRowSelected,
    .getViewState                         = &ctune_UI_MainWin_getViewState,
    .setViewState                         = &ctune_UI_MainWin_setViewState,
    .browserPaneIsInFocus                 = &ctune_UI_MainWin_browserPaneisInFocus,

    .ctrl = {
        .updateFavourites                 = &ctune_UI_MainWin_ctrl_updateFavourites,
        .toggleFavourite                  = &ctune_UI_MainWin_ctrl_toggleFavourite,
        .themeFavourites                  = &ctune_UI_MainWin_ctrl_themeFavourites,
        .loadSearchResults                = &ctune_UI_MainWin_ctrl_loadSearchResults,
        .clearSearchResults               = &ctune_UI_MainWin_ctrl_clearSearchResults,
        .setLargeRow                      = &ctune_UI_MainWin_ctrl_setLargeRow,
        .setCurrStation                   = &ctune_UI_MainWin_ctrl_setCurrStation,
        .playSelectedStation              = &ctune_UI_MainWin_ctrl_playSelectedStation,
    },

    .nav = {
        .enter                            = &ctune_UI_MainWin_nav_enter,
        .selectUp                         = &ctune_UI_MainWin_nav_selectUp,
        .selectDown                       = &ctune_UI_MainWin_nav_selectDown,
        .selectLeft                       = &ctune_UI_MainWin_nav_selectLeft,
        .selectRight                      = &ctune_UI_MainWin_nav_selectRight,
        .selectPageUp                     = &ctune_UI_MainWin_nav_selectPageUp,
        .selectPageDown                   = &ctune_UI_MainWin_nav_selectPageDown,
        .selectHome                       = &ctune_UI_MainWin_nav_selectHome,
        .selectEnd                        = &ctune_UI_MainWin_nav_selectEnd,
        .switchFocus                      = &ctune_UI_MainWin_nav_switchFocus,
    },

    .print = {
        .songInfo                         = &ctune_UI_MainWin_print_songInfo,
        .volume                           = &ctune_UI_MainWin_print_volume,
        .playbackState                    = &ctune_UI_MainWin_print_playbackState,
        .searchingState                   = &ctune_UI_MainWin_print_searchingState,
        .error                            = &ctune_UI_MainWin_print_error,
        .statusMsg                        = &ctune_UI_MainWin_print_statusMsg,
        .clearMsgLine                     = &ctune_UI_MainWin_print_clearMsgLine,
    },

    .cb = {
        .setStationStateGetterCallback    = &ctune_UI_MainWin_cb_setStationStateGetterCallback,
        .setPlayStationCallback           = &ctune_UI_MainWin_cb_setPlayStationCallback,
        .setOpenInfoDialogCallback        = &ctune_UI_MainWin_cb_setOpenInfoDialogCallback,
    },
};