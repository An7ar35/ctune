#include "OptionsMenu.h"

#include "../../dto/PluginInfo.h"
#include "../definitions/KeyBinding.h"
#include "../definitions/Theme.h"
#include "../EventQueue.h"
#include "../Resizer.h"
#include "ContextHelp.h"

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
        const char                * text      = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_CONFIGURATION );
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
        ctune_UIConfig_t * ui_config   = om->cb.getUIConfig();
        ctune_UIPreset_e   curr_preset = ui_config->theme.preset;

        for( int id = CTUNE_UIPRESET_DEFAULT; id < CTUNE_UIPRESET_COUNT; ++id ) {
            if( id != curr_preset ) {
                const char                * text      = ctune_UIPreset.str( id );
                CbPayload_t               * payload   = createCbPayload( om, &om->cache.payloads, om->cb.setUIPreset, id );
                ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, text, payload, ctrlMenuFunctionCb );

                if( payload && menu_item ) {
                    max_text_width = ctune_max_ul( max_text_width, strlen( text ) );

                } else {
                    CTUNE_LOG( CTUNE_LOG_ERROR,
                               "[ctune_UI_Dialog_OptionsMenu_populateUIThemeMenu( %p )] Failed creation of menu item '%s'.",
                               om, text
                    );
                    error_state = true;
                }
            }
        }
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
 * [PRIVATE] Populates the Mouse's "Click interval" sub-menu
 * @param om Pointer to ctune_UI_OptionsMenu_t object
 * @param root Pointer to SlideMenu item from which to spawn the menu from
 * @return Success
 */
static bool ctune_UI_Dialog_OptionsMenu_populateMouseIntervalResolutionMenu( ctune_UI_OptionsMenu_t * om, ctune_UI_SlideMenu_Item_t * root ) {
    bool error_state = false;

    if( ctune_UI_SlideMenu.createMenu( &root->sub_menu, root->parent_menu, root->index ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_Dialog_OptionsMenu_populateMouseIntervalResolutionMenu( %p, %p )] "
                   "Failed to create sub menu for item ('%s').",
                   om, root, root->text._raw
        );

        error_state = true;
        goto end;
    }

    size_t max_text_width = om->cache.slide_menu_property.cols;

    { //"Go back" entry
        const char                * text      = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_MOUSE );
        ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_PARENT, text, NULL, NULL );

        if( menu_item ) {
            max_text_width = ctune_max_ul( max_text_width, strlen( text ) );

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Dialog_OptionsMenu_populateMouseIntervalResolutionMenu( %p, %p )] Failed creation of menu item '%s'.",
                       om, root, text
            );

            error_state = true;
            goto end;
        }
    }

    { //Mouse click interval resolutions
        ctune_UIConfig_t          * ui_config   = om->cb.getUIConfig();
        const ctune_MouseInterval_e curr_preset = ctune_UIConfig.mouse.clickIntervalPreset( ui_config );

        for( ctune_MouseInterval_e id = 0; id < CTUNE_MOUSEINTERVAL_COUNT; ++id ) {
            const int resolution = ctune_MouseInterval.value( id );

            String_t text = String.init();

            ctune_ltos( resolution, &text );
            String.append_back( &text, "ms" );

            if( id == CTUNE_MOUSEINTERVAL_DEFAULT ) {
                String.append_back( &text, " (" );
                String.append_back( &text, om->cb.getDisplayText( CTUNE_UI_TEXT_DEFAULT ) );
                String.append_back( &text, ")" );
            }

            if( id == curr_preset ) {
                String.append_back( &text, " *" );
            }

            CbPayload_t               * payload   = createCbPayload( om, &om->cache.payloads, om->cb.setMouseResolution, id );
            ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, text._raw, payload, ctrlMenuFunctionCb );

            if( payload && menu_item ) {
                max_text_width = ctune_max_ul( max_text_width, String.length( &text ) );

            } else {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_UI_Dialog_OptionsMenu_populateMouseIntervalResolutionMenu( %p )] Failed creation of menu item '%s'.",
                           om, text._raw
                );
                error_state = true;
            }

            String.free( &text );
        }
    }

    if( !ctune_utoi( max_text_width, &om->cache.slide_menu_property.cols ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_UI_Dialog_OptionsMenu_populateMouseIntervalResolutionMenu( %p, %p )] Failed to cast to integer (%lu).",
                   om, root, max_text_width
        );
        error_state = true;
    }

    end:
        return !( error_state );
}

/**
 * [PRIVATE] Populates the "Mouse" sub-menu
 * @param om Pointer to ctune_UI_OptionsMenu_t object
 * @param root Pointer to SlideMenu item from which to spawn the menu from
 * @return Success
 */
static bool ctune_UI_Dialog_OptionsMenu_populateMouseMenu( ctune_UI_OptionsMenu_t * om, ctune_UI_SlideMenu_Item_t * root ) {
    bool error_state = false;

    if( ctune_UI_SlideMenu.createMenu( &root->sub_menu, root->parent_menu, root->index ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_Dialog_OptionsMenu_populateMouseMenu( %p, %p )] "
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
                       "[ctune_UI_Dialog_OptionsMenu_populateMouseMenu( %p, %p )] Failed creation of menu item '%s'.",
                       om, root, text
            );

            error_state = true;
            goto end;
        }
    }

    const bool curr_state = om->cb.mouseSupport( om->cache.curr_panel_id, FLAG_GET_VALUE );

    { //"Enable/Disable mouse support" entry
        const ctune_Flag_e action     = ( curr_state ? FLAG_SET_OFF : FLAG_SET_ON );
        const char *       text       = om->cb.getDisplayText( ( curr_state ? CTUNE_UI_TEXT_MOUSE_DISABLE : CTUNE_UI_TEXT_MOUSE_ENABLE ) );

        CbPayload_t               * payload   = createCbPayload( om, &om->cache.payloads, om->cb.mouseSupport, action );
        ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, text, payload, ctrlMenuFunctionCb );

        if( payload && menu_item ) {
            max_text_width = ctune_max_ul( max_text_width, strlen( text ) );

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Dialog_OptionsMenu_populateMouseMenu( %p, %p )] Failed creation of menu item '%s'.",
                       om, root, text
            );
            error_state = true;
        }
    }

    if( curr_state == FLAG_SET_ON ) { //Mouse click resolution menu
        const char                * text      = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_MOUSE_CLICK_INTERVAL );
        ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_MENU, text, NULL, NULL );

        if( menu_item && ctune_UI_Dialog_OptionsMenu_populateMouseIntervalResolutionMenu( om, menu_item ) ) {
            //max_text_width must be reloaded since it might have been changed in the "populate" call
            max_text_width = ctune_max_ul( om->cache.slide_menu_property.cols, strlen( text ) );

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Dialog_OptionsMenu_populateMouseMenu( %p, %p )] Failed creation of menu item '%s'.",
                       om, root, text
            );
            error_state = true;
        }
    }

    if( !ctune_utoi( max_text_width, &om->cache.slide_menu_property.cols ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_UI_Dialog_OptionsMenu_populateMouseMenu( %p, %p )] Failed to cast to integer (%lu).",
                   om, root, max_text_width
        );
        error_state = true;
    }

    end:
        return !( error_state );
}

/**
 * [PRIVATE] Populates the "Player" plugins sub-menu
 * @param om Pointer to ctune_UI_OptionsMenu_t object
 * @param root Pointer to SlideMenu item from which to spawn the menu from
 * @return Success
 */
static bool ctune_UI_Dialog_OptionsMenu_populatePlayerPluginsMenu( ctune_UI_OptionsMenu_t * om, ctune_UI_SlideMenu_Item_t * root ) {
    bool error_state = false;

    if( ctune_UI_SlideMenu.createMenu( &root->sub_menu, root->parent_menu, root->index ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_Dialog_OptionsMenu_populatePlayerPluginsMenu( %p, %p )] "
                   "Failed to create sub menu for item ('%s').",
                   om, root, root->text._raw
        );

        error_state = true;
        goto end;
    }

    size_t max_text_width = om->cache.slide_menu_property.cols;

    { //"Go back" entry
        const char                * text      = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_PLUGINS );
        ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_PARENT, text, NULL, NULL );

        if( menu_item ) {
            max_text_width = ctune_max_ul( max_text_width, strlen( text ) );

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Dialog_OptionsMenu_populatePlayerPluginsMenu( %p, %p )] Failed creation of menu item '%s'.",
                       om, root, text
            );

            error_state = true;
            goto end;
        }
    }

    { //Player plugins
        const Vector_t * list = om->cb.pluginList( CTUNE_PLUGIN_IN_STREAM_PLAYER );

        for( size_t i = 0; i < Vector.size( list ); ++i ) {
            ctune_PluginInfo_t * info = Vector.at( (Vector_t *) list, i );
            String_t             text = String.init();

            String.set( &text, info->name );

            if( info->selected ) {
                String.append_back( &text, " *" );
            }

            CbPayload_t               * payload   = createCbPayload( om, &om->cache.payloads, om->cb.setPlayPlugin, (int) info->id );
            ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, text._raw, payload, ctrlMenuFunctionCb );

            if( payload && menu_item ) {
                max_text_width = ctune_max_ul( max_text_width, String.length( &text ) );

            } else {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_UI_Dialog_OptionsMenu_populatePlayerPluginsMenu( %p )] Failed creation of menu item '%s'.",
                           om, text._raw
                );
                error_state = true;
            }

            String.free( &text );
        }
    }

    if( !ctune_utoi( max_text_width, &om->cache.slide_menu_property.cols ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_UI_Dialog_OptionsMenu_populatePlayerPluginsMenu( %p, %p )] Failed to cast to integer (%lu).",
                   om, root, max_text_width
        );
        error_state = true;
    }

    end:
        return !( error_state );
}

/**
 * [PRIVATE] Populates the "Sound Servers" plugins sub-menu
 * @param om Pointer to ctune_UI_OptionsMenu_t object
 * @param root Pointer to SlideMenu item from which to spawn the menu from
 * @return Success
 */
static bool ctune_UI_Dialog_OptionsMenu_populateSoundServerPluginsMenu( ctune_UI_OptionsMenu_t * om, ctune_UI_SlideMenu_Item_t * root ) {
    bool error_state = false;

    if( ctune_UI_SlideMenu.createMenu( &root->sub_menu, root->parent_menu, root->index ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_Dialog_OptionsMenu_populateSoundServerPluginsMenu( %p, %p )] "
                   "Failed to create sub menu for item ('%s').",
                   om, root, root->text._raw
        );

        error_state = true;
        goto end;
    }

    size_t max_text_width = om->cache.slide_menu_property.cols;

    { //"Go back" entry
        const char                * text      = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_PLUGINS );
        ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_PARENT, text, NULL, NULL );

        if( menu_item ) {
            max_text_width = ctune_max_ul( max_text_width, strlen( text ) );

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Dialog_OptionsMenu_populateSoundServerPluginsMenu( %p, %p )] Failed creation of menu item '%s'.",
                       om, root, text
            );

            error_state = true;
            goto end;
        }
    }

    { //Player plugins
        const Vector_t * list = om->cb.pluginList( CTUNE_PLUGIN_OUT_AUDIO_SERVER );

        for( size_t i = 0; i < Vector.size( list ); ++i ) {
            ctune_PluginInfo_t * info = Vector.at( (Vector_t *) list, i );
            String_t             text = String.init();

            String.set( &text, info->name );

            if( info->selected ) {
                String.append_back( &text, " *" );
            }

            CbPayload_t               * payload   = createCbPayload( om, &om->cache.payloads, om->cb.setSrvPlugin, (int) info->id );
            ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, text._raw, payload, ctrlMenuFunctionCb );

            if( payload && menu_item ) {
                max_text_width = ctune_max_ul( max_text_width, String.length( &text ) );

            } else {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_UI_Dialog_OptionsMenu_populateSoundServerPluginsMenu( %p )] Failed creation of menu item '%s'.",
                           om, text._raw
                );
                error_state = true;
            }

            String.free( &text );
        }
    }

    if( !ctune_utoi( max_text_width, &om->cache.slide_menu_property.cols ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_UI_Dialog_OptionsMenu_populateSoundServerPluginsMenu( %p, %p )] Failed to cast to integer (%lu).",
                   om, root, max_text_width
        );
        error_state = true;
    }

    end:
        return !( error_state );
}

/**
 * [PRIVATE] Populates the "Recorder" plugins sub-menu
 * @param om Pointer to ctune_UI_OptionsMenu_t object
 * @param root Pointer to SlideMenu item from which to spawn the menu from
 * @return Success
 */
static bool ctune_UI_Dialog_OptionsMenu_populateRecorderPluginsMenu( ctune_UI_OptionsMenu_t * om, ctune_UI_SlideMenu_Item_t * root ) {
    bool error_state = false;

    if( ctune_UI_SlideMenu.createMenu( &root->sub_menu, root->parent_menu, root->index ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_Dialog_OptionsMenu_populateRecorderPluginsMenu( %p, %p )] "
                   "Failed to create sub menu for item ('%s').",
                   om, root, root->text._raw
        );

        error_state = true;
        goto end;
    }

    size_t max_text_width = om->cache.slide_menu_property.cols;

    { //"Go back" entry
        const char                * text      = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_PLUGINS );
        ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_PARENT, text, NULL, NULL );

        if( menu_item ) {
            max_text_width = ctune_max_ul( max_text_width, strlen( text ) );

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Dialog_OptionsMenu_populateRecorderPluginsMenu( %p, %p )] Failed creation of menu item '%s'.",
                       om, root, text
            );

            error_state = true;
            goto end;
        }
    }

    { //Player plugins
        const Vector_t * list = om->cb.pluginList( CTUNE_PLUGIN_OUT_AUDIO_RECORDER );

        for( size_t i = 0; i < Vector.size( list ); ++i ) {
            ctune_PluginInfo_t * info = Vector.at( (Vector_t *) list, i );
            String_t             text = String.init();

            String.set( &text, info->name );

            if( info->selected ) {
                String.append_back( &text, " *" );
            }

            CbPayload_t               * payload   = createCbPayload( om, &om->cache.payloads, om->cb.setRecPlugin, (int) info->id );
            ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, text._raw, payload, ctrlMenuFunctionCb );

            if( payload && menu_item ) {
                max_text_width = ctune_max_ul( max_text_width, String.length( &text ) );

            } else {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_UI_Dialog_OptionsMenu_populateRecorderPluginsMenu( %p )] Failed creation of menu item '%s'.",
                           om, text._raw
                );
                error_state = true;
            }

            String.free( &text );
        }
    }

    if( !ctune_utoi( max_text_width, &om->cache.slide_menu_property.cols ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_UI_Dialog_OptionsMenu_populateRecorderPluginsMenu( %p, %p )] Failed to cast to integer (%lu).",
                   om, root, max_text_width
        );
        error_state = true;
    }

    end:
        return !( error_state );
}

/**
 * [PRIVATE] Populates the "Plugins" sub-menu
 * @param om Pointer to ctune_UI_OptionsMenu_t object
 * @param root Pointer to SlideMenu item from which to spawn the menu from
 * @return Success
 */
static bool ctune_UI_Dialog_OptionsMenu_populatePluginsMenu( ctune_UI_OptionsMenu_t * om, ctune_UI_SlideMenu_Item_t * root ) {
    bool error_state = false;

    if( ctune_UI_SlideMenu.createMenu( &root->sub_menu, root->parent_menu, root->index ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_Dialog_OptionsMenu_populatePluginsMenu( %p, %p )] "
                   "Failed to create sub menu for item ('%s').",
                   om, root, root->text._raw
        );

        error_state = true;
        goto end;
    }

    size_t max_text_width = om->cache.slide_menu_property.cols;

    { //"Go back" entry
        const char                * text      = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_OPTIONS );
        ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_PARENT, text, NULL, NULL );

        if( menu_item ) {
            max_text_width = ctune_max_ul( max_text_width, strlen( text ) );

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Dialog_OptionsMenu_populatePluginsMenu( %p, %p )] Failed creation of menu item '%s'.",
                       om, root, text
            );

            error_state = true;
            goto end;
        }
    }

    if( om->cb.pluginList != NULL ) { //Plugin menu
        const char                * text      = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_PLUGINS_PLAYERS );
        ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_MENU, text, NULL, NULL );

        if( menu_item && ctune_UI_Dialog_OptionsMenu_populatePlayerPluginsMenu( om, menu_item ) ) {
            //max_text_width must be reloaded since it might have been changed in the "populate" call
            max_text_width = ctune_max_ul( om->cache.slide_menu_property.cols, strlen( text ) );

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Dialog_OptionsMenu_populatePluginsMenu( %p, %p )] Failed creation of menu item '%s'.",
                       om, root, text
            );
            error_state = true;
        }
    }

    if( om->cb.pluginList != NULL ) { //Plugin menu
        const char                * text      = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_PLUGINS_SOUND_SERVER );
        ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_MENU, text, NULL, NULL );

        if( menu_item && ctune_UI_Dialog_OptionsMenu_populateSoundServerPluginsMenu( om, menu_item ) ) {
            //max_text_width must be reloaded since it might have been changed in the "populate" call
            max_text_width = ctune_max_ul( om->cache.slide_menu_property.cols, strlen( text ) );

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Dialog_OptionsMenu_populatePluginsMenu( %p, %p )] Failed creation of menu item '%s'.",
                       om, root, text
            );
            error_state = true;
        }
    }

    if( om->cb.pluginList != NULL ) { //Plugin menu
        const char                * text      = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_PLUGINS_RECORDING );
        ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_MENU, text, NULL, NULL );

        if( menu_item && ctune_UI_Dialog_OptionsMenu_populateRecorderPluginsMenu( om, menu_item ) ) {
            //max_text_width must be reloaded since it might have been changed in the "populate" call
            max_text_width = ctune_max_ul( om->cache.slide_menu_property.cols, strlen( text ) );

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Dialog_OptionsMenu_populatePluginsMenu( %p, %p )] Failed creation of menu item '%s'.",
                       om, root, text
            );
            error_state = true;
        }
    }


    if( !ctune_utoi( max_text_width, &om->cache.slide_menu_property.cols ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_UI_Dialog_OptionsMenu_populatePluginsMenu( %p, %p )] Failed to cast to integer (%lu).",
                   om, root, max_text_width
        );
        error_state = true;
    }

    end:
        return !( error_state );
}

/**
 * [PRIVATE] Populates the "Stream timeout" sub-menu
 * @param om   Pointer to ctune_UI_OptionsMenu_t object
 * @param root Pointer to SlideMenu item from which to spawn the menu from
 * @return Success
 */
static bool ctune_UI_Dialog_OptionsMenu_populateStreamTimeoutMenu( ctune_UI_OptionsMenu_t * om, ctune_UI_SlideMenu_Item_t * root ) {
    bool error_state = false;

    if( ctune_UI_SlideMenu.createMenu( &root->sub_menu, root->parent_menu, root->index ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_Dialog_OptionsMenu_populateStreamTimeoutMenu( %p, %p )] "
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
                       "[ctune_UI_Dialog_OptionsMenu_populateStreamTimeoutMenu( %p, %p )] Failed creation of menu item '%s'.",
                       om, root, text
            );

            error_state = true;
            goto end;
        }
    }

    { //Stream timeout value entries
        const int min_timeout     = 2;
        const int max_timeout     = 10;
        const int current_timeout = om->cb.streamTimeout( om->cache.curr_panel_id, -1 );

        for( int i = min_timeout; i <= max_timeout; ++i ) {
            String_t text = String.init();

            ctune_ltos( i, &text );

            if( i == current_timeout ) {
                String.append_back( &text, "s *" );
            } else {
                String.append_back( &text, "s" );
            }

            CbPayload_t               * payload   = createCbPayload( om, &om->cache.payloads, om->cb.streamTimeout, i );
            ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, text._raw, payload, ctrlMenuFunctionCb );

            if( payload && menu_item ) {
                max_text_width = ctune_max_ul( max_text_width, String.length( &text ) );

            } else {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_UI_Dialog_OptionsMenu_populateStreamTimeoutMenu( %p )] Failed creation of menu item '%s'.",
                           om, text._raw
                );
                error_state = true;
            }

            String.free( &text );
        }
    }

    if( !ctune_utoi( max_text_width, &om->cache.slide_menu_property.cols ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_UI_Dialog_OptionsMenu_populateStreamTimeoutMenu( %p, %p )] Failed to cast to integer (%lu).",
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
        const char                * text      = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_OPTIONS );
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

    if( om->cb.getUIConfig != NULL && om->cb.setUIPreset != NULL ) { //UI themes menu
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

    if( om->cb.unicodeIcons != NULL ) { //"ASCII/Unicode icons" entry
        const bool         curr_state = om->cb.unicodeIcons( om->cache.curr_panel_id, FLAG_GET_VALUE );
        const ctune_Flag_e action     = ( curr_state ? FLAG_SET_OFF : FLAG_SET_ON );
        const char *       text       = om->cb.getDisplayText( ( curr_state ? CTUNE_UI_TEXT_ICONS_ASCII : CTUNE_UI_TEXT_ICONS_UNICODE ) );

        CbPayload_t               * payload   = createCbPayload( om, &om->cache.payloads, om->cb.unicodeIcons, action );
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

    if( om->cb.getUIConfig != NULL && om->cb.mouseSupport != NULL ) { //Mouse menu
        const char                * text      = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_MOUSE );
        ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_MENU, text, NULL, NULL );

        if( menu_item && ctune_UI_Dialog_OptionsMenu_populateMouseMenu( om, menu_item ) ) {
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

    if( om->cb.pluginList != NULL ) { //Plugin menu
        const char                * text      = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_PLUGINS );
        ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_MENU, text, NULL, NULL );

        if( menu_item && ctune_UI_Dialog_OptionsMenu_populatePluginsMenu( om, menu_item ) ) {
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

    if( om->cb.setRecDir != NULL ) { //"Set recording directory" menu item
        const char * text = om->cb.getDisplayText( CTUNE_UI_TEXT_SET_RECORDING_PATH );

        CbPayload_t               * payload   = createCbPayload( om, &om->cache.payloads, om->cb.setRecDir, 0 );
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

    if( om->cb.streamTimeout != NULL ) { //Stream timeout menu
        const char                * text      = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_STREAM_TIMEOUT );
        ctune_UI_SlideMenu_Item_t * menu_item = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_MENU, text, NULL, NULL );

        if( menu_item && ctune_UI_Dialog_OptionsMenu_populateStreamTimeoutMenu( om, menu_item ) ) {
            max_text_width = ctune_max_ul( max_text_width, strlen( text ) );

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
        [SORT_ITEM_GO_BACK   ] = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_OPTIONS ),
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
            .getUIConfig         = NULL,
            .setUIPreset         = NULL,
            .mouseSupport        = NULL,
            .unicodeIcons        = NULL,
            .streamTimeout       = NULL,
        }
    };
}

/**
 * Initialised a ctune_UI_OptionsMenu_t object
 * @param om         Pointer to ctune_UI_OptionsMenu_t object
 * @param mouse_ctrl Flag to turn init mouse controls
 * @return Success
 */
static bool ctune_UI_Dialog_OptionsMenu_init( ctune_UI_OptionsMenu_t * om, bool mouse_ctrl ) {
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

    ctune_UI_SlideMenu.setCanvasProperties( &om->menu, &om->cache.slide_menu_property, mouse_ctrl );

    const bool borderwin_is_init = ctune_UI_BorderWin.init( &om->border_win,
                                                            &om->cache.border_win_property,
                                                            om->cb.getDisplayText( CTUNE_UI_TEXT_WIN_TITLE_OPTIONMENU ),
                                                            mouse_ctrl );
    if( !borderwin_is_init ) {
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

    const bool borderwin_is_init = ctune_UI_BorderWin.init( &opt_menu->border_win,
                                               &opt_menu->cache.border_win_property,
                                               opt_menu->cb.getDisplayText( CTUNE_UI_TEXT_WIN_TITLE_OPTIONMENU ),
                                               ctune_UI_SlideMenu.mouseCtrl( &opt_menu->menu ) );

    if( !borderwin_is_init ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_Dialog_OptionsMenu_resize( %p )] Failed to create border window.", om );
        return; //EARLY RETURN
    }

    ctune_UI_SlideMenu.resize( &opt_menu->menu );
    ctune_UI_BorderWin.show( &opt_menu->border_win );
    ctune_UI_SlideMenu.show( &opt_menu->menu );
    keypad( opt_menu->menu.canvas_win, TRUE ); //reinit since `ctune_UI_SlideMenu.resize(..)` will re-create its `canvas_win`
}

/**
 * [PRIVATE] Handle mouse event
 * @param om    Pointer to ctune_UI_OptionsMenu_t object
 * @param event Mouse event
 */
static void ctune_UI_Dialog_OptionsMenu_handleMouseEvent( ctune_UI_OptionsMenu_t * om, MEVENT * event ) {
    const int win_ctrl = ctune_UI_BorderWin.isCtrlButton( &om->border_win, event->y, event->x );
    const int scroll   = ctune_UI_SlideMenu.isScrollButton( &om->menu, event->y, event->x );

    if( event->bstate & BUTTON1_CLICKED ) {
        if( scroll & CTUNE_UI_SCROLL_UP ) {
            ctune_UI_SlideMenu.navKeyPageUp( &om->menu );
        } else if( scroll & CTUNE_UI_SCROLL_DOWN ) {
            ctune_UI_SlideMenu.navKeyPageDown( &om->menu );
        } else if( win_ctrl & CTUNE_UI_WINCTRLMASK_CLOSE ) {
            om->cache.input_captured = false;
        } else {
            ctune_UI_SlideMenu.selectAt( &om->menu, event->y, event->x );
        }

    } else if( event->bstate & BUTTON1_DOUBLE_CLICKED && scroll == 0 ) {
        ctune_UI_SlideMenu.selectAt( &om->menu, event->y, event->x );
        ctune_UI_SlideMenu.show( &om->menu );
        ctune_UI_SlideMenu.navKeyEnter( &om->menu );

    } else if( event->bstate & BUTTON3_CLICKED ) {
        if( scroll & CTUNE_UI_SCROLL_UP ) {
            ctune_UI_SlideMenu.navKeyFirst( &om->menu );

        } else if( scroll & CTUNE_UI_SCROLL_DOWN ) {
            ctune_UI_SlideMenu.navKeyLast( &om->menu );
        }
    }
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
    MEVENT       mouse_event;

    while( om->cache.input_captured ) {
        character = wgetch( om->menu.canvas_win );

        switch( ctune_UI_KeyBinding.getAction( CTUNE_UI_CTX_OPT_MENU, character ) ) {
            case CTUNE_UI_ACTION_ERR: {
                if( ctune_UI_Resizer.resizingRequested() ) {
                    ctune_UI_Resizer.resize();
                }

                if( !ctune_UI_EventQueue.empty() ) {
                    ctune_UI_EventQueue.flush();
                }
            } break;

            case CTUNE_UI_ACTION_HELP: {
                ctune_UI_ContextHelp.show( CTUNE_UI_CTX_OPT_MENU );
                ctune_UI_ContextHelp.captureInput();
            } break;

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

            case CTUNE_UI_ACTION_MOUSE_EVENT : {
                if( getmouse( &mouse_event ) == OK ) {
                    ctune_UI_Dialog_OptionsMenu_handleMouseEvent( om, &mouse_event );
                }
            } break;

            default: {
                om->cache.input_captured = false;
            } break;
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
 * Sets the callback method to get a pointer to the UIConfig object
 * @param om       Pointer to ctune_UI_OptionsMenu_t object
 * @param callback Callback function
 */
static void ctune_UI_Dialog_OptionsMenu_cb_setGetUIConfigCallback( ctune_UI_OptionsMenu_t * om, ctune_UIConfig_t * (* callback)( void ) ) {
    if( om != NULL ) {
        om->cb.getUIConfig = callback;
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
 * Sets the callback method to set the mouse support
 * @param om       Pointer to ctune_UI_OptionsMenu_t object
 * @param callback Callback function
 */
static void ctune_UI_Dialog_OptionsMenu_cb_setMouseSupportCallback( ctune_UI_OptionsMenu_t * om, OptionsMenuCb_fn callback ) {
    if( om != NULL ) {
        om->cb.mouseSupport = callback;
    }
}

/**
 * Sets the callback method to set the mouse's click-interval resolution in the configuration
 * @param om       Pointer to ctune_UI_OptionsMenu_t object
 * @param callback Callback function
 */
static void ctune_UI_Dialog_OptionsMenu_cb_setMouseResolutionCallback( ctune_UI_OptionsMenu_t * om, OptionsMenuCb_fn callback ) {
    if( om != NULL ) {
        om->cb.setMouseResolution = callback;
    }
}

/**
 * Sets the callback method to set unicode icons on/off
 * @param om       Pointer to ctune_UI_OptionsMenu_t object
 * @param callback Callback function
 */
static void ctune_UI_Dialog_OptionsMenu_cb_setUnicodeIconsCallback( ctune_UI_OptionsMenu_t * om, OptionsMenuCb_fn callback ) {
    if( om != NULL ) {
        om->cb.unicodeIcons = callback;
    }
}

/**
 * Sets the callback method to set/get the stream timeout value in the configuration
 * @param om       Pointer to ctune_UI_OptionsMenu_t object
 * @param callback Callback function
 */
static void ctune_UI_Dialog_OptionsMenu_cb_setStreamTimeoutValueCallback( ctune_UI_OptionsMenu_t * om, OptionsMenuCb_fn callback ) {
    if( om != NULL ) {
        om->cb.streamTimeout = callback;
    }
}

/**
 * Sets the callback method to get a list of plugins
 * @param om       Pointer to ctune_UI_OptionsMenu_t object
 * @param callback Callback function
 */
static void ctune_UI_Dialog_OptionsMenu_cb_setPluginListCallback( ctune_UI_OptionsMenu_t * om, const Vector_t * (* callback)( ctune_PluginType_e ) ) {
    if( om != NULL ) {
        om->cb.pluginList = callback;
    }
}

/**
 * Sets the callback methods to set plugins in the configuration
 * @param om                       Pointer to ctune_UI_OptionsMenu_t object
 * @param setPlayPlugin_callback   Plugin setter callback function
 * @param setSndSrvPlugin_callback Plugin setter callback function
 * @param setRecPlugin_callback    Plugin setter callback function
 */
static void ctune_UI_Dialog_OptionsMenu_cb_setPluginSetterCallbacks( ctune_UI_OptionsMenu_t * om,
                                                                     OptionsMenuCb_fn setPlayPlugin_callback,
                                                                     OptionsMenuCb_fn setSndSrvPlugin_callback,
                                                                     OptionsMenuCb_fn setRecPlugin_callback )
{
    if( om != NULL ) {
        om->cb.setPlayPlugin = setPlayPlugin_callback;
        om->cb.setSrvPlugin  = setSndSrvPlugin_callback;
        om->cb.setRecPlugin  = setRecPlugin_callback;
    }
}

/**
 * Sets the callback method to set the recording directory path
 * @param om       Pointer to ctune_UI_OptionsMenu_t object
 * @param callback Callback function
 */
static void ctune_UI_Dialog_OptionsMenu_cb_setRecordingDirPathCallback( ctune_UI_OptionsMenu_t * om, OptionsMenuCb_fn callback ) {
    if( om != NULL ) {
        om->cb.setRecDir = callback;
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
        .setGetUIConfigCallback             = &ctune_UI_Dialog_OptionsMenu_cb_setGetUIConfigCallback,
        .setSetUIPresetCallback             = &ctune_UI_Dialog_OptionsMenu_cb_setSetUIPresetCallback,
        .setMouseSupportCallback            = &ctune_UI_Dialog_OptionsMenu_cb_setMouseSupportCallback,
        .setMouseResolutionCallback         = &ctune_UI_Dialog_OptionsMenu_cb_setMouseResolutionCallback,
        .setUnicodeIconsCallback            = &ctune_UI_Dialog_OptionsMenu_cb_setUnicodeIconsCallback,
        .setStreamTimeoutValueCallback      = &ctune_UI_Dialog_OptionsMenu_cb_setStreamTimeoutValueCallback,
        .setPluginListCallback              = &ctune_UI_Dialog_OptionsMenu_cb_setPluginListCallback,
        .setPluginSetterCallbacks           = &ctune_UI_Dialog_OptionsMenu_cb_setPluginSetterCallbacks,
        .setRecordingDirPathCallback        = &ctune_UI_Dialog_OptionsMenu_cb_setRecordingDirPathCallback,
    },
};