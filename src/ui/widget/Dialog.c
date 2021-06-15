#include "Dialog.h"

#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <panel.h>

#include "../../logger/Logger.h"
#include "../definitions/Theme.h"
#include "../../utils/utilities.h"

/**
 * [PRIVATE] Scroll content by a specified amount
 * @param dialog UI_Dialog_t object
 * @param y      Rows (+ down, - up)
 * @param x      Columns (+ right, - left)
 */
static void ctune_UI_Dialog_scrollBy( ctune_UI_Dialog_t * dialog, int y, int x ) {
    ctune_UI_ScrollWin.scrollPad( &dialog->canvas, y, x );

    if( y != 0 ) {
        int max = 0;

        if( !ctune_utoi( ctune_UI_ScrollBar.getTotalIncrements( &dialog->scrollbar.y ), &max ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Dialog_scrollBy( %p, %i, %i )] Failed to cast vertical scrollbar's total increment (%lu) - aborting...",
                       dialog, y, x, ctune_UI_ScrollBar.getTotalIncrements( &dialog->scrollbar.y )
            );

            return; //EARLY RETURN
        }

        int    shift = ( abs( y ) < max ? y : ( y < 0 ? -max : max ) );
        size_t pos   = 0;

        if( y > 0 ) { //i.e. scroll down
            pos = ( max - shift > dialog->scrollbar.scroll_pos_y ? ( shift + dialog->scrollbar.scroll_pos_y ) : max );

        } else if( y < 0 ) { //i.e.: scroll up
            pos = ( abs( y ) < dialog->scrollbar.scroll_pos_y ? ( dialog->scrollbar.scroll_pos_y + shift ) : 0 );
        }

        if( ctune_UI_ScrollBar.setPosition( &dialog->scrollbar.y, pos ) ) {
            dialog->scrollbar.scroll_pos_y = pos;
            ctune_UI_ScrollBar.show( &dialog->scrollbar.y );
        }
    }

    if( x != 0 ) {
        int max = 0;

        if( !ctune_utoi( ctune_UI_ScrollBar.getTotalIncrements( &dialog->scrollbar.x ), &max ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Dialog_scrollBy( %p, %i, %i )] Failed to cast horizontal scrollbar's total increment (%lu) - aborting...",
                       dialog, y, x, ctune_UI_ScrollBar.getTotalIncrements( &dialog->scrollbar.y )
            );

            return; //EARLY RETURN
        }

        int    shift = ( abs( x ) < max ? x : ( x < 0 ? -max : max ) );
        size_t pos   = 0;

        if( x > 0 ) { //i.e.: scroll right
            pos = ( max - shift > dialog->scrollbar.scroll_pos_x ? ( shift + dialog->scrollbar.scroll_pos_x ) : max );

        } else if( x < 0 ) { //i.e.: scroll left
            pos = ( abs( x ) < dialog->scrollbar.scroll_pos_x ? ( dialog->scrollbar.scroll_pos_x + shift ) : 0 );
        }

        if( ctune_UI_ScrollBar.setPosition( &dialog->scrollbar.x, pos ) ) {
            dialog->scrollbar.scroll_pos_x = pos;
            ctune_UI_ScrollBar.show( &dialog->scrollbar.x );
        }
    }
}

/**
 * Creates an initialised Dialog_t object
 * @return Initialised Dialog_t object
 */
static ctune_UI_Dialog_t ctune_UI_Dialog_init( void ) {
    return (ctune_UI_Dialog_t) {
        .border_win       = ctune_UI_BorderWin.create( ctune_UI_Theme.color( CTUNE_UI_ITEM_DIALOG_WIN ) ),
        .scrollbar.y_init = false,
        .scrollbar.x_init = false,
        .autoscroll       = { 0, 0 },
    };
}

/**
 * Creates a scroll window (pad)
 * @param dialog UI_Dialog_t object
 * @param rows   Row size of the scrolling pad
 * @param cols   Col size of the scrolling pad
 */
static void ctune_UI_Dialog_createScrollWin( ctune_UI_Dialog_t * dialog, int rows, int cols ) {
    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_UI_Dialog_createScrollWin( %p, %i, %i )] Scrolling window: rows = %i, cols = %i",
               dialog, rows, cols, rows, cols
    );

    dialog->canvas = ctune_UI_ScrollWin.init( rows, cols );
}

/**
 * Create a border window (a scroll windows must have been created first!)
 * @param dialog   UI_Dialog_t object
 * @param parent   Parent window dimensions (to fit border into)
 * @param title    Title to print on the window border
 * @param margins  Window margin values
 */
static void ctune_UI_Dialog_createBorderWin( ctune_UI_Dialog_t * dialog, const WindowProperty_t * parent, const char * title, const WindowMargin_t * margins ) {
    if( dialog->canvas.pad == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_Dialog_createBorderWin( %p, %p, \"%s\", %p )] Error: ScrollWin not created prior (pad=%p) - aborting...",
                   dialog, parent, title, margins,
                   dialog->canvas.pad
        );

        return; //EARLY return
    }

    WindowMargin_t auto_margin = ( margins != NULL ? (*margins) : (WindowMargin_t) { 0, 0, 0, 0 } );
    int border_win_dimension_y = ( dialog->canvas.rows + ( auto_margin.top  + auto_margin.bottom ) + 2 );
    int border_win_dimension_x = ( dialog->canvas.cols + ( auto_margin.left + auto_margin.right  ) + 2 );

    if( border_win_dimension_y <= parent->rows && border_win_dimension_x < parent->cols ) { //no scrolling
        dialog->property.rows = border_win_dimension_y;
        dialog->property.cols = border_win_dimension_x;

    } else { //scrolling will be needed (chicken & egg problem when adjusting margins)
        CTUNE_LOG( CTUNE_LOG_TRACE,
                   "[ctune_UI_Dialog_createBorderWin( %p, %p, \"%s\", %p )] Scrolling required.",
                   dialog, parent, title, margins
        );

        calc_vsize: //vertical dimensions
            if( border_win_dimension_y <= parent->rows ) {
                dialog->property.rows = border_win_dimension_y;
            } else { //scrolling is required
                ctune_UI_ScrollWin.setScrollingY( &dialog->canvas, true );

                if( auto_margin.right == 0 ) {
                    auto_margin.right       = 1;
                    border_win_dimension_x += 1;
                }

                dialog->property.rows = parent->rows;
            }

        //calc_hsize: //horizontal dimensions
            if( border_win_dimension_x <= parent->cols ) {
                dialog->property.cols = border_win_dimension_x;
            } else {
                ctune_UI_ScrollWin.setScrollingX( &dialog->canvas, true );

                dialog->property.cols = parent->cols;

                if( auto_margin.bottom == 0 ) {
                    auto_margin.bottom      = 1;
                    border_win_dimension_y += 1;

                    goto calc_vsize; //GOTO as need to check vertical fitting again since Y dimension has changed
                }
            }
    }

    dialog->property.pos_y = ( ( parent->rows - dialog->property.rows ) / 2 ) + parent->pos_y;
    dialog->property.pos_x = ( ( parent->cols - dialog->property.cols ) / 2 ) + parent->pos_x;

    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_UI_Dialog_createBorderWin( %p, %p, \"%s\", { %i, %i, %i, %i } )] "
               "Creating in parent (%ix%i) a border window of size %ix%i at (%i, %i) with auto-margins { %i, %i, %i, %i }",
               dialog, parent, title, margins->top, margins->right, margins->bottom, margins->left,
               parent->rows, parent->cols, dialog->property.rows, dialog->property.cols, dialog->property.pos_y, dialog->property.pos_x,
               auto_margin.top, auto_margin.right, auto_margin.bottom, auto_margin.left
    );

    if( !ctune_UI_BorderWin.init( &dialog->border_win, &dialog->property, title ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_Dialog_createBorderWin( %p, %p, \"%s\", { %i, %i, %i, %i } )] Failed to create the border window.",
                   dialog, parent, title, margins->top, margins->right, margins->bottom, margins->left
        );
    }

    const int viewbox_dimension_y = ( dialog->property.rows  - auto_margin.top  - auto_margin.bottom ) - 2;
    const int viewbox_dimension_x = ( dialog->property.cols  - auto_margin.left - auto_margin.right  ) - 2;
    const int viewbox_pos_y       = ( dialog->property.pos_y + auto_margin.top  + 1 );
    const int viewbox_pos_x       = ( dialog->property.pos_x + auto_margin.left + 1 );

    ctune_UI_ScrollWin.resizeViewBox( &dialog->canvas, viewbox_dimension_y, viewbox_dimension_x );
    ctune_UI_ScrollWin.moveViewBox( &dialog->canvas, viewbox_pos_y, viewbox_pos_x );
    ctune_UI_ScrollWin.logState( &dialog->canvas );

    //calculate absolute position and dimension of vertical scrollbar
    dialog->scrollbar.y_property   = (WindowProperty_t) { viewbox_dimension_y, 1, viewbox_pos_y, ( viewbox_pos_x + viewbox_dimension_x ) }; //far right
    dialog->scrollbar.x_property   = (WindowProperty_t) { 1, viewbox_dimension_x, ( viewbox_pos_y + viewbox_dimension_y ), viewbox_pos_x }; //bottom
    dialog->scrollbar.scroll_pos_y = 0;
    dialog->scrollbar.scroll_pos_x = 0;

    if( dialog->scrollbar.y_init ) {
        ctune_UI_ScrollBar.free( &dialog->scrollbar.y );
        dialog->scrollbar.y_init = false;
    }

    if( dialog->scrollbar.x_init ) {
        ctune_UI_ScrollBar.free( &dialog->scrollbar.x );
        dialog->scrollbar.x_init = false;
    }

    if( ctune_UI_ScrollWin.isScrollableX( &dialog->canvas ) && ctune_UI_ScrollWin.isScrollableY( &dialog->canvas ) ) {
        mvwaddstr( dialog->border_win.window, dialog->property.rows - 2, dialog->property.cols - 2, "·" );
    }

    if( ctune_UI_ScrollWin.isScrollableX( &dialog->canvas ) ) {
        dialog->scrollbar.x      = ctune_UI_ScrollBar.init( &dialog->scrollbar.x_property, BOTTOM, true );
        dialog->scrollbar.x_init = true;
        ctune_UI_ScrollBar.initCanvas( &dialog->scrollbar.x );
        ctune_UI_ScrollBar.setScrollLength( &dialog->scrollbar.x, dialog->canvas.cols );
    }

    if( ctune_UI_ScrollWin.isScrollableY( &dialog->canvas ) ) {
        dialog->scrollbar.y      = ctune_UI_ScrollBar.init( &dialog->scrollbar.y_property, RIGHT, true );
        dialog->scrollbar.y_init = true;
        ctune_UI_ScrollBar.initCanvas( &dialog->scrollbar.y );
        ctune_UI_ScrollBar.setScrollLength( &dialog->scrollbar.y, dialog->canvas.rows );
    }
}

/**
 * Gets the scrollable state on the vertical axis
 * @param dialog UI_Dialog_t object
 * @return Scrollable state
 */
static bool ctune_UI_Dialog_isScrollableY( ctune_UI_Dialog_t * dialog ) {
    return ctune_UI_ScrollWin.isScrollableY( &dialog->canvas );
}

/**
 * Gets the scrollable state on the horizontal axis
 * @param dialog UI_Dialog_t object
 * @return Scrollable state
 */
static bool ctune_UI_Dialog_isScrollableX( ctune_UI_Dialog_t * dialog ) {
    return ctune_UI_ScrollWin.isScrollableX( &dialog->canvas );
}

/**
 * Sets the auto-scroll threshold offset
 * @param dialog UI_Dialog_t object
 * @param y      Vertical offset
 * @param x      Horizontal offset
 */
static void ctune_UI_Dialog_setAutoScrollOffset( ctune_UI_Dialog_t * dialog, int y, int x ) {
    dialog->autoscroll.offset_y = y;
    dialog->autoscroll.offset_x = x;
}

/**
 * Scroll content up
 * @param dialog UI_Dialog_t object
 */
static void ctune_UI_Dialog_scrollUp( ctune_UI_Dialog_t * dialog ) {
    if( ctune_UI_ScrollWin.isScrollableY( &dialog->canvas ) ) {
        ctune_UI_ScrollWin.scrollPad( &dialog->canvas, -1, 0 );

        const size_t new_pos = ( dialog->scrollbar.scroll_pos_y > 0 ? ( dialog->scrollbar.scroll_pos_y - 1 ) : 0 );

        if( ctune_UI_ScrollBar.setPosition( &dialog->scrollbar.y, new_pos ) ) {
            dialog->scrollbar.scroll_pos_y = new_pos;
            ctune_UI_ScrollBar.show( &dialog->scrollbar.y );
        }
    }
}

/**
 * Scroll content right
 * @param dialog UI_Dialog_t object
 */
static void ctune_UI_Dialog_scrollRight( ctune_UI_Dialog_t * dialog ) {
    if( ctune_UI_ScrollWin.isScrollableX( &dialog->canvas ) ) {
        ctune_UI_ScrollWin.scrollPad( &dialog->canvas, 0, +1 );

        const size_t new_pos = ( dialog->scrollbar.scroll_pos_x < INT_MAX ? ( dialog->scrollbar.scroll_pos_x + 1 ) : INT_MAX );

        if( ctune_UI_ScrollBar.setPosition( &dialog->scrollbar.x, new_pos ) ) {
            dialog->scrollbar.scroll_pos_x = new_pos;
            ctune_UI_ScrollBar.show( &dialog->scrollbar.x );
        }
    }
}

/**
 * Scroll content down
 * @param dialog UI_Dialog_t object
 */
static void ctune_UI_Dialog_scrollDown( ctune_UI_Dialog_t * dialog ) {
    if( ctune_UI_ScrollWin.isScrollableY( &dialog->canvas ) ) {
        ctune_UI_ScrollWin.scrollPad( &dialog->canvas, +1, 0 );

        const size_t new_pos = ( dialog->scrollbar.scroll_pos_y < INT_MAX ? ( dialog->scrollbar.scroll_pos_y + 1 ) : INT_MAX );

        if( ctune_UI_ScrollBar.setPosition( &dialog->scrollbar.y, new_pos ) ) {
            dialog->scrollbar.scroll_pos_y = new_pos;
            ctune_UI_ScrollBar.show( &dialog->scrollbar.y );
        }
    }
}

/**
 * Scroll content left
 * @param dialog UI_Dialog_t object
 */
static void ctune_UI_Dialog_scrollLeft( ctune_UI_Dialog_t * dialog ) {
    if( ctune_UI_ScrollWin.isScrollableX( &dialog->canvas ) ) {
        ctune_UI_ScrollWin.scrollPad( &dialog->canvas, 0, -1 );

        const size_t new_pos = ( dialog->scrollbar.scroll_pos_x > 0 ? ( dialog->scrollbar.scroll_pos_x - 1 ) : 0 );

        if( ctune_UI_ScrollBar.setPosition( &dialog->scrollbar.x, new_pos ) ) {
            dialog->scrollbar.scroll_pos_x = new_pos;
            ctune_UI_ScrollBar.show( &dialog->scrollbar.x );
        }
    }
}

/**
 * Scroll back to the top
 * @param dialog UI_Dialog_t object
 */
static void ctune_UI_Dialog_scrollHome( ctune_UI_Dialog_t * dialog ) {
    if( ctune_UI_ScrollWin.isScrollableY( &dialog->canvas ) ) {
        ctune_UI_ScrollWin.scrollPad( &dialog->canvas, -dialog->canvas.rows, 0 );

        if( ctune_UI_ScrollBar.setPosition( &dialog->scrollbar.y, 0 ) ) {
            dialog->scrollbar.scroll_pos_y = 0;
            ctune_UI_ScrollBar.show( &dialog->scrollbar.y );
        }
    }

    if( ctune_UI_ScrollWin.isScrollableX( &dialog->canvas ) ) {
        ctune_UI_ScrollWin.scrollPad( &dialog->canvas, 0, -dialog->canvas.cols );

        if( ctune_UI_ScrollBar.setPosition( &dialog->scrollbar.x, 0 ) ) {
            dialog->scrollbar.scroll_pos_x = 0;
            ctune_UI_ScrollBar.show( &dialog->scrollbar.x );
        }
    }
}

/**
 * Scroll back to the top (horizontal scroll untouched)
 * @param dialog UI_Dialog_t object
 */
static void ctune_UI_Dialog_scrollTop( ctune_UI_Dialog_t * dialog ) {
    if( ctune_UI_ScrollWin.isScrollableY( &dialog->canvas ) ) {
        ctune_UI_ScrollWin.scrollPad( &dialog->canvas, -dialog->canvas.rows, 0 );

        if( ctune_UI_ScrollBar.setPosition( &dialog->scrollbar.y, 0 ) ) {
            dialog->scrollbar.scroll_pos_y = 0;
            ctune_UI_ScrollBar.show( &dialog->scrollbar.y );
        }
    }
}

/**
 * Scroll down to the bottom (horizontal scroll untouched)
 * @param dialog UI_Dialog_t object
 */
static void ctune_UI_Dialog_scrollBottom( ctune_UI_Dialog_t * dialog ) {
    if( ctune_UI_ScrollWin.isScrollableY( &dialog->canvas ) ) {
        ctune_UI_ScrollWin.scrollPad( &dialog->canvas, dialog->canvas.rows, 0 );

        const size_t end = ctune_UI_ScrollBar.getTotalIncrements( &dialog->scrollbar.y );

        if( ctune_UI_ScrollBar.setPosition( &dialog->scrollbar.y, end ) ) {
            dialog->scrollbar.scroll_pos_y = end;
            ctune_UI_ScrollBar.show( &dialog->scrollbar.y );
        }
    }
}

/**
 * Scroll to the far left (vertical scroll untouched)
 * @param dialog UI_Dialog_t object
 */
static void ctune_UI_Dialog_scrollLeftEdge( ctune_UI_Dialog_t * dialog ) {
    if( ctune_UI_ScrollWin.isScrollableX( &dialog->canvas ) ) {
        ctune_UI_ScrollWin.scrollPad( &dialog->canvas, 0, -dialog->canvas.cols );

        if( ctune_UI_ScrollBar.setPosition( &dialog->scrollbar.x, 0 ) ) {
            dialog->scrollbar.scroll_pos_x = 0;
            ctune_UI_ScrollBar.show( &dialog->scrollbar.x );
        }
    }
}

/**
 * Scroll to the far right (vertical scroll untouched)
 * @param dialog UI_Dialog_t object
 */
static void ctune_UI_Dialog_scrollRightEdge( ctune_UI_Dialog_t * dialog ) {
    if( ctune_UI_ScrollWin.isScrollableX( &dialog->canvas ) ) {
        ctune_UI_ScrollWin.scrollPad( &dialog->canvas, 0, dialog->canvas.cols );

        const size_t end = ctune_UI_ScrollBar.getTotalIncrements( &dialog->scrollbar.x );

        if( ctune_UI_ScrollBar.setPosition( &dialog->scrollbar.x, end ) ) {
            dialog->scrollbar.scroll_pos_x = end;
            ctune_UI_ScrollBar.show( &dialog->scrollbar.x );
        }
    }
}

/**
 * Triggers autoscroll
 * @param dialog UI_Dialog_t object
 * @param y      Current cursor row position on pad
 * @param x      Current cursor column position on pad
 */
static void ctune_UI_Dialog_autoScroll( ctune_UI_Dialog_t * dialog, int y, int x ) {
    const WindowProperty_t viewbox = ctune_UI_ScrollWin.getViewProperty( &dialog->canvas );

    if( ctune_UI_Dialog_isScrollableY( dialog ) && y >= 0 ) {
        const int viewbox_top    = viewbox.pos_y;
        const int viewbox_bottom = ( viewbox.pos_y + viewbox.rows );
        const int threshold_up   = ( y - dialog->autoscroll.offset_y );
        const int threshold_down = ( y + dialog->autoscroll.offset_y );

        CTUNE_LOG( CTUNE_LOG_DEBUG,
                   "[ctune_UI_Dialog_autoScroll( %p, %i, %i )] "
                   "(Y) viewbox: %i->%i, threshold: %i<-(%i)->%i",
                   dialog, y, x,
                   viewbox_top, viewbox_bottom, threshold_up, y, threshold_down
        );

        if( threshold_up < viewbox_top ) { //i.e.: scroll up
            ctune_UI_Dialog_scrollBy( dialog, -( viewbox_top - threshold_up ), 0 );

        } else if( threshold_down > viewbox_bottom ) { //i.e.: scroll down
            ctune_UI_Dialog_scrollBy( dialog, +( threshold_down - viewbox_bottom ), 0 );
        }
    }

    if( ctune_UI_Dialog_isScrollableX( dialog ) && x >= 0 ) {
        const int viewbox_left    = viewbox.pos_x;
        const int viewbox_right   = ( viewbox.pos_x + viewbox.cols );
        const int threshold_left  = ( x - dialog->autoscroll.offset_x );
        const int threshold_right = ( x + dialog->autoscroll.offset_x );

        CTUNE_LOG( CTUNE_LOG_DEBUG,
                   "[ctune_UI_Dialog_autoScroll( %p, %i, %i )] "
                   "(X) viewbox: %i -> %i, threshold: %i -> %i",
                   dialog, y, x, viewbox_left, viewbox_right, threshold_left, threshold_right
        );

        if( threshold_left < viewbox_left ) {
            ctune_UI_Dialog_scrollBy( dialog, 0, -( viewbox_left - threshold_left ) );

        } else if( threshold_right > viewbox_right ) {
            ctune_UI_Dialog_scrollBy( dialog, 0, +( threshold_right - viewbox_right ) );
        }
    }
}

/**
 * Moves the dialog panel to the top and un-hides it
 * @param dialog UI_Dialog_t object
 */
static void ctune_UI_Dialog_show( ctune_UI_Dialog_t * dialog ) {
    ctune_UI_BorderWin.show( &dialog->border_win );

    if( ctune_UI_ScrollWin.isScrollableY( &dialog->canvas ) )
        ctune_UI_ScrollBar.show( &dialog->scrollbar.y );

    if( ctune_UI_ScrollWin.isScrollableX( &dialog->canvas ) )
        ctune_UI_ScrollBar.show( &dialog->scrollbar.x );

    update_panels();
    ctune_UI_ScrollWin.refreshView( &dialog->canvas );
}

/**
 * Repaints the view with updated content
 * @param dialog UI_Dialog_t object
 */
static void ctune_UI_Dialog_refreshView( ctune_UI_Dialog_t * dialog ) {
    if( ctune_UI_ScrollWin.isScrollableY( &dialog->canvas ) )
        ctune_UI_ScrollBar.show( &dialog->scrollbar.y );

    if( ctune_UI_ScrollWin.isScrollableX( &dialog->canvas ) )
        ctune_UI_ScrollBar.show( &dialog->scrollbar.x );

    ctune_UI_ScrollWin.refreshView( &dialog->canvas );
}

/**
 * Moves the dialog panel to the bottom and hides it
 * @param dialog UI_Dialog_t object
 */
static void ctune_UI_Dialog_hide( ctune_UI_Dialog_t * dialog ) {
    ctune_UI_BorderWin.hide( &dialog->border_win );

    if( ctune_UI_ScrollWin.isScrollableY( &dialog->canvas ) )
        ctune_UI_ScrollBar.hide( &dialog->scrollbar.y );

    if( ctune_UI_ScrollWin.isScrollableX( &dialog->canvas ) )
        ctune_UI_ScrollBar.hide( &dialog->scrollbar.x );

    update_panels();
}

/**
 * De-allocates resources
 * @param dialog UI_Dialog_t object
 */
static void ctune_UI_Dialog_free( ctune_UI_Dialog_t * dialog ) {
    if( dialog ) {
        dialog->property             = (WindowProperty_t) { 0, 0, 0, 0 };
        dialog->scrollbar.y_property = (WindowProperty_t) { 0, 0, 0, 0 };
        dialog->scrollbar.x_property = (WindowProperty_t) { 0, 0, 0, 0 };

        ctune_UI_BorderWin.free( &dialog->border_win );
        ctune_UI_ScrollBar.free( &dialog->scrollbar.y );
        ctune_UI_ScrollBar.free( &dialog->scrollbar.x );
        ctune_UI_ScrollWin.free( &dialog->canvas );

        dialog->scrollbar.x_init = false;
        dialog->scrollbar.y_init = false;

        CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_UI_Dialog_free( %p )] Dialog freed.", dialog );
    }
}

/**
 * Namespace constructor
 */
const struct ctune_UI_Widget_Dialog_Namespace ctune_UI_Dialog = {
    .init                = &ctune_UI_Dialog_init,
    .createScrollWin     = &ctune_UI_Dialog_createScrollWin,
    .createBorderWin     = &ctune_UI_Dialog_createBorderWin,
    .isScrollableY       = &ctune_UI_Dialog_isScrollableY,
    .isScrollableX       = &ctune_UI_Dialog_isScrollableX,
    .setAutoScrollOffset = &ctune_UI_Dialog_setAutoScrollOffset,
    .scrollUp            = &ctune_UI_Dialog_scrollUp,
    .scrollRight         = &ctune_UI_Dialog_scrollRight,
    .scrollDown          = &ctune_UI_Dialog_scrollDown,
    .scrollLeft          = &ctune_UI_Dialog_scrollLeft,
    .scrollHome          = &ctune_UI_Dialog_scrollHome,
    .scrollTop           = &ctune_UI_Dialog_scrollTop,
    .scrollBottom        = &ctune_UI_Dialog_scrollBottom,
    .scrollLeftEdge      = &ctune_UI_Dialog_scrollLeftEdge,
    .scrollRightEdge     = &ctune_UI_Dialog_scrollRightEdge,
    .autoScroll          = &ctune_UI_Dialog_autoScroll,
    .show                = &ctune_UI_Dialog_show,
    .refreshView         = &ctune_UI_Dialog_refreshView,
    .hide                = &ctune_UI_Dialog_hide,
    .free                = &ctune_UI_Dialog_free,
};