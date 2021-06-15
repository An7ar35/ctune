#include "RSListWin.h"

#include "../../logger/Logger.h"
#include "../definitions/Theme.h"

#define LARGE_ENTRY_ROW_HEIGHT  3
#define SMALL_ENTRY_ROW_HEIGHT  1
#define MAX_BITRATE_FIELD_WIDTH 6

/**
 * [PRIVATE] Gets the theme for a row based on given flags
 * @param in_focus  Window is in focus
 * @param selected  Row is selected
 * @return Theme colour ID
 */
static int ctune_UI_RSListWin_getRowTheme( bool in_focus, bool selected ) {
    return ( selected ? ctune_UI_Theme.color( ( in_focus ? CTUNE_UI_ITEM_ROW_SELECTED_FOCUSED : CTUNE_UI_ITEM_ROW_SELECTED_UNFOCUSED ) )
                      : ctune_UI_Theme.color( CTUNE_UI_ITEM_ROW ) );
}

/**
 * [PRIVATE] Gets the theme for the 'Queued' icon based on given flags
 * @param in_focus Window is in focus
 * @param selected Row is selected
 * @return Theme colour ID
 */
static int ctune_UI_RSListWin_getQueuedIconTheme( bool in_focus, bool selected ) {
    return ( selected ? ctune_UI_Theme.color( ( in_focus ? CTUNE_UI_ITEM_QUEUED_INV_FOCUSED : CTUNE_UI_ITEM_QUEUED_INV_UNFOCUSED ) )
                      : ctune_UI_Theme.color( CTUNE_UI_ITEM_QUEUED ) );
}

/**
 * [PRIVATE] Gets the theme for 'favourite' station text based on given flags
 * @param win      RSListWin_t object
 * @param src      Source of entry
 * @param selected Row is selected
 * @return Theme colour ID
 */
static int ctune_UI_RSListWin_getFavTextTheme( ctune_UI_RSListWin_t * win, ctune_StationSrc_e src, bool selected ) {
    if( win->row.theme_favs ) {
        switch( src ) {
            case CTUNE_STATIONSRC_LOCAL: {
                if( selected )
                    return ctune_UI_Theme.color( win->in_focus
                                                 ? CTUNE_UI_ITEM_TXT_SELECTED_FOCUSED_FAV_LOCAL
                                                 : CTUNE_UI_ITEM_TXT_SELECTED_UNFOCUSED_FAV_LOCAL
                    );
                else
                    return ctune_UI_Theme.color( CTUNE_UI_ITEM_TXT_FAV_LOCAL );
            };

            case CTUNE_STATIONSRC_RADIOBROWSER: {
                if( selected )
                    return ctune_UI_Theme.color( win->in_focus
                                                 ? CTUNE_UI_ITEM_TXT_SELECTED_FOCUSED_FAV_REMOTE
                                                 : CTUNE_UI_ITEM_TXT_SELECTED_UNFOCUSED_FAV_REMOTE
                    );
                else
                    return ctune_UI_Theme.color( CTUNE_UI_ITEM_TXT_FAV_REMOTE );
            }

            default: {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_UI_RSListWin_getFavTextTheme( %p, %i, %i )] "
                           "Unimplemented case: %i - using default.",
                           win, src, selected, src
                );

                return ctune_UI_RSListWin_getRowTheme( win->in_focus, selected );
            };
        }

    } else {
        return ctune_UI_RSListWin_getRowTheme( win->in_focus, selected );
    }
}

/**
 * [PRIVATE] Callback to fetch more items to append to the current radio station list
 * @param win RSListWin_t object
 * @return Success
 */
static bool ctune_UI_RSListWin_fetchMoreItems( ctune_UI_RSListWin_t * win ) {
    if( win->cb.getStations == NULL ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_UI_RSListWin_fetchMoreItems( %p )] "
                   "Trying to fetch more items (i.e.: had a  ctrl fetch row) with a NULL `getStation(..)` callback!",
                   win
        );

        return false; //EARLY RETURN
    }

    Vector_t results = Vector.init( sizeof( ctune_RadioStationInfo_t ), ctune_RadioStationInfo.freeContent );

    size_t new_offset = ctune_RadioBrowserFilter.get.resultOffset( win->cache.filter )
                      + ctune_RadioBrowserFilter.get.resultLimit( win->cache.filter );

    ctune_RadioBrowserFilter.set.resultOffset( win->cache.filter, new_offset );

    if( win->cb.getStations( win->cache.filter, &results ) ) {
        if( ctune_UI_RSListWin.appendResults( win, &results ) ) {
            Vector.clear_vector( &results );
            return true;
        }
    }

    Vector.clear_vector( &results );
    return false;
}

/**
 * [PRIVATE] Select a row in the list of RSI entries
 * @param win    RSListWin_t object
 * @param offset Offset to move selection by
 */
static void ctune_UI_RSListWin_selectRow( ctune_UI_RSListWin_t * win, int offset ) {
    if( Vector.empty( &win->entries ) )
        return;

    if( offset < 0 ) {
        if( win->row.selected < abs( offset ) )
            win->row.selected = 0;
        else
            win->row.selected += offset;
    }

    if( offset > 0 ) {
        if( ( win->row.selected + offset ) >= Vector.size( &win->entries ) )
            win->row.selected = Vector.size( &win->entries ) - ( win->row.show_ctrl_row ? 0 : 1 );
        else
            win->row.selected += offset;
    }

    win->redraw = true;
}

/**
 * [PRIVATE] Prints an item row in slim format
 * @param win     RSListWin_t object
 * @param rsi     RadioStationInfo_t object
 * @param i       List index for the radio station
 * @param win_row Pointer to the windows's row index
 */
static void ctune_UI_RSListWin_printSmallItemRow( ctune_UI_RSListWin_t * win, ctune_RadioStationInfo_t * rsi, int i, int * win_row ) {
    unsigned rsi_state   = win->cb.getStationState( rsi );
    int      info_col    = 0;

    if( !ctune_utoi( ( win->canvas_property->cols - ( win->sizes.cc_ln + win->sizes.kbps_ln + 1 ) ), &info_col ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_RSListWin_printSmallItemRow( %p, %p, %i, %p )] "
                   "Integer overflow: country code column position (%i)",
                   win, rsi, i, win_row, ( win->canvas_property->cols - win->sizes.cc_ln ) );
    }

    //'Fav' check is here so that what is shown is always based on the most updated list of favourites
    if( win->cb.isFavourite != NULL )
        ctune_RadioStationInfo.set.favourite( rsi, win->cb.isFavourite( rsi ) );

    const char             * station_name    = ctune_RadioStationInfo.get.stationName( rsi );
    const char             * station_cc      = ctune_RadioStationInfo.get.countryCode( rsi );
    ulong                    station_bitrate = ctune_RadioStationInfo.get.bitrate( rsi );
    const ctune_StationSrc_e station_src     = ctune_RadioStationInfo.get.stationSource( rsi );
    int                      row_theme       = ctune_UI_RSListWin_getRowTheme( win->in_focus, ( i == win->row.selected ) );

    //e.g. line #1: "Station na...   128k/DE"
    if( (*win_row) < win->canvas_property->rows ) {
        int name_col = 0;

        wattron( win->canvas_win, row_theme );
        mvwprintw( win->canvas_win, (*win_row), 0, "%0*c", win->canvas_property->cols, ' ' );

        if( ( rsi_state & ctune_RadioStationInfo.IS_QUEUED ) ) {
            int queued_theme = ctune_UI_RSListWin_getQueuedIconTheme( win->in_focus, ( i == win->row.selected ) );

            wattron( win->canvas_win, queued_theme );
            mvwprintw( win->canvas_win, (*win_row), name_col, "%c ", '>' );
            wattroff( win->canvas_win, queued_theme );

            name_col += 2;
        }

        if( ( rsi_state & ctune_RadioStationInfo.IS_FAV ) ) {
            int fav_theme = ctune_UI_RSListWin_getFavTextTheme( win, station_src, ( i == win->row.selected ) );
            wattron( win->canvas_win, fav_theme );
            mvwprintw( win->canvas_win, (*win_row), name_col, "%-*s", info_col, station_name );
            wattroff( win->canvas_win, fav_theme );
            wattron( win->canvas_win, row_theme );
        } else {
            wattron( win->canvas_win, row_theme );
            mvwprintw( win->canvas_win, (*win_row), name_col, "%-*s", info_col, station_name );
        }

        mvwprintw( win->canvas_win, (*win_row), info_col, "%*lu%s|%s",
                   MAX_BITRATE_FIELD_WIDTH, station_bitrate, win->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_BITRATE_UNIT_SHORT ),
                   ( station_cc == NULL  || strlen( station_cc ) != 2 ? "??" : station_cc ) );

        wattroff( win->canvas_win, row_theme );
        *win_row += 1;
    }
}

/**
 * [PRIVATE] Prints an item row in large format
 * @param win     RSListWin_t object
 * @param rsi     RadioStationInfo_t object
 * @param i       List index for the radio station
 * @param win_row Pointer to the windows's row index
 */
static void ctune_UI_RSListWin_printLargeItemRow( ctune_UI_RSListWin_t * win, ctune_RadioStationInfo_t * rsi, int i, int * win_row ) {
    if( rsi == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_RSListWin_printLargeItemRow( %p, %p, %i, %p )] RSI is NULL.",
                   win, rsi, i, win_row
        );

        return; //EARLY RETURN
    }

    unsigned rsi_state   = win->cb.getStationState( rsi );
    int      cc_col      = 0;
    int      bitrate_col = 0;

    if( !ctune_utoi( ( win->canvas_property->cols - win->sizes.cc_ln ), &cc_col ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_RSListWin_printLargeItemRow( %p, %p, %i, %p )] "
                   "Integer overflow: country code column position (%i)",
                   win, rsi, i, win_row, ( win->canvas_property->cols - win->sizes.cc_ln ) );
    }

    if( !ctune_utoi( ( win->canvas_property->cols - win->sizes.kbps_ln ), &bitrate_col ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_RSListWin_printLargeItemRow( %p, %p, %i, %p )] "
                   "Integer overflow: bitrate column position (%i)",
                   win, rsi, i, win_row, ( win->canvas_property->cols - win->sizes.kbps_ln ) );
    }

    //'Fav' check is here so that what is shown is always based on the most updated list of favourites
    if( win->cb.isFavourite != NULL )
        ctune_RadioStationInfo.set.favourite( rsi, win->cb.isFavourite( rsi ) );

    //e.g. line #1: "Station name... DE"
    //     line #2: "tag1, tag...  128k"
    //     line #3: "------------------"
    const char             * station_name    = ctune_RadioStationInfo.get.stationName( rsi );
    const char             * station_cc      = ctune_RadioStationInfo.get.countryCode( rsi );
    const char             * station_tags    = ctune_RadioStationInfo.get.tags( rsi );
    ulong                    station_bitrate = ctune_RadioStationInfo.get.bitrate( rsi );
    const ctune_StationSrc_e station_src     = ctune_RadioStationInfo.get.stationSource( rsi );
    int                      row_theme       = ctune_UI_RSListWin_getRowTheme( win->in_focus, ( i == win->row.selected ) );

    wattron( win->canvas_win, row_theme );

    //line #1
    if( (*win_row) < win->canvas_property->rows ) {
        int name_col = 0;

        mvwprintw( win->canvas_win, (*win_row), 0, "%0*c", win->canvas_property->cols, ' ' );

        if( ( rsi_state & ctune_RadioStationInfo.IS_QUEUED ) ) {
            int queued_theme = ctune_UI_RSListWin_getQueuedIconTheme( win->in_focus, ( i == win->row.selected ) );

            wattron( win->canvas_win, queued_theme );
            mvwprintw( win->canvas_win, (*win_row), name_col, "%c ", '>' );
            wattroff( win->canvas_win, queued_theme );

            name_col += 2;
        }

        if( ( rsi_state & ctune_RadioStationInfo.IS_FAV ) ) {
            int fav_theme = ctune_UI_RSListWin_getFavTextTheme( win, station_src, ( i == win->row.selected ) );
            wattron( win->canvas_win, fav_theme );
            mvwprintw( win->canvas_win, (*win_row), name_col, "%-*s", cc_col, station_name );
            wattroff( win->canvas_win, fav_theme );
            wattron( win->canvas_win, row_theme );
        } else {
            wattron( win->canvas_win, row_theme );
            mvwprintw( win->canvas_win, (*win_row), name_col, "%-*s", cc_col, station_name );
        }

        mvwprintw( win->canvas_win, (*win_row), cc_col, "%s", ( station_cc == NULL || strlen( station_cc ) != 2 ? "??" : station_cc ) );

        *win_row += 1;
    }

    //line #2
    if( (*win_row) < win->canvas_property->rows ) {
        mvwprintw( win->canvas_win, (*win_row), 0, "%0*c", win->canvas_property->cols, ' ' );
        mvwprintw( win->canvas_win, (*win_row), 0, "%-*s", bitrate_col, ( station_tags != NULL ? station_tags : "" ) );
        mvwprintw( win->canvas_win, (*win_row), bitrate_col, "%*lu%s", MAX_BITRATE_FIELD_WIDTH, station_bitrate, win->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_BITRATE_UNIT_SHORT ) );

        //FIXME on tags with right->left texts like arabic the bitrate value and tags string are swapped
        // but are correctly displayed when the text is themed (A_BOLD, colouring... but not B/W or W/B.. why??)
        // · works ok with a string passed as such: `mvwaddstr( win->canvas_win, (*win_row), bitrate_col, "bitrate" );`
        //   but not when it's dynamic (tried char buffer[] and String_t to no avail)
        // -> Note that the problem also occurs in the `printSmallItemRow( .. )`
        *win_row += 1;
    }

    wattroff( win->canvas_win, row_theme );

    //line #3
    if( (*win_row) < win->canvas_property->rows ) {
        wattron( win->canvas_win, ctune_UI_Theme.color( CTUNE_UI_ITEM_TAB_BG ) );
        mvwhline( win->canvas_win, (*win_row), 0, ACS_HLINE, win->canvas_property->cols );
        wattroff( win->canvas_win, ctune_UI_Theme.color( CTUNE_UI_ITEM_TAB_BG ) );
        *win_row += 1;
    }
}

/**
 * [PRIVATE] Prints a control row in slim format
 * @param win     RSListWin_t object
 * @param win_row Pointer to the windows's row index
 */
static void ctune_UI_RSListWin_printSmallCtrlRow( ctune_UI_RSListWin_t * win, int * win_row ) {
    String_t line1 = String.init();

    if( win->row.ctrl_row_fn != NULL ) {
        String.set( &line1, win->cb.getDisplayText( CTUNE_UI_TEXT_FETCH_MORE_RESULTS ) );
    } else {
        String.set( &line1, win->cb.getDisplayText( CTUNE_UI_TEXT_NULL_RSI_RESULTS ) );
    }

    const size_t row_mid             = ( win->canvas_property->cols / 2 );
    const size_t final_line1_txt_mid = ( String.length( &line1 ) / 2 );
    int          final_line1_col     = 0;

    //assumption: the text will always fit?
    if( !ctune_utoi( ( row_mid - final_line1_txt_mid ), &final_line1_col ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_RSListWin_printSmallCtrlRow( %p, %p )] "
                   "Calculating the mid-point of row caused an integer overflow - using col=0.",
                   win, win_row
        );
    }


    int row_theme = ctune_UI_RSListWin_getRowTheme( win->in_focus, ( win->row.selected >= Vector.size( &win->entries ) ) );
    wattron( win->canvas_win, row_theme );

    //line #1
    mvwhline( win->canvas_win, (*win_row), 0, ' ', win->canvas_property->cols );
    mvwprintw( win->canvas_win, (*win_row)++, final_line1_col, "%s", line1._raw );

    wattroff( win->canvas_win, row_theme );
    String.free( &line1 );
}

/**
 * [PRIVATE] Prints a control row in large format
 * @param win     RSListWin_t object
 * @param win_row Pointer to the windows's row index
 */
static void ctune_UI_RSListWin_printLargeCtrlRow( ctune_UI_RSListWin_t * win, int * win_row ) {
    String_t line1 = String.init();
    String_t line2 = String.init();

    if( win->row.ctrl_row_fn != NULL ) {
        String.set( &line1, win->cb.getDisplayText( CTUNE_UI_TEXT_FETCH_MORE_RESULTS ) );
        String.set( &line2, win->cb.getDisplayText( CTUNE_UI_TEXT_ELLIPSIS_LINE ) );
    } else {
        String.set( &line1, win->cb.getDisplayText( CTUNE_UI_TEXT_NULL_RSI_RESULTS ) );
        String.set( &line2, win->cb.getDisplayText( CTUNE_UI_TEXT_STOP_LINE ) );
    }

    const size_t row_mid             = ( win->canvas_property->cols / 2 );
    const size_t final_line1_txt_mid = ( String.length( &line1 ) / 2 );
    const size_t final_line2_txt_mid = ( String.length( &line2 ) / 2 );
    int          final_line1_col     = 0;
    int          final_line2_col     = 0;

    //assumption: the text will always fit?
    if( !ctune_utoi( ( row_mid - final_line1_txt_mid ), &final_line1_col ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_RSListWin_printLargeCtrlRow( %p, %p )] "
                   "Calculating the mid-point of row caused an integer overflow - using col=0.",
                   win, win_row
        );
    }

    if( !ctune_utoi( ( row_mid - final_line2_txt_mid ), &final_line2_col ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_RSListWin_printLargeCtrlRow( %p, %p )] "
                   "Calculating the mid-point of row caused an integer overflow - using col=0.",
                   win, win_row
        );
    }

    int row_theme = ctune_UI_RSListWin_getRowTheme( win->in_focus, ( win->row.selected >= Vector.size( &win->entries ) )
    );
    wattron( win->canvas_win, row_theme );

    //line #1
    mvwhline( win->canvas_win, (*win_row), 0, ' ', win->canvas_property->cols );
    mvwprintw( win->canvas_win, (*win_row)++, final_line1_col, "%s", line1._raw );

    //line #2
    if( (*win_row) < win->canvas_property->rows ) {
        mvwhline( win->canvas_win, (*win_row), 0, ' ', win->canvas_property->cols );
        mvwprintw( win->canvas_win, (*win_row), final_line2_col, "%s", line2._raw );
    }

    wattroff( win->canvas_win, row_theme );
    String.free( &line1 );
    String.free( &line2 );
}

/**
 * [PRIVATE] Draw entries to the canvas window
 * @param win    RSListWin_t object
 * @param resize Flag for resizing the canvas/panel
 */
static void ctune_UI_RSListWin_drawCanvas( ctune_UI_RSListWin_t * win, bool resize ) {
    const size_t complete_entries_per_page = ( win->canvas_property->rows / win->row.row_height );
    const size_t page_index_offset         = complete_entries_per_page - 1;
    const size_t partly_shown_entry        = ( ( win->canvas_property->rows % win->row.row_height ) ? 1 : 0 );

    if( resize ) {
        if( win->canvas_panel ) {
            del_panel( win->canvas_panel );
            win->canvas_panel = NULL;
        }

        if( win->canvas_win ) {
            delwin( win->canvas_win );
            win->canvas_win = NULL;
        }

        win->row.first_on_page = 0;
        win->row.last_on_page  = win->row.first_on_page + ( page_index_offset + partly_shown_entry );
    }

    if( win->canvas_win == NULL ) {
        win->canvas_win   = newwin( win->canvas_property->rows, win->canvas_property->cols, win->canvas_property->pos_y, win->canvas_property->pos_x );
        win->canvas_panel = new_panel( win->canvas_win );
    } else {
        werase( win->canvas_win );
    }

    if( win->indicator_win != NULL ) {
        del_panel( win->indicator_panel );
        delwin( win->indicator_win );
    }


    //calculate the page range based on the currently selected RSI
    if( win->row.selected <= win->row.first_on_page ) {
        win->row.first_on_page = win->row.selected;
        win->row.last_on_page  = win->row.first_on_page + ( page_index_offset + partly_shown_entry );

    } else if( win->row.selected >= win->row.last_on_page ) {
        if( partly_shown_entry > 0 ) { //partly shown
            win->row.last_on_page = win->row.selected + 1;
            win->row.first_on_page = win->row.selected - page_index_offset;
        } else if( win->row.selected > win->row.last_on_page ) {
            win->row.last_on_page = win->row.selected;
            win->row.first_on_page = win->row.last_on_page - page_index_offset;
        }
    }


    //calculate the scrolling indicator properties + create window from that
    String_t entry_count = String.init();
    ctune_utos( Vector.size( &win->entries ), &entry_count );

    const size_t entry_count_ln     = String.length( &entry_count ); // max of "n" length
    const int    entry_indicator_ln = (int) ( entry_count_ln * 2 ) + 3; // "[n/n]"
    const int    indicator_pos_y    = ( win->canvas_property->pos_y + win->canvas_property->rows );
    const int    indicator_pos_x    = ( ( win->canvas_property->cols / 2 ) - ( entry_indicator_ln / 2 ) ) + win->canvas_property->pos_x;

    win->indicator_win   = newwin( 1, entry_indicator_ln, indicator_pos_y, indicator_pos_x );
    win->indicator_panel = new_panel( win->indicator_win );

    wattron( win->indicator_win, ctune_UI_Theme.color( CTUNE_UI_ITEM_TAB_BG ) );

    if( win->row.selected >= Vector.size( &win->entries ) )
        mvwprintw( win->indicator_win, 0, 0, "[%*c/%lu]", entry_count_ln, '-', Vector.size( &win->entries ) );
    else
        mvwprintw( win->indicator_win, 0, 0, "[%*i/%lu]", entry_count_ln, ( win->row.selected + 1 ), Vector.size( &win->entries ) );

    wattroff( win->indicator_win, ctune_UI_Theme.color( CTUNE_UI_ITEM_TAB_BG ) );

    String.free( &entry_count );


    //Fill canvas with entries in the page range
    int row = 0;
    for( size_t i = win->row.first_on_page; i <= win->row.last_on_page; ++i ) {
        if( i >= Vector.size( &win->entries ) )
            break;

        ctune_RadioStationInfo_t * rsi = Vector.at( &win->entries, i );

        if( win->row.large_row )
            ctune_UI_RSListWin_printLargeItemRow( win, rsi, i, &row );
        else
            ctune_UI_RSListWin_printSmallItemRow( win, rsi, i, &row );
    }

    //Write control row if required and in page
    if( win->row.show_ctrl_row && row < win->canvas_property->rows ) {
        if( win->row.large_row )
            ctune_UI_RSListWin_printLargeCtrlRow( win, &row );
        else
            ctune_UI_RSListWin_printSmallCtrlRow( win, &row );
    }

    win->redraw = false;
}

/**
 * Creates an initialised RSListWin_t
 * @param canvas_property Canvas property to base sizes on
 * @param getDisplayText  Callback method to get UI text
 * @param getStations     Callback method to fetch more stations
 * @param toggleFavourite Callback method to toggle a station's "favourite" status
 * @param getStationState Callback method to get a station's queued/favourite state
 * @return Initialised object
 */
static ctune_UI_RSListWin_t ctune_UI_RSListWin_init(
    const WindowProperty_t * canvas_property,
    const char * (* getDisplayText)( ctune_UI_TextID_e ),
    bool         (* getStations)( ctune_RadioBrowserFilter_t *, Vector_t * ),
    bool         (* toggleFavourite)( ctune_RadioStationInfo_t *, ctune_StationSrc_e ),
    unsigned     (* getStationState)( const ctune_RadioStationInfo_t * ) )
{
    return (ctune_UI_RSListWin_t) {
        .entries         = Vector.init( sizeof( ctune_RadioStationInfo_t ), ctune_RadioStationInfo.freeContent ),
        .canvas_property = canvas_property,
        .canvas_panel    = NULL,
        .canvas_win      = NULL,
        .indicator_panel = NULL,
        .indicator_win   = NULL,
        .redraw          = true,
        .in_focus        = true,
        .row = {
            .large_row     = false,
            .theme_favs    = true,
            .row_height    = SMALL_ENTRY_ROW_HEIGHT,
            .first_on_page = 0,
            .last_on_page  = 0,
            .selected      = 0,
            .show_ctrl_row = false,
            .ctrl_row_fn   = NULL,
        },
        .sizes = {
            .name_ln = 0,
            .cc_ln   = 2, //always 2 chars
            .tags_ln = 0,
            .kbps_ln = MAX_BITRATE_FIELD_WIDTH + strlen( getDisplayText( CTUNE_UI_TEXT_LABEL_BITRATE_UNIT_SHORT ) ),
        },
        .cache = {
            .filter = NULL
        },
        .cb = {
            .getDisplayText  = getDisplayText,
            .getStations     = getStations,
            .toggleFavourite = toggleFavourite,
            .getStationState = getStationState,
        },
    };
}

/**
 * Sets big row displaying on/off (false: 1 x line, true: 2 x lines + line delimiter)
 * @param win        RSListWin_t object
 * @param large_flag Flag to turn feature on/off
 */
static void ctune_UI_RSListWin_setLargeRow( ctune_UI_RSListWin_t * win, bool large_flag ) {
    win->row.large_row  = large_flag;
    win->row.row_height = ( large_flag ? LARGE_ENTRY_ROW_HEIGHT : SMALL_ENTRY_ROW_HEIGHT );
}

/**
 * Sets the theming of 'favourite' stations on/off
 * @param win        RSListWin_t object
 * @param theme_flag Switch to turn on/off theming for rows of favourite stations
 */
void ctune_UI_RSListWin_setFavTheming( ctune_UI_RSListWin_t * win, bool theme_flag ) {
    win->row.theme_favs = theme_flag;
}

/**
 * Sets the internal flag to show a control row
 * @param win           RSListWin_t object
 * @param show_ctrl_row Show control row flag
 */
static void ctune_UI_RSListWin_showCtrlRow( ctune_UI_RSListWin_t * win, bool show_ctrl_row ) {
    win->row.show_ctrl_row = show_ctrl_row;
}

/**
 * Loads a set of results into RSListWin's internal store
 * @param win     RSListWin_t object
 * @param results Collection of RadioStationInfo_t objects
 * @param filter  Filter used (used for fetching more with the ctrl row when applicable)
 * @return Success
 */
static bool ctune_UI_RSListWin_loadResults( ctune_UI_RSListWin_t * win, Vector_t * results, const ctune_RadioBrowserFilter_t * filter ) {
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_RSListWin_loadResults( %p, %p, %p )] Loading stations to RSListWin.", win, results, filter );

    if( results == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_RSListWin_loadResults( %p, %p, %p )] Vector of results is NULL.", win, results, filter );
        return false;
    }

    if( win->cache.filter != NULL ) { //clear out old filter
        ctune_RadioBrowserFilter.freeContent( win->cache.filter );
        free( win->cache.filter );
        win->cache.filter = NULL;
    }

    if( filter != NULL ) { //copy filter
        if( ( win->cache.filter = malloc( sizeof( ctune_RadioBrowserFilter_t ) ) ) == NULL ) {
            ctune_err.set( CTUNE_ERR_MALLOC );
            win->row.ctrl_row_fn = NULL;

            CTUNE_LOG( CTUNE_LOG_FATAL,
                       "[ctune_UI_RSListWin_loadResults( %p, &p, &p )] "
                       "Failed memory allocation for cached RadioBrowserFilter_t.",
                       win, results, filter );

            return false; //EARLY RETURN
        }

        *win->cache.filter = ctune_RadioBrowserFilter.init();
        ctune_RadioBrowserFilter.copy( filter, win->cache.filter );
    }

    //reset everything
    Vector.reinit( &win->entries );
    win->sizes.name_ln     = 0;
    win->sizes.tags_ln     = 0;
    win->row.first_on_page = 0;
    win->row.last_on_page  = 0;
    win->row.selected      = 0;

    //fill ListWin_t entry Vector with results
    for( size_t i = 0; i < Vector.size( results ); ++i ) {
        ctune_RadioStationInfo_t * rsi  = Vector.at( results, i );
        ctune_RadioStationInfo_t * copy = Vector.init_back( &win->entries, ctune_RadioStationInfo.init );

        if( copy == NULL ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_RSListWin_loadResults( %p, %p, %p )] "
                       "Failed to Initialise RSI (i=%lu) in Vector (<%s>)",
                       win, results, filter, i, ctune_RadioStationInfo.get.stationUUID( rsi )
            );
            return false;
        };

        ctune_RadioStationInfo.copy( rsi, copy );

        size_t name_ln = ( ctune_RadioStationInfo.get.stationName( rsi ) != NULL ? strlen( ctune_RadioStationInfo.get.stationName( rsi ) ) : 0 );
        size_t tags_ln = ( ctune_RadioStationInfo.get.tags( rsi )        != NULL ? strlen( ctune_RadioStationInfo.get.tags( rsi )        ) : 0 );

        if( name_ln > win->sizes.name_ln )
            win->sizes.name_ln = name_ln;
        if( tags_ln > win->sizes.tags_ln )
            win->sizes.tags_ln = tags_ln;
    }

    //set final row
    if( Vector.empty( &win->entries ) || ( win->cache.filter != NULL && ctune_RadioBrowserFilter.get.resultLimit( win->cache.filter ) != 0 ) ) {
        win->row.ctrl_row_fn = &ctune_UI_RSListWin_fetchMoreItems;
    } else {
        win->row.ctrl_row_fn = NULL;
    }

    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_UI_RSListWin_loadResults( %p, %p, %p )] Loaded %lu stations.",
               win, results, filter, Vector.size( &win->entries )
    );

    win->redraw = true;
    return true;
}

/**
 * Loads a set of results to append to RSListWin's internal store
 * @param win     RSListWin_t object
 * @param results Collection of RadioStationInfo_t objects
 * @return Success
 */
static bool ctune_UI_RSListWin_appendResults( ctune_UI_RSListWin_t * win, Vector_t * results ) {
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_RSListWin_appendResults( %p, %p )] Appending stations to RSListWin.", win, results );

    if( results == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_RSListWin_appendResults( %p, %p )] Vector of results is NULL.", win, results )
        return false;
    }

    //append ListWin_t entry Vector with results
    for( size_t i = 0; i < Vector.size( results ); ++i ) {
        ctune_RadioStationInfo_t * rsi  = Vector.at( results, i );
        ctune_RadioStationInfo_t * copy = Vector.init_back( &win->entries, ctune_RadioStationInfo.init );

        if( copy == NULL ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_RSListWin_appendResults( %p, %p )] "
                       "Failed to Initialise RSI (i=%lu) in Vector (<%s>)",
                       win, results, i, ctune_RadioStationInfo.get.stationUUID( rsi )
            );
            return false;
        };

        ctune_RadioStationInfo.copy( rsi, copy );

        size_t name_ln = ( ctune_RadioStationInfo.get.stationName( rsi ) != NULL ? strlen( ctune_RadioStationInfo.get.stationName( rsi ) ) : 0 );
        size_t tags_ln = ( ctune_RadioStationInfo.get.tags( rsi )        != NULL ? strlen( ctune_RadioStationInfo.get.tags( rsi ) )        : 0 );

        if( name_ln > win->sizes.name_ln )
            win->sizes.name_ln = name_ln;
        if( tags_ln > win->sizes.tags_ln )
            win->sizes.tags_ln = tags_ln;
    }

    //set final row
    if( Vector.empty( results ) || Vector.size( results ) < ctune_RadioBrowserFilter.get.resultLimit( win->cache.filter ) ) { //looks like there is no more to be fetched
        win->row.ctrl_row_fn = NULL;
    } else {
        win->row.ctrl_row_fn = &ctune_UI_RSListWin_fetchMoreItems;
    }

    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_UI_RSListWin_appendResults( %p, %p )] Loaded %lu/%lu stations.",
               win, results, Vector.size( results ), Vector.size( &win->entries )
    );

    win->redraw = true;
    return true;
}

/**
 * Clears whatever rows there are present and defaults back to "no results"
 * @param win RSListWin_t object
 */
static void ctune_UI_RSListWin_loadNothing( ctune_UI_RSListWin_t * win ) {
    //reset everything
    Vector.reinit( &win->entries );
    win->sizes.name_ln     = 0;
    win->sizes.tags_ln     = 0;
    win->row.first_on_page = 0;
    win->row.last_on_page  = 0;
    win->row.selected      = 0;

    //set final row
    win->row.ctrl_row_fn = NULL;

    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_RSListWin_loadNothing( %p )] Loaded nothing.", win );

    win->redraw = true;
}

/**
 * Sets the redraw flag on
 * @param win RSListWin_t object
 */
static void ctune_UI_RSListWin_setRedraw( ctune_UI_RSListWin_t * win ) {
    win->redraw = true;
}

/**
 * Sets the 'in focus' flag
 * @param win   RSListWin_t object
 * @param focus Flag value
 */
static void ctune_UI_RSListWin_setFocus( ctune_UI_RSListWin_t * win, bool focus ) {
    win->in_focus = focus;
}

/**
 * Change selected row to previous entry
 * @param win RSListWin_t object
 */
static void ctune_UI_RSListWin_selectUp( ctune_UI_RSListWin_t * win ) {
    ctune_UI_RSListWin_selectRow( win, -1 );
}

/**
 * Change selected row to next entry
 * @param win RSListWin_t object
 */
static void ctune_UI_RSListWin_selectDown( ctune_UI_RSListWin_t * win ) {
    ctune_UI_RSListWin_selectRow( win, +1 );
}

/**
 * Change selected row to entry a page length away
 * @param win RSListWin_t object
 */
static void ctune_UI_RSListWin_selectPageUp( ctune_UI_RSListWin_t * win ) {
    int xtra = ( ( win->canvas_property->rows % win->row.row_height ) > 0 ? 0 : 1 );
    ctune_UI_RSListWin_selectRow( win, -( win->canvas_property->rows / win->row.row_height + xtra) );
}

/**
 * Change selected row to entry a page length away
 * @param win RSListWin_t object
 */
static void ctune_UI_RSListWin_selectPageDown( ctune_UI_RSListWin_t * win ) {
    int xtra = ( ( win->canvas_property->rows % win->row.row_height ) > 0 ? 0 : 1 );
    ctune_UI_RSListWin_selectRow( win, +( win->canvas_property->rows / win->row.row_height + xtra ) );
}

/**
 * Change selection to the first item in the list
 * @param win RSListWin_t object
 */
static void ctune_UI_RSListWin_selectFirst( ctune_UI_RSListWin_t * win ) {
    win->row.selected = 0;
    win->redraw       = true;
}

/**
 * Change selection to the last item in the list
 * @param win RSListWin_t object
 */
static void ctune_UI_RSListWin_selectLast( ctune_UI_RSListWin_t * win ) {
    win->row.selected = (int) Vector.size( &win->entries ) - ( win->row.show_ctrl_row ? 0 : 1 );
    win->redraw       = true;
}

/**
 * Toggles the "favourite" status of the currently selected item
 * @param win RSListWin_t object
 */
static void ctune_UI_RSListWin_toggleFav( ctune_UI_RSListWin_t * win ) {
    if( win->row.selected >= Vector.size( &win->entries ) ) {
        CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_UI_RSListWin_toggleFav( %p )] Out-of-range row selected.", win );
        return; //EARLY RETURN
    }

    ctune_RadioStationInfo_t * rsi = Vector.at( &win->entries, win->row.selected );
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_RSListWin_toggleFav( %p )] Toggling 'favourite' state of station", rsi );

    if( rsi != NULL ) {
        if( !win->cb.toggleFavourite( rsi, ctune_RadioStationInfo.get.stationSource( rsi ) ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_RSListWin_toggleFav( %p )] Failed toggling.", win );
        }

        win->redraw = true;

    } else {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_RSListWin_toggleFav( %p )] Selected row (%ul) returned NULL in Vector_t of entries.",
                   win, win->row.selected
        );
    }
}

/**
 * Gets a RSI pointer to the currently selected item in list or if ctrl row then trigger callback
 * @param RSListWin_t object
 * @return RadioStationInfo_t object pointer or NULL if out of range of the collection/is ctrl row
 */
static const ctune_RadioStationInfo_t * ctune_UI_RSListWin_getSelected( ctune_UI_RSListWin_t * win ) {
    if( win->row.selected > Vector.size( &win->entries ) ) {
        CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_UI_RSListWin_getSelected( %p )] Out-of-range row selected.", win );
        return NULL; //EARLY RETURN
    }

    if( win->row.selected == Vector.size( &win->entries ) ) { //CTRL row selected
        CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_UI_RSListWin_getSelected( %p )] CTRL row selected.", win );
        if( win->row.ctrl_row_fn != NULL ) {
            CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_UI_RSListWin_getSelected( %p )] CTRL row cb exists.", win );
            if( !win->row.ctrl_row_fn( win ) ) {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_UI_RSListWin_getSelected( %p )] "
                           "Ctrl row function failed.",
                           win
                );

                return NULL; //EARLY RETURN
            }
        }

        return NULL; //EARLY RETURN
    }

    return Vector.at( &win->entries, win->row.selected );
}

/**
 * Gets the internal index of the currently selected row
 * @param win RSListWin_t object
 * @return Index
 */
static ctune_UI_RSListWin_PageState_t ctune_UI_RSListWin_getSelectedIndex( ctune_UI_RSListWin_t * win ) {
    return (ctune_UI_RSListWin_PageState_t) {
        .first_on_page = win->row.first_on_page,
        .last_on_page  = win->row.last_on_page,
        .selected      = win->row.selected
    };
}

/**
 * Sets the selected row by its index
 * @param win   RSListWin_t object
 * @param index Index to set
 * @return Success
 */
static bool ctune_UI_RSListWin_setSelected( ctune_UI_RSListWin_t * win, ctune_UI_RSListWin_PageState_t state ) {
    if( state.selected > Vector.size( &win->entries ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_RSListWin_setSelected( %p, %lu )] "
                   "Index is out of range of current entries.",
                   win
        );

        return false; //EARLY RETURN
    }

    if( state.selected != win->row.selected ) {
        win->row.first_on_page = state.first_on_page;
        win->row.last_on_page  = state.last_on_page;
        win->row.selected      = state.selected;
        win->row.selected      = state.selected;
        win->redraw            = true;
    }

    return true;
}

/**
 * Checks if the current row selected is the ctrl row
 * @param win RSListWin_t object
 * @return is the Ctrl row
 */
static bool ctune_UI_RSListWin_isCtrlRowSelected( ctune_UI_RSListWin_t * win ) {
    return ( win->row.selected == Vector.size( &win->entries ) );
}

/**
 * Triggers the Ctrl row callback if there is one
 * @param win RSListWin_t object
 * @return Callback's success (also `true` if no callback)
 */
static bool ctune_UI_RSListWin_triggerCtrlRow( ctune_UI_RSListWin_t * win ) {
    if( win->row.ctrl_row_fn != NULL ) {
        if( !win->row.ctrl_row_fn( win ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_RSListWin_getSelected( %p )] "
                       "Ctrl row function failed.",
                       win
            );

            return false; //EARLY RETURN
        }
    }

    return true;
}

/**
 * Show updated window
 * @param win RSListWin_t object
 * @return Success
 */
static bool ctune_UI_RSListWin_show( ctune_UI_RSListWin_t * win ) {
    if( win == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_RSListWin_show( %p )] RSListWin is NULL.", win );
        return false; //EARLY RETURN
    }

    if( win->redraw ) {
        ctune_UI_RSListWin_drawCanvas( win, false );
    }

    if( win->canvas_panel == NULL || win->indicator_panel == NULL ||
        win->canvas_win   == NULL || win->indicator_win   == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_RSListWin_show( %p )] NULL Panel/Win detected.", win );
        return false; //EARLY RETURN
    }

    top_panel( win->canvas_panel );
    top_panel( win->indicator_panel );

    wrefresh( win->canvas_win );
    wrefresh( win->indicator_win );

    update_panels();
    doupdate();

    win->redraw = false;

    return true;
}

/**
 * Redraws the window
 * @param win RSListWin_t object
 */
static void ctune_UI_RSListWin_resize( ctune_UI_RSListWin_t * win ) {
    CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_UI_RSListWin_resize( %p )] Resize event called.", win );

    if( win == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_RSListWin_resize( %p )] RSListWin is NULL.", win );
        return; //EARLY RETURN
    }

    ctune_UI_RSListWin_drawCanvas( win, true );

    if( win->canvas_panel == NULL || win->indicator_panel == NULL ||
        win->canvas_win   == NULL || win->indicator_win   == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_RSListWin_resize( %p )] NULL Panel/Win detected.", win );
        return; //EARLY RETURN
    }

    top_panel( win->canvas_panel );
    top_panel( win->indicator_panel );

    wrefresh( win->canvas_win );
    wrefresh( win->indicator_win );

    update_panels();
    doupdate();
}

/**
 * De-allocates RSListWin variables
 * @param win RSListWin_t object
 */
static void ctune_UI_RSListWin_free( ctune_UI_RSListWin_t * win ) {
    if( win ) {
        win->redraw = true;

        if( win->canvas_panel ) {
            del_panel( win->canvas_panel );
            win->canvas_panel = NULL;
        }

        if( win->canvas_win ) {
            delwin( win->canvas_win );
            win->canvas_win = NULL;
        }

        if( win->indicator_panel ) {
            del_panel( win->indicator_panel );
            win->indicator_panel = NULL;
        }

        if( win->indicator_win != NULL ) {
            delwin( win->indicator_win );
            win->indicator_win = NULL;
        }

        win->row.first_on_page = 0;
        win->row.last_on_page  = 0;
        win->row.selected      = 0;
        Vector.clear_vector( &win->entries );

        if( win->cache.filter != NULL ) {
            ctune_RadioBrowserFilter.freeContent( win->cache.filter );
            free( win->cache.filter );
            win->cache.filter = NULL;
        }

        CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_RSListWin_free( %p )] RSListWin freed.", win );
    }
};

/**
 * Namespace constructor
 */
struct ctune_UI_RSListWinClass ctune_UI_RSListWin = {
    .init              = &ctune_UI_RSListWin_init,
    .setLargeRow       = &ctune_UI_RSListWin_setLargeRow,
    .themeFavourites   = &ctune_UI_RSListWin_setFavTheming,
    .showCtrlRow       = &ctune_UI_RSListWin_showCtrlRow,
    .loadResults       = &ctune_UI_RSListWin_loadResults,
    .appendResults     = &ctune_UI_RSListWin_appendResults,
    .loadNothing       = &ctune_UI_RSListWin_loadNothing,
    .selectUp          = &ctune_UI_RSListWin_selectUp,
    .selectDown        = &ctune_UI_RSListWin_selectDown,
    .selectPageUp      = &ctune_UI_RSListWin_selectPageUp,
    .selectPageDown    = &ctune_UI_RSListWin_selectPageDown,
    .selectFirst       = &ctune_UI_RSListWin_selectFirst,
    .selectLast        = &ctune_UI_RSListWin_selectLast,
    .toggleFav         = &ctune_UI_RSListWin_toggleFav,
    .getSelected       = &ctune_UI_RSListWin_getSelected,
    .getPageState      = &ctune_UI_RSListWin_getSelectedIndex,
    .setPageState      = &ctune_UI_RSListWin_setSelected,
    .isCtrlRowSelected = &ctune_UI_RSListWin_isCtrlRowSelected,
    .triggerCtrlRow    = &ctune_UI_RSListWin_triggerCtrlRow,
    .setRedraw         = &ctune_UI_RSListWin_setRedraw,
    .setFocus          = &ctune_UI_RSListWin_setFocus,
    .show              = &ctune_UI_RSListWin_show,
    .resize            = &ctune_UI_RSListWin_resize,
    .free              = &ctune_UI_RSListWin_free,
};