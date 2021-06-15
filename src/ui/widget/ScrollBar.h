#ifndef CTUNE_UI_WIDGET_SCROLLBAR_H
#define CTUNE_UI_WIDGET_SCROLLBAR_H

#include <stdbool.h>
#include <ncurses.h>
#include <panel.h>

#include "../datastructure/WindowProperty.h"

typedef enum ctune_UI_Widget_ScrollBar_Pos {
    TOP = 0, BOTTOM, //horizontal
    LEFT, RIGHT,     //vertical
} ctune_UI_ScrollBar_Pos_e;

/**
 * Vertical ScrollBar
 * @param redraw            Redraw canvas flag
 * @param canvas_properties Parent canvas dimensions and position
 * @param bar_position      Relative position of the scrollbar to the parent canvas (LEFT/RIGHT/TOP/BOTTOM edges)
 * @param canvas_panel      Canvas panel
 * @param canvas_win        Canvas window
 * @param always_show       Flag to show the scroller (true: always show, false: show only when there is scrolling)
 * @param bar_length        Total length of the displayed scrollbar
 * @param scroll_length     Total scroll length
 * @param page_count        Number of overlapping page views to scroll
 * @param page_increments   Total number of increments available (i.e. number of offsets for the scroller)
 * @param page_inc_val      Increment value for a scroll page
 * @param scroller_pos      increment position offset for the scroller
 * @param scroller_size     Size of the scroller displayed inside the scroll bar
 */
typedef struct ctune_UI_Widget_ScrollBar {
    bool                     redraw;
    const WindowProperty_t * canvas_properties;
    ctune_UI_ScrollBar_Pos_e bar_position;
    PANEL                  * canvas_panel;
    WINDOW                 * canvas_win;

    bool                     always_show;
    size_t                   bar_length;
    size_t                   scroll_length;
    size_t                   page_count;
    size_t                   page_increments;
    double                   page_inc_val;
    double                   scroller_pos;
    size_t                   scroller_size;

} ctune_UI_ScrollBar_t;

/**
 * ScrollBar namespace
 */
extern const struct ctune_UI_Widget_ScrollBar_Namespace {
    /**
     * Initialises a slide menu
     * @param canvas      Pointer to parent canvas property
     * @param pos         Relative position of the vertical scroll bar
     * @param always_show Flag to always show the scroller in the scroll bar
     * @return ctune_UI_ScrollBar_t object
     */
    ctune_UI_ScrollBar_t (* init)( const WindowProperty_t * canvas, ctune_UI_ScrollBar_Pos_e pos, bool always_show );

    /**
     * Sets the redraw flag on
     * @param scrollbar ctune_UI_ScrollBar_t object
     */
    void (* setRedraw)( ctune_UI_ScrollBar_t * scrollbar );

    /**
     * Sets the base scrolling length and resets position to 0
     * @param scrollbar ctune_UI_ScrollBar_t object
     * @param content_size Size of the content represented by the scrollbar
     */
    void (* setScrollLength)( ctune_UI_ScrollBar_t * scrollbar, size_t content_size );

    /**
     * Sets the flag to always show the scroller in the scroll bar
     * @param scrollbar ctune_UI_ScrollBar_t object
     * @param flag      Flag value
     */
    void (* setAlwaysShowFlag)( ctune_UI_ScrollBar_t * scrollbar, bool flag );

    /**
     * Sets the current position on the scrolling length
     * @param scrollbar ctune_UI_ScrollBar_t object
     * @param pos       Scroll position on the set length
     * @return Scroll event
     */
    bool (* setPosition)( ctune_UI_ScrollBar_t * scrollbar, size_t pos );

    /**
     * Gets the calculated total number of page increments
     * @param scrollbar ctune_UI_ScrollBar_t object
     * @return Page increments available
     */
    size_t (* getTotalIncrements)( const ctune_UI_ScrollBar_t * scrollbar );

    /**
     * Initiate the canvas window/panel
     * @param scrollbar ctune_UI_ScrollBar_t object
     */
    void (* initCanvas)( ctune_UI_ScrollBar_t * scrollbar );

    /**
     * Show updated window
     * @param scrollbar ctune_UI_ScrollBar_t object
     * @return Success
     */
    bool (* show)( ctune_UI_ScrollBar_t * scrollbar );

    /**
     * Moves the scrollbar panel to the bottom and hides it
     * @param scrollbar ctune_UI_ScrollBar_t object
     */
    void (* hide)( ctune_UI_ScrollBar_t * scrollbar );

    /**
     * De-allocates a slide menu's content
     * @param scrollbar ctune_UI_ScrollBar_t object
     */
    void (* free)( ctune_UI_ScrollBar_t * scrollbar );

} ctune_UI_ScrollBar;

#endif //CTUNE_UI_WIDGET_SCROLLBAR_H
