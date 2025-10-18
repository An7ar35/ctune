#ifndef CTUNE_UI_WIDGET_BORDERWIN_H
#define CTUNE_UI_WIDGET_BORDERWIN_H

#ifdef NO_NCURSESW
    #include <ncurses.h>
    #include <panel.h>
#else
    #include <ncursesw/ncurses.h>
    #include <ncursesw/panel.h>
#endif

#include "../types/WinCtrlMask.h"
#include "../datastructure/WindowProperty.h"

/**
 * BorderWin object
 * @param window Ncurses window
 * @param panel  Ncurses panel
 * @param colour Ncurses colour ID to use
 * @param ctrl   Positions of window control(s)
 */
typedef struct ctune_UI_Widget_BorderWin {
    WINDOW                 * window;
    PANEL                  * panel;
    int                      colour;

    struct {
        bool             show;
        WindowProperty_t close_btn;
    } ctrl;

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
     * @param show_ctrl  Flag to paint window controls
     * @return Success
     */
    bool (* init)( ctune_UI_BorderWin_t * border_win, WindowProperty_t * property, const char * title, bool show_ctrl );

    /**
     * Checks if area at coordinate is a control button
     * @param border_win Pointer to an initialised ctune_UI_BorderWin_t object
     * @param y          Row location on screen
     * @param x          Column location on screen
     * @return Control button mask
     */
    ctune_UI_WinCtrlMask_m (* isCtrlButton)( ctune_UI_BorderWin_t * border_win, int y, int x );

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
