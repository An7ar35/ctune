#include "OptionsMenu.h"

#include "../definitions/KeyBinding.h"
#include "../definitions/Theme.h"
#include "../Resizer.h"

/**
 * Generic payload package
 * @param curr_panel ctune_UI_PanelID_e
 * @param arg Argument (int)
 * @param action Callback function
 */
typedef struct {
    ctune_UI_PanelID_e * curr_panel;
    int                  arg;
    int (* action)( ctune_UI_PanelID_e tab, int arg );
} CbPayload_t;

/**
 * [PRIVATE] Helper method to create a CbPayload_t
 * @param om            Pointer to ctune_UI_OptionsMenu_t object
 * @param payload_store Vector of payloads to add new payload to
 * @param callback      Method callback
 * @param arg           Integer argument
 * @return Pointer to created payload or NULL if error
 */
static CbPayload_t * createCbPayload( ctune_UI_OptionsMenu_t * om, Vector_t * payload_store, OptionsMenuCb_fn callback, int arg ) {
    CbPayload_t * payload = Vector.emplace_back( payload_store );

    if( payload != NULL ) {
        payload->curr_panel = &om->cache.curr_panel_id;
        payload->arg        = arg;
        payload->action     = callback;

    } else {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[createCbPayload( %p, %p, %p, %d )] "
                   "Failed to create payload.",
                   om, payload_store, callback, arg );
    }

    return payload;
}

/**
 * [PRIVATE] Generic menu control method
 * @param menu_item Current menu item
 * @return Success
 */
static bool ctrlMenuFunctionCb( ctune_UI_SlideMenu_Item_t * menu_item ) {
    CbPayload_t * payload = (CbPayload_t *) menu_item->data;

    if( payload == NULL || payload->curr_panel == NULL )
        return false; //EARLY RETURN

    payload->action( *payload->curr_panel, payload->arg );
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
 * [PRIVATE] Populates the "UI Theme" sub-menu
 * @param om Pointer to ctune_UI_OptionsMenu_t object
 * @param root Pointer to SlideMenu item from which to spawn the menu from
 * @return Success
 */
static bool ctune_UI_Dialog_OptionsMenu_populateUIThemeMenu( ctune_UI_OptionsMenu_t * om, ctune_UI_SlideMenu_Item_t * root ) {
    bool error_state = false;

    if( ctune_UI_SlideMenu.createMenu( &root->sub_menu, root->parent_menu, root->index ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_Dialog_OptionsMenu_populateUIThemeMenu( %p, %p )] "
                   "Failed to create sub menu for item ('%s').",
                   om, root, root->text._raw
        );

        error_state = true;
        goto end;
    }

    size_t max_text_width = om->cache.slide_menu_property.cols;

    { //"Go back" entry
        const char                * text      = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_UI_THEME );
        ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_PARENT, text, NULL, NULL );

        if( menu_item ) {
            max_text_width = ctune_max_ul( max_text_width, strlen( text ) );

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Dialog_OptionsMenu_populateUIThemeMenu( %p, %p )] Failed creation of menu item '%s'.",
                       om, root, text
            );

            error_state = true;
            goto end;
        }
    }

    { //UI presets entries
        Vector_t presets = Vector.init( sizeof( ctune_UIPreset_t ), NULL );

        om->cb.getUIPresets( &presets );

        if( Vector.empty( &presets ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Dialog_OptionsMenu_populateUIThemeMenu( %p, %p )] Failed creation of menu items (preset list).",
                       om, root
            );

            error_state = true;
            Vector.clear_vector( &presets );
            goto end;
        }

        for( size_t i = 0; i < Vector.size( &presets ); ++i ) {
            ctune_UIPreset_t * preset = Vector.at( &presets, i );

            if( !preset->in_use ) {
                CbPayload_t               * payload   = createCbPayload( om, &om->cache.payloads, om->cb.setUIPreset, preset->id );
                ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, preset->name, payload, ctrlMenuFunctionCb );

                if( payload && menu_item ) {
                    max_text_width = ctune_max_ul( max_text_width, strlen( preset->name ) );

                } else {
                    CTUNE_LOG( CTUNE_LOG_ERROR,
                               "[ctune_UI_Dialog_OptionsMenu_populateRootMenu( %p )] Failed creation of menu item '%s'.",
                               om, preset->name
                    );
                    error_state = true;
                }
            }
        }

        Vector.clear_vector( &presets );
    }

    if( !ctune_utoi( max_text_width, &om->cache.slide_menu_property.cols ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_UI_Dialog_OptionsMenu_populateUIThemeMenu( %p, %p )] Failed to cast to integer (%lu).",
                   om, root, max_text_width
        );
        error_state = true;
    }

    end:
        return !( error_state );
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

    if( om->cb.getUIPresets != NULL && om->cb.setUIPreset != NULL ) { //UI themes menu
        const char                * text      = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_UI_THEME );
        ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_MENU, text, NULL, NULL );

        if( menu_item && ctune_UI_Dialog_OptionsMenu_populateUIThemeMenu( om, menu_item ) ) {
            //max_text_width must be reloaded since it might have been changed in the "populate" call
            max_text_width = ctune_max_ul( om->cache.slide_menu_property.cols, strlen( text ) );

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Dialog_OptionsMenu_populateConfigMenu( %p, %p )] Failed creation of menu item '%s'.",
                       om, root, text
            );
            error_state = true;
        }
    }

    if( om->cb.listRowSizeLarge != NULL ) { //"Set row size" entry
        const bool         curr_state = om->cb.listRowSizeLarge( om->cache.curr_panel_id, FLAG_GET_VALUE );
        const ctune_Flag_e action     = ( curr_state ? FLAG_SET_OFF : FLAG_SET_ON );
        const char *       text       = om->cb.getDisplayText( ( curr_state ? CTUNE_UI_TEXT_ROWSIZE_1X : CTUNE_UI_TEXT_ROWSIZE_2X ) );

        CbPayload_t               * payload   = createCbPayload( om, &om->cache.payloads, om->cb.listRowSizeLarge, action );
        ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, text, payload, ctrlMenuFunctionCb );

        if( payload && menu_item ) {
            max_text_width = ctune_max_ul( max_text_width, strlen( text ) );

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Dialog_OptionsMenu_populateConfigMenu( %p, %p )] Failed creation of menu item '%s'.",
                       om, root, text
            );
            error_state = true;
        }
    }

    if( om->cb.favTabTheming != NULL ) { //"Toggle theming" entry
        const bool         curr_state = om->cb.favTabTheming( om->cache.curr_panel_id, FLAG_GET_VALUE );
        const ctune_Flag_e action     = ( curr_state ? FLAG_SET_OFF : FLAG_SET_ON );
        const char *       text       = om->cb.getDisplayText( ( curr_state ? CTUNE_UI_TEXT_FAV_THEMING_OFF : CTUNE_UI_TEXT_FAV_THEMING_ON ) );

        CbPayload_t               * payload   = createCbPayload( om, &om->cache.payloads, om->cb.favTabTheming, action );
        ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, text, payload, ctrlMenuFunctionCb );

        if( payload && menu_item ) {
            max_text_width = ctune_max_ul( max_text_width, strlen( text ) );

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Dialog_OptionsMenu_populateConfigMenu( %p, %p )] Failed creation of menu item '%s'.",
                       om, root, text
            );
            error_state = true;
        }
    }

    if( om->cb.favTabTheming != NULL && om->cb.favTabCustomTheming != NULL ) { //"Toggle custom theme colouring" entry
        const int  fav_theming_state     = om->cb.favTabTheming( om->cache.curr_panel_id, FLAG_GET_VALUE );
        const int  custom_theming_states = om->cb.favTabCustomTheming( om->cache.curr_panel_id, FLAG_GET_VALUE );
        const bool ui_custom_preset      = custom_theming_states & 0b10; //i.e.: is the UI using the 'custom' preset?
        const bool fav_custom_theme      = custom_theming_states & 0b01; //i.e.: is the fav tab using the 'custom' UI theme preset for the station source colouring

        if( fav_theming_state && !ui_custom_preset ) {
            const ctune_Flag_e action = ( fav_custom_theme ? FLAG_SET_OFF : FLAG_SET_ON );
            const char *       text   = om->cb.getDisplayText( ( fav_custom_theme ? CTUNE_UI_TEXT_FAV_USE_PRESET_COLOURS : CTUNE_UI_TEXT_FAV_USE_CUSTOM_COLOURS ) );

            CbPayload_t               * payload   = createCbPayload( om, &om->cache.payloads, om->cb.favTabCustomTheming, action );
            ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, text, payload, ctrlMenuFunctionCb );

            if( payload && menu_item ) {
                max_text_width = ctune_max_ul( max_text_width, strlen( text ) );

            } else {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_UI_Dialog_OptionsMenu_populateConfigMenu( %p, %p )] Failed creation of menu item '%s'.",
                           om, root, text
                );
                error_state = true;
            }
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
    CbPayload_t               * payloads  [SORT_ITEM_COUNT];
    Vector_t                  * payload_store = &om->cache.payloads; //shortcut

    payloads[SORT_ITEM_GO_BACK   ] = NULL;
    payloads[SORT_ITEM_NAME      ] = createCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_NAME );
    payloads[SORT_ITEM_NAME_R    ] = createCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_NAME_DESC );
    payloads[SORT_ITEM_TAGS      ] = createCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_TAGS );
    payloads[SORT_ITEM_TAGS_R    ] = createCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_TAGS_DESC );
    payloads[SORT_ITEM_COUNTRY   ] = createCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_COUNTRY );
    payloads[SORT_ITEM_COUNTRY_R ] = createCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_COUNTRY_DESC );
    payloads[SORT_ITEM_CC        ] = createCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_COUNTRYCODE );
    payloads[SORT_ITEM_CC_R      ] = createCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_COUNTRYCODE_DESC );
    payloads[SORT_ITEM_STATE     ] = createCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_STATE );
    payloads[SORT_ITEM_STATE_R   ] = createCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_STATE_DESC );
    payloads[SORT_ITEM_LANGUAGE  ] = createCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_LANGUAGE );
    payloads[SORT_ITEM_LANGUAGE_R] = createCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_LANGUAGE_DESC );
    payloads[SORT_ITEM_CODEC     ] = createCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_CODEC );
    payloads[SORT_ITEM_CODEC_R   ] = createCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_CODEC_DESC );
    payloads[SORT_ITEM_BITRATE   ] = createCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_BITRATE );
    payloads[SORT_ITEM_BITRATE_R ] = createCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_BITRATE_DESC );
    payloads[SORT_ITEM_SOURCE    ] = createCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_SOURCE );
    payloads[SORT_ITEM_SOURCE_R  ] = createCbPayload( om, payload_store, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_SOURCE_DESC );

    menu_items[SORT_ITEM_GO_BACK   ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_PARENT, menu_text[SORT_ITEM_GO_BACK], NULL, NULL );
    menu_items[SORT_ITEM_NAME      ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[ SORT_ITEM_NAME ], payloads[ SORT_ITEM_NAME ], ctrlMenuFunctionCb );
    menu_items[SORT_ITEM_NAME_R    ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[ SORT_ITEM_NAME_R ], payloads[ SORT_ITEM_NAME_R ], ctrlMenuFunctionCb );
    menu_items[SORT_ITEM_TAGS      ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[ SORT_ITEM_TAGS ], payloads[ SORT_ITEM_TAGS ], ctrlMenuFunctionCb );
    menu_items[SORT_ITEM_TAGS_R    ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[ SORT_ITEM_TAGS_R ], payloads[ SORT_ITEM_TAGS_R ], ctrlMenuFunctionCb );
    menu_items[SORT_ITEM_COUNTRY   ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[ SORT_ITEM_COUNTRY ], payloads[ SORT_ITEM_COUNTRY ], ctrlMenuFunctionCb );
    menu_items[SORT_ITEM_COUNTRY_R ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[ SORT_ITEM_COUNTRY_R ], payloads[ SORT_ITEM_COUNTRY_R ], ctrlMenuFunctionCb );
    menu_items[SORT_ITEM_CC        ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[ SORT_ITEM_CC ], payloads[ SORT_ITEM_CC ], ctrlMenuFunctionCb );
    menu_items[SORT_ITEM_CC_R      ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[ SORT_ITEM_CC_R ], payloads[ SORT_ITEM_CC_R ], ctrlMenuFunctionCb );
    menu_items[SORT_ITEM_STATE     ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[ SORT_ITEM_STATE ], payloads[ SORT_ITEM_STATE ], ctrlMenuFunctionCb );
    menu_items[SORT_ITEM_STATE_R   ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[ SORT_ITEM_STATE_R ], payloads[ SORT_ITEM_STATE_R ], ctrlMenuFunctionCb );
    menu_items[SORT_ITEM_LANGUAGE  ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[ SORT_ITEM_LANGUAGE ], payloads[ SORT_ITEM_LANGUAGE ], ctrlMenuFunctionCb );
    menu_items[SORT_ITEM_LANGUAGE_R] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[ SORT_ITEM_LANGUAGE_R ], payloads[ SORT_ITEM_LANGUAGE_R ], ctrlMenuFunctionCb );
    menu_items[SORT_ITEM_CODEC     ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[ SORT_ITEM_CODEC ], payloads[ SORT_ITEM_CODEC ], ctrlMenuFunctionCb );
    menu_items[SORT_ITEM_CODEC_R   ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[ SORT_ITEM_CODEC_R ], payloads[ SORT_ITEM_CODEC_R ], ctrlMenuFunctionCb );
    menu_items[SORT_ITEM_BITRATE   ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[ SORT_ITEM_BITRATE ], payloads[ SORT_ITEM_BITRATE ], ctrlMenuFunctionCb );
    menu_items[SORT_ITEM_BITRATE_R ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[ SORT_ITEM_BITRATE_R ], payloads[ SORT_ITEM_BITRATE_R ], ctrlMenuFunctionCb );
    menu_items[SORT_ITEM_SOURCE    ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[ SORT_ITEM_SOURCE ], payloads[ SORT_ITEM_SOURCE ], ctrlMenuFunctionCb );
    menu_items[SORT_ITEM_SOURCE_R  ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[ SORT_ITEM_SOURCE_R ], payloads[ SORT_ITEM_SOURCE_R ], ctrlMenuFunctionCb );

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
        CbPayload_t               * payload   = createCbPayload( om, &om->cache.payloads, om->cb.addNewStation, 0 );
        ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( &om->menu.root, CTUNE_UI_SLIDEMENU_LEAF, text, payload, ctrlMenuFunctionCb );

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
        CbPayload_t               * payload   = createCbPayload( om, &om->cache.payloads, om->cb.editStation, 0 );
        ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( &om->menu.root, CTUNE_UI_SLIDEMENU_LEAF, text, payload, ctrlMenuFunctionCb );

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
        CbPayload_t               * payload   = createCbPayload( om, &om->cache.payloads, om->cb.toggleFavourite, 0 );
        ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( &om->menu.root, CTUNE_UI_SLIDEMENU_LEAF, text, payload, ctrlMenuFunctionCb );

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
        CbPayload_t               * payload   = createCbPayload( om, &om->cache.payloads, om->cb.syncUpstream, 0 );
        ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( &om->menu.root, CTUNE_UI_SLIDEMENU_LEAF, text, payload, ctrlMenuFunctionCb );

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

    if( om->cb.favTabTheming != NULL || om->cb.listRowSizeLarge != NULL ) { //Configuration menu
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
            .payloads            = Vector.init( sizeof( CbPayload_t ), NULL ),
        },
        .cb = {
            .getDisplayText      = getDisplayText,
            .sortStationList     = NULL,
            .addNewStation       = NULL,
            .editStation         = NULL,
            .toggleFavourite     = NULL,
            .syncUpstream        = NULL,
            .favTabTheming       = NULL,
            .favTabCustomTheming = NULL,
            .listRowSizeLarge    = NULL,
            .getUIPresets        = NULL,
            .setUIPreset         = NULL,
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

    ctune_UI_SlideMenu.setFocus( &om->menu, true );

    bool success = ctune_UI_BorderWin.show( &om->border_win )
                && ctune_UI_SlideMenu.show( &om->menu );

    if( success ) {
        ctune_UI_Resizer.push( ctune_UI_OptionsMenu.resize, om );
    }

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
    ctune_UI_BorderWin.show( &opt_menu->border_win );
    ctune_UI_SlideMenu.show( &opt_menu->menu );
    keypad( opt_menu->menu.canvas_win, TRUE ); //reinit since `ctune_UI_SlideMenu.resize(..)` will re-create its `canvas_win`
}

/**
 * Pass keyboard input to the form
 * @param om Pointer to ctune_UI_OptionsMenu_t object
 */
static void ctune_UI_Dialog_OptionsMenu_captureInput( ctune_UI_OptionsMenu_t * om ) {
    keypad( om->menu.canvas_win, TRUE );
    curs_set( 0 );

    om->cache.input_captured = true;
    volatile int character;

    while( om->cache.input_captured ) {
        character = wgetch( om->menu.canvas_win );

        switch( ctune_UI_KeyBinding.getAction( CTUNE_UI_CTX_OPT_MENU, character ) ) {
            case CTUNE_UI_ACTION_ERR         : //fallthrough
            case CTUNE_UI_ACTION_RESIZE      : break;
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
        Vector.clear_vector( &om->cache.payloads );
        om->initialised = false;

        CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_Dialog_OptionsMenu_free( %p )] OptionsMenu freed.", om );
    }
}

/**
 * Sets the callback method to sort the station list
 * @param om       Pointer to ctune_UI_OptionsMenu_t object
 * @param callback Callback function
 */
static void ctune_UI_Dialog_OptionsMenu_cb_setSortStationList( ctune_UI_OptionsMenu_t * om, OptionsMenuCb_fn callback ) {
    if( om != NULL ) {
        om->cb.sortStationList = callback;
    }
}

/**
 * Sets the callback method to open new station dialog
 * @param om       Pointer to ctune_UI_OptionsMenu_t object
 * @param callback Callback function
 */
static void ctune_UI_Dialog_OptionsMenu_cb_setAddNewStation( ctune_UI_OptionsMenu_t * om, OptionsMenuCb_fn callback ) {
    if( om != NULL ) {
        om->cb.addNewStation = callback;
    }
}

/**
 * Sets the callback method to open edit station dialog
 * @param om       Pointer to ctune_UI_OptionsMenu_t object
 * @param callback Callback function
 */
static void ctune_UI_Dialog_OptionsMenu_cb_setEditStation( ctune_UI_OptionsMenu_t * om, OptionsMenuCb_fn callback ) {
    if( om != NULL ) {
        om->cb.editStation = callback;
    }
}

/**
 * Sets the callback method to toggle a station's "favourite" status
 * @param om       Pointer to ctune_UI_OptionsMenu_t object
 * @param callback Callback function
 */
static void ctune_UI_Dialog_OptionsMenu_cb_setToggleFavourite( ctune_UI_OptionsMenu_t * om, OptionsMenuCb_fn callback ) {
    if( om != NULL ) {
        om->cb.toggleFavourite = callback;
    }
}

/**
 * Sets the callback method to sync favourites from remote sources with their upstream counterpart
 * @param om       Pointer to ctune_UI_OptionsMenu_t object
 * @param callback Callback function
 */
static void ctune_UI_Dialog_OptionsMenu_cb_setSyncCurrSelectedStation( ctune_UI_OptionsMenu_t * om, OptionsMenuCb_fn callback ) {
    if( om != NULL ) {
        om->cb.syncUpstream = callback;
    }
}

/**
 * Sets the callback method to set/get the "favourite" tab's theming
 * @param om       Pointer to ctune_UI_OptionsMenu_t object
 * @param callback Callback function
 */
static void ctune_UI_Dialog_OptionsMenu_cb_setFavThemingCallback( ctune_UI_OptionsMenu_t * om, OptionsMenuCb_fn callback ) {
    if( om != NULL ) {
        om->cb.favTabTheming = callback;
    }
}

/**
 * Sets the callback method to set/get the state of custom colour theming on the favourite's tab
 * @param om       Pointer to ctune_UI_OptionsMenu_t object
 * @param callback Callback function
 */
static void ctune_UI_Dialog_OptionsMenu_cb_setFavTabCustomThemingCallback( ctune_UI_OptionsMenu_t * om, OptionsMenuCb_fn callback ) {
    if( om != NULL ) {
        om->cb.favTabCustomTheming = callback;
    }
}

/**
 * Sets the callback method to set the current tab station list's row size to large
 * @param om       Pointer to ctune_UI_OptionsMenu_t object
 * @param callback Callback function
 */
static void ctune_UI_Dialog_OptionsMenu_cb_setSetListRowSizeLarge( ctune_UI_OptionsMenu_t * om, OptionsMenuCb_fn callback ) {
    if( om != NULL ) {
        om->cb.listRowSizeLarge = callback;
    }
}

/**
 * Sets the callback method to get the list of available UI colour pallet presets
 * @param om       Pointer to ctune_UI_OptionsMenu_t object
 * @param callback Callback function
 */
static void ctune_UI_Dialog_OptionsMenu_cb_setGetUIPresetCallback( ctune_UI_OptionsMenu_t * om, void (* callback)( Vector_t * ) ) {
    if( om != NULL ) {
        om->cb.getUIPresets = callback;
    }
}

/**
 * Sets the callback method to set a UI colour pallet preset
 * @param om       Pointer to ctune_UI_OptionsMenu_t object
 * @param callback Callback function
 */
static void ctune_UI_Dialog_OptionsMenu_cb_setSetUIPresetCallback( ctune_UI_OptionsMenu_t * om, OptionsMenuCb_fn callback ) {
    if( om != NULL ) {
        om->cb.setUIPreset = callback;
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
        .setFavouriteTabThemingCallback     = &ctune_UI_Dialog_OptionsMenu_cb_setFavThemingCallback,
        .setFavTabCustomThemingCallback     = &ctune_UI_Dialog_OptionsMenu_cb_setFavTabCustomThemingCallback,
        .setListRowSizeLargeCallback        = &ctune_UI_Dialog_OptionsMenu_cb_setSetListRowSizeLarge,
        .setGetUIPresetCallback             = &ctune_UI_Dialog_OptionsMenu_cb_setGetUIPresetCallback,
        .setSetUIPresetCallback             = &ctune_UI_Dialog_OptionsMenu_cb_setSetUIPresetCallback,
    },
};