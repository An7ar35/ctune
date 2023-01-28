#include "BorderWin.h"

#include <string.h>

#include "../../logger/Logger.h"
#include "../definitions/Icons.h"

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
        .ctrl   = {
            .show      = false,
            .close_btn = { 0, 0, 0, 0 },
        },
    };
}

/**
 * Initialised a border window
 * @param border_win Pointer to an initialised ctune_UI_BorderWin_t object
 * @param property   Size/Position properties of the border window
 * @param title      Title (can be NULL if none is desired)
 * @param show_ctrl  Flag to paint window controls
 * @return Success
 */
static bool ctune_UI_Widget_BorderWin_init( ctune_UI_BorderWin_t * border_win, WindowProperty_t * property, const char * title, bool show_ctrl ) {
    if( border_win == NULL || property == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_Widget_BorderWin_init( %p, %p, \"%s\", %s )] One or more args is NULL.",
                   border_win, property, ( title == NULL ? "" : title ), ( show_ctrl ? "true" : "false" )
        );

        return false; //EARLY RETURN
    }

    if( border_win->panel ) {
        del_panel( border_win->panel );
    }

    if( border_win->window ) {
        delwin( border_win->window );
    }

    border_win->window = newwin( property->rows, property->cols, property->pos_y, property->pos_x );
    border_win->panel  = new_panel( border_win->window );

    wattron( border_win->window, border_win->colour );

    box( border_win->window, 0, 0 );

    int title_col = 0;

    if( title != NULL ) {
        title_col = ( ( property->cols / 2 ) - ( (int) strlen( title ) / 2 ) );
        mvwprintw( border_win->window, 0, title_col, "%s", title );
    }

    if( ( border_win->ctrl.show = show_ctrl ) ) {
        border_win->ctrl.close_btn = (WindowProperty_t){
            .pos_y = 0,
            .pos_x = 1,
            .rows  = 1,
            .cols  = 3 //"[X]"
        };

        if( title_col > 4 ) {
            //              ┌[X][ Search for Station(s) ]───┐
            //               ^  ^                           │
            // close_btn.pos_y  title_col
            mvwprintw( border_win->window,
                       border_win->ctrl.close_btn.pos_y,
                       border_win->ctrl.close_btn.pos_x,
                       "[%s]",
                       ctune_UI_Icons.icon( CTUNE_UI_ICON_WINCTRL_CLOSE ) );

        } else {
            CTUNE_LOG( CTUNE_LOG_WARNING,
                       "[ctune_UI_Widget_BorderWin_init( %p, %p, \"%s\", %s )] Not enough space to show window control (cols available: %i).",
                       border_win, property, ( title == NULL ? "" : title ), ( show_ctrl ? "true" : "false" ), title_col
            );

            border_win->ctrl.show = false;
        }
    }

    wattroff( border_win->window, border_win->colour );

    return true;
}

/**
 * Checks if area at coordinate is a control button
 * @param border_win Pointer to an initialised ctune_UI_BorderWin_t object
 * @param y          Row location on screen
 * @param x          Column location on screen
 * @return Control button mask
 */
static ctune_UI_WinCtrlMask_m ctune_UI_Widget_BorderWin_isCtrlButton( ctune_UI_BorderWin_t * border_win, int y, int x ) {
    if( border_win && border_win->ctrl.show ) {
        if( wmouse_trafo( border_win->window, &y, &x, false ) && y == 0 ) { //i.e. in window and on first row where the controls are
            if( x >= border_win->ctrl.close_btn.pos_x && x < ( border_win->ctrl.close_btn.pos_x + border_win->ctrl.close_btn.cols ) ) {
                return CTUNE_UI_WINCTRLMASK_CLOSE;
            }
        }
    }

    return 0;
}

/**
 * Shows border window (no refresh done)
 * @param border_win Pointer to initialised ctune_UI_BorderWin_t object
 * @return Error-free success
 */
static bool ctune_UI_Widget_BorderWin_show( ctune_UI_BorderWin_t * border_win ) {
    if( border_win == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_Widget_BorderWin_show( %p )] NULL pointer argument.", border_win );
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
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_Widget_BorderWin_hide( %p )] NULL pointer argument.", border_win );
        return false; //EARLY RETURN
    }

    if( border_win->window == NULL || border_win->panel == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_Widget_BorderWin_hide( %p )] BorderWin not initialised.", border_win );
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
    .create       = &ctune_UI_Widget_BorderWin_create,
    .init         = &ctune_UI_Widget_BorderWin_init,
    .isCtrlButton = &ctune_UI_Widget_BorderWin_isCtrlButton,
    .show         = &ctune_UI_Widget_BorderWin_show,
    .hide         = &ctune_UI_Widget_BorderWin_hide,
    .free         = &ctune_UI_Widget_BorderWin_free,
};
