#include "ScrollBar.h"

#include "../definitions/Theme.h"
#include "../definitions/Icons.h"
#include "logger/src/Logger.h"
#include "../../utils/utilities.h"

/**
 * [PRIVATE] Gets the text description of a position
 * @param pos Scrollbar position ID
 * @return Description string for position
 */
static const char * ctune_UI_Widget_ScrollBar_posStr( ctune_UI_ScrollBar_Pos_e pos ) {
    static const char * arr[] = {
        [LEFT  ] = "left",
        [RIGHT ] = "right",
        [TOP   ] = "top",
        [BOTTOM] = "bottom",
    };

    if( pos >= 0 && pos <= 3 )
        return arr[pos];

    return "Unknown";
}

/**
 * [PRIVATE] Draw entries to the canvas window
 * @param scrollbar Pointer to ctune_UI_ScrollBar_t object
 */
static void ctune_UI_Widget_ScrollBar_drawCanvas( ctune_UI_ScrollBar_t * scrollbar ) {
    if( scrollbar->canvas_win == NULL ) {
        ctune_UI_ScrollBar.initCanvas( scrollbar );
    }

    werase( scrollbar->canvas_win );

    if( scrollbar->always_show || scrollbar->scroll_length > (uint) scrollbar->canvas_properties->rows ) {
        int  scroller_size  = 0;
        int  display_offset = 0;
        bool error_flag     = false;

        if( !ctune_utoi( scrollbar->scroller_size, &scroller_size ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Widget_ScrollBar_drawCanvas( %p )] Failed to cast %lu to an integer.",
                       scrollbar, scrollbar->scroller_size
            );

            error_flag = true;
        }

        size_t offset    = 0;
        double mid_point = ( (double) scrollbar->scroll_length ) / 2;

        if( scrollbar->scroller_pos < mid_point ) {
            offset = (size_t) floor( scrollbar->scroller_pos );
        } else {
            offset = (size_t) ceil( scrollbar->scroller_pos );
        }

        if( !ctune_utoi( offset, &display_offset ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Widget_ScrollBar_drawCanvas( %p )] Failed to cast %f to an integer.",
                       scrollbar, scrollbar->scroller_pos
            );

            error_flag = true;
        }

        if( !error_flag ) {
            wattron( scrollbar->canvas_win, ctune_UI_Theme.color( CTUNE_UI_ITEM_SCROLL_INDICATOR ) );

            if( scrollbar->scroll_ctrl ) { //show arrow control icons
                display_offset += 1; //shift bar to just after control character

                switch( scrollbar->bar_position ) {
                    case TOP: //fallthrough
                    case BOTTOM: {
                        mvwaddstr( scrollbar->canvas_win, 0, 0, ctune_UI_Icons.icon( CTUNE_UI_ICON_SCROLL_LEFT ) );
                        mvwhline( scrollbar->canvas_win, 0, display_offset, ACS_HLINE, scroller_size );
                        mvwaddstr( scrollbar->canvas_win, 0, ( scrollbar->bar_canvas_length - 1 ), ctune_UI_Icons.icon( CTUNE_UI_ICON_SCROLL_RIGHT ) );
                    } break;

                    case LEFT: //fallthrough
                    case RIGHT: {
                        mvwaddstr( scrollbar->canvas_win, 0, 0, ctune_UI_Icons.icon( CTUNE_UI_ICON_SCROLL_UP ) );
                        mvwvline( scrollbar->canvas_win, display_offset, 0, ACS_VLINE, scroller_size );
                        mvwaddstr( scrollbar->canvas_win, ( scrollbar->bar_canvas_length - 1 ), 0, ctune_UI_Icons.icon( CTUNE_UI_ICON_SCROLL_DOWN ) );
                    } break;
                }

            } else {
                switch( scrollbar->bar_position ) {
                    case TOP   : //fallthrough
                    case BOTTOM: {
                        mvwhline( scrollbar->canvas_win, 0, display_offset, ACS_HLINE, scroller_size );
                    } break;

                    case LEFT : //fallthrough
                    case RIGHT: {
                        mvwvline( scrollbar->canvas_win, display_offset, 0, ACS_VLINE, scroller_size );
                    } break;
                }
            }

            wattroff( scrollbar->canvas_win, ctune_UI_Theme.color( CTUNE_UI_ITEM_SCROLL_INDICATOR ) );
        }
    }

    scrollbar->redraw = false;
}

/**
 * Initialises a slide menu
 * @param canvas      Pointer to parent canvas property
 * @param pos         Relative position of the vertical scroll bar
 * @param always_show Flag to always show the scroller in the scroll bar
 * @return Pointer to ctune_UI_ScrollBar_t object
 */
static ctune_UI_ScrollBar_t ctune_UI_Widget_ScrollBar_init( const WindowProperty_t * canvas, ctune_UI_ScrollBar_Pos_e pos, bool always_show ) {
    return (ctune_UI_ScrollBar_t) {
        .canvas_properties = canvas,
        .canvas_win        = NULL,
        .canvas_panel      = NULL,
        .bar_position      = pos,
        .redraw            = true,
        .scroll_ctrl       = false,
        .always_show       = always_show,
        .bar_canvas_length = ( pos == LEFT || pos == RIGHT ? canvas->rows : canvas->cols ),
        .bar_length        = ( pos == LEFT || pos == RIGHT ? canvas->rows : canvas->cols ),
        .scroll_length     = ( pos == LEFT || pos == RIGHT ? canvas->rows : canvas->cols ),
        .page_count        = 1,
        .page_increments   = 0,
        .curr_increment    = 0,
        .page_inc_val      = 1.0,
        .scroller_size     = 1,
        .scroller_pos      = 0.0,
    };
}

/**
 * Sets the redraw flag on
 * @param scrollbar Pointer to ctune_UI_ScrollBar_t object
 */
static void ctune_UI_Widget_ScrollBar_setRedraw( ctune_UI_ScrollBar_t * scrollbar ) {
    scrollbar->redraw = true;
}

/**
 * Sets the flag to show the scroll controls in the scroll bar
 * @param scrollbar Pointer to ctune_UI_ScrollBar_t object
 * @param flag      Flag value
 */
static void ctune_UI_Widget_ScrollBar_setShowControls( ctune_UI_ScrollBar_t * scrollbar, bool flag ) {
    scrollbar->scroll_ctrl = flag;
}

/**
 * Sets the flag to always show the scroller in the scroll bar
 * @param scrollbar Pointer to ctune_UI_ScrollBar_t object
 * @param flag      Flag value
 */
static void ctune_UI_Widget_ScrollBar_setAlwaysShow( ctune_UI_ScrollBar_t * scrollbar, bool flag ) {
    scrollbar->always_show = flag;
}

/**
 * Sets the base scrolling length
 * @param scrollbar    Pointer to ctune_UI_ScrollBar_t object
 * @param content_size Size to the content represented by the scrollbar
 */
static void ctune_UI_Widget_ScrollBar_setScrollLength( ctune_UI_ScrollBar_t * scrollbar, size_t content_size ) {
    //init base values
    scrollbar->bar_length        = ( scrollbar->bar_position == LEFT || scrollbar->bar_position == RIGHT
                                     ? scrollbar->canvas_properties->rows
                                     : scrollbar->canvas_properties->cols );
    scrollbar->bar_canvas_length = scrollbar->bar_length;
    scrollbar->scroll_length     = content_size;
    scrollbar->page_count        = 0;
    scrollbar->scroller_size     = 1;
    scrollbar->page_increments   = 0;
    scrollbar->curr_increment    = 0;
    scrollbar->page_inc_val      = 1.0;
    scrollbar->scroller_pos      = 0.0;

    if( scrollbar->scroll_ctrl ) {
        if( scrollbar->bar_length <= 2 ) {
            CTUNE_LOG( CTUNE_LOG_WARNING,
                       "[ctune_UI_Widget_ScrollBar_setScrollLength( %p, %lu )] "
                       "Not enough space scroll controls to be displayed.",
                       scrollbar, content_size
            );

            ctune_UI_ScrollBar.setShowControls( scrollbar, false );

        } else {
            scrollbar->bar_length -= 2; //make space for scroll arrow icons
        }
    }

    if( scrollbar->scroll_length > scrollbar->bar_length ) {
        //total number of overlapping pages until the very end of the source content can be seen
        scrollbar->page_count      = ( scrollbar->scroll_length - scrollbar->bar_canvas_length ) + 1;
        scrollbar->page_increments = ( scrollbar->page_count - 1 );

    } else {
        scrollbar->page_count      = 1;
        scrollbar->page_increments = 0;
    }


    if( scrollbar->page_count < scrollbar->bar_length ) {
        //fill the scroll bar minus the increments needed to switch pages
        scrollbar->scroller_size = ( scrollbar->bar_length - scrollbar->page_increments );

    } else if ( ( scrollbar->page_count > 0 ) && ( scrollbar->page_increments > scrollbar->bar_length ) ) {
        scrollbar->scroller_size = 1; //smallest scroller size
        scrollbar->page_inc_val  = ( (double) scrollbar->bar_length / (double) scrollbar->page_count );
    }

    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_UI_Widget_ScrollBar_setScrollLength( %p, %lu )] "
               "Scroll length set (show ctrl = %s, bar length = %lu, bar canvas length = % lu, page count = %lu, "
               "page increments = %lu, page inc. val = %f, scroller size = %lu, position = %s ).",
               scrollbar, content_size,
               ( scrollbar->scroll_ctrl ? "true" : "false" ), scrollbar->bar_length, scrollbar->bar_canvas_length, scrollbar->page_count,
               scrollbar->page_increments, scrollbar->page_inc_val, scrollbar->scroller_size,
               ctune_UI_Widget_ScrollBar_posStr( scrollbar->bar_position )
    );

    scrollbar->redraw = true;
}

/**
 * Sets the current position on the scrolling length
 * @param scrollbar Pointer to ctune_UI_ScrollBar_t object
 * @param pos       Scroll position on the set length
 * @return Scroll event
 */
static bool ctune_UI_Widget_ScrollBar_setPosition( ctune_UI_ScrollBar_t * scrollbar, size_t pos ) {
    if( scrollbar->page_increments > 0 ) {
        const bool   limit_reached = ( pos >= scrollbar->page_increments ); //a.k.a last page
        const double prev_pos      = scrollbar->scroller_pos;

        if( limit_reached ) {
            scrollbar->curr_increment = scrollbar->page_increments;
            scrollbar->scroller_pos   = (double)( scrollbar->bar_length - scrollbar->scroller_size );
        } else {
            scrollbar->curr_increment = pos;
            scrollbar->scroller_pos   = ( (double) pos * scrollbar->page_inc_val );
        }

        CTUNE_LOG( CTUNE_LOG_DEBUG,
                   "[ctune_UI_Widget_ScrollBar_setPosition( %p, %lu )] Scroller position: %f->%f/%lu, page increment: %lu/%lu",
                   scrollbar, pos,
                   prev_pos, scrollbar->scroller_pos, ( scrollbar->bar_length - scrollbar->scroller_size ),
                   scrollbar->curr_increment, scrollbar->page_increments
        );

        scrollbar->redraw = true;

        return ( scrollbar->scroller_pos != prev_pos );
    }

    return false;
}

/**
 * Checks if area at coordinate is a scroll button
 * @param scrollbar Pointer to ctune_UI_ScrollBar_t object
 * @param y         Row location on screen
 * @param x         Column location on screen
 * @return Scroll mask
 */
static ctune_UI_ScrollMask_m ctune_UI_Widget_ScrollBar_isScrollButton( ctune_UI_ScrollBar_t * scrollbar, int y, int x ) {
    if( scrollbar->scroll_ctrl ) {
        if( wmouse_trafo( scrollbar->canvas_win, &y, &x, false ) ) {
            switch( scrollbar->bar_position ) {
                case TOP   : //fallthrough
                case BOTTOM: {
                    if( x == 0 ) {
                        return ctune_UI_ScrollMask.createMask( 0, -1 ); //EARLY RETURN
                    }

                    if( x == scrollbar->canvas_properties->cols - 1 ) {
                        return ctune_UI_ScrollMask.createMask( 0, +1 ); //EARLY RETURN
                    }
                } break;

                case LEFT : //fallthrough
                case RIGHT: {
                    if( y == 0 ) {
                        return ctune_UI_ScrollMask.createMask( -1, 0 ); //EARLY RETURN
                    }

                    if( y == scrollbar->canvas_properties->rows - 1 ) {
                        return ctune_UI_ScrollMask.createMask( +1, 0 ); //EARLY RETURN
                    }
                } break;
            }
        }
    }

    return 0;
}

/**
 * Gets the calculated total number of page increments
 * @param scrollbar Pointer to ctune_UI_ScrollBar_t object
 * @return Page increments available
 */
static size_t ctune_UI_Widget_ScrollBar_getTotalIncrements( const ctune_UI_ScrollBar_t * scrollbar ) {
    return scrollbar->page_increments;
}

/**
 * Initiate the canvas window/panel
 * @param scrollbar Pointer to ctune_UI_ScrollBar_t object
 */
static void ctune_UI_Widget_ScrollBar_initCanvas( ctune_UI_ScrollBar_t * scrollbar ) {
    if( scrollbar->canvas_win != NULL ) {
        del_panel( scrollbar->canvas_panel );
        delwin( scrollbar->canvas_win );
    }

    const int total_bar_length = ( scrollbar->bar_position == LEFT || scrollbar->bar_position == RIGHT
                                   ? scrollbar->canvas_properties->rows
                                   : scrollbar->canvas_properties->cols );

    switch( scrollbar->bar_position ) {
        case TOP:
            scrollbar->canvas_win = newwin( 1,
                                            total_bar_length,
                                            0,
                                            scrollbar->canvas_properties->pos_x );
            break;
        case BOTTOM:
            scrollbar->canvas_win = newwin( 1,
                                            total_bar_length,
                                            ( scrollbar->canvas_properties->pos_y + scrollbar->canvas_properties->rows - 1 ),
                                            scrollbar->canvas_properties->pos_x );
            break;
        case LEFT:
            scrollbar->canvas_win = newwin( total_bar_length,
                                            1,
                                            scrollbar->canvas_properties->pos_y,
                                            0 );
            break;
        case RIGHT:
            scrollbar->canvas_win = newwin( total_bar_length,
                                            1,
                                            scrollbar->canvas_properties->pos_y,
                                            ( scrollbar->canvas_properties->pos_x + scrollbar->canvas_properties->cols - 1 ) );
            break;
        default: //just in case...
            CTUNE_LOG( CTUNE_LOG_FATAL,
                       "[ctune_UI_Widget_ScrollBar_initCanvas( %p )] Unknown bar position: %i",
                       scrollbar, scrollbar->bar_position
            );
    }

    scrollbar->canvas_panel = new_panel( scrollbar->canvas_win );
}

/**
 * Increment scroll position
 * @param scrollbar Pointer to ctune_UI_ScrollBar_t object
 * @param inc       Increment value
 */
static void ctune_UI_Widget_ScrollBar_incrementPosition( ctune_UI_ScrollBar_t * scrollbar, size_t inc ) {
    if( inc <= INT_MAX && scrollbar->curr_increment <= ( INT_MAX - inc ) ) {
        scrollbar->curr_increment += inc;

        if( scrollbar->curr_increment > scrollbar->page_increments ) {
            scrollbar->curr_increment = scrollbar->page_increments;
        }

        ctune_UI_Widget_ScrollBar_setPosition( scrollbar, scrollbar->curr_increment );
    }
}

/**
 * Increment scroll position
 * @param scrollbar Pointer to ctune_UI_ScrollBar_t object
 * @param dec       Decrement value
 */
static void ctune_UI_Widget_ScrollBar_decrementPosition( ctune_UI_ScrollBar_t * scrollbar, size_t dec ) {
    if( dec > 0 ) {
        if( dec < scrollbar->curr_increment ) {
            scrollbar->curr_increment -= dec;
        } else {
            scrollbar->curr_increment = 0;
        }

        ctune_UI_Widget_ScrollBar_setPosition( scrollbar, scrollbar->curr_increment );
    }
}

/**
 * Show updated window
 * @param scrollbar Pointer to ctune_UI_ScrollBar_t object
 * @return Success
 */
static bool ctune_UI_Widget_ScrollBar_show( ctune_UI_ScrollBar_t * scrollbar ) {
    if( scrollbar == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_Widget_ScrollBar_show( %p )] ScrollBar is NULL.", scrollbar );
        return false; //EARLY RETURN
    }

    if( scrollbar->redraw ) {
        ctune_UI_Widget_ScrollBar_drawCanvas( scrollbar );
    }

    if( scrollbar->canvas_panel == NULL || scrollbar->canvas_win == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_Widget_ScrollBar_show( %p )] NULL Panel/Win detected.", scrollbar );
        return false;
    }

    top_panel( scrollbar->canvas_panel );
    wrefresh( scrollbar->canvas_win );

    update_panels();
    doupdate();

    scrollbar->redraw = false;

    return true;
}

/**
 * Moves the scrollbar panel to the bottom and hides it
 * @param scrollbar Pointer to ctune_UI_ScrollBar_t object
 */
static void ctune_UI_Widget_ScrollBar_hide( ctune_UI_ScrollBar_t * scrollbar ) {
    if( scrollbar->canvas_panel ) {
        bottom_panel( scrollbar->canvas_panel );
        hide_panel( scrollbar->canvas_panel );
        update_panels();
    }
}

/**
 * De-allocates a slide menu's content
 * @param scrollbar Pointer to ctune_UI_ScrollBar_t object
 */
static void ctune_UI_Widget_ScrollBar_free( ctune_UI_ScrollBar_t * scrollbar ) {
    if( scrollbar ) {
        scrollbar->redraw = true;
        if( scrollbar->canvas_panel ) {
            del_panel( scrollbar->canvas_panel );
            scrollbar->canvas_panel = NULL;
        }
        if( scrollbar->canvas_win ) {
            delwin( scrollbar->canvas_win );
            scrollbar->canvas_win = NULL;
        }

        scrollbar->redraw        = true,
        scrollbar->scroll_length = ( scrollbar->canvas_properties != NULL ? scrollbar->canvas_properties->rows : 0 );
        scrollbar->page_count    = 1;
        scrollbar->page_inc_val  = 1.0;
        scrollbar->scroller_size = 1;
        scrollbar->scroller_pos  = 0.0;

        CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_UI_Widget_ScrollBar_free( %p )] ScrollBar freed.", scrollbar );
    }
}

/**
 * Namespace constructor
 */
const struct ctune_UI_Widget_ScrollBar_Namespace ctune_UI_ScrollBar = {
    .init               = &ctune_UI_Widget_ScrollBar_init,
    .setRedraw          = &ctune_UI_Widget_ScrollBar_setRedraw,
    .setShowControls    = &ctune_UI_Widget_ScrollBar_setShowControls,
    .setAlwaysShow      = &ctune_UI_Widget_ScrollBar_setAlwaysShow,
    .setScrollLength    = &ctune_UI_Widget_ScrollBar_setScrollLength,
    .setPosition        = &ctune_UI_Widget_ScrollBar_setPosition,
    .isScrollButton     = &ctune_UI_Widget_ScrollBar_isScrollButton,
    .getTotalIncrements = &ctune_UI_Widget_ScrollBar_getTotalIncrements,
    .initCanvas         = &ctune_UI_Widget_ScrollBar_initCanvas,
    .incrementPosition  = &ctune_UI_Widget_ScrollBar_incrementPosition,
    .decrementPosition  = &ctune_UI_Widget_ScrollBar_decrementPosition,
    .show               = &ctune_UI_Widget_ScrollBar_show,
    .hide               = &ctune_UI_Widget_ScrollBar_hide,
    .free               = &ctune_UI_Widget_ScrollBar_free,
};