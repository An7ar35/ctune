#include "OptionsMenu.h"

#include "../definitions/KeyBinding.h"
#include "../definitions/Theme.h"
#include "../Resizer.h"

enum RootMenuItems { ROOT_ITEM_SORT = 0,    ROOT_ITEM_NEW,     ROOT_ITEM_EDIT,      ROOT_ITEM_TOGGLE,
                     ROOT_ITEM_SYNC,        ROOT_ITEM_COUNT };

enum SortMenuItems { SORT_ITEM_GO_BACK = 0, SORT_ITEM_NAME,    SORT_ITEM_NAME_R,    SORT_ITEM_TAGS,
                     SORT_ITEM_TAGS_R,      SORT_ITEM_COUNTRY, SORT_ITEM_COUNTRY_R, SORT_ITEM_CC,
                     SORT_ITEM_CC_R,        SORT_ITEM_STATE,   SORT_ITEM_STATE_R,   SORT_ITEM_LANGUAGE,
                     SORT_ITEM_LANGUAGE_R,  SORT_ITEM_CODEC,   SORT_ITEM_CODEC_R,   SORT_ITEM_BITRATE,
                     SORT_ITEM_BITRATE_R,   SORT_ITEM_SOURCE,  SORT_ITEM_SOURCE_R,  SORT_ITEM_COUNT };

typedef struct {
    ctune_UI_PanelID_e * curr_panel;
    void (* action)( ctune_UI_PanelID_e tab );
} CbPayload_t ;

typedef struct {
    ctune_UI_PanelID_e            * curr_panel;
    ctune_RadioStationInfo_SortBy_e attr;
    void (* action)( ctune_UI_PanelID_e tab, ctune_RadioStationInfo_SortBy_e attr );
} SortCbPayload_t;

/**
 * [PRIVATE] Helper method to create a CbPayload_t in the cache
 * @param om Pointer to ctune_UI_OptionsMenu_t object
 * @param fn Method callback
 * @return Pointer to created payload or NULL if error
 */
static CbPayload_t * ctune_UI_Dialog_OptionsMenu_createCbPayload( ctune_UI_OptionsMenu_t * om, void (* fn)( ctune_UI_PanelID_e ) ) {
    CbPayload_t * payload = Vector.emplace_back( &om->cache.lvl1_menu_payloads );

    if( payload != NULL ) {
        payload->curr_panel = &om->cache.curr_panel_id;
        payload->action     = fn;

    } else {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_Dialog_OptionsMenu_createCbPayload( %p, %p )] "
                   "Failed to create payload.",
                   om, fn );
    }

    return payload;
}

/**
 * [PRIVATE] Helper method to create a CbPayload_t in the cache
 * @param om Pointer to ctune_UI_OptionsMenu_t object
 * @param fn Method callback
 * @param attr Station attribute to sort by
 * @param reverse Flag to reverse the sorting of the attribute
 * @return Pointer to created payload or NULL if error
 */
static SortCbPayload_t * ctune_UI_Dialog_OptionsMenu_createSortCbPayload( ctune_UI_OptionsMenu_t * om,
                                                                          void (* fn)( ctune_UI_PanelID_e, ctune_RadioStationInfo_SortBy_e ),
                                                                          ctune_RadioStationInfo_SortBy_e attr )
{
    SortCbPayload_t * payload = Vector.emplace_back( &om->cache.lvl2_menu_payloads );

    if( payload != NULL ) {
        payload->curr_panel = &om->cache.curr_panel_id;
        payload->attr       = attr;
        payload->action     = fn;

    } else {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_Dialog_OptionsMenu_createSortCbPayload( %p, %p, %i )] "
                   "Failed to create payload.",
                   om, fn, attr
        );
    }

    return payload;
}

/**
 * [PRIVATE] Root menu control method
 * @param menu_item Current menu item
 * @return Success
 */
static bool rootCtrlMenuFunctionCb( ctune_UI_SlideMenu_Item_t * menu_item ) {
    CbPayload_t * payload = (CbPayload_t *) menu_item->data;

    if( payload == NULL || payload->curr_panel == NULL )
        return false; //EARLY RETURN

    payload->action( *payload->curr_panel );
    return true;
}

/**
 * [PRIVATE] Sort sub-menu control method
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
 * [PRIVATE] Calculates the display properties of the SlideMenu and BorderWin
 * @param om             Pointer to ctune_UI_OptionsMenu_t object
 * @param lvl1_menu_text Array of menu item display text for 1st level menu
 * @param lvl2_menu_text Array of menu item display text for 2nd level menu
 * @return Success
 */
static bool ctune_UI_Dialog_OptionsMenu_calculateProperties( ctune_UI_OptionsMenu_t * om, const char **lvl1_menu_text, const char **lvl2_menu_text ) {
    bool                   error_state = false;
    WindowProperty_t     * content     = &om->cache.slide_menu_property;
    WindowProperty_t     * borders     = &om->cache.border_win_property;
    const WindowMargin_t * margins     = &om->margins;

    { //optimum height/width for lvl1 menu to be shown
        size_t max_txt_width = 0;

        content->rows = ROOT_ITEM_COUNT;

        for( int i = 0; i < ROOT_ITEM_COUNT; ++i )
            max_txt_width = ctune_max_ul( max_txt_width, strlen( lvl1_menu_text[ i ] ) );

        for( int i = 0; i < SORT_ITEM_COUNT; ++i )
            max_txt_width = ctune_max_ul( max_txt_width, strlen( lvl2_menu_text[ i ] ) );

        max_txt_width += 4; //to have space for the scrolling bar and '<', '>' nav indicators

        if( !ctune_utoi( max_txt_width, &content->cols ) ) {
            CTUNE_LOG( CTUNE_LOG_FATAL,
                       "[ctune_UI_Dialog_OptionsMenu_calculateSlideMenuProperties( %p )] Failed to cast to integer (%lu).",
                       om, max_txt_width
            );

            error_state = true;
            goto end;
        }
    }

    { //adjust h/w based on parent screen real estate and taking into account the borders and margins
        const int border_t = 1;
        const int border_b = 1;
        const int border_l = 1;
        const int border_r = 1;

        const int max_content_width  = ( om->parent->cols - margins->left - margins->right  - border_l - border_r );
        const int max_content_height = ( om->parent->rows - margins->top  - margins->bottom - border_t - border_b );

        if( content->cols > max_content_width ) {
            CTUNE_LOG( CTUNE_LOG_WARNING,
                       "[ctune_UI_Dialog_OptionsMenu_calculateProperties( %p )] "
                       "SlideMenu content too wide - clipping will occur.",
                       om
            );
        }

        if( content->rows > max_content_height ) {
            CTUNE_LOG( CTUNE_LOG_DEBUG,
                       "[ctune_UI_Dialog_OptionsMenu_calculateProperties( %p )] "
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
    }

    end:
        return !( error_state );
}

/**
 * [PRIVATE] Populates the sorting sub-menu
 * @param om        Pointer to ctune_UI_OptionsMenu_t object
 * @param root      Root of sort menu
 * @param menu_text Array of menu text
 * @return Success
 */
static bool ctune_UI_Dialog_OptionsMenu_populateSortMenu( ctune_UI_OptionsMenu_t * om, ctune_UI_SlideMenu_Item_t * root, const char **menu_text ) {
    bool error_state = false;

    if( ctune_UI_SlideMenu.createMenu( &root->sub_menu, root->parent_menu, root->index ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_Dialog_OptionsMenu_populateSortMenu( %p, %p, %p )] "
                   "Failed to create sub menu for item ('%s').",
                   om, root, menu_text, root->text._raw
        );

        error_state = true;
        goto end;
    }

    bool              err     [SORT_ITEM_COUNT];
    SortCbPayload_t * payloads[SORT_ITEM_COUNT];

    payloads[SORT_ITEM_GO_BACK   ] = NULL;
    payloads[SORT_ITEM_NAME      ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_NAME );
    payloads[SORT_ITEM_NAME_R    ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_NAME_DESC );
    payloads[SORT_ITEM_TAGS      ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_TAGS );
    payloads[SORT_ITEM_TAGS_R    ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_TAGS_DESC );
    payloads[SORT_ITEM_COUNTRY   ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_COUNTRY );
    payloads[SORT_ITEM_COUNTRY_R ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_COUNTRY_DESC );
    payloads[SORT_ITEM_CC        ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_COUNTRYCODE );
    payloads[SORT_ITEM_CC_R      ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_COUNTRYCODE_DESC );
    payloads[SORT_ITEM_STATE     ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_STATE );
    payloads[SORT_ITEM_STATE_R   ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_STATE_DESC );
    payloads[SORT_ITEM_LANGUAGE  ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_LANGUAGE );
    payloads[SORT_ITEM_LANGUAGE_R] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_LANGUAGE_DESC );
    payloads[SORT_ITEM_CODEC     ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_CODEC );
    payloads[SORT_ITEM_CODEC_R   ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_CODEC_DESC );
    payloads[SORT_ITEM_BITRATE   ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_BITRATE );
    payloads[SORT_ITEM_BITRATE_R ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_BITRATE_DESC );
    payloads[SORT_ITEM_SOURCE    ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_SOURCE );
    payloads[SORT_ITEM_SOURCE_R  ] = ctune_UI_Dialog_OptionsMenu_createSortCbPayload( om, om->cb.sortStationList, CTUNE_RADIOSTATIONINFO_SORTBY_SOURCE_DESC );

    err[SORT_ITEM_GO_BACK   ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_PARENT, menu_text[SORT_ITEM_GO_BACK], NULL, NULL );
    err[SORT_ITEM_NAME      ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_NAME], payloads[SORT_ITEM_NAME], sortByCtrlFunctionCb );
    err[SORT_ITEM_NAME_R    ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_NAME_R], payloads[SORT_ITEM_NAME_R], sortByCtrlFunctionCb );
    err[SORT_ITEM_TAGS      ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_TAGS], payloads[SORT_ITEM_TAGS], sortByCtrlFunctionCb );
    err[SORT_ITEM_TAGS_R    ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_TAGS_R], payloads[SORT_ITEM_TAGS_R], sortByCtrlFunctionCb );
    err[SORT_ITEM_COUNTRY   ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_COUNTRY], payloads[SORT_ITEM_COUNTRY], sortByCtrlFunctionCb );
    err[SORT_ITEM_COUNTRY_R ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_COUNTRY_R], payloads[SORT_ITEM_COUNTRY_R], sortByCtrlFunctionCb );
    err[SORT_ITEM_CC        ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_CC], payloads[SORT_ITEM_CC], sortByCtrlFunctionCb );
    err[SORT_ITEM_CC_R      ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_CC_R], payloads[SORT_ITEM_CC_R], sortByCtrlFunctionCb );
    err[SORT_ITEM_STATE     ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_STATE], payloads[SORT_ITEM_STATE], sortByCtrlFunctionCb );
    err[SORT_ITEM_STATE_R   ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_STATE_R], payloads[SORT_ITEM_STATE_R], sortByCtrlFunctionCb );
    err[SORT_ITEM_LANGUAGE  ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_LANGUAGE], payloads[SORT_ITEM_LANGUAGE], sortByCtrlFunctionCb );
    err[SORT_ITEM_LANGUAGE_R] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_LANGUAGE_R], payloads[SORT_ITEM_LANGUAGE_R], sortByCtrlFunctionCb );
    err[SORT_ITEM_CODEC     ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_CODEC], payloads[SORT_ITEM_CODEC], sortByCtrlFunctionCb );
    err[SORT_ITEM_CODEC_R   ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_CODEC_R], payloads[SORT_ITEM_CODEC_R], sortByCtrlFunctionCb );
    err[SORT_ITEM_BITRATE   ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_BITRATE], payloads[SORT_ITEM_BITRATE], sortByCtrlFunctionCb );
    err[SORT_ITEM_BITRATE_R ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_BITRATE_R], payloads[SORT_ITEM_BITRATE_R], sortByCtrlFunctionCb );
    err[SORT_ITEM_SOURCE    ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_SOURCE], payloads[SORT_ITEM_SOURCE], sortByCtrlFunctionCb );
    err[SORT_ITEM_SOURCE_R  ] = ctune_UI_SlideMenu.createMenuItem( root->sub_menu, CTUNE_UI_SLIDEMENU_LEAF, menu_text[SORT_ITEM_SOURCE_R], payloads[SORT_ITEM_SOURCE_R], sortByCtrlFunctionCb );

    for( int i = 0; i < SORT_ITEM_COUNT; ++i ) {
        if( payloads[i] == NULL && i != SORT_ITEM_GO_BACK ) {
            error_state = true;

            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Dialog_OptionsMenu_populateSortMenu( %p, %p, %p )] "
                       "Failed creation payload for menu item #%i ('%s').",
                       om, root, menu_text, i, menu_text[i]
            );
        }

        if( !err[i] ) {
            error_state = true;

            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Dialog_OptionsMenu_populateSortMenu( %p, %p, %p )] "
                       "Failed creation of menu item #%i ('%s').",
                       om, root, menu_text, i, menu_text[i]
            );
        }
    }

    end:
        return !( error_state );
}

/**
 * [PRIVATE] Creates root menu items
 * @param om        Pointer to ctune_UI_OptionsMenu_t object
 * @param menu_text Array containing the menu text
 * @return Success
 */
static bool ctune_UI_Dialog_OptionsMenu_populateRootMenu( ctune_UI_OptionsMenu_t * om, const char **menu_text ) {
    bool error_state = false;

    bool          err     [ROOT_ITEM_COUNT];
    CbPayload_t * payloads[ROOT_ITEM_COUNT];

    payloads[ROOT_ITEM_SORT  ] = NULL; //only goes to sub menu/no side effects required
    payloads[ROOT_ITEM_NEW   ] = ctune_UI_Dialog_OptionsMenu_createCbPayload( om, om->cb.addNewStation );
    payloads[ROOT_ITEM_EDIT  ] = ctune_UI_Dialog_OptionsMenu_createCbPayload( om, om->cb.editStation );
    payloads[ROOT_ITEM_TOGGLE] = ctune_UI_Dialog_OptionsMenu_createCbPayload( om, om->cb.toggleFavourite );
    payloads[ROOT_ITEM_SYNC  ] = ctune_UI_Dialog_OptionsMenu_createCbPayload( om, om->cb.syncUpstream );

    err[ROOT_ITEM_SORT  ] = ctune_UI_SlideMenu.createMenuItem( &om->menu.root, CTUNE_UI_SLIDEMENU_MENU, menu_text[ROOT_ITEM_SORT], payloads[ROOT_ITEM_SORT], NULL );
    err[ROOT_ITEM_NEW   ] = ctune_UI_SlideMenu.createMenuItem( &om->menu.root, CTUNE_UI_SLIDEMENU_LEAF, menu_text[ROOT_ITEM_NEW], payloads[ROOT_ITEM_NEW], rootCtrlMenuFunctionCb );
    err[ROOT_ITEM_EDIT  ] = ctune_UI_SlideMenu.createMenuItem( &om->menu.root, CTUNE_UI_SLIDEMENU_LEAF, menu_text[ROOT_ITEM_EDIT], payloads[ROOT_ITEM_EDIT], rootCtrlMenuFunctionCb );
    err[ROOT_ITEM_TOGGLE] = ctune_UI_SlideMenu.createMenuItem( &om->menu.root, CTUNE_UI_SLIDEMENU_LEAF, menu_text[ROOT_ITEM_TOGGLE], payloads[ROOT_ITEM_TOGGLE], rootCtrlMenuFunctionCb );
    err[ROOT_ITEM_SYNC  ] = ctune_UI_SlideMenu.createMenuItem( &om->menu.root, CTUNE_UI_SLIDEMENU_LEAF, menu_text[ROOT_ITEM_SYNC], payloads[ROOT_ITEM_SYNC], rootCtrlMenuFunctionCb );

    for( int i = 0; i < ROOT_ITEM_COUNT; ++i ) {
        if( payloads[i] == NULL && i != ROOT_ITEM_SORT ) {
            error_state = true;

            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Dialog_OptionsMenu_populateRootMenu( %p, %p )] "
                       "Failed creation payload for menu item #%i ('%s').",
                       om, menu_text, i, menu_text[i]
            );
        }

        if( !err[i] ) {
            error_state = true;

            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_Dialog_OptionsMenu_populateRootMenu( %p, %p )] "
                       "Failed creation of menu item #%i ('%s').",
                       om, menu_text, i, menu_text[i]
            );
        }
    }

    return !( error_state );
}

/**
 * Creates an uninitialised ctune_UI_OptionsMenu_t
 * @param parent          Canvas property for parent window
 * @param getDisplayText  Callback method to get UI text
 * @param sortStationList Callback method to sort station list
 * @param addNewStation   Callback method to open new station dialog
 * @param editStation     Callback method to open edit station dialog
 * @param toggleFavourite Callback method to toggle a station's "favourite" status
 * @param syncUpstream    Callback method to sync favourites from remote sources with their upstream counterpart
 * @return Initialised ctune_UI_OptionsMenu_t object
 */
static ctune_UI_OptionsMenu_t ctune_UI_Dialog_OptionsMenu_create( const WindowProperty_t * parent,
                                 const char * (* getDisplayText)( ctune_UI_TextID_e ),
                                 void         (* sortStationList)( ctune_UI_PanelID_e, ctune_RadioStationInfo_SortBy_e ),
                                 void         (* addNewStation)( ctune_UI_PanelID_e ),
                                 void         (* editStation)( ctune_UI_PanelID_e ),
                                 void         (* toggleFavourite)( ctune_UI_PanelID_e ),
                                 void         (* syncUpstream)( ctune_UI_PanelID_e ) )
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
            .lvl1_menu_payloads  = Vector.init( sizeof( CbPayload_t ), NULL ),
            .lvl2_menu_payloads  = Vector.init( sizeof( SortCbPayload_t ), NULL ),
        },
        .cb = {
            .getDisplayText  = getDisplayText,
            .sortStationList = sortStationList,
            .addNewStation   = addNewStation,
            .editStation     = editStation,
            .toggleFavourite = toggleFavourite,
            .syncUpstream    = syncUpstream,
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

    const char * lvl1_menu_text[ROOT_ITEM_COUNT] = {
        [ROOT_ITEM_SORT      ] = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_SORT_STATIONS ),
        [ROOT_ITEM_NEW       ] = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_NEW_STATION   ),
        [ROOT_ITEM_EDIT      ] = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_EDIT_STATION  ),
        [ROOT_ITEM_TOGGLE    ] = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_TOGGLE_FAV    ),
        [ROOT_ITEM_SYNC      ] = om->cb.getDisplayText( CTUNE_UI_TEXT_MENU_SYNC_UPSTREAM )
    };

    const char * lvl2_menu_text[SORT_ITEM_COUNT] = {
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

    if( !ctune_UI_Dialog_OptionsMenu_calculateProperties( om, lvl1_menu_text, lvl2_menu_text ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_UI_Dialog_OptionsMenu_init( %p )] Failed properties calculation.", om );
        error_state = true;
        goto end;
    }

    om->menu = ctune_UI_SlideMenu.init( &om->cache.slide_menu_property );

    if( !ctune_UI_Dialog_OptionsMenu_populateRootMenu( om, lvl1_menu_text ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_UI_Dialog_OptionsMenu_init( %p )] Failed populate menu.", om );
        error_state = true;
        goto end;
    }

    ctune_UI_SlideMenu_Item_t * sort_root = Vector.at( &om->menu.root.items, 0 );

    if( sort_root != NULL ) {
        if( !ctune_UI_Dialog_OptionsMenu_populateSortMenu( om, sort_root, lvl2_menu_text ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_Dialog_OptionsMenu_init( %p )] Failed to populate sort sub-menu.", om );
            error_state = true;
            goto end;
        };

    } else {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_Dialog_OptionsMenu_init( %p )] Root item Vector_t looks empty.", om );
        error_state = true;
        goto end;
    }

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
static void ctune_UI_Dialog_OptionsMenu_captureInput( ctune_UI_OptionsMenu_t * om, ctune_UI_PanelID_e tab ) {
    om->cache.curr_panel_id = tab;

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
        Vector.clear_vector( &om->cache.lvl1_menu_payloads );
        Vector.clear_vector( &om->cache.lvl2_menu_payloads );
        om->initialised = false;

        CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_Dialog_OptionsMenu_free( %p )] OptionsMenu freed.", om );
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
};