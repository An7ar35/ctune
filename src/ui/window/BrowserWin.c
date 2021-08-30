#include "BrowserWin.h"

#include "../../logger/Logger.h"
#include "../../datastructure/String.h"
#include "../../enum/ListCategory.h"
#include "../../dto/CategoryItem.h"

//==================================[ PRIVATE METHODS/STRUCTS ]=====================================

// The callback methods for the menu items and their payload data types is a way to have dynamic
// creation/editing of the sub-menus since we don't actually know what these prior to a network call
// (one via a `ctune_Controller` callback). Each menu item is given a pointer to its relevant
// payload and control method (the callbacks further down).
//
// Admittedly, it's a bit convoluted but it keeps SlideMenu generic enough so that it can be reused
// to create menus with different content in some future development iteration. We shall see...!
//
// EAD, 10 March 2021

static bool ctune_UI_BrowserWin_categoryCtrlFunctionCb( ctune_UI_SlideMenu_Item_t * menu_item );
static bool ctune_UI_BrowserWin_subCategoryCtrlFunctionCb( ctune_UI_SlideMenu_Item_t * menu_item );

typedef struct {
    ctune_UI_BrowserWin_t * browser_win;
    ctune_ListCategory_e    category;
} CategoryPayload_t;

typedef struct {
    ctune_UI_BrowserWin_t * browser_win;
    ctune_ByCategory_e      by_category;
} SubCategoryPayload_t;

/**
 * [PRIVATE] Creates a CategoryPayload_t in the cache
 * @param win ctune_UI_BrowserWin_t object
 * @param cat ctune_ListCategory_e enum
 * @return Pointer to created payload or NULL if error
 */
static CategoryPayload_t * ctune_UI_BrowserWin_createCategoryPayload( ctune_UI_BrowserWin_t * win, ctune_ListCategory_e cat ) {
    CategoryPayload_t * payload = Vector.emplace_back( &win->cache.lvl1_menu_payloads );

    if( payload != NULL ) {
        payload->browser_win = win;
        payload->category    = cat;
    }

    return payload;
}

/**
 * [PRIVATE] Creates a SubCategoryPayload_t in the cache
 * @param win ctune_UI_BrowserWin_t object
 * @param cat Parent ctune_ListCategory_e enum
 * @return Pointer to created payload or NULL if error
 */
static SubCategoryPayload_t * ctune_UI_BrowserWin_createSubCategoryPayload( ctune_UI_BrowserWin_t * win, ctune_ListCategory_e cat ) {
    SubCategoryPayload_t * payload = Vector.emplace_back( &win->cache.lvl1_menu_payloads );

    if( payload != NULL ) {
        payload->browser_win = win;
        payload->by_category = win->cache.cat2bycat[cat];
    }

    return payload;
}

/**
 * [PRIVATE/CALLBACK] Callback method to fetch sub-category results and populate the right RSListWin pane
 * @param menu_item ctune_UI_SlideMenu_Item_t object
 * @return Error free success
 */
static bool ctune_UI_BrowserWin_subCategoryCtrlFunctionCb( ctune_UI_SlideMenu_Item_t * menu_item ) {
    bool                   error_state  = false;
    SubCategoryPayload_t * payload      = (SubCategoryPayload_t *) menu_item->data;
    Vector_t             * result_cache = &payload->browser_win->cache.rsi_results;

    if( !Vector.empty( &payload->browser_win->cache.rsi_results ) ) //clear old cache
        Vector.reinit( &payload->browser_win->cache.rsi_results );

    if( !payload->browser_win->cb.getStationsBy( payload->by_category, menu_item->text._raw, result_cache ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_BrowserWin_subCategoryCtrlFunctionCb( %p )] Fetching stations failed.",
                   menu_item
        );

        error_state = true;
        goto end;
    }

    if( !ctune_UI_RSListWin.loadResults( &payload->browser_win->right_pane, result_cache, NULL ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_BrowserWin_subCategoryCtrlFunctionCb( %p )] Loading station results into RSListWin failed.",
                   menu_item
        );

        error_state = true;
        goto end;
    }

    ctune_UI_BrowserWin.setFocus( payload->browser_win, FOCUS_PANE_RIGHT );
    ctune_UI_RSListWin.show( &payload->browser_win->right_pane );

    end:
        return !( error_state );
}

/**
 * [PRIVATE/CALLBACK] Callback method to fetch categories and populate a the sub menu of an item with the results
 * @param menu_item ctune_UI_SlideMenu_Item_t object
 * @return Error free success (if error then ctrl trigger will be kept 'live' regardless of settings)
 */
static bool ctune_UI_BrowserWin_categoryCtrlFunctionCb( ctune_UI_SlideMenu_Item_t * menu_item ) {
    bool                error_state = false;
    CategoryPayload_t * payload     = (CategoryPayload_t *) menu_item->data;
    Vector_t            results     = Vector.init( sizeof( ctune_CategoryItem_t ), ctune_CategoryItem.freeContent );

    if( !payload->browser_win->cb.getCatItems( payload->category, NULL, &results ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_BrowserWin_categoryCtrlFunctionCb( %p )] Failed to fetch sub-categories for menu ('%s').",
                   menu_item, menu_item->text._raw
        );

        ctune_err.set( CTUNE_ERR_ACTION_FETCH );
        error_state = true;
        goto end;
    }

    if( menu_item->sub_menu != NULL ) { //clear sub menu if present
        Vector.clear_vector( &menu_item->sub_menu->items );
        free( menu_item->sub_menu );
        menu_item->sub_menu = NULL;
    }

    if( ctune_UI_SlideMenu.createMenu( &menu_item->sub_menu, menu_item->parent_menu, menu_item->index ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_BrowserWin_categoryCtrlFunctionCb( %p )] Failed to create sub menu for item ('%s').",
                   menu_item, menu_item->text._raw
        );

        error_state = true;
        goto end;
    }

    if( !ctune_UI_SlideMenu.createMenuItem( menu_item->sub_menu, CTUNE_UI_SLIDEMENU_PARENT, menu_item->text._raw, NULL, NULL ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_BrowserWin_categoryCtrlFunctionCb( %p )] "
                   "Failed to create parent shortcut menu item '%s' for menu '%s'.",
                   menu_item, menu_item->text._raw, menu_item->text._raw
        );

        error_state = true;
    }

    //fill sub menu
    for( size_t i = 0; i < Vector.size( &results ); ++i ) {
        ctune_CategoryItem_t * cat             = Vector.at( &results, i );
        SubCategoryPayload_t * sub_cat_payload = ctune_UI_BrowserWin_createSubCategoryPayload( payload->browser_win, payload->category );

        if( sub_cat_payload == NULL ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_BrowserWin_categoryCtrlFunctionCb( %p )] "
                       "Failed to create payload for menu item '%s' in menu '%s'.",
                       menu_item, cat->name, menu_item->text._raw
            );

            error_state = true;

        } else if( !ctune_UI_SlideMenu.createMenuItem( menu_item->sub_menu,
                                                       CTUNE_UI_SLIDEMENU_LEAF,
                                                       cat->name,
                                                       sub_cat_payload,
                                                       ctune_UI_BrowserWin_subCategoryCtrlFunctionCb ) )
        {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_BrowserWin_categoryCtrlFunctionCb( %p )] "
                       "Failed to create menu item '%s' for menu '%s'.",
                       menu_item, cat->name, menu_item->text._raw
            );

            error_state = true;
        }
    }

    if( !error_state && payload->browser_win->cache_menu && menu_item->ctrl_trigger )
        menu_item->ctrl_trigger = false;

    end:
        Vector.clear_vector( &results );
        return !( error_state );
}

//=======================================[ PUBLIC METHODS ]=========================================
/**
 * Creates an initialised ctune_UI_BrowserWin_t
 * @param left_canvas     Canvas property for the left pane
 * @param right_canvas    Canvas property for the right pane
 * @param getDisplayText  Callback method to get UI text
 * @param getStations     Callback method to fetch more stations
 * @param getCatItems     Callback method to fetch station search category items
 * @param toggleFavourite Callback method to toggle a station's "favourite" status
 * @param getStationState Callback method to get a station's queued/favourite state
 * @return Initialised ctune_UI_BrowserWin_t object
 */
static ctune_UI_BrowserWin_t ctune_UI_BrowserWin_init( const WindowProperty_t * left_canvas,
                                                       const WindowProperty_t * right_canvas,
                                                       const char * (* getDisplayText)( ctune_UI_TextID_e ),
                                                       bool (* getStations)( ctune_RadioBrowserFilter_t *, Vector_t * ),
                                                       bool (* getCatItems)( const ctune_ListCategory_e, const ctune_RadioBrowserFilter_t *, Vector_t * ),
                                                       bool (* getStationsBy)( const ctune_ByCategory_e, const char *, Vector_t * ),
                                                       bool (* toggleFavourite)( ctune_RadioStationInfo_t *, ctune_StationSrc_e ),
                                                       unsigned (* getStationState)( const ctune_RadioStationInfo_t * ) )
{
    return (ctune_UI_BrowserWin_t) {
        .pane_focus         = FOCUS_PANE_LEFT,
        .left_pane          = ctune_UI_SlideMenu.init( left_canvas ),
        .right_pane         = ctune_UI_RSListWin.init( right_canvas, getDisplayText, getStations, toggleFavourite, getStationState ),
        .cache_menu         = false,
        .cache = {
            .cat2ui_text_enum = {
                [RADIOBROWSER_CATEGORY_COUNTRIES   ] = CTUNE_UI_TEXT_CAT_COUNTRIES,
                [RADIOBROWSER_CATEGORY_COUNTRYCODES] = CTUNE_UI_TEXT_CAT_COUNTRY_CODES,
                [RADIOBROWSER_CATEGORY_CODECS      ] = CTUNE_UI_TEXT_CAT_CODECS,
                [RADIOBROWSER_CATEGORY_STATES      ] = CTUNE_UI_TEXT_CAT_STATES,
                [RADIOBROWSER_CATEGORY_LANGUAGES   ] = CTUNE_UI_TEXT_CAT_LANGUAGES,
                [RADIOBROWSER_CATEGORY_TAGS        ] = CTUNE_UI_TEXT_CAT_TAGS
            },
            .cat2bycat = {
                [RADIOBROWSER_CATEGORY_COUNTRIES   ] = RADIOBROWSER_STATION_BY_COUNTRY_EXACT,
                [RADIOBROWSER_CATEGORY_COUNTRYCODES] = RADIOBROWSER_STATION_BY_COUNTRY_CODE_EXACT,
                [RADIOBROWSER_CATEGORY_CODECS      ] = RADIOBROWSER_STATION_BY_CODEC_EXACT,
                [RADIOBROWSER_CATEGORY_STATES      ] = RADIOBROWSER_STATION_BY_STATE_EXACT,
                [RADIOBROWSER_CATEGORY_LANGUAGES   ] = RADIOBROWSER_STATION_BY_LANGUAGE_EXACT,
                [RADIOBROWSER_CATEGORY_TAGS        ] = RADIOBROWSER_STATION_BY_TAG_EXACT
            },
            .lvl1_menu_payloads = Vector.init( sizeof( CategoryPayload_t ), NULL ),
            .lvl2_menu_payloads = Vector.init( sizeof( SubCategoryPayload_t ), NULL ),
            .rsi_results        = Vector.init( sizeof( ctune_RadioStationInfo_t ), ctune_RadioStationInfo.freeContent ),
        },
        .cb = {
            .getDisplayText = getDisplayText,
            .getStations    = getStations,
            .getCatItems    = getCatItems,
            .getStationsBy  = getStationsBy,
        },
    };
}

/**
 * Sets big row displaying on/off in right pane
 * @param win        ctune_UI_BrowserWin_t object
 * @param large_flag Flag to turn feature on/off
 */
static void ctune_UI_BrowserWin_setLargeRow( ctune_UI_BrowserWin_t * win, bool large_flag ) {
    ctune_UI_RSListWin.setLargeRow( &win->right_pane, large_flag );
}

/**
 * Sets the theming of 'favourite' stations on/off
 * @param win        RSListWin_t object
 * @param theme_flag Switch to turn on/off theming for rows of favourite stations
 */
void ctune_UI_BrowserWin_themeFavourites( ctune_UI_BrowserWin_t * win, bool theme_flag ) {
    ctune_UI_RSListWin.themeFavourites( &win->right_pane, theme_flag );
}

/**
 * Show the control row in the right pane
 * @param win           ctune_UI_BrowserWin_t object
 * @param show_ctrl_row Show control row flag
 */
static void ctune_UI_BrowserWin_showCtrlRow( ctune_UI_BrowserWin_t * win, bool show_ctrl_row ) {
    ctune_UI_RSListWin.showCtrlRow( &win->right_pane, show_ctrl_row );
}

/**
 * Creates root menu items
 * @param win ctune_UI_BrowserWin_t object
 * @return Success
 */
static bool ctune_UI_BrowserWin_populateRootMenu( ctune_UI_BrowserWin_t * win ) {
    bool error_state = false;

    ctune_UI_BrowserWin.setFocus( win, FOCUS_PANE_LEFT );

    for( int i = 0; i < RADIOBROWSER_CATEGORY_COUNT; ++i ) {
        CategoryPayload_t * payload   = ctune_UI_BrowserWin_createCategoryPayload( win, i );
        const char        * menu_text = win->cb.getDisplayText( win->cache.cat2ui_text_enum[ i ] );

        if( payload == NULL ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_BrowserWin_populateRootMenu( %p )] "
                       "Failed payload allocation.",
                       win
            );

            error_state = true;

        } else if( !ctune_UI_SlideMenu.createMenuItem( &win->left_pane.root,
                                                       CTUNE_UI_SLIDEMENU_MENU,
                                                       menu_text,
                                                       payload,
                                                       ctune_UI_BrowserWin_categoryCtrlFunctionCb ) )
        {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_BrowserWin_populateRootMenu( %p )] "
                       "Failed to create menu item %i: \"%s\"",
                       win, i, menu_text
            );

            error_state = true;
        }
    }

    return !( error_state );
}

/**
 * Sets the redraw flag on
 * @param win ctune_UI_SlideMenu_t object
 * @param redraw_all Flag to hard-set redraw on both panes
 */
static void ctune_UI_BrowserWin_setRedraw( ctune_UI_BrowserWin_t * win, bool redraw_all ) {
    if( win->pane_focus == FOCUS_PANE_LEFT || redraw_all ) {
        ctune_UI_SlideMenu.setRedraw( &win->left_pane );
        ctune_UI_RSListWin.setRedraw( &win->right_pane ); //in case curr station playing is on that pane and is changed from another tab (fav/search)

    } else if( win->pane_focus == FOCUS_PANE_RIGHT ) {
        ctune_UI_RSListWin.setRedraw( &win->right_pane );
    }
}

/**
 * Sets the pane focus
 * @param win  ctune_UI_SlideMenu_t object
 * @param pane PaneFocus_e value
 */
static void ctune_UI_BrowserWin_setFocus( ctune_UI_BrowserWin_t * win, ctune_UI_BrowserWin_PaneFocus_e pane ) {
    if( ( win->pane_focus = pane ) == FOCUS_PANE_RIGHT ) {
        ctune_UI_SlideMenu.setFocus( &win->left_pane, false );
        ctune_UI_RSListWin.setFocus( &win->right_pane, true );
    } else {
        ctune_UI_SlideMenu.setFocus( &win->left_pane, true );
        ctune_UI_RSListWin.setFocus( &win->right_pane, false );
    }

    ctune_UI_SlideMenu.setRedraw( &win->left_pane );
    ctune_UI_RSListWin.setRedraw( &win->right_pane );
}

/**
 * Swaps pane focus
 * @param win ctune_UI_SlideMenu_t object
 */
static void ctune_UI_BrowserWin_switchFocus( ctune_UI_BrowserWin_t * win ) {
    if( win->pane_focus == FOCUS_PANE_LEFT ) {
        ctune_UI_BrowserWin_setFocus( win, FOCUS_PANE_RIGHT );
    } else {
        ctune_UI_BrowserWin_setFocus( win, FOCUS_PANE_LEFT );
    }
}

/**
 * Checks if given pane is currently in focus
 * @param win ctune_UI_BrowserWin_t object
 * @param pane PaneFocus_e enum ID
 * @return Focussed state
 */
static bool ctune_UI_BrowserWin_isInFocus( ctune_UI_BrowserWin_t * win, ctune_UI_BrowserWin_PaneFocus_e pane ) {
    return win->pane_focus == pane;
}

/**
 * Show updated window
 * @param win ctune_UI_BrowserWin_t object
 * @return Success
 */
static bool ctune_UI_BrowserWin_show( ctune_UI_BrowserWin_t * win ) {
    if( win == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_BrowserWin_show( %p )] BrowserWin is NULL.", win );
        return false; //EARLY RETURN
    }

    return ctune_UI_SlideMenu.show( &win->left_pane )
        && ctune_UI_RSListWin.show( &win->right_pane );
}

/**
 * Resize the BrowserWindow
 * @param win Pointer to ctune_UI_BrowserWin_t object
 */
static void ctune_UI_BrowserWin_resize( void * win ) {
    CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_UI_BrowserWin_resize( %p )] Resize event called.", win );

    if( win == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_BrowserWin_resize( %p )] BrowserWin is NULL.", win );
        return; //EARLY RETURN
    }

    ctune_UI_SlideMenu.resize( &((ctune_UI_BrowserWin_t *) win)->left_pane );
    ctune_UI_RSListWin.resize( &((ctune_UI_BrowserWin_t *) win)->right_pane );
}

/**
 * Activate selection
 * @param win ctune_UI_BrowserWin_t object
 */
static void ctune_UI_BrowserWin_navKeyEnter( ctune_UI_BrowserWin_t * win ) {
    if( win->pane_focus == FOCUS_PANE_LEFT )
        ctune_UI_SlideMenu.navKeyEnter( &win->left_pane );
}

/**
 * Change selected row to previous entry
 * @param win ctune_UI_BrowserWin_t object
 */
static void ctune_UI_BrowserWin_navKeyUp( ctune_UI_BrowserWin_t * win ) {
    if( win->pane_focus == FOCUS_PANE_LEFT ) {
        ctune_UI_SlideMenu.navKeyUp( &win->left_pane );

    } else if( win->pane_focus == FOCUS_PANE_RIGHT ) {
        ctune_UI_RSListWin.selectUp( &win->right_pane );
    }
}

/**
 * Change selected row to next entry
 * @param win ctune_UI_BrowserWin_t object
 */
static void ctune_UI_BrowserWin_navKeyDown( ctune_UI_BrowserWin_t * win ) {
    if( win->pane_focus == FOCUS_PANE_LEFT ) {
        ctune_UI_SlideMenu.navKeyDown( &win->left_pane );

    } else if( win->pane_focus == FOCUS_PANE_RIGHT ) {
        ctune_UI_RSListWin.selectDown( &win->right_pane );
    }
}

/**
 * Change pane focus/trigger menu control function of row depending on circumstances
 * @param win ctune_UI_BrowserWin_t object
 */
static void ctune_UI_BrowserWin_navKeyRight( ctune_UI_BrowserWin_t * win ) {
    if( win->pane_focus == FOCUS_PANE_LEFT )
        ctune_UI_SlideMenu.navKeyRight( &win->left_pane );
}

/**
 * Change to the parent win/item of currently selected row
 * @param win ctune_UI_BrowserWin_t object
 */
static void ctune_UI_BrowserWin_navKeyLeft( ctune_UI_BrowserWin_t * win ) {
    if( win->pane_focus == FOCUS_PANE_LEFT ) {
        ctune_UI_SlideMenu.navKeyLeft( &win->left_pane );

    } else if( win->pane_focus == FOCUS_PANE_RIGHT ) {
        ctune_UI_BrowserWin_setFocus( win, FOCUS_PANE_LEFT );
    }
}

/**
 * Change selected row to entry a page length away
 * @param win ctune_UI_BrowserWin_t object
 */
static void ctune_UI_BrowserWin_navKeyPageUp( ctune_UI_BrowserWin_t * win ) {
    if( win->pane_focus == FOCUS_PANE_LEFT ) {
        ctune_UI_SlideMenu.navKeyPageUp( &win->left_pane );

    } else if( win->pane_focus == FOCUS_PANE_RIGHT ) {
        ctune_UI_RSListWin.selectPageUp( &win->right_pane );
    }
}

/**
 * Change selected row to entry a page length away
 * @param win ctune_UI_BrowserWin_t object
 */
static void ctune_UI_BrowserWin_navKeyPageDown( ctune_UI_BrowserWin_t * win ) {
    if( win->pane_focus == FOCUS_PANE_LEFT ) {
        ctune_UI_SlideMenu.navKeyPageDown( &win->left_pane );

    } else if( win->pane_focus == FOCUS_PANE_RIGHT ) {
        ctune_UI_RSListWin.selectPageDown( &win->right_pane );
    }
}

/**
 * Change selection to the first item in the list
 * @param win ctune_UI_BrowserWin_t object
 */
static void ctune_UI_BrowserWin_navKeyFirst( ctune_UI_BrowserWin_t * win ) {
    if( win->pane_focus == FOCUS_PANE_LEFT ) {
        ctune_UI_SlideMenu.navKeyFirst( &win->left_pane );

    } else if( win->pane_focus == FOCUS_PANE_RIGHT ) {
        ctune_UI_RSListWin.selectFirst( &win->right_pane );
    }
}

/**
 * Change selection to the last item in the list
 * @param win ctune_UI_BrowserWin_t object
 */
static void ctune_UI_BrowserWin_navKeyLast( ctune_UI_BrowserWin_t * win ) {
    if( win->pane_focus == FOCUS_PANE_LEFT ) {
        ctune_UI_SlideMenu.navKeyLast( &win->left_pane );

    } else if( win->pane_focus == FOCUS_PANE_RIGHT ) {
        ctune_UI_RSListWin.selectLast( &win->right_pane );
    }
}

/**
 * Toggles the "favourite" status of the currently selected radio station
 * @param win ctune_UI_BrowserWin_t object
 */
static void ctune_UI_BrowserWin_toggleFav( ctune_UI_BrowserWin_t * win ) {
    if( win->pane_focus == FOCUS_PANE_RIGHT ) {
        ctune_UI_RSListWin.toggleFav( &win->right_pane );
    }
}

/**
 * Gets a RSI pointer to the currently selected item in the right pane or if ctrl row then trigger callback
 * @param ctune_UI_BrowserWin_t object
 * @return RadioStationInfo_t object pointer or NULL if out of range of the collection/is ctrl row
 */
static const ctune_RadioStationInfo_t * ctune_UI_BrowserWin_getSelectedStation( ctune_UI_BrowserWin_t * win ) {
    if( win->pane_focus == FOCUS_PANE_RIGHT )
        return ctune_UI_RSListWin.getSelected( &win->right_pane );

    CTUNE_LOG( CTUNE_LOG_ERROR,
               "[ctune_UI_BrowserWin_getSelectedStation( %p )] Trying to get selected station when SlideMenu is in focus.",
               win
    );

    return NULL;
}

/**
 * Checks if the current row selected in the right pane is the ctrl row
 * @param win ctune_UI_BrowserWin_t object
 * @return is the Ctrl row
 */
static bool ctune_UI_BrowserWin_isCtrlRowSelected( ctune_UI_BrowserWin_t * win ) {
    return ctune_UI_RSListWin.isCtrlRowSelected( &win->right_pane );
}

/**
 * De-allocates all content of BrowserWin object
 * @param win ctune_UI_BrowserWin_t object
 */
static void ctune_UI_BrowserWin_free( ctune_UI_BrowserWin_t * win ) {
    if( win ) {
        ctune_UI_SlideMenu.free( &win->left_pane );
        ctune_UI_RSListWin.free( &win->right_pane );
        Vector.clear_vector( &win->cache.lvl1_menu_payloads );
        Vector.clear_vector( &win->cache.lvl2_menu_payloads );
        Vector.clear_vector( &win->cache.rsi_results );

        CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_BrowserWin_free( %p )] BrowserWin freed.", win );
    }
}

/**
 * Namespace constructor
 */
const struct ctune_UI_BrowserWin_Namespace ctune_UI_BrowserWin = {
    .init               = &ctune_UI_BrowserWin_init,
    .setLargeRow        = &ctune_UI_BrowserWin_setLargeRow,
    .themeFavourites    = &ctune_UI_BrowserWin_themeFavourites,
    .showCtrlRow        = &ctune_UI_BrowserWin_showCtrlRow,
    .populateRootMenu   = &ctune_UI_BrowserWin_populateRootMenu,
    .setRedraw          = &ctune_UI_BrowserWin_setRedraw,
    .setFocus           = &ctune_UI_BrowserWin_setFocus,
    .switchFocus        = &ctune_UI_BrowserWin_switchFocus,
    .isInFocus          = &ctune_UI_BrowserWin_isInFocus,
    .resize             = &ctune_UI_BrowserWin_resize,
    .show               = &ctune_UI_BrowserWin_show,
    .navKeyEnter        = &ctune_UI_BrowserWin_navKeyEnter,
    .navKeyUp           = &ctune_UI_BrowserWin_navKeyUp,
    .navKeyDown         = &ctune_UI_BrowserWin_navKeyDown,
    .navKeyRight        = &ctune_UI_BrowserWin_navKeyRight,
    .navKeyLeft         = &ctune_UI_BrowserWin_navKeyLeft,
    .navKeyPageUp       = &ctune_UI_BrowserWin_navKeyPageUp,
    .navKeyPageDown     = &ctune_UI_BrowserWin_navKeyPageDown,
    .navKeyHome         = &ctune_UI_BrowserWin_navKeyFirst,
    .navKeyEnd          = &ctune_UI_BrowserWin_navKeyLast,
    .toggleFav          = &ctune_UI_BrowserWin_toggleFav,
    .getSelectedStation = &ctune_UI_BrowserWin_getSelectedStation,
    .isCtrlRowSelected  = &ctune_UI_BrowserWin_isCtrlRowSelected,
    .free               = &ctune_UI_BrowserWin_free,
};