#ifndef CTUNE_UI_WIDGET_SLIDEMENU_H
#define CTUNE_UI_WIDGET_SLIDEMENU_H

//   SlideMenu
// +------------+       +------------+
// | item a    >|       | < item a   |
// | item b    >|       |------------|
// | item c    >|  ===> | sub item 1 |
// |     .      |       | sub item 2 |
// |     .      |       |     .      |
// |     .      |       |     .      |
// +------------+       +------------+

#ifdef NO_NCURSESW
    #include <ncurses.h>
    #include <panel.h>
#else
    #include <ncursesw/ncurses.h>
    #include <ncursesw/panel.h>
#endif

#include "../../datastructure/String.h"
#include "../../datastructure/Vector.h"
#include "../datastructure/WindowProperty.h"
#include "../widget/ScrollBar.h"

typedef enum ctune_UI_Widget_SlideMenu_MenuItemType {
    CTUNE_UI_SLIDEMENU_PARENT,
    CTUNE_UI_SLIDEMENU_MENU,
    CTUNE_UI_SLIDEMENU_LEAF,
} ctune_UI_SlideMenu_ItemType_e;

/**
 * SlideMenu Menu data-type
 * @param items    Container for the menu items
 * @param parent   Pointer to parent menu
 * @param parent_i Index of parent menu item in its own parent menu
 */
typedef struct ctune_UI_Widget_SlideMenu_Menu {
    Vector_t                                items;
    struct ctune_UI_Widget_SlideMenu_Menu * parent;
    size_t                                  parent_i;

} ctune_UI_SlideMenu_Menu_t;

/**
 * SlideMenu Level entry data-type
 * @param index        Item index position in parent menu
 * @param text         Level entry text to display
 * @param sub_menu     Pointer to sub-menu if any (NULL for none)
 * @param type         Menu item type
 * @param parent_menu  Pointer to parent menu
 * @param data         Optional pointer to a payload for the control function (WILL NOT BE FREED!)
 * @param ctrl_trigger Enable/disable flag for the control function
 * @param ctrl_fn      Optional pointer to control function to trigger action on menu item
 */
typedef struct ctune_UI_Widget_SlideMenu_Item {
    size_t                                  index;
    String_t                                text;
    struct ctune_UI_Widget_SlideMenu_Menu * sub_menu;
    ctune_UI_SlideMenu_ItemType_e           type;
    struct ctune_UI_Widget_SlideMenu_Menu * parent_menu;
    void                                  * data;
    bool                                    ctrl_trigger;

    bool (* ctrl_fn)( struct ctune_UI_Widget_SlideMenu_Item * item );

} ctune_UI_SlideMenu_Item_t;

/**
 * SlideMenu
 * @param redraw           Display redraw flag
 * @param in_focus         Flag to indicate if the SlideMenu is in focus (for UI theming purposes)
 * @param mouse_ctrl       Flag to show mouse controls
 * @param root             Root level menu container
 * @param canvas_property  Pointer to canvas properties to abide to
 * @param canvas_panel     Canvas panel
 * @param canvas_win       Canvas window
 * @param scrollbar_change Flag to reset the scroll dimensions of the scrollbar
 * @param scrollbar        Scrollbar widget
 * @param row              Row properties (general/current)
 */
typedef struct {
    bool                      redraw;
    bool                      in_focus;
    bool                      mouse_ctrl;
    ctune_UI_SlideMenu_Menu_t root;
    const WindowProperty_t  * canvas_property;
    PANEL                   * canvas_panel;
    WINDOW                  * canvas_win;
    bool                      update_scrollbar;
    ctune_UI_ScrollBar_t      scrollbar;

    struct {
        ctune_UI_SlideMenu_Menu_t * curr_menu;
        size_t                      depth;
        size_t                      first_on_page;
        size_t                      last_on_page;
        size_t                      selected;
    } row;

} ctune_UI_SlideMenu_t;


/**
 * SlideMenu namespace
 */
extern const struct ctune_UI_Widget_SlideMenu_Namespace {
    /**
     * Creates a slide menu without a known canvas (use `setCanvasProperties(..)` to set one prior to usage)
     * @return Pointer to a ctune_UI_SlideMenu_t object
     */
    ctune_UI_SlideMenu_t (* create)( void );

    /**
     * Creates a slide menu with a known canvas
     * @param canvas_property Pointer to canvas sizes to abide to
     * @return Pointer to a ctune_UI_SlideMenu_t object
     */
    ctune_UI_SlideMenu_t (* init)( const WindowProperty_t * canvas_property );

    /**
     * Sets a canvas for the slide menu
     * @param menu            Pointer to a ctune_UI_SlideMenu_t object
     * @param canvas_property Pointer to canvas sizes to abide to
     * @param mouse_ctrl      Flag for mouse control on the scrollbars
     */
    void (* setCanvasProperties)( ctune_UI_SlideMenu_t * menu, const WindowProperty_t * canvas_property, bool mouse_ctrl );

    /**
     * Create/allocates a sub-menu
     * @param menu_ptr     Pointer to the menu pointer
     * @param parent       Pointer to menu's parent menu
     * @param parent_index Index position of the menu in the parent's menu
     * @return Pointer to sub menu (NULL on failure)
     */
    ctune_UI_SlideMenu_Menu_t * (* createMenu)( ctune_UI_SlideMenu_Menu_t ** menu_ptr, ctune_UI_SlideMenu_Menu_t * parent, size_t parent_index );

    /**
     * Add a menu item to a menu
     * @param menu    Parent menu to add item to
     * @param type    Menu item type
     * @param text    Display text for item
     * @param data    Optional pointer to data associated with the menu item (NULL for none)
     * @param ctrl_fn Control function for item (or NULL for none)
     * @return Pointer to menu item (NULL on failure)
     */
    ctune_UI_SlideMenu_Item_t * (* createMenuItem)( ctune_UI_SlideMenu_Menu_t * menu, ctune_UI_SlideMenu_ItemType_e type, const char * text, void * data, bool (* ctrl_fn)( ctune_UI_SlideMenu_Item_t * ) );

    /**
     * Switch mouse control UI on/off
     * @param menu            Pointer to a ctune_UI_SlideMenu_t object
     * @param mouse_ctrl_flag Flag to turn feature on/off
     */
    void (* setMouseCtrl)( ctune_UI_SlideMenu_t * menu, bool mouse_ctrl_flag );
    
    /**
     * Gets the mouse control status
     * @param menu Pointer to a ctune_UI_SlideMenu_t object
     * @return Mouse control state
     */
    bool (* mouseCtrl)( ctune_UI_SlideMenu_t * menu );

    /**
     * Sets the redraw flag on
     * @param menu Pointer to a ctune_UI_SlideMenu_t object
     */
    void (* setRedraw)( ctune_UI_SlideMenu_t * menu );

    /**
     * Sets the 'in focus' flag
     * @param win   Pointer to a ctune_UI_SlideMenu_t object
     * @param focus Flag value
     */
    void (* setFocus)( ctune_UI_SlideMenu_t * menu, bool focus );

    /**
     * Activate control function on the current menu and/or move to its submenu
     * @param menu Pointer to a ctune_UI_SlideMenu_t object
     */
    void (* navKeyEnter)( ctune_UI_SlideMenu_t * menu );

    /**
     * Change selected row to previous entry
     * @param menu Pointer to a ctune_UI_SlideMenu_t object
     */
    void (* navKeyUp)( ctune_UI_SlideMenu_t * menu );

    /**
     * Change selected row to next entry
     * @param menu Pointer to a ctune_UI_SlideMenu_t object
     */
    void (* navKeyDown)( ctune_UI_SlideMenu_t * menu );

    /**
     * Change to the submenu/activate ctrl function of currently selected row
     * @param menu Pointer to a ctune_UI_SlideMenu_t object
     */
    void (* navKeyRight)( ctune_UI_SlideMenu_t * menu );

    /**
     * Change to the parent menu/item of currently selected row
     * @param menu Pointer to a ctune_UI_SlideMenu_t object
     */
    void (* navKeyLeft)( ctune_UI_SlideMenu_t * menu );

    /**
     * Change selected row to entry a page length away
     * @param menu Pointer to a ctune_UI_SlideMenu_t object
     */
    void (* navKeyPageUp)( ctune_UI_SlideMenu_t * menu );

    /**
     * Change selected row to entry a page length away
     * @param menu Pointer to a ctune_UI_SlideMenu_t object
     */
    void (* navKeyPageDown)( ctune_UI_SlideMenu_t * menu );

    /**
     * Change selection to the first item in the list
     * @param menu Pointer to a ctune_UI_SlideMenu_t object
     */
    void (* navKeyFirst)( ctune_UI_SlideMenu_t * menu );

    /**
     * Change selection to the last item in the list
     * @param menu Pointer to a ctune_UI_SlideMenu_t object
     */
    void (* navKeyLast)( ctune_UI_SlideMenu_t * menu );

    /**
     * Select at given coordinates
     * @param menu RSListWin_t object
     * @param y    Row location on screen
     * @param x    Column location on screen
     */
    void (* selectAt)( ctune_UI_SlideMenu_t * menu, int y, int x );

    /**
     * Checks if area at coordinate is a scroll button
     * @param menu Pointer to a ctune_UI_SlideMenu_t object
     * @param y    Row location on screen
     * @param x    Column location on screen
     * @return Scroll mask
     */
    ctune_UI_ScrollMask_m (* isScrollButton)( ctune_UI_SlideMenu_t * menu, int y, int x );

    /**
     * Resets selected position to the first item in the root menu
     * @param menu Pointer to a ctune_UI_SlideMenu_t object
     */
    void (* reset)( ctune_UI_SlideMenu_t * menu );

    /**
     * Resizes the menu
     * @param menu Pointer to Pointer to a ctune_UI_SlideMenu_t object
     */
    void (* resize)( ctune_UI_SlideMenu_t * menu );

    /**
     * Show updated window
     * @param menu Pointer to a ctune_UI_SlideMenu_t object
     * @return Success
     */
    bool (* show)( ctune_UI_SlideMenu_t * menu );

    /**
     * Hides the SlideMenu (no refresh done)
     * @param menu Pointer to a ctune_UI_SlideMenu_t object
     */
    void (* hide)( ctune_UI_SlideMenu_t * menu );

    /**
     * De-allocates a slide menu's content
     * @param menu Pointer to a ctune_UI_SlideMenu_t object
     */
    void (* free)( ctune_UI_SlideMenu_t * menu );

} ctune_UI_SlideMenu;


#endif //CTUNE_UI_WIDGET_SLIDEMENU_H
