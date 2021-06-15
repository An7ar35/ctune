#include "ScrollWin.h"

#include <stdlib.h>
#include "../../logger/Logger.h"

//https://man7.org/linux/man-pages/man3/idlok.3x.html

/**
 * Creates a scrolling window without initiating the pad
 * @return Blank (zeroed) ScrollWin_t object
 */
static ctune_UI_ScrollWin_t ctune_UI_ScrollWin_initBlank( void ) {
    return (struct ctune_UI_Widget_ScrollWin) {
        .pad       = NULL,
        .rows      = 0,
        .cols      = 0,
        .viewbox   = { .from_y = 0, .from_x = 0, .to_y = 0, .to_x = 0 },
        .pos       = { 0, 0 },
        .scrolling = { false, false },
    };
}

/**
 * Object constructor
 * @param height Pad row height
 * @param width  Pad col width
 * @return Initialised ScrollWin_t
 */
static ctune_UI_ScrollWin_t ctune_UI_ScrollWin_init( int height, int width ) {
    CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_UI_ScrollWin_init( %d, %d )] Creating a ScrollWin_t object.", height, width );

    return (struct ctune_UI_Widget_ScrollWin) {
        .pad       = newpad( height, width ),
        .rows      = height,
        .cols      = width,
        .viewbox   = { .from_y = 0, .from_x = 0, .to_y = height, .to_x = width },
        .pos       = { 0, 0 },
        .scrolling = { false, false },
    };
}

/**
 * Creates the pad for the scrolling window (will free prior pad if not NULL)
 * @param sw     ScrollWin_t object
 * @param height Pad row height
 * @param width  Pad col width
 */
static void ctune_UI_ScrollWin_createPad( ctune_UI_ScrollWin_t * sw, int height, int width ) {
    if( sw->pad != NULL )
        delwin( sw->pad );
    sw->pad = newpad( height, width );
}

/**
 * Refreshes the pad view
 * @param sw ScrollWin_t object
 */
static void ctune_UI_ScrollWin_refreshView( ctune_UI_ScrollWin_t * sw ) {
    prefresh( sw->pad, sw->pos.y, sw->pos.x, sw->viewbox.from_y, sw->viewbox.from_x, sw->viewbox.to_y, sw->viewbox.to_x );
}

/**
 * Refresh pad view
 * @param sw         ScrollWin_t object
 * @param viewsize_y Pad's view box row size
 * @param viewsize_x Pad's view box column size
 */
static void ctune_UI_ScrollWin_refresh( ctune_UI_ScrollWin_t * sw, int viewsize_y, int viewsize_x ) {
    //calculates the position of the bottom right corner of the viewing box on the pad
    sw->viewbox.to_y = sw->viewbox.from_y + viewsize_y;
    sw->viewbox.to_x = sw->viewbox.from_x + viewsize_x;

    ctune_UI_ScrollWin.setScrollingY( sw, ( viewsize_y < sw->rows ) );
    ctune_UI_ScrollWin.setScrollingX( sw, ( viewsize_x < sw->cols ) );

    prefresh( sw->pad, sw->pos.y, sw->pos.x, sw->viewbox.from_y, sw->viewbox.from_x, sw->viewbox.to_y, sw->viewbox.to_x );
}

/**
 * Scroll pad
 * @param sw    ScrollWin_t object
 * @param nrows Number of rows to scroll down by (negative number will scroll up)
 * @param ncols Number of columns to scroll right by (negative number will scroll left)
 */
static void ctune_UI_ScrollWin_scrollPad( ctune_UI_ScrollWin_t * sw, int nrows, int ncols ) {
    if( sw->scrolling.y ) {
        if( nrows < 0 && abs( nrows ) > sw->pos.y ) { //going too far up
            sw->pos.y = 0;
        } else {
            const int max_scroll = sw->rows - ( sw->viewbox.to_y - sw->viewbox.from_y ) - 1;

            if( max_scroll - nrows >= sw->pos.y )
                sw->pos.y += nrows;
            else //going too far down
                sw->pos.y += ( max_scroll - sw->pos.y );
        }
    }

    if( sw->scrolling.x ) {
        if( ncols < 0 && sw->pos.x < abs( ncols ) ) //going too far left
            sw->pos.x = 0;
        else {
            const int max_scroll = sw->cols - ( sw->viewbox.to_x - sw->viewbox.from_x ) - 1;

            if( max_scroll - ncols >= sw->pos.x )
                sw->pos.x += ncols;
            else //going too far right
                sw->pos.x += ( max_scroll - sw->pos.x );
        }
    }
}

/**
 * Moves the pad viewbox top left corner
 * @param sw ScrollWin_t object
 * @param y  Row
 * @param x  Col
 */
static void ctune_UI_ScrollWin_moveViewBox( ctune_UI_ScrollWin_t * sw, int y, int x ) {
    if( y >= 0 ) {
        int viewbox_rows   = sw->viewbox.to_y - sw->viewbox.from_y;
        sw->viewbox.from_y = y;
        sw->viewbox.to_y   = y + viewbox_rows;
    }
    if( x >= 0 ) {
        int viewbox_cols   = sw->viewbox.to_x - sw->viewbox.from_x;
        sw->viewbox.from_x = x;
        sw->viewbox.to_x   = x + viewbox_cols;
    }
}

/**
 * Switch the hardware auto-insert/delete line functionality
 * @param sw    ScrollWin_t object
 * @param state State (on=true, off=false)
 */
static void ctune_UI_ScrollWin_setInsDelLine( ctune_UI_ScrollWin_t * sw, bool state ) {
    idlok( sw->pad, state );
}

/**
 * Sets the scrolling flags for both X and Y
 * @param sw    ScrollWin_t object
 * @param state Scroll state
 */
static void ctune_UI_ScrollWin_setScrolling( ctune_UI_ScrollWin_t * sw, bool state ) {
    scrollok( sw->pad, state );
    sw->scrolling.y = state;
    sw->scrolling.x = state;
}

/**
 * Sets the scrolling flags for the Y axis
 * @param sw    ScrollWin_t object
 * @param state Scroll state
 */
static void ctune_UI_ScrollWin_setScrollingY( ctune_UI_ScrollWin_t * sw, bool state ) {
    if( !sw->scrolling.x && state == false )
        scrollok( sw->pad, false );

    if( state == true )
        scrollok( sw->pad, true );

    sw->scrolling.y = state;
}

/**
 * Sets the scrolling flags for the X axis
 * @param sw    ScrollWin_t object
 * @param state Scroll state
 */
static void ctune_UI_ScrollWin_setScrollingX( ctune_UI_ScrollWin_t * sw, bool state ) {
    if( !sw->scrolling.y && state == false )
        scrollok( sw->pad, false );

    if( state == true )
        scrollok( sw->pad, true );

    sw->scrolling.x = state;
}

/**
 * Gets the scrollable state of the window (X || Y)
 * @param sw ScrollWin_t object
 */
static bool ctune_UI_ScrollWin_isScrollable( ctune_UI_ScrollWin_t * sw ) {
    return ( sw->scrolling.y || sw->scrolling.x );
}

/**
 * Gets the scrollable state of the window in the Y axis (rows)
 * @param sw ScrollWin_t object
 */
static bool ctune_UI_ScrollWin_isScrollableY( ctune_UI_ScrollWin_t * sw ) {
    return sw->scrolling.y;
}

/**
 * Gets the scrollable state of the window in the X axis (cols)
 * @param sw ScrollWin_t object
 */
static bool ctune_UI_ScrollWin_isScrollableX( ctune_UI_ScrollWin_t * sw ) {
    return sw->scrolling.x;
}

/**
 * Resizes the viewbox
 * @param sw   ScrollWin_t object
 * @param rows Height
 * @param cols Width
 */
static void ctune_UI_ScrollWin_resizeViewBox( ctune_UI_ScrollWin_t * sw, int rows, int cols ) {
    sw->viewbox.to_y = sw->viewbox.from_y + rows - 1;
    sw->viewbox.to_x = sw->viewbox.from_x + cols - 1;
    ctune_UI_ScrollWin.setScrollingY( sw, ( rows < sw->rows ) );
    ctune_UI_ScrollWin.setScrollingX( sw, ( cols < sw->cols ) );
}

/**
 * Gets the current properties of the displayed/viewable pad section
 * @param sw ScrollWin_t object
 * @return Properties (top-left corner pos on pad and view-box size)
 */
static WindowProperty_t ctune_UI_ScrollWin_getViewProperty( const ctune_UI_ScrollWin_t * sw ) {
    return (WindowProperty_t) {
        .pos_y = sw->pos.y,
        .pos_x = sw->pos.x,
        .rows  = ( sw->viewbox.to_y - sw->viewbox.from_y ),
        .cols  = ( sw->viewbox.to_x - sw->viewbox.from_x ),
    };
}

/**
 * Resizes the pad
 * @param sw   ScrollWin_t object
 * @param rows Vertical size in rows
 * @param cols Horizontal size in cols
 * @return Success
 */
static bool ctune_UI_ScrollWin_resizePad( ctune_UI_ScrollWin_t * sw, int rows, int cols ) {
    CTUNE_LOG( CTUNE_LOG_DEBUG, "Trying to resize pad: %i x %i", rows, cols );
    if( wresize( sw->pad, rows, cols ) == OK ) {
        sw->rows        = rows;
        sw->cols        = cols;
        sw->scrolling.y = ( ( sw->viewbox.to_y - sw->viewbox.from_y ) < sw->rows );
        sw->scrolling.x = ( ( sw->viewbox.to_x - sw->viewbox.from_x ) < sw->cols );
        return true;
    }

    return false;
}

/**
 * Clears the content of the pad
 * @param sw ScrollWin_t object
 */
static void ctune_UI_ScrollWin_clearwin( ctune_UI_ScrollWin_t * sw ) {
    werase( sw->pad );
}

/**
 * Prints all properties to logger
 * @param sw ScrollWin_t object
 */
static void ctune_UI_ScrollWin_logState( ctune_UI_ScrollWin_t * sw ) {
    if( sw != NULL ) {
        CTUNE_LOG( CTUNE_LOG_DEBUG,
                   "[ctune_UI_ScrollWin_logState( %p )] "
                   "pad: %i x %i, "
                   "viewbox/pad: (%i,%i), "
                   "viewbox/win: (%i,%i)->(%i,%i) "
                   "scroll: y=%i, x=%i",
                   sw,
                   sw->rows, sw->cols,
                   sw->pos.y, sw->pos.x,
                   sw->viewbox.from_y, sw->viewbox.from_x, sw->viewbox.to_y, sw->viewbox.to_x,
                   sw->scrolling.y , sw->scrolling.x
        );

    } else {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_ScrollWin_logState( %p )] ScrollWin_t pointer is NULL.", sw );
    }
}

/**
 * Re-initializes a ScrollWin_t object
 * @param sw ScrollWin_t object
 * @param height Total height of the pad
 * @param width  Total width of the pad
 */
static void ctune_UI_ScrollWin_reinit( ctune_UI_ScrollWin_t * sw, int height, int width ) {
    delwin( sw->pad );
    sw->pad            = newpad( height, width );
    sw->rows           = height;
    sw->cols           = width;
    sw->viewbox.from_y = 0;
    sw->viewbox.from_x = 0;
    sw->viewbox.to_y   = height;
    sw->viewbox.to_x   = width;
    sw->pos.y          = 0;
    sw->pos.x          = 0;
    sw->scrolling.y    = false;
    sw->scrolling.x    = false;
}

/**
 * Frees resources
 * @param sw ScrollWin object to free content of
 */
static void ctune_UI_ScrollWin_free( ctune_UI_ScrollWin_t * sw ) {
    if( sw ) {
        if( sw->pad != NULL )
            delwin( sw->pad );
        sw->pad            = NULL;
        sw->rows           = 0;
        sw->cols           = 0;
        sw->viewbox.from_y = 0;
        sw->viewbox.from_x = 0;
        sw->viewbox.to_y   = 0;
        sw->viewbox.to_x   = 0;
        sw->pos.y          = 0;
        sw->pos.x          = 0;
        sw->scrolling.y    = false;
        sw->scrolling.x    = false;

        CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_UI_ScrollWin_free( %p )] ScrollWin freed.", sw );
    }
}


/**
 * Namespace constructor
 */
const struct ctune_UI_Widget_ScrollWin_Namespace ctune_UI_ScrollWin = {
    .initBlank       = &ctune_UI_ScrollWin_initBlank,
    .init            = &ctune_UI_ScrollWin_init,
    .createPad       = &ctune_UI_ScrollWin_createPad,
    .refreshView     = &ctune_UI_ScrollWin_refreshView,
    .redraw          = &ctune_UI_ScrollWin_refresh,
    .scrollPad       = &ctune_UI_ScrollWin_scrollPad,
    .moveViewBox     = &ctune_UI_ScrollWin_moveViewBox,
    .setInsDelLine   = &ctune_UI_ScrollWin_setInsDelLine,
    .setScrolling    = &ctune_UI_ScrollWin_setScrolling,
    .setScrollingY   = &ctune_UI_ScrollWin_setScrollingY,
    .setScrollingX   = &ctune_UI_ScrollWin_setScrollingX,
    .isScrollable    = &ctune_UI_ScrollWin_isScrollable,
    .isScrollableY   = &ctune_UI_ScrollWin_isScrollableY,
    .isScrollableX   = &ctune_UI_ScrollWin_isScrollableX,
    .resizeViewBox   = &ctune_UI_ScrollWin_resizeViewBox,
    .resizePad       = &ctune_UI_ScrollWin_resizePad,
    .getViewProperty = &ctune_UI_ScrollWin_getViewProperty,
    .clearPad        = &ctune_UI_ScrollWin_clearwin,
    .reinit          = &ctune_UI_ScrollWin_reinit,
    .logState        = &ctune_UI_ScrollWin_logState,
    .free            = &ctune_UI_ScrollWin_free,
};
