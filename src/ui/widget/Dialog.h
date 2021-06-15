#ifndef CTUNE_UI_WIDGET_DIALOG_H
#define CTUNE_UI_WIDGET_DIALOG_H

#include <panel.h>

#include "../widget/ScrollWin.h"
#include "../widget/ScrollBar.h"
#include "../widget/BorderWin.h"
#include "../datastructure/WindowProperty.h"
#include "../datastructure/WindowMargin.h"

 /**
  * Dialog object
  * @param property   Dialog window property
  * @param border_win Border window
  * @param canvas     Dialog content scrolling window
  * @param scrollbar  Scrollbar properties
  * @param autoscroll Auto-scrolling properties
  */
 typedef struct ctune_UI_Widget_Dialog {
     WindowProperty_t     property;
     ctune_UI_BorderWin_t border_win;
     ctune_UI_ScrollWin_t canvas;

     struct {
         bool                 y_init;
         bool                 x_init;
         WindowProperty_t     y_property;
         WindowProperty_t     x_property;
         ctune_UI_ScrollBar_t y;
         ctune_UI_ScrollBar_t x;
         size_t               scroll_pos_x;
         size_t               scroll_pos_y;

     } scrollbar;

     struct {
         int offset_y;
         int offset_x;
//         int prev_pos_y;
//         int prev_pos_x;

     } autoscroll;

} ctune_UI_Dialog_t;


extern const struct ctune_UI_Widget_Dialog_Namespace {
    /**
     * Creates an initialised Dialog_t object
     * @return Initialised Dialog_t object
     */
    ctune_UI_Dialog_t (* init)( void );

    /**
     * Creates a scroll windows
     * @param dialog UI_Dialog_t object
     * @param rows   Row size of the scrolling pad
     * @param cols   Col size of the scrolling pad
     */
    void (* createScrollWin)( ctune_UI_Dialog_t * dialog, int rows, int cols );

    /**
     * Create a border window (a scroll windows must have been created first!)
     * @param dialog   UI_Dialog_t object
     * @param parent   Parent window dimensions (to fit border into)
     * @param title    Title to print on the window border
     * @param margins  Window margin values
     */
    void (* createBorderWin)( ctune_UI_Dialog_t * dialog, const WindowProperty_t * parent, const char * title, const WindowMargin_t * margins );

    /**
     * Gets the scrollable state on the vertical axis
     * @param dialog UI_Dialog_t object
     * @return Scrollable state
     */
    bool (* isScrollableY)( ctune_UI_Dialog_t * dialog );

    /**
     * Gets the scrollable state on the horizontal axis
     * @param dialog UI_Dialog_t object
     * @return Scrollable state
     */
    bool (* isScrollableX)( ctune_UI_Dialog_t * dialog );

    /**
     * Sets the auto-scroll threshold offset
     * @param dialog UI_Dialog_t object
     * @param y      Vertical offset
     * @param x      Horizontal offset
     */
    void (* setAutoScrollOffset)( ctune_UI_Dialog_t * dialog, int y, int x );

    /**
     * Scroll content up
     * @param dialog UI_Dialog_t object
     */
    void (* scrollUp)( ctune_UI_Dialog_t * dialog );

    /**
     * Scroll content right
     * @param dialog UI_Dialog_t object
     */
    void (* scrollRight)( ctune_UI_Dialog_t * dialog );

    /**
     * Scroll content down
     * @param dialog UI_Dialog_t object
     */
    void (* scrollDown)( ctune_UI_Dialog_t * dialog );

    /**
     * Scroll content left
     * @param dialog UI_Dialog_t object
     */
    void (* scrollLeft)( ctune_UI_Dialog_t * dialog );

    /**
     * Scroll back to the left/top
     * @param dialog UI_Dialog_t object
     */
    void (* scrollHome)( ctune_UI_Dialog_t * dialog );

    /**
     * Scroll back to the top (horizontal scroll untouched)
     * @param dialog UI_Dialog_t object
     */
    void(* scrollTop)( ctune_UI_Dialog_t * dialog );

    /**
     * Scroll down to the bottom (horizontal scroll untouched)
     * @param dialog UI_Dialog_t object
     */
    void(* scrollBottom)( ctune_UI_Dialog_t * dialog );

    /**
     * Scroll to the far left (vertical scroll untouched)
     * @param dialog UI_Dialog_t object
     */
    void(* scrollLeftEdge)( ctune_UI_Dialog_t * dialog );

    /**
     * Scroll to the far right (vertical scroll untouched)
     * @param dialog UI_Dialog_t object
     */
    void(* scrollRightEdge)( ctune_UI_Dialog_t * dialog );

    /**
     * Triggers autoscroll
     * @param dialog UI_Dialog_t object
     * @param y      Current cursor row position on pad
     * @param x      Current cursor column position on pad
     */
    void (* autoScroll)( ctune_UI_Dialog_t * dialog, int y, int x );

    /**
     * Moves the dialog panel to the top and un-hides it
     * @param dialog UI_Dialog_t object
     */
    void (* show)( ctune_UI_Dialog_t * dialog );

    /**
     * Repaints the view with updated content
     * @param dialog UI_Dialog_t object
     */
    void (* refreshView)( ctune_UI_Dialog_t * dialog );

    /**
     * Moves the dialog panel to the bottom top and hides it
     * @param dialog UI_Dialog_t object
     */
    void (* hide)( ctune_UI_Dialog_t * dialog );

    /**
     * De-allocates resources
     * @param dialog UI_Dialog_t object
     */
    void (* free)( ctune_UI_Dialog_t * dialog );

} ctune_UI_Dialog;

#endif //CTUNE_UI_WIDGET_DIALOG_H
