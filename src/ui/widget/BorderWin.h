#ifndef CTUNE_UI_WIDGET_BORDERWIN_H
#define CTUNE_UI_WIDGET_BORDERWIN_H

#include <ncurses.h>
#include <panel.h>
#include "../datastructure/WindowProperty.h"

/**
 * BorderWin object
 * @param window Ncurses window
 * @param panel  Ncurses panel
 * @param colour Ncurses colour ID to use
 */
typedef struct ctune_UI_Widget_BorderWin {
    WINDOW                 * window;
    PANEL                  * panel;
    int                      colour;

} ctune_UI_BorderWin_t;

/**
 * BorderWin namespace
 */
extern const struct ctune_UI_Widget_BorderWin_Namespace {
    /**
     * Creates an a BorderWin_t object ready for initialisation
     * @param colour Colour attribute ID to use for the border/title
     * @return BorderWin_t object
     */
    ctune_UI_BorderWin_t (* create)( int colour );

    /**
     * Initialised a border window
     * @param border_win Pointer to an initialised ctune_UI_BorderWin_t object
     * @param property   Size/Position properties of the border window
     * @param title      Title (can be NULL if none is desired)
     * @return Success
     */
    bool (* init)( ctune_UI_BorderWin_t * border_win, WindowProperty_t * property, const char * title );

    /**
     * Shows border window (no refresh done)
     * @param border_win Pointer to initialised ctune_UI_BorderWin_t object
     * @return Error-free success
     */
    bool (* show)( ctune_UI_BorderWin_t * border_win );

    /**
     * Hides border window (no refresh done)
     * @param border_win Pointer to initialised ctune_UI_BorderWin_t object
     * @return Error-free success
     */
    bool (* hide)( ctune_UI_BorderWin_t * border_win );

    /**
     * De-allocates resources nested in BorderWin_t
     * @param border_win Pointer to ctune_UI_BorderWin_t object
     */
    void (* free)( ctune_UI_BorderWin_t * border_win );

} ctune_UI_BorderWin;


#endif //CTUNE_UI_WIDGET_BORDERWIN_H
