#include "SlideMenu.h"

#include "../../logger/Logger.h"
#include "../definitions/Theme.h"

/**
 * [PRIVATE] Select a row in the menu
 * @param win    ctune_UI_SlideMenu_t object
 * @param offset_x Horizontal offset to move selection by
 * @param offset_y Vertical offset to move selection by
 */
static bool ctune_UI_SlideMenu_navKeyRow( ctune_UI_SlideMenu_t * menu, int offset_x, int offset_y ) {
    if( menu->row.curr_menu == NULL || Vector.empty( &menu->row.curr_menu->items ) )
        return false; //EARLY RETURN

    ctune_UI_SlideMenu_Item_t * item = Vector.at( &menu->row.curr_menu->items, menu->row.selected );

    if( offset_x > 0 ) { // going right
        if( item != NULL && item->sub_menu != NULL ) {
            menu->row.curr_menu    = item->sub_menu;
            menu->row.selected     = 0;
            menu->row.depth       += 1;
            menu->update_scrollbar = true;
        }

    } else if( offset_x < 0 ) { //going left
        if( item != NULL && item->type == CTUNE_UI_SLIDEMENU_PARENT && item->parent_menu != NULL && item->parent_menu->parent != NULL ) {
            menu->row.curr_menu    = item->parent_menu->parent;
            menu->row.selected     = item->parent_menu->parent_i;
            menu->row.depth       -= 1;
            menu->update_scrollbar = true;
        }
    }

    if( offset_y < 0 ) { //going up
        if( menu->row.selected < abs( offset_y ) ) {
            menu->row.selected = 0;
        } else {
            menu->row.selected += offset_y;
        }

    } else if ( offset_y > 0 ) { //going down
        if( ( menu->row.selected + offset_y ) >= Vector.size( &menu->row.curr_menu->items ) ) {
            menu->row.selected = Vector.size( &menu->row.curr_menu->items ) - 1;
        } else {
            menu->row.selected += offset_y;
        }
    }

    menu->redraw = true;
    return true;
}

/**
 * [PRIVATE] De-allocates a menu item's content
 * @param menu_item Pointer to ctune_UI_SlideMenu_Item_t object
 */
static void ctune_UI_SlideMenu_freeSlideMenuItem( void * menu_item ) {
    if( menu_item != NULL ) {
        ctune_UI_SlideMenu_Item_t *item = ( ctune_UI_SlideMenu_Item_t * ) menu_item;

        String.free( &item->text );

        if( item->sub_menu != NULL ) {
            Vector.clear_vector( &item->sub_menu->items );
            free( item->sub_menu );
            item->sub_menu = NULL;
        }
    }
}

/**
 * [PRIVATE] Checks if currently selected row has a control function with a 'live' trigger
 * @param menu ctune_UI_SlideMenu_t object
 * @return State
 */
static bool ctune_UI_SlideMenu_hasCtrlFunction( ctune_UI_SlideMenu_t * menu ) {
    if( menu != NULL ) {
        ctune_UI_SlideMenu_Item_t * item = Vector.at( &menu->row.curr_menu->items, menu->row.selected );

        if( item != NULL )
            return ( item->ctrl_trigger && item->ctrl_fn != NULL ); //EARLY RETURN
    }

    return false;
}

/**
 * [PRIVATE] Triggers the control function of the selected menu item
 * @param menu ctune_UI_SlideMenu_t object
 * @return Success state of ctrl function
 */
static bool ctune_UI_SlideMenu_triggerCtrlFunction( ctune_UI_SlideMenu_t * menu ) {
    if( menu != NULL ) {
        ctune_UI_SlideMenu_Item_t *item = Vector.at( &menu->row.curr_menu->items, menu->row.selected );

        if( item != NULL && item->ctrl_fn != NULL && item->ctrl_trigger )
            return item->ctrl_fn( item ); //EARLY RETURN
    }

    return false;
}

/**
 * [PRIVATE] Draw entries to the canvas window
 * @param menu   ctune_UI_SlideMenu_t object
 * @param resize Flag for resizing the canvas/panel
 */
static void ctune_UI_SlideMenu_drawCanvas( ctune_UI_SlideMenu_t * menu, bool resize ) {
    const size_t page_index_offset = menu->canvas_property->rows - 1;

    if( resize ) {
        if( menu->canvas_panel ) {
            del_panel( menu->canvas_panel );
            menu->canvas_panel = NULL;
        }

        if( menu->canvas_win ) {
            delwin( menu->canvas_win );
            menu->canvas_win = NULL;
        }

        ctune_UI_ScrollBar.free( &menu->scrollbar );
        menu->scrollbar = ctune_UI_ScrollBar.init( menu->canvas_property, RIGHT, false );

        menu->update_scrollbar  = true;
        menu->row.first_on_page = 0;
        menu->row.last_on_page  = menu->row.first_on_page + page_index_offset;
    }

    if( menu->canvas_win == NULL ) {
        menu->canvas_win   = newwin( menu->canvas_property->rows, menu->canvas_property->cols, menu->canvas_property->pos_y, menu->canvas_property->pos_x );
        menu->canvas_panel = new_panel( menu->canvas_win );
    } else {
        werase( menu->canvas_win );
    }

    //calculate the page range based on the currently selected menu item
    if( menu->row.selected <= menu->row.first_on_page ) {
        menu->row.first_on_page = menu->row.selected;
        menu->row.last_on_page  = menu->row.first_on_page + page_index_offset;

    } else if( menu->row.selected > menu->row.last_on_page ) {
        menu->row.last_on_page  = menu->row.selected;
        menu->row.first_on_page = menu->row.last_on_page - page_index_offset;
    }

    //init pointer to current menu
    if( menu->row.curr_menu == NULL )
        menu->row.curr_menu = &menu->root;


    //set the scrollbar
    if( menu->update_scrollbar ) {
        ctune_UI_ScrollBar.setScrollLength( &menu->scrollbar, Vector.size( &menu->row.curr_menu->items ) );
        menu->update_scrollbar = false;
    }

    ctune_UI_ScrollBar.setPosition( &menu->scrollbar, menu->row.first_on_page );


    //Fill canvas with entries in the page range
    int row = 0;
    for( size_t i = menu->row.first_on_page; i <= menu->row.last_on_page; ++i ) {
        if( i >= Vector.size( &menu->row.curr_menu->items ) )
            break;

        const ctune_UI_SlideMenu_Item_t * item = Vector.at( &menu->row.curr_menu->items, i );

        if( item != NULL ) {
            int row_theme = ( menu->row.selected == i
                              ? ctune_UI_Theme.color( ( menu->in_focus ? CTUNE_UI_ITEM_ROW_SELECTED_FOCUSED : CTUNE_UI_ITEM_ROW_SELECTED_UNFOCUSED ) )
                              : ctune_UI_Theme.color( CTUNE_UI_ITEM_ROW ) );

            wattron( menu->canvas_win, row_theme );

            mvwhline( menu->canvas_win, row, 0, ' ', menu->canvas_property->cols );
            if( item->type == CTUNE_UI_SLIDEMENU_PARENT )
                mvwprintw( menu->canvas_win, row, 0, "%c ", '<' );

            mvwprintw( menu->canvas_win, row, 2, "%s", item->text._raw );

            if( item->type == CTUNE_UI_SLIDEMENU_MENU )
                mvwprintw( menu->canvas_win, row, ( menu->canvas_property->cols - 3 ), " %c", '>' );

            wattroff( menu->canvas_win, row_theme );

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_SlideMenu_drawCanvas( %p )] Item i=%lu is NULL (%p)",
                       menu, menu->row.selected, menu->row.curr_menu
            );
        }

        ++row;
    }

    menu->redraw = false;
}

/**
 * Initialises a slide menu
 * @param canvas_property Pointer to canvas sizes to abide to
 * @return ctune_UI_SlideMenu_t object
 */
static ctune_UI_SlideMenu_t ctune_UI_SlideMenu_init( const WindowProperty_t * canvas_property ) {
    return (ctune_UI_SlideMenu_t) {
        .canvas_property  = canvas_property,
        .canvas_panel     = NULL,
        .canvas_win       = NULL,
        .update_scrollbar = true,
        .scrollbar        = ctune_UI_ScrollBar.init( canvas_property, RIGHT, false ),
        .redraw           = true,
        .root = {
            .parent   = NULL,
            .parent_i = 0,
            .items    = Vector.init( sizeof( ctune_UI_SlideMenu_Item_t ), ctune_UI_SlideMenu_freeSlideMenuItem ),
        },
        .row = {
            .curr_menu     = NULL,
            .depth         = 0,
            .selected      = 0,
            .first_on_page = 0,
            .last_on_page  = 0,
        }
    };
}

/**
 * Create/allocates a sub-menu
 * @param menu_ptr     Pointer to the menu pointer
 * @param parent       Pointer to menu's parent menu
 * @param parent_index Index position of the menu in the parent's menu
 * @return Pointer to sub menu (NULL on failure)
 */
static ctune_UI_SlideMenu_Menu_t * ctune_UI_SlideMenu_createMenu(
    ctune_UI_SlideMenu_Menu_t ** menu_ptr,
    ctune_UI_SlideMenu_Menu_t *  parent,
    size_t                       parent_index )
{
    if( menu_ptr == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_SlideMenu_createMenu( %p, %p, %lu )] "
                   "Pointer to menu pointer is NULL.",
                   menu_ptr, parent, parent_index
        );

        return NULL; //EARLY RETURN
    }

    if( ( (*menu_ptr) = malloc( sizeof( ctune_UI_SlideMenu_Menu_t ) ) ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_SlideMenu_createMenu( %p, %p, %lu )] "
                   "Failed to allocate memory for the menu.",
                   menu_ptr, parent, parent_index
        );

        return NULL; //EARLY RETURN
    }

    (*menu_ptr)->parent   = parent;
    (*menu_ptr)->parent_i = parent_index;
    (*menu_ptr)->items    = Vector.init( sizeof( ctune_UI_SlideMenu_Item_t ), ctune_UI_SlideMenu_freeSlideMenuItem );

    return (*menu_ptr);
}

/**
 * Add a menu item to a menu
 * @param menu    Parent menu to add item to
 * @param type    Menu item type
 * @param text    Display text for item
 * @param data    Optional pointer to data associated with the menu item (NULL for none)
 * @param ctrl_fn Control function for item (or NULL for none)
 * @return Success
 */
static bool ctune_UI_SlideMenu_createMenuItem(
    ctune_UI_SlideMenu_Menu_t   * menu,
    ctune_UI_SlideMenu_ItemType_e type,
    const char                  * text,
    void                        * data,
    bool (* ctrl_fn)( ctune_UI_SlideMenu_Item_t * ) )
{
    if( menu == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_SlideMenu_createMenuItem( %p, \"%s\", %p, %i, %p )] "
                   "Menu pointer is NULL.",
                   menu, text, data, type, ctrl_fn
        );

        return false; //EARLY RETURN
    }

    ctune_UI_SlideMenu_Item_t * item = Vector.emplace_back( &menu->items );

    if( item == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_SlideMenu_createMenuItem( %p, \"%s\", %p, %i, %p )] "
                   "Failed to add item to root menu.",
                   menu, text, data, type, ctrl_fn
        );

        return false; //EARLY RETURN
    }

    item->index           = ( Vector.size( &menu->items ) - 1 );
    item->parent_menu     = menu;
    item->sub_menu        = NULL;
    item->type            = type;
    item->text            = String.init();
    item->data            = data;
    item->ctrl_trigger    = ( ctrl_fn != NULL );
    item->ctrl_fn         = ctrl_fn;

    if( !String.set( &item->text, text ) ) {
        if( !Vector.empty( &menu->items ) )
            Vector.remove( &menu->items, Vector.size( &menu->items ) - 1 );
        return false;
    }

    return true;
}


/**
 * Sets the redraw flag on
 * @param menu ctune_UI_SlideMenu_t object
 */
static void ctune_UI_SlideMenu_setRedraw( ctune_UI_SlideMenu_t * menu ) {
    menu->redraw = true;
    ctune_UI_ScrollBar.setRedraw( &menu->scrollbar );
}

/**
 * Sets the 'in focus' flag
 * @param win   ctune_UI_SlideMenu_t object
 * @param focus Flag value
 */
static void ctune_UI_SlideMenu_setFocus( ctune_UI_SlideMenu_t * menu, bool focus ) {
    menu->in_focus = focus;
}

/**
 * Activate the action on the currently selected menu item based on its type
 * @param menu ctune_UI_SlideMenu_t object
 */
static void ctune_UI_SlideMenu_navKeyEnter( ctune_UI_SlideMenu_t * menu ) {
    if( menu != NULL ) {
        ctune_UI_SlideMenu_Item_t * item = Vector.at( &menu->row.curr_menu->items, menu->row.selected );

        switch( item->type ) {
            case CTUNE_UI_SLIDEMENU_PARENT: {
                ctune_UI_SlideMenu_navKeyRow( menu, -1, 0 );
            } break;

            case CTUNE_UI_SLIDEMENU_MENU: //fallthrough
            case CTUNE_UI_SLIDEMENU_LEAF: {
                if( ctune_UI_SlideMenu_hasCtrlFunction( menu ) ) {
                    if( !ctune_UI_SlideMenu_triggerCtrlFunction( menu ) ) {
                        CTUNE_LOG( CTUNE_LOG_ERROR,
                                   "[ctune_UI_SlideMenu_navKeyEnter( %p )] Failed to trigger ctrl function on menu item.",
                                   menu
                        );
                    }
                }

                ctune_UI_SlideMenu_navKeyRow( menu, +1, 0 );
            } break;

            default:
                break;
        }
    }
}

/**
 * Change selected row to previous entry
 * @param menu ctune_UI_SlideMenu_t object
 */
static void ctune_UI_SlideMenu_navKeyUp( ctune_UI_SlideMenu_t * menu ) {
    ctune_UI_SlideMenu_navKeyRow( menu, 0, -1 );
}

/**
 * Change selected row to next entry
 * @param menu ctune_UI_SlideMenu_t object
 */
static void ctune_UI_SlideMenu_navKeyDown( ctune_UI_SlideMenu_t * menu ) {
    ctune_UI_SlideMenu_navKeyRow( menu, 0, +1 );
}

/**
 * Change to the submenu/activate ctrl function of currently selected row
 * @param menu ctune_UI_SlideMenu_t object
 */
static void ctune_UI_SlideMenu_navKeyRight( ctune_UI_SlideMenu_t * menu ) {
    if( ctune_UI_SlideMenu_hasCtrlFunction( menu ) ) {
        if( !ctune_UI_SlideMenu_triggerCtrlFunction( menu ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_SlideMenu_navKeyRight( %p )] Failed to trigger ctrl function on menu item.",
                       menu
            );
        }
    }

    ctune_UI_SlideMenu_navKeyRow( menu, +1, 0 );
}

/**
 * Change to the parent menu/item of currently selected row
 * @param menu ctune_UI_SlideMenu_t object
 */
static void ctune_UI_SlideMenu_navKeyLeft( ctune_UI_SlideMenu_t * menu ) {
    ctune_UI_SlideMenu_navKeyRow( menu, -1, 0 );
}

/**
 * Change selected row to entry a page length away
 * @param menu ctune_UI_SlideMenu_t object
 */
static void ctune_UI_SlideMenu_navKeyPageUp( ctune_UI_SlideMenu_t * menu ) {
    ctune_UI_SlideMenu_navKeyRow( menu, 0, -( menu->canvas_property->rows ) );
}

/**
 * Change selected row to entry a page length away
 * @param menu ctune_UI_SlideMenu_t object
 */
static void ctune_UI_SlideMenu_navKeyPageDown( ctune_UI_SlideMenu_t * menu ) {
    ctune_UI_SlideMenu_navKeyRow( menu, 0, +( menu->canvas_property->rows ) );
}

/**
 * Change selection to the first item in the list
 * @param menu ctune_UI_SlideMenu_t object
 */
static void ctune_UI_SlideMenu_navKeyFirst( ctune_UI_SlideMenu_t * menu ) {
    menu->row.selected = 0;
    menu->redraw       = true;
}

/**
 * Change selection to the last item in the list
 * @param menu ctune_UI_SlideMenu_t object
 */
static void ctune_UI_SlideMenu_navKeyLast( ctune_UI_SlideMenu_t * menu ) {
    menu->row.selected = (int) ( Vector.empty( &menu->row.curr_menu->items ) ? 0 : Vector.size( &menu->row.curr_menu->items ) - 1 );
    menu->redraw       = true;
}

/**
 * Resets selected position to the first item in the root menu
 * @param menu ctune_UI_SlideMenu_t object
 */
static void ctune_UI_SlideMenu_reset( ctune_UI_SlideMenu_t * menu ) {
    menu->row.selected     = 0;
    menu->row.depth        = 0;
    menu->row.curr_menu    = &menu->root;
    menu->redraw           = true;
    menu->update_scrollbar = true;
}

/**
 * Resizes the menu
 * @param menu Pointer to ctune_UI_SlideMenu_t object
 */
static void ctune_UI_SlideMenu_resize( ctune_UI_SlideMenu_t * menu ) {
    CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_UI_SlideMenu_resize( %p )] Resize event called.", menu );

    if( menu == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_SlideMenu_resize( %p )] SlideMenu is NULL.", menu );
        return; //EARLY RETURN
    }

    ctune_UI_SlideMenu_drawCanvas( menu, true );

    if( menu->canvas_panel == NULL || menu->canvas_win == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_SlideMenu_show( %p )] NULL Panel/Win detected.", menu );
        return;
    }

    top_panel( menu->canvas_panel );
    wrefresh( menu->canvas_win );

    ctune_UI_ScrollBar.show( &menu->scrollbar );

    update_panels();
    doupdate();

    menu->redraw = false;
}

/**
 * Show updated window
 * @param menu ctune_UI_SlideMenu_t object
 * @return Success
 */
static bool ctune_UI_SlideMenu_show( ctune_UI_SlideMenu_t * menu ) {
    if( menu == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_SlideMenu_show( %p )] SlideMenu is NULL.", menu );
        return false; //EARLY RETURN
    }

    if( menu->redraw ) {
        ctune_UI_SlideMenu_drawCanvas( menu, false );
    }

    if( menu->canvas_panel == NULL || menu->canvas_win == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_SlideMenu_show( %p )] NULL Panel/Win detected.", menu );
        return false;
    }

    top_panel( menu->canvas_panel );
    wrefresh( menu->canvas_win );

    ctune_UI_ScrollBar.show( &menu->scrollbar );

    update_panels();
    doupdate();

    menu->redraw = false;

    return true;
}

/**
 * Hides the SlideMenu (no refresh done)
 * @param menu ctune_UI_SlideMenu_t object
 */
static void ctune_UI_SlideMenu_hide( ctune_UI_SlideMenu_t * menu ) {
    if( menu != NULL ) {
        if( menu->canvas_panel ) {
            bottom_panel( menu->canvas_panel );
            hide_panel( menu->canvas_panel );
        }

        ctune_UI_ScrollBar.hide( &menu->scrollbar );
    }
}

/**
 * De-allocates a slide menu's content
 * @param menu ctune_UI_SlideMenu_t object
 */
static void ctune_UI_SlideMenu_free( ctune_UI_SlideMenu_t * menu ) {
    if( menu ) {
        menu->redraw = true;

        if( menu->canvas_panel ) {
            del_panel( menu->canvas_panel );
            menu->canvas_panel = NULL;
        }

        if( menu->canvas_win ) {
            delwin( menu->canvas_win );
            menu->canvas_win = NULL;
        }

        menu->row.curr_menu        = &menu->root;
        menu->row.selected         = 0;
        menu->row.depth            = 0;
        menu->row.first_on_page    = 0;
        menu->row.last_on_page     = 0;
        Vector.clear_vector( &menu->root.items );

        menu->update_scrollbar     = true;
        ctune_UI_ScrollBar.free( &menu->scrollbar );

        CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_UI_SlideMenu_free( %p )] SlideMenu freed.", menu );
    }
}

/**
 * Namespace constructor
 */
const struct ctune_UI_Widget_SlideMenu_Namespace ctune_UI_SlideMenu = {
    .init                = &ctune_UI_SlideMenu_init,
    .createMenu          = &ctune_UI_SlideMenu_createMenu,
    .createMenuItem      = &ctune_UI_SlideMenu_createMenuItem,
    .setRedraw           = &ctune_UI_SlideMenu_setRedraw,
    .setFocus            = &ctune_UI_SlideMenu_setFocus,
    .navKeyEnter         = &ctune_UI_SlideMenu_navKeyEnter,
    .navKeyUp            = &ctune_UI_SlideMenu_navKeyUp,
    .navKeyDown          = &ctune_UI_SlideMenu_navKeyDown,
    .navKeyRight         = &ctune_UI_SlideMenu_navKeyRight,
    .navKeyLeft          = &ctune_UI_SlideMenu_navKeyLeft,
    .navKeyPageUp        = &ctune_UI_SlideMenu_navKeyPageUp,
    .navKeyPageDown      = &ctune_UI_SlideMenu_navKeyPageDown,
    .navKeyFirst         = &ctune_UI_SlideMenu_navKeyFirst,
    .navKeyLast          = &ctune_UI_SlideMenu_navKeyLast,
    .reset               = &ctune_UI_SlideMenu_reset,
    .resize              = &ctune_UI_SlideMenu_resize,
    .show                = &ctune_UI_SlideMenu_show,
    .hide                = &ctune_UI_SlideMenu_hide,
    .free                = &ctune_UI_SlideMenu_free,
};