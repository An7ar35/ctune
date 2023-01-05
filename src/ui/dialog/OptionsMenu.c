#include "OptionsMenu.h"

#include "../definitions/KeyBinding.h"
#include "../definitions/Theme.h"
#include "../Resizer.h"

typedef struct {
    ctune_UI_PanelID_e * curr_panel;
    void (* action)( ctune_UI_PanelID_e tab );
} GenericCbPayload_t;

typedef struct {
    ctune_UI_PanelID_e * curr_panel;
    ctune_Flag_e         flag;
    bool (* action)( ctune_UI_PanelID_e tab, ctune_Flag_e action_flag );
} FlaggedCbPayload_t ;

typedef struct {
    ctune_UI_PanelID_e            * curr_panel;
    ctune_RadioStationInfo_SortBy_e attr;
    void (* action)( ctune_UI_PanelID_e tab, ctune_RadioStationInfo_SortBy_e attr );
} SortCbPayload_t;

/**
 * [PRIVATE] Helper method to create a GenericCbPayload_t
 * @param om            Pointer to ctune_UI_OptionsMenu_t object
 * @parma payload_store Vector of payloads to add new payload to
 * @param fn            Method callback
 * @return Pointer to created payload or NULL if error
 */
static GenericCbPayload_t * ctune_UI_Dialog_OptionsMenu_createGenericCbPayload( ctune_UI_OptionsMenu_t * om,
                                                                                Vector_t * payload_store,
                                                                                void (* fn)( ctune_UI_PanelID_e ) )
{
    GenericCbPayload_t * payload = Vector.emplace_back( payload_store );

    if( payload != NULL ) {
        payload->curr_panel = &om->cache.curr_panel_id;
        payload->action     = fn;

    } else {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_Dialog_OptionsMenu_createGenericCbPayload( %p, %p, %p )] "
                   "Failed to create payload.",
                   om, payload_store, fn );
    }

    return payload;
}

/**
 * [PRIVATE] Helper method to create a FlaggedCbPayload_t
 * @param om            Pointer to ctune_UI_OptionsMenu_t object
 * @param payload_store Vector of payloads to add new payload to
 * @param fn            Method callback
 * @param flag          Flag to pass to the callback
 * @return Pointer to created payload or NULL if error
 */
static FlaggedCbPayload_t * ctune_UI_Dialog_OptionsMenu_createFlaggedCbPayload( ctune_UI_OptionsMenu_t * om,
                                                                                Vector_t * payload_store,
                                                                                bool (* fn)( ctune_UI_PanelID_e, ctune_Flag_e ),
                                                                                ctune_Flag_e flag )
{
    FlaggedCbPayload_t * payload = Vector.emplace_back( payload_store );

    if( payload != NULL ) {
        payload->curr_panel = &om->cache.curr_panel_id;
        payload->flag       = flag;
        payload->action     = fn;

    } else {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_Dialog_OptionsMenu_createFlaggedCbPayload( %p, %p, %p, %i )] "
                   "Failed to create payload.",
                   om, payload_store, fn, flag );
    }

    return payload;
}

/**
 * [PRIVATE] Helper method to create a SortCbPayload_t
 * @param om            Pointer to ctune_UI_OptionsMenu_t object
 * @param payload_store Vector of payloads to add new payload to
 * @param fn            Method callback
 * @param attr          Station attribute to sort by
 * @return Pointer to created payload or NULL if error
 */
static SortCbPayload_t * ctune_UI_Dialog_OptionsMenu_createSortCbPayload( ctune_UI_OptionsMenu_t * om,
                                                                          Vector_t * payload_store,
                                                                          void (* fn)( ctune_UI_PanelID_e, ctune_RadioStationInfo_SortBy_e ),
                                                                          ctune_RadioStationInfo_SortBy_e attr )
{
    SortCbPayload_t * payload = Vector.emplace_back( payload_store );

    if( payload != NULL ) {
        payload->curr_panel = &om->cache.curr_panel_id;
        payload->attr       = attr;
        payload->action     = fn;

    } else {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_Dialog_OptionsMenu_createSortCbPayload( %p, %p, %p, %i )] "
                   "Failed to create payload.",
                   om, payload_store, fn, attr
        );
    }

    return payload;
}

/**
 * [PRIVATE] Generic menu control method
 * @param menu_item Current menu item
 * @return Success
 */
static bool genericCtrlMenuFunctionCb( ctune_UI_SlideMenu_Item_t * menu_item ) {
    GenericCbPayload_t * payload = (GenericCbPayload_t *) menu_item->data;

    if( payload == NULL || payload->curr_panel == NULL )
        return false; //EARLY RETURN

    payload->action( *payload->curr_panel );
    return true;
}

/**
 * [PRIVATE] Flagged sub-menu control method
 * @param menu_item Current menu item
 * @return Success
 */
static bool sortByCtrlFunctionCb( ctune_UI_SlideMenu_Item_t * menu_item ) {
    SortCbPayload_t * payload = (SortCbPayload_t *) menu_item->data;

    if( payload == NULL || payload->curr_panel == NULL )
        return false; //EARLY RETURN

    payload->action( *payload->curr_panel, payload->attr );
    return true;
}

/**
 * [PRIVATE] Sort sub-menu control method
 * @param menu_item Current menu item
 * @return Success
 */
static bool flaggedCtrlFunctionCb( ctune_UI_SlideMenu_Item_t * menu_item ) {
    FlaggedCbPayload_t * payload = (FlaggedCbPayload_t *) menu_item->data;

    if( payload == NULL || payload->curr_panel == NULL )
        return false; //EARLY RETURN

    payload->action( *payload->curr_panel, payload->flag );
    return true;
}


/**
 * [PRIVATE] Calculate the display properties for the menu items in teh context of the menu
 * @param om Pointer to ctune_UI_OptionsMenu_t object
 * @param array Menu item text array
 * @param array_len Menu item text array length
 * @return Success
 */
static bool ctune_UI_Dialog_OptionsMenu_calculateMenuItemProperties( ctune_UI_OptionsMenu_t * om, const char **array, int array_len ) {
    WindowProperty_t * content       = &om->cache.slide_menu_property;
    size_t             max_txt_width = content->cols;

    for( int i = 0; i < array_len; ++i ) {
        max_txt_width = ctune_max_ul( max_txt_width, strlen( array[ i ] ) );
    }

    if( !ctune_utoi( max_txt_width, &content->cols ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_UI_Dialog_OptionsMenu_calculateMenuItemProperties( %p, %p, %d )] Failed to cast to integer (%lu).",
                   om, array, array_len, max_txt_width
        );

        return false; //EARLY RETURN
    }

    return true;
}

/**
 * [PRIVATE] Calculate the display properties for the menu in the context of the parent display
 * @param om Pointer to ctune_UI_OptionsMenu_t object
 * @return Success
 */
bool ctune_UI_Dialog_OptionsMenu_calculateMenuDisplayProperties( ctune_UI_OptionsMenu_t * om ) {
    const WindowMargin_t * margins = &om->margins;
    WindowProperty_t     * borders = &om->cache.border_win_property;
    WindowProperty_t     * content = &om->cache.slide_menu_property;

    content->rows = 6; //optimum height for root menu

    size_t max_text_width = content->cols + 5; //to have space for the scrolling bar and '<', '>' nav indicators

    if( !ctune_utoi( max_text_width, &content->cols ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_UI_Dialog_OptionsMenu_calculateSlideMenuProperties( %p )] Failed to cast to integer (%lu).",
                   om, max_text_width
        );

        return false; //EARLY RETURN
    }

    const int border_t = 1;
    const int border_b = 1;
    const int border_l = 1;
    const int border_r = 1;

    const int max_content_width  = ( om->parent->cols - margins->left - margins->right  - border_l - border_r );
    const int max_content_height = ( om->parent->rows - margins->top  - margins->bottom - border_t - border_b );

    if( content->cols > max_content_width ) {
        CTUNE_LOG( CTUNE_LOG_WARNING,
                   "[ctune_UI_Dialog_OptionsMenu_calculateDisplayProperties( %p )] "
                   "SlideMenu content too wide - clipping will occur.",
                   om
        );
    }

    if( content->rows > max_content_height ) {
        CTUNE_LOG( CTUNE_LOG_DEBUG,
                   "[ctune_UI_Dialog_OptionsMenu_calculateDisplayProperties( %p )] "
                   "SlideMenu content too tall - scrolling enabled.",
                   om
        );
    }

    content->rows  = ( content->rows > max_content_height ? max_content_height : content->rows );
    content->cols  = ( content->cols > max_content_width  ? max_content_width  : content->cols );

    borders->rows  = ( content->rows + margins->top  + margins->bottom + border_t + border_b );
    borders->cols  = ( content->cols + margins->left + margins->right  + border_l + border_r );

    content->pos_y = ( content->rows > max_content_height ? 0 : ( ( om->parent->rows - borders->rows ) / 2 ) );
    content->pos_x = ( content->cols > max_content_width  ? 0 : ( ( om->parent->cols - borders->cols ) / 2 ) );

    borders->pos_y = ( content->pos_y - margins->top  - border_t );
    borders->pos_x = ( content->pos_x - margins->left - border_l );

    return true;
}

/**
 * [PRIVATE] Populates the "configuration" sub-menu
 * @param om Pointer to ctune_UI_OptionsMenu_t object
 * @param root Pointer to SlideMenu item from which to spawn the menu from
 * @return Success
 */
static bool ctune_UI_Dialog_OptionsMenu_populateConfigMenu( ctune_UI_OptionsMenu_t * om, ctune_UI_SlideMenu_Item_t * root ) {
    bool error_state = false;

    if( ctune_UI_SlideMenu.createMenu( &root->sub_menu, root->parent_menu, root->index ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_Dialog_OptionsMenu_populateConfigMenu( %p, %p )] "
                   "Failed to create sub menu for item ('%s').",
                   om, root, root->text._raw
        );

        error_state = true;
        goto end;
    }

    size_t max_text_width = om->cache.slide_menu_property.cols;

    { //"Go back" entry
        const char                * text      = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_CONFIGURATION );
        ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_PARENT, text, NULL, NULL );

        if( menu_item ) {
            max_text_width = ctune_max_ul( max_text_width, strlen( text ) );

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Dialog_OptionsMenu_populateConfigMenu( %p, %p )] Failed creation of menu item '%s'.",
                       om, root, text
            );

            error_state = true;
            goto end;
        }
    }

    if( om->cb.listRowSizeLarge != NULL ) { //"Set row size" entry
        const bool         curr_state = om->cb.listRowSizeLarge( om->cache.curr_panel_id, FLAG_GET_VALUE );
        const ctune_Flag_e action     = ( curr_state ? FLAG_SET_OFF : FLAG_SET_ON );
        const char *       text       = om->cb.getDisplayText( ( curr_state ? CTUNE_UI_TEXT_ROWSIZE_1X : CTUNE_UI_TEXT_ROWSIZE_2X ) );

        FlaggedCbPayload_t        * payload   = ctune_UI_Dialog_OptionsMenu_createFlaggedCbPayload( om, &om->cache.flagged_payloads, om->cb.listRowSizeLarge, action );
        ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, text, payload, flaggedCtrlFunctionCb );

        if( payload && menu_item ) {
            max_text_width = ctune_max_ul( max_text_width, strlen( text ) );

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Dialog_OptionsMenu_populateFavThemingMenu( %p, %p )] Failed creation of menu item '%s'.",
                       om, root, text
            );
            error_state = true;
        }
    }

    if( om->cb.favouriteTabTheming != NULL ) { //"Toggle theming" entry
        const bool         curr_state = om->cb.favouriteTabTheming( om->cache.curr_panel_id, FLAG_GET_VALUE );
        const ctune_Flag_e action     = ( curr_state ? FLAG_SET_OFF : FLAG_SET_ON );
        const char *       text       = om->cb.getDisplayText( ( curr_state ? CTUNE_UI_TEXT_FAV_THEMING_OFF : CTUNE_UI_TEXT_FAV_THEMING_ON ) );

        FlaggedCbPayload_t        * payload   = ctune_UI_Dialog_OptionsMenu_createFlaggedCbPayload( om, &om->cache.flagged_payloads, om->cb.favouriteTabTheming, action );
        ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, text, payload, flaggedCtrlFunctionCb );

        if( payload && menu_item ) {
            max_text_width = ctune_max_ul( max_text_width, strlen( text ) );

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Dialog_OptionsMenu_populateFavThemingMenu( %p, %p )] Failed creation of menu item '%s'.",
                       om, root, text
            );
            error_state = true;
        }
    }

    if( !ctune_utoi( max_text_width, &om->cache.slide_menu_property.cols ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_UI_Dialog_OptionsMenu_populateConfigMenu( %p, %p )] Failed to cast to integer (%lu).",
                   om, root, max_text_width
        );
        error_state = true;
    }

    end:
        return !( error_state );
}

/**
 * [PRIVATE] Populates the "Sort stations" sub-menu
 * @param om Pointer to ctune_UI_OptionsMenu_t object
 * @param root Pointer to SlideMenu item from which to spawn the menu from
 * @return Success
 */
static bool ctune_UI_Dialog_OptionsMenu_populateSortMenu( ctune_UI_OptionsMenu_t * om, ctune_UI_SlideMenu_Item_t * root ) {
    bool error_state = false;

    enum SortMenuItems {
        SORT_ITEM_GO_BACK = 0, SORT_ITEM_NAME,    SORT_ITEM_NAME_R,    SORT_ITEM_TAGS,
        SORT_ITEM_TAGS_R,      SORT_ITEM_COUNTRY, SORT_ITEM_COUNTRY_R, SORT_ITEM_CC,
        SORT_ITEM_CC_R,        SORT_ITEM_STATE,   SORT_ITEM_STATE_R,   SORT_ITEM_LANGUAGE,
        SORT_ITEM_LANGUAGE_R,  SORT_ITEM_CODEC,   SORT_ITEM_CODEC_R,   SORT_ITEM_BITRATE,
        SORT_ITEM_BITRATE_R,   SORT_ITEM_SOURCE,  SORT_ITEM_SOURCE_R,  SORT_ITEM_COUNT
    };

    const char * menu_text[SORT_ITEM_COUNT] = {
        [SORT_ITEM_GO_BACK   ] = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_SORT_STATIONS ),
        [SORT_ITEM_NAME      ] = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_SORT_STATIONS_NAME ),
        [SORT_ITEM_NAME_R    ] = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_SORT_STATIONS_NAME_R ),
        [SORT_ITEM_TAGS      ] = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_SORT_STATIONS_TAGS ),
        [SORT_ITEM_TAGS_R    ] = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_SORT_STATIONS_TAGS_R ),
        [SORT_ITEM_COUNTRY   ] = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_SORT_STATIONS_COUNTRY ),
        [SORT_ITEM_COUNTRY_R ] = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_SORT_STATIONS_COUNTRY_R ),
        [SORT_ITEM_CC        ] = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_SORT_STATIONS_CC ),
        [SORT_ITEM_CC_R      ] = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_SORT_STATIONS_CC_R ),
        [SORT_ITEM_STATE     ] = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_SORT_STATIONS_STATE ),
        [SORT_ITEM_STATE_R   ] = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_SORT_STATIONS_STATE_R ),
        [SORT_ITEM_LANGUAGE  ] = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_SORT_STATIONS_LANGUAGE ),
        [SORT_ITEM_LANGUAGE_R] = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_SORT_STATIONS_LANGUAGE_R ),
        [SORT_ITEM_CODEC     ] = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_SORT_STATIONS_CODEC ),
        [SORT_ITEM_CODEC_R   ] = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_SORT_STATIONS_CODEC_R ),
        [SORT_ITEM_BITRATE   ] = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_SORT_STATIONS_BITRATE ),
        [SORT_ITEM_BITRATE_R ] = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_SORT_STATIONS_BITRATE_R ),
        [SORT_ITEM_SOURCE    ] = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_SORT_STATIONS_SOURCE ),
        [SORT_ITEM_SOURCE_R  ] = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_SORT_STATIONS_SOURCE_R ),
    };

    if( ctune_UI_SlideMenu.createMenu( &root->sub_menu, root->parent_menu, root->index ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_Dialog_OptionsMenu_populateSortMenu( %p, %p )] "
                   "Failed to create sub menu for item ('%s').",
                   om, root, root->text._raw
        );

        error_state = true;
        goto end;
    }

    ctune_UI_SlideMenu_Item_t * menu_items[SORT_ITEM_COUNT];
    SortCbPayload_t           * payloads  [SORT_ITEM_COUNT];
    Vector_t                  * payload_store = &om->cache.sorting_payloads; //shortcut

    payloads[SORT_ITEM_GO_BACK   ] = NULL;
    payloads[SORT_ITEM_NAME      ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_NAME );
    payloads[SORT_ITEM_NAME_R    ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_NAME_DESC );
    payloads[SORT_ITEM_TAGS      ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_TAGS );
    payloads[SORT_ITEM_TAGS_R    ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_TAGS_DESC );
    payloads[SORT_ITEM_COUNTRY   ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_COUNTRY );
    payloads[SORT_ITEM_COUNTRY_R ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_COUNTRY_DESC );
    payloads[SORT_ITEM_CC        ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_COUNTRYCODE );
    payloads[SORT_ITEM_CC_R      ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_COUNTRYCODE_DESC );
    payloads[SORT_ITEM_STATE     ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_STATE );
    payloads[SORT_ITEM_STATE_R   ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_STATE_DESC );
    payloads[SORT_ITEM_LANGUAGE  ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_LANGUAGE );
    payloads[SORT_ITEM_LANGUAGE_R] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_LANGUAGE_DESC );
    payloads[SORT_ITEM_CODEC     ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_CODEC );
    payloads[SORT_ITEM_CODEC_R   ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_CODEC_DESC );
    payloads[SORT_ITEM_BITRATE   ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_BITRATE );
    payloads[SORT_ITEM_BITRATE_R ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_BITRATE_DESC );
    payloads[SORT_ITEM_SOURCE    ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_SOURCE );
    payloads[SORT_ITEM_SOURCE_R  ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_SOURCE_DESC );

    menu_items[SORT_ITEM_GO_BACK   ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_PARENT, menu_text[SORT_ITEM_GO_BACK], NULL, NULL );
    menu_items[SORT_ITEM_NAME      ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_NAME], payloads[SORT_ITEM_NAME], sortByCtrlFunctionCb );
    menu_items[SORT_ITEM_NAME_R    ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_NAME_R], payloads[SORT_ITEM_NAME_R], sortByCtrlFunctionCb );
    menu_items[SORT_ITEM_TAGS      ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_TAGS], payloads[SORT_ITEM_TAGS], sortByCtrlFunctionCb );
    menu_items[SORT_ITEM_TAGS_R    ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_TAGS_R], payloads[SORT_ITEM_TAGS_R], sortByCtrlFunctionCb );
    menu_items[SORT_ITEM_COUNTRY   ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_COUNTRY], payloads[SORT_ITEM_COUNTRY], sortByCtrlFunctionCb );
    menu_items[SORT_ITEM_COUNTRY_R ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_COUNTRY_R], payloads[SORT_ITEM_COUNTRY_R], sortByCtrlFunctionCb );
    menu_items[SORT_ITEM_CC        ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_CC], payloads[SORT_ITEM_CC], sortByCtrlFunctionCb );
    menu_items[SORT_ITEM_CC_R      ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_CC_R], payloads[SORT_ITEM_CC_R], sortByCtrlFunctionCb );
    menu_items[SORT_ITEM_STATE     ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_STATE], payloads[SORT_ITEM_STATE], sortByCtrlFunctionCb );
    menu_items[SORT_ITEM_STATE_R   ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_STATE_R], payloads[SORT_ITEM_STATE_R], sortByCtrlFunctionCb );
    menu_items[SORT_ITEM_LANGUAGE  ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_LANGUAGE], payloads[SORT_ITEM_LANGUAGE], sortByCtrlFunctionCb );
    menu_items[SORT_ITEM_LANGUAGE_R] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_LANGUAGE_R], payloads[SORT_ITEM_LANGUAGE_R], sortByCtrlFunctionCb );
    menu_items[SORT_ITEM_CODEC     ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_CODEC], payloads[SORT_ITEM_CODEC], sortByCtrlFunctionCb );
    menu_items[SORT_ITEM_CODEC_R   ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_CODEC_R], payloads[SORT_ITEM_CODEC_R], sortByCtrlFunctionCb );
    menu_items[SORT_ITEM_BITRATE   ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_BITRATE], payloads[SORT_ITEM_BITRATE], sortByCtrlFunctionCb );
    menu_items[SORT_ITEM_BITRATE_R ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_BITRATE_R], payloads[SORT_ITEM_BITRATE_R], sortByCtrlFunctionCb );
    menu_items[SORT_ITEM_SOURCE    ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_SOURCE], payloads[SORT_ITEM_SOURCE], sortByCtrlFunctionCb );
    menu_items[SORT_ITEM_SOURCE_R  ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_SOURCE_R], payloads[SORT_ITEM_SOURCE_R], sortByCtrlFunctionCb );

    for( int i = 0; i < SORT_ITEM_COUNT; ++i ) {
        if( payloads[i] == NULL && i != SORT_ITEM_GO_BACK ) {
            error_state = true;

            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Dialog_OptionsMenu_populateSortMenu( %p, %p )] "
                       "Failed creation payload for menu item #%i ('%s').",
                       om, root, i, menu_text[i]
            );
        }

        if( menu_items[i] == NULL ) {
            error_state = true;

            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Dialog_OptionsMenu_populateSortMenu( %p, %p )] "
                       "Failed creation of menu item #%i ('%s').",
                       om, root, i, menu_text[i]
            );
        }
    }

    if( !ctune_UI_Dialog_OptionsMenu_calculateMenuItemProperties( om, menu_text, SORT_ITEM_COUNT ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_UI_Dialog_OptionsMenu_populateSortMenu( %p, %p )] "
                   "Failed item properties calculation.",
                   om, root
        );

        error_state = true;
        goto end;
    }

    end:
        return !( error_state );
}

/**
 * [PRIVATE] Creates root menu items
 * @param om Pointer to ctune_UI_OptionsMenu_t object
 * @return Success
 */
static int ctune_UI_Dialog_OptionsMenu_populateRootMenu( ctune_UI_OptionsMenu_t * om ) {
    bool   error_state    = false;
    size_t max_text_width = om->cache.slide_menu_property.cols;

    if( om->cb.sortStationList != NULL ) { //Sorting menu
        const char                * text      = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_SORT_STATIONS );
        ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( &om->menu.root, CTUNE_UI_SLIDEMENU_MENU, text, NULL, NULL );

        if( menu_item && ctune_UI_Dialog_OptionsMenu_populateSortMenu( om, menu_item ) ) {
            max_text_width = ctune_max_ul( max_text_width, strlen( text ) );

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Dialog_OptionsMenu_populateRootMenu( %p )] Failed creation of menu item '%s'.",
                       om, text
            );
            error_state = true;
        }
    }

    if( om->cb.addNewStation != NULL ) { //Add new Station
        const char                * text      = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_NEW_STATION );
        GenericCbPayload_t        * payload   = ctune_UI_Dialog_OptionsMenu_createGenericCbPayload( om, &om->cache.generic_payloads, om->cb.addNewStation );
        ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( &om->menu.root, CTUNE_UI_SLIDEMENU_LEAF, text, payload, genericCtrlMenuFunctionCb );

        if( payload && menu_item ) {
            max_text_width = ctune_max_ul( max_text_width, strlen( text ) );

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Dialog_OptionsMenu_populateRootMenu( %p )] Failed creation of menu item '%s'.",
                       om, text
            );
            error_state = true;
        }
    }

    if( om->cb.editStation != NULL ) { //Edit selected station
        const char                * text      = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_EDIT_STATION );
        GenericCbPayload_t        * payload   = ctune_UI_Dialog_OptionsMenu_createGenericCbPayload( om, &om->cache.generic_payloads, om->cb.editStation );
        ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( &om->menu.root, CTUNE_UI_SLIDEMENU_LEAF, text, payload, genericCtrlMenuFunctionCb );

        if( payload && menu_item ) {
            max_text_width = ctune_max_ul( max_text_width, strlen( text ) );

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Dialog_OptionsMenu_populateRootMenu( %p )] Failed creation of menu item '%s'.",
                       om, text
            );
            error_state = true;
        }
    }

    if( om->cb.toggleFavourite != NULL ) { //Remove selected favourite station
        const char                * text      = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_TOGGLE_FAV );
        GenericCbPayload_t        * payload   = ctune_UI_Dialog_OptionsMenu_createGenericCbPayload( om, &om->cache.generic_payloads, om->cb.toggleFavourite );
        ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( &om->menu.root, CTUNE_UI_SLIDEMENU_LEAF, text, payload, genericCtrlMenuFunctionCb );

        if( payload && menu_item ) {
            max_text_width = ctune_max_ul( max_text_width, strlen( text ) );

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Dialog_OptionsMenu_populateRootMenu( %p )] Failed creation of menu item '%s'.",
                       om, text
            );
            error_state = true;
        }
    }

    if( om->cb.syncUpstream != NULL ) { //Synchronize selected station with upstream
        const char                * text      = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_SYNC_UPSTREAM );
        GenericCbPayload_t        * payload   = ctune_UI_Dialog_OptionsMenu_createGenericCbPayload( om, &om->cache.generic_payloads, om->cb.syncUpstream );
        ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( &om->menu.root, CTUNE_UI_SLIDEMENU_LEAF, text, payload, genericCtrlMenuFunctionCb );

        if( payload && menu_item ) {
            max_text_width = ctune_max_ul( max_text_width, strlen( text ) );

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Dialog_OptionsMenu_populateRootMenu( %p )] Failed creation of menu item '%s'.",
                       om, text
            );
            error_state = true;
        }
    }

    if( om->cb.favouriteTabTheming != NULL || om->cb.listRowSizeLarge != NULL ) { //Configuration menu
        const char                * text      = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_CONFIGURATION );
        ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( &om->menu.root, CTUNE_UI_SLIDEMENU_MENU, text, NULL, NULL );

        if( menu_item && ctune_UI_Dialog_OptionsMenu_populateConfigMenu( om, menu_item ) ) {
            //max_text_width must be reloaded since it might have been changed in the "populate" call
            max_text_width = ctune_max_ul( om->cache.slide_menu_property.cols, strlen( text ) );

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Dialog_OptionsMenu_populateRootMenu( %p )] Failed creation of menu item '%s'.",
                       om, text
            );
            error_state = true;
        }
    }

    if( !ctune_utoi( max_text_width, &om->cache.slide_menu_property.cols ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_UI_Dialog_OptionsMenu_populateRootMenu( %p )] Failed to cast to integer (%lu).",
                   om, max_text_width
        );
        error_state = true;
    }

    return !( error_state );
}

/**
 * Creates an uninitialised ctune_UI_OptionsMenu_t
 * @param parent          Canvas property for parent window
 * @param parent_id       Parent panel ID
 * @param getDisplayText  Callback method to get UI text
 * @return Initialised ctune_UI_OptionsMenu_t object
 */
static ctune_UI_OptionsMenu_t ctune_UI_Dialog_OptionsMenu_create( const WindowProperty_t * parent,
                                                                  ctune_UI_PanelID_e parent_id,
                                                                  const char * (* getDisplayText)( ctune_UI_TextID_e ) )
{
    return (ctune_UI_OptionsMenu_t) {
        .initialised = false,
        .parent      = parent,
        .margins     = { 0, 0, 0, 0 },
        .border_win  = ctune_UI_BorderWin.create( ctune_UI_Theme.color( CTUNE_UI_ITEM_DIALOG_WIN ) ),
        .cache = {
            .input_captured      = false,
            .slide_menu_property = { 0, 0, 0, 0 },
            .border_win_property = { 0, 0, 0, 0 },
            .curr_panel_id       = parent_id,
            .generic_payloads    = Vector.init( sizeof( GenericCbPayload_t ), NULL ),
            .flagged_payloads    = Vector.init( sizeof( FlaggedCbPayload_t ), NULL ),
            .sorting_payloads    = Vector.init( sizeof( SortCbPayload_t ), NULL ),
        },
        .cb = {
            .getDisplayText      = getDisplayText,
            .sortStationList     = NULL,
            .addNewStation       = NULL,
            .editStation         = NULL,
            .toggleFavourite     = NULL,
            .syncUpstream        = NULL,
            .favouriteTabTheming = NULL,
            .listRowSizeLarge = NULL,
        }
    };
}

/**
 * Initialised a ctune_UI_OptionsMenu_t object
 * @param om Pointer to ctune_UI_OptionsMenu_t object
 * @return Success
 */
static bool ctune_UI_Dialog_OptionsMenu_init( ctune_UI_OptionsMenu_t * om ) {
    if( om == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_Dialog_OptionsMenu_init( %p )] Argument is NULL.", om );
        return false; //EARLY RETURN
    }

    if( om->initialised ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_Dialog_OptionsMenu_init( %p )] Already initialised.", om );
        return false; //EARLY RETURN
    }

    bool error_state = false;

    om->menu = ctune_UI_SlideMenu.create();

    if( !ctune_UI_Dialog_OptionsMenu_populateRootMenu( om ) ) { //starts the population chain for all relevant entries and sub menus
        CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_UI_Dialog_OptionsMenu_init( %p )] Failed populate root menu.", om );
        error_state = true;
        goto end;
    }

    if( !ctune_UI_Dialog_OptionsMenu_calculateMenuDisplayProperties( om ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_UI_Dialog_OptionsMenu_init( %p )] Failed display properties calculation.", om );
        error_state = true;
        goto end;
    }

    ctune_UI_SlideMenu.setCanvasProperties( &om->menu, &om->cache.slide_menu_property );

    if( !ctune_UI_BorderWin.init( &om->border_win, &om->cache.border_win_property, om->cb.getDisplayText( CTUNE_UI_TEXT_WIN_TITLE_OPTIONMENU ) ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_Dialog_OptionsMenu_init( %p )] Failed to create border window.", om );
        error_state = true;
        goto end;
    }

    end:
        return !( error_state );
}

/**
 * Sets the redraw flag on
 * @param om Pointer to ctune_UI_OptionsMenu_t object
 */
static void ctune_UI_Dialog_OptionsMenu_setRedraw( ctune_UI_OptionsMenu_t * om ) {
    ctune_UI_SlideMenu.setRedraw( &om->menu );
}

/**
 * Show updated window
 * @param om Pointer to ctune_UI_OptionsMenu_t object
 * @return Success
 */
static bool ctune_UI_Dialog_OptionsMenu_show( ctune_UI_OptionsMenu_t * om ) {
    if( om == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_Dialog_OptionsMenu_show( %p )] OptionsMenu is NULL.", om );
        return false; //EARLY RETURN
    }

    bool success = ctune_UI_BorderWin.show( &om->border_win )
                && ctune_UI_SlideMenu.show( &om->menu );

    if( success )
        ctune_UI_Resizer.push( ctune_UI_OptionsMenu.resize, om );

    return success;
}

/**
 * Redraws the dialog
 * @param om Pointer to ctune_UI_OptionsMenu_t object
 */
static void ctune_UI_Dialog_OptionsMenu_resize( void * om ) {
    CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_UI_Dialog_OptionsMenu_resize( %p )] Resize event called.", om );

    ctune_UI_OptionsMenu_t * opt_menu = om;

    { //recalculates relative position of menu/border win to the terminal screen
        WindowProperty_t     * content     = &opt_menu->cache.slide_menu_property;
        WindowProperty_t     * borders     = &opt_menu->cache.border_win_property;
        const WindowMargin_t * margins     = &opt_menu->margins;

        const int border_t = 1;
        const int border_l = 1;

        content->pos_y = ( opt_menu->parent->rows - borders->rows ) / 2;
        content->pos_x = ( opt_menu->parent->cols - borders->cols ) / 2;
        borders->pos_y = ( content->pos_y - margins->top  - border_t );
        borders->pos_x = ( content->pos_x - margins->left - border_l );
    }

    if( !ctune_UI_BorderWin.init( &opt_menu->border_win, &opt_menu->cache.border_win_property, opt_menu->cb.getDisplayText( CTUNE_UI_TEXT_WIN_TITLE_OPTIONMENU ) ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_Dialog_OptionsMenu_resize( %p )] Failed to create border window.", om );
        return;
    }

    ctune_UI_SlideMenu.resize( &opt_menu->menu );
    keypad( opt_menu->menu.canvas_win, FALSE );
}

/**
 * Pass keyboard input to the form
 * @param om Pointer to ctune_UI_OptionsMenu_t object
 */
static void ctune_UI_Dialog_OptionsMenu_captureInput( ctune_UI_OptionsMenu_t * om ) {
    resized: //without this goto tag, the input capture exits since the key-input window gets destroyed and recreated
        keypad( om->menu.canvas_win, TRUE );
        curs_set( 0 );

        om->cache.input_captured = true;
        volatile int character;

        while( om->cache.input_captured ) {
            character = wgetch( om->menu.canvas_win );

            switch( ctune_UI_KeyBinding.getAction( CTUNE_UI_CTX_OPT_MENU, character ) ) {
                case CTUNE_UI_ACTION_RESIZE      : { ctune_UI_Resizer.resize();                      } goto resized;
                case CTUNE_UI_ACTION_ESC         : { om->cache.input_captured = false;               } break;
                case CTUNE_UI_ACTION_SELECT_PREV : { ctune_UI_SlideMenu.navKeyUp( &om->menu );       } break;
                case CTUNE_UI_ACTION_SELECT_NEXT : { ctune_UI_SlideMenu.navKeyDown( &om->menu );     } break;
                case CTUNE_UI_ACTION_GO_LEFT     : { ctune_UI_SlideMenu.navKeyLeft( &om->menu );     } break;
                case CTUNE_UI_ACTION_GO_RIGHT    : { ctune_UI_SlideMenu.navKeyRight( &om->menu );    } break;
                case CTUNE_UI_ACTION_SELECT_FIRST: { ctune_UI_SlideMenu.navKeyFirst( &om->menu );    } break;
                case CTUNE_UI_ACTION_SELECT_LAST : { ctune_UI_SlideMenu.navKeyLast( &om->menu);      } break;
                case CTUNE_UI_ACTION_PAGE_UP     : { ctune_UI_SlideMenu.navKeyPageUp( &om->menu );   } break;
                case CTUNE_UI_ACTION_PAGE_DOWN   : { ctune_UI_SlideMenu.navKeyPageDown( &om->menu ); } break;
                case CTUNE_UI_ACTION_TRIGGER     : { ctune_UI_SlideMenu.navKeyEnter( &om->menu );    } break;
                default                          : { om->cache.input_captured = false;               } break;
            }

            if( om->cache.input_captured ) {
                ctune_UI_BorderWin.show( &om->border_win );
                ctune_UI_SlideMenu.show( &om->menu );
            }
        }

        ctune_UI_SlideMenu.hide( &om->menu );
        ctune_UI_SlideMenu.reset( &om->menu );
        ctune_UI_BorderWin.hide( &om->border_win );

        update_panels();
        doupdate();

        ctune_UI_Resizer.pop();

        keypad( om->menu.canvas_win, FALSE );
}

/**
 * Exit method to break teh input capture loop (can be used as external callback)
 * @param om Pointer to ctune_UI_OptionsMenu_t object
 */
static void ctune_UI_Dialog_OptionsMenu_close( ctune_UI_OptionsMenu_t * om ) {
    om->cache.input_captured = false;
}

/**
 * De-allocates all content of OptionsMenu object
 * @param om Pointer to ctune_UI_OptionsMenu_t object
 */
static void ctune_UI_Dialog_OptionsMenu_free( ctune_UI_OptionsMenu_t * om ) {
    if( om ) {
        ctune_UI_BorderWin.free( &om->border_win );
        ctune_UI_SlideMenu.free( &om->menu );
        Vector.clear_vector( &om->cache.generic_payloads );
        Vector.clear_vector( &om->cache.flagged_payloads );
        Vector.clear_vector( &om->cache.sorting_payloads );
        om->initialised = false;

        CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_Dialog_OptionsMenu_free( %p )] OptionsMenu freed.", om );
    }
}

/**
 * Sets the callback method to sort the station list
 * @param om Pointer to ctune_UI_OptionsMenu_t object
 * @param fn Callback function
 */
static void ctune_UI_Dialog_OptionsMenu_cb_setSortStationList( ctune_UI_OptionsMenu_t * om, void (*fn)( ctune_UI_PanelID_e , ctune_RadioStationInfo_SortBy_e ) ) {
    if( om != NULL ) {
        om->cb.sortStationList = fn;
    }
}

/**
 * Sets the callback method to open new station dialog
 * @param om Pointer to ctune_UI_OptionsMenu_t object
 * @param fn Callback function
 */
static void ctune_UI_Dialog_OptionsMenu_cb_setAddNewStation( ctune_UI_OptionsMenu_t * om, void (* fn)( ctune_UI_PanelID_e ) ) {
    if( om != NULL ) {
        om->cb.addNewStation = fn;
    }
}

/**
 * Sets the callback method to open edit station dialog
 * @param om Pointer to ctune_UI_OptionsMenu_t object
 * @param fn Callback function
 */
static void ctune_UI_Dialog_OptionsMenu_cb_setEditStation( ctune_UI_OptionsMenu_t * om, void (* fn)( ctune_UI_PanelID_e ) ) {
    if( om != NULL ) {
        om->cb.editStation = fn;
    }
}

/**
 * Sets the callback method to toggle a station's "favourite" status
 * @param om Pointer to ctune_UI_OptionsMenu_t object
 * @param fn Callback function
 */
static void ctune_UI_Dialog_OptionsMenu_cb_setToggleFavourite( ctune_UI_OptionsMenu_t * om, void (* fn)( ctune_UI_PanelID_e ) ) {
    if( om != NULL ) {
        om->cb.toggleFavourite = fn;
    }
}

/**
 * Sets the callback method to sync favourites from remote sources with their upstream counterpart
 * @param om Pointer to ctune_UI_OptionsMenu_t object
 * @param fn Callback function
 */
static void ctune_UI_Dialog_OptionsMenu_cb_setSyncCurrSelectedStation( ctune_UI_OptionsMenu_t * om, void (* fn)( ctune_UI_PanelID_e ) ) {
    if( om != NULL ) {
        om->cb.syncUpstream = fn;
    }
}

/**
 * Sets the callback method to set/get the "favourite" tab's theming
 * @param om Pointer to ctune_UI_OptionsMenu_t object
 * @param fn Callback function
 */
static void ctune_UI_Dialog_OptionsMenu_cb_favTheming( ctune_UI_OptionsMenu_t * om, bool (* fn)( ctune_UI_PanelID_e, ctune_Flag_e ) ) {
    if( om != NULL ) {
        om->cb.favouriteTabTheming = fn;
    }
}

/**
 * Sets the callback method to set the current tab station list's row size to large
 * @param om Pointer to ctune_UI_OptionsMenu_t object
 * @param fn Callback function
 */
static void ctune_UI_Dialog_OptionsMenu_cb_setSetListRowSizeLarge( ctune_UI_OptionsMenu_t * om, bool (* fn)( ctune_UI_PanelID_e, ctune_Flag_e ) ) {
    if( om != NULL ) {
        om->cb.listRowSizeLarge = fn;
    }
}



/**
 * Namespace constructor
 */
const struct ctune_UI_Dialog_OptionsMenu_Namespace ctune_UI_OptionsMenu = {
    .create       = &ctune_UI_Dialog_OptionsMenu_create,
    .init         = &ctune_UI_Dialog_OptionsMenu_init,
    .setRedraw    = &ctune_UI_Dialog_OptionsMenu_setRedraw,
    .show         = &ctune_UI_Dialog_OptionsMenu_show,
    .resize       = &ctune_UI_Dialog_OptionsMenu_resize,
    .captureInput = &ctune_UI_Dialog_OptionsMenu_captureInput,
    .close        = &ctune_UI_Dialog_OptionsMenu_close,
    .free         = &ctune_UI_Dialog_OptionsMenu_free,
    .cb = {
        .setSortStationListCallback         = &ctune_UI_Dialog_OptionsMenu_cb_setSortStationList,
        .setAddNewStationCallback           = &ctune_UI_Dialog_OptionsMenu_cb_setAddNewStation,
        .setEditStationCallback             = &ctune_UI_Dialog_OptionsMenu_cb_setEditStation,
        .setToggleFavouriteCallback         = &ctune_UI_Dialog_OptionsMenu_cb_setToggleFavourite,
        .setSyncCurrSelectedStationCallback = &ctune_UI_Dialog_OptionsMenu_cb_setSyncCurrSelectedStation,
        .setFavouriteTabThemingCallback     = &ctune_UI_Dialog_OptionsMenu_cb_favTheming,
        .setListRowSizeLargeCallback        = &ctune_UI_Dialog_OptionsMenu_cb_setSetListRowSizeLarge,
    },
};