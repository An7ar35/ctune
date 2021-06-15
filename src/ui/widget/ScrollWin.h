#ifndef CTUNE_UI_WIDGET_SCROLLWIN_H
#define CTUNE_UI_WIDGET_SCROLLWIN_H

#include <ncurses.h>

#include "../datastructure/WindowProperty.h"

/**
 * ScrollWin_t object
 * @param pad       ncurses Pad window
 * @param rows      Pad size in rows
 * @param cols      Pad size in columns
 * @param viewbox   Property pad's view-box on the screen
 * @param pos       Position of the top left corner of the viewbox on the pad
 * @param scrolling Scrolling requirement to see all content
 */
typedef struct ctune_UI_Widget_ScrollWin {
    WINDOW * pad;
    int      rows;
    int      cols;

    /**
     * View-box screen position
     * @param from_y Top-left corner row
     * @param from_x Top-left corner column
     * @param to_y   Bottom-right corner row
     * @param to_x   Bottom-right corner column
     */
    struct {
        int from_y;
        int from_x;
        int to_y;
        int to_x;
    } viewbox;

    /**
     * Position of the view-box on the pad
     * @param y Top-left corner row on pad
     * @param x Top-left corner column on pad
     */
    struct {
        int y;
        int x;
    } pos;

    /**
     * Scrolling state
     * @param y Y-axis scrolling required to see content
     * @param x X-axis scrolling required to see content
     */
    struct {
        bool y;
        bool x;

    } scrolling;

} ctune_UI_ScrollWin_t;

/**
 * ScrollWin class - convenience wrapper for the ncurse pad window functionalities
 */
extern const struct ctune_UI_Widget_ScrollWin_Namespace {
    /**
     * Creates a scrolling window without initiating the pad (use `.createPad(..)` after)
     * @return Blank (zeroed) ScrollWin_t object
     */
    ctune_UI_ScrollWin_t (* initBlank)( void );

    /**
     * Create a scrolling windows pad
     * @param height Total height of the pad
     * @param width  Total width of the pad
     * @return ScrollWin_t object
     */
    ctune_UI_ScrollWin_t (* init)( int height, int width );

    /**
     * Creates the pad for the scrolling window (will free prior pad if not NULL)
     * @param sw     ScrollWin_t object
     * @param height Pad row height
     * @param width  Pad col width
     */
    void (* createPad)( ctune_UI_ScrollWin_t * sw, int height, int width );

    /**
     * Refreshes the pad view
     * @param sw ScrollWin_t object
     */
    void (* refreshView)( ctune_UI_ScrollWin_t * sw );

    /**
     * Redraw pad view with a different view-box size
     * @param sw         ScrollWin_t object
     * @param viewsize_y View-box row size
     * @param viewsize_x View-box column size
     */
    void (* redraw)( ctune_UI_ScrollWin_t * sw, int viewsize_y, int viewsize_x );

    /**
     * Shift the pad view (i.e. scroll)
     * @param sw    ScrollWin_t object
     * @param nrows Number of rows to scroll down by (negative number will scroll up)
     * @param ncols Number of columns to scroll right by (negative number will scroll left)
     */
    void (* scrollPad)( ctune_UI_ScrollWin_t * sw, int nrows, int ncols );

    /**
     * Moves the pad view-box top left corner
     * @param sw ScrollWin_t object
     * @param y  Row
     * @param x  Col
     */
    void (* moveViewBox)( ctune_UI_ScrollWin_t * sw, int y, int x );

    /**
     * Switch the hardware auto-insert/delete line functionality
     * @param sw    ScrollWin_t object
     * @param state State (on=true, off=false)
     */
    void (* setInsDelLine)( ctune_UI_ScrollWin_t * sw, bool state );

    /**
     * Sets the scrolling flags for both X and Y
     * @param sw    ScrollWin_t object
     * @param state Scroll state
     */
    void (* setScrolling)( ctune_UI_ScrollWin_t * sw, bool state );

    /**
     * Sets the scrolling flags for the Y axis
     * @param sw    ScrollWin_t object
     * @param state Scroll state
     */
    void (* setScrollingY)(ctune_UI_ScrollWin_t * sw,  bool state );

    /**
     * Sets the scrolling flags for the X axis
     * @param sw    ScrollWin_t object
     * @param state Scroll state
     */
    void (* setScrollingX)( ctune_UI_ScrollWin_t * sw, bool state );

    /**
     * Gets the scrollable state of the window (X || Y)
     * @param sw ScrollWin_t object
     */
    bool (* isScrollable)( ctune_UI_ScrollWin_t * sw );

    /**
     * Gets the scrollable state of the window in the Y axis (rows)
     * @param sw ScrollWin_t object
     */
    bool (* isScrollableY)( ctune_UI_ScrollWin_t * sw );

    /**
     * Gets the scrollable state of the window in the X axis (cols)
     * @param sw ScrollWin_t object
     */
    bool (* isScrollableX)( ctune_UI_ScrollWin_t * sw );

    /**
     * Resizes the view box of the pad
     * @param sw   ScrollWin_t object
     * @param rows Height
     * @param cols Width
     */
    void (* resizeViewBox)( ctune_UI_ScrollWin_t * sw, int rows, int cols );

    /**
     * Gets the current properties of the displayed/viewable pad section
     * @param sw ScrollWin_t object
     * @return Properties (top-left corner pos on pad and view-box size)
     */
    WindowProperty_t (* getViewProperty)( const ctune_UI_ScrollWin_t * sw );

    /**
     * Resizes the pad
     * @param sw   ScrollWin_t object
     * @param rows Vertical size in rows
     * @param cols Horizontal size in cols
     * @return Success
     */
    bool (* resizePad)( ctune_UI_ScrollWin_t * sw, int rows, int cols );

    /**
     * Clears the content of the pad
     * @param sw ScrollWin_t object
     */
    void (* clearPad)( ctune_UI_ScrollWin_t * sw );

    /**
     * Re-initializes a ScrollWin_t object
     * @param sw ScrollWin_t object
     * @param height Total height of the pad
     * @param width  Total width of the pad
     */
    void (* reinit)( ctune_UI_ScrollWin_t * sw, int height, int width );

    /**
     * Prints all properties to logger
     * @param sw ScrollWin_t object
     */
    void (* logState)( ctune_UI_ScrollWin_t * sw );

    /**
     * Frees resources
     * @param sw ScrollWin object to free content of
     */
    void (* free)( ctune_UI_ScrollWin_t * sw );

} ctune_UI_ScrollWin;

#endif //CTUNE_UI_WIDGET_SCROLLWIN_H
