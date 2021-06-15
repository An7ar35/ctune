#include "BorderWin.h"

#include <string.h>

#include "../../logger/Logger.h"

/**
 * Creates an a BorderWin_t object ready for initialisation
 * @param colour Colour attribute ID to use for the border/title
 * @return BorderWin_t object
 */
static ctune_UI_BorderWin_t ctune_UI_Widget_BorderWin_create( int colour ) {
    return (ctune_UI_BorderWin_t) {
        .window = NULL,
        .panel  = NULL,
        .colour = colour,
    };
}

/**
 * Initialised a border window
 * @param border_win Pointer to an initialised ctune_UI_BorderWin_t object
 * @param property   Size/Position properties of the border window
 * @param title      Title (can be NULL if none is desired)
 * @return Success
 */
static bool ctune_UI_Widget_BorderWin_init( ctune_UI_BorderWin_t * border_win, WindowProperty_t * property, const char * title ) {
    if( border_win == NULL || property == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "ctune_UI_Widget_BorderWin_init( %p, %p, \"%s\" ) One or more args is NULL.",
                   border_win, property, ( title == NULL ? "" : title )
        );

        return false; //EARLY RETURN
    }

    if( border_win->panel )
        del_panel( border_win->panel );
    if( border_win->window )
        delwin( border_win->window );

    border_win->window = newwin( property->rows, property->cols, property->pos_y, property->pos_x );
    border_win->panel  = new_panel( border_win->window );

    wattron( border_win->window, border_win->colour );

    box( border_win->window, 0, 0 );

    if( title != NULL ) {
        const int title_col = ( ( property->cols / 2 ) - ( (int) strlen( title ) / 2 ) );
        mvwprintw( border_win->window, 0, title_col, "%s", title );
    }

    wattroff( border_win->window, border_win->colour );

    return true;
}

/**
 * Shows border window (no refresh done)
 * @param border_win Pointer to initialised ctune_UI_BorderWin_t object
 * @return Error-free success
 */
static bool ctune_UI_Widget_BorderWin_show( ctune_UI_BorderWin_t * border_win ) {
    if( border_win == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "ctune_UI_Widget_BorderWin_show( %p ) NULL pointer argument.", border_win );
        return false; //EARLY RETURN
    }

    if( border_win->window == NULL || border_win->panel == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "ctune_UI_Widget_BorderWin_show( %p ) BorderWin not initialised.", border_win );
        return false; //EARLY RETURN
    }

    top_panel( border_win->panel );
    show_panel( border_win->panel );

    return true;
}

/**
 * Hides border window (no refresh done)
 * @param border_win Pointer to initialised ctune_UI_BorderWin_t object
 * @return Error-free success
 */
static bool ctune_UI_Widget_BorderWin_hide( ctune_UI_BorderWin_t * border_win ) {
    if( border_win == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "ctune_UI_Widget_BorderWin_hide( %p ) NULL pointer argument.", border_win );
        return false; //EARLY RETURN
    }

    if( border_win->window == NULL || border_win->panel == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "ctune_UI_Widget_BorderWin_hide( %p ) BorderWin not initialised.", border_win );
        return false; //EARLY RETURN
    }

    bottom_panel( border_win->panel );
    hide_panel( border_win->panel );

    return true;
}

/**
 * De-allocates resources nested in BorderWin_t
 * @param border_win Pointer to ctune_UI_BorderWin_t object
 */
static void ctune_UI_Widget_BorderWin_free( ctune_UI_BorderWin_t * border_win ) {
    if( border_win ) {
        if( border_win->panel ) {
            del_panel( border_win->panel );
            border_win->panel = NULL;
        }

        if( border_win->window ) {
            delwin( border_win->window );
            border_win->window = NULL;
        }

        CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_UI_Widget_BorderWin_free( %p )] BorderWin freed.", border_win );
    }
}


/**
 * Namespace constructor
 */
const struct ctune_UI_Widget_BorderWin_Namespace ctune_UI_BorderWin = {
    .create = &ctune_UI_Widget_BorderWin_create,
    .init   = &ctune_UI_Widget_BorderWin_init,
    .show   = &ctune_UI_Widget_BorderWin_show,
    .hide   = &ctune_UI_Widget_BorderWin_hide,
    .free   = &ctune_UI_Widget_BorderWin_free,
};
