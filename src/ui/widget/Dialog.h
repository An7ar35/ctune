#ifndef CTUNE_UI_WIDGET_DIALOG_H
#define CTUNE_UI_WIDGET_DIALOG_H

#ifdef NO_NCURSESW
    #include <panel.h>
#else
    #include <ncursesw/panel.h>
#endif

#include "../types/ScrollMask.h"
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

     } scrollbar;

     struct {
         int offset_y;
         int offset_x;

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
     * @param dialog     UI_Dialog_t object
     * @param parent     Parent window dimensions (to fit border into)
     * @param title      Title to print on the window border
     * @param margins    Window margin values
     * @param mouse_ctrl Flag for mouse control on the scrollbars
     */
    void (* createBorderWin)( ctune_UI_Dialog_t * dialog, const WindowProperty_t * parent, const char * title, const WindowMargin_t * margins, bool mouse_ctrl );

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
     * Incrementally scroll the window
     * @param dialog UI_Dialog_t object
     * @param mask   Scroll mask
     */
    void (* incrementalScroll)( ctune_UI_Dialog_t * dialog, ctune_UI_ScrollMask_m mask );

    /**
     * Scroll to the edge
     * @param dialog UI_Dialog_t object
     * @param mask   Scroll mask
     */
    void (* edgeScroll)( ctune_UI_Dialog_t * dialog, ctune_UI_ScrollMask_m mask );

    /**
     * Triggers autoscroll
     * @param dialog UI_Dialog_t object
     * @param y      Current cursor row position on pad
     * @param x      Current cursor column position on pad
     */
    void (* autoScroll)( ctune_UI_Dialog_t * dialog, int y, int x );

    /**
     * Checks if area at coordinate is a window control
     * @param dialog UI_Dialog_t object
     * @param y      Row location on screen
     * @param x      Column location on screen
     * @return Window control mask (includes scrolling)
     */
    ctune_UI_WinCtrlMask_m (* isWinControl)( ctune_UI_Dialog_t * dialog, int y, int x );

    /**
     * Gets the current properties of the displayed/viewable ScrollWin pad section
     * @param dialog UI_Dialog_t object
     * @return Properties (top-left corner pos on pad and view-box size)
     */
    WindowProperty_t (* getViewProperty)( ctune_UI_Dialog_t * dialog );

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
