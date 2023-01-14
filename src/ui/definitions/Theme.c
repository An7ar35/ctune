#include "Theme.h"

#include <ncurses.h>

#include "../../logger/Logger.h"

/**
 * Theme items
 */
enum Theme_Item {
    THEME_STD = 1,
    THEME_STD_INV,
    THEME_ROW,
    THEME_ROW_SELECTED_FOCUSED,
    THEME_ROW_SELECTED_UNFOCUSED,
    THEME_ROW_LOCAL_FAV_TXT,
    THEME_ROW_LOCAL_FAV_TXT_SELECTED_FOCUSED,
    THEME_ROW_LOCAL_FAV_TXT_SELECTED_UNFOCUSED,
    THEME_ROW_REMOTE_FAV_TXT,
    THEME_ROW_REMOTE_FAV_TXT_SELECTED_FOCUSED,
    THEME_ROW_REMOTE_FAV_TXT_SELECTED_UNFOCUSED,
    THEME_QUEUED,
    THEME_QUEUED_FAV,
    THEME_QUEUED_INV_FOCUSED,
    THEME_QUEUED_INV_UNFOCUSED,
    THEME_ICON_ON,
    THEME_ICON_OFF,
    THEME_FIELD_DFLT,
    THEME_FIELD_INVALID,
    THEME_BUTTON_DFLT,
    THEME_BUTTON_VALID,
    THEME_BUTTON_INVALID,
};

bool is_initialised = false;

static int attributes[CTUNE_UI_ITEM_COUNT] = {
    [CTUNE_UI_ITEM_TITLE                            ] = COLOR_PAIR( THEME_STD_INV ),
    [CTUNE_UI_ITEM_STATUS_BAR1                      ] = COLOR_PAIR( THEME_ICON_OFF ),
    [CTUNE_UI_ITEM_STATUS_BAR2                      ] = COLOR_PAIR( THEME_STD_INV ),
    [CTUNE_UI_ITEM_STATUS_BAR3                      ] = COLOR_PAIR( THEME_STD_INV ),
    [CTUNE_UI_ITEM_MSG_LINE                         ] = COLOR_PAIR( THEME_STD ),
    [CTUNE_UI_ITEM_TAB_BG                           ] = COLOR_PAIR( THEME_STD ),
    [CTUNE_UI_ITEM_TAB_CURR                         ] = COLOR_PAIR( THEME_STD_INV ),
    [CTUNE_UI_ITEM_DIALOG_WIN                       ] = COLOR_PAIR( THEME_STD ) | A_BOLD,
    [CTUNE_UI_ITEM_ROW                              ] = COLOR_PAIR( THEME_ROW ),
    [CTUNE_UI_ITEM_ROW_SELECTED_FOCUSED             ] = COLOR_PAIR( THEME_ROW_SELECTED_FOCUSED ) | A_BOLD,
    [CTUNE_UI_ITEM_ROW_SELECTED_UNFOCUSED           ] = COLOR_PAIR( THEME_ROW_SELECTED_UNFOCUSED ),
    [CTUNE_UI_ITEM_TXT_FAV_LOCAL                    ] = COLOR_PAIR( THEME_ROW_LOCAL_FAV_TXT ),
    [CTUNE_UI_ITEM_TXT_SELECTED_FOCUSED_FAV_LOCAL   ] = COLOR_PAIR( THEME_ROW_LOCAL_FAV_TXT_SELECTED_FOCUSED ) | A_BOLD,
    [CTUNE_UI_ITEM_TXT_SELECTED_UNFOCUSED_FAV_LOCAL ] = COLOR_PAIR( THEME_ROW_LOCAL_FAV_TXT_SELECTED_UNFOCUSED ),
    [CTUNE_UI_ITEM_TXT_FAV_REMOTE                   ] = COLOR_PAIR( THEME_ROW_REMOTE_FAV_TXT ),
    [CTUNE_UI_ITEM_TXT_SELECTED_FOCUSED_FAV_REMOTE  ] = COLOR_PAIR( THEME_ROW_REMOTE_FAV_TXT_SELECTED_FOCUSED ) | A_BOLD,
    [CTUNE_UI_ITEM_TXT_SELECTED_UNFOCUSED_FAV_REMOTE] = COLOR_PAIR( THEME_ROW_REMOTE_FAV_TXT_SELECTED_UNFOCUSED ),
    [CTUNE_UI_ITEM_TXT_HELP_HEADING                 ] = COLOR_PAIR( THEME_STD ) | A_BOLD,
    [CTUNE_UI_ITEM_SCROLL_INDICATOR                 ] = COLOR_PAIR( THEME_STD ) | A_BOLD,
    [CTUNE_UI_ITEM_QUEUED                           ] = COLOR_PAIR( THEME_QUEUED ),
    [CTUNE_UI_ITEM_QUEUED_INV_FOCUSED               ] = COLOR_PAIR( THEME_QUEUED_INV_FOCUSED ),
    [CTUNE_UI_ITEM_QUEUED_INV_UNFOCUSED             ] = COLOR_PAIR( THEME_QUEUED_INV_UNFOCUSED ),
    [CTUNE_UI_ITEM_PLAYBACK_ON                      ] = COLOR_PAIR( THEME_ICON_ON ),
    [CTUNE_UI_ITEM_PLAYBACK_OFF                     ] = COLOR_PAIR( THEME_ICON_OFF ),
    [CTUNE_UI_ITEM_FIELD_DFLT                       ] = COLOR_PAIR( THEME_FIELD_DFLT ),
    [CTUNE_UI_ITEM_FIELD_INVALID                    ] = COLOR_PAIR( THEME_FIELD_INVALID ),
    [CTUNE_UI_ITEM_BUTTON_DFLT                      ] = COLOR_PAIR( THEME_BUTTON_DFLT ),
    [CTUNE_UI_ITEM_BUTTON_VALID                     ] = COLOR_PAIR( THEME_BUTTON_VALID ),
    [CTUNE_UI_ITEM_BUTTON_INVALID                   ] = COLOR_PAIR( THEME_BUTTON_INVALID )
};

/**
 * [PRIVATE]
 * @param theme Theme to set
 */
static void ctune_UI_Theme_set( ctune_UI_Theme_t * theme ) {
    //         theme item                                   foreground                         background
    init_pair( THEME_STD,                                   theme->foreground,                 theme->background                 );
    init_pair( THEME_STD_INV,                               theme->background,                 theme->foreground                 );
    init_pair( THEME_ROW,                                   theme->rows.foreground,            theme->rows.background            );
    init_pair( THEME_ROW_SELECTED_FOCUSED,                  theme->rows.selected_focused_fg,   theme->rows.selected_focused_bg   );
    init_pair( THEME_ROW_SELECTED_UNFOCUSED,                theme->rows.selected_unfocused_fg, theme->rows.selected_unfocused_bg );
    init_pair( THEME_ROW_LOCAL_FAV_TXT,                     theme->rows.favourite_local_fg,    theme->rows.background            );
    init_pair( THEME_ROW_LOCAL_FAV_TXT_SELECTED_FOCUSED,    theme->rows.favourite_local_fg,    theme->rows.selected_focused_bg   );
    init_pair( THEME_ROW_LOCAL_FAV_TXT_SELECTED_UNFOCUSED,  theme->rows.favourite_local_fg,    theme->rows.selected_unfocused_bg );
    init_pair( THEME_ROW_REMOTE_FAV_TXT,                    theme->rows.favourite_remote_fg,   theme->rows.background            );
    init_pair( THEME_ROW_REMOTE_FAV_TXT_SELECTED_FOCUSED,   theme->rows.favourite_remote_fg,   theme->rows.selected_focused_bg   );
    init_pair( THEME_ROW_REMOTE_FAV_TXT_SELECTED_UNFOCUSED, theme->rows.favourite_remote_fg,   theme->rows.selected_unfocused_bg );
    init_pair( THEME_QUEUED,                                theme->icons.queued_station,       theme->rows.background            );
    init_pair( THEME_QUEUED_FAV,                            theme->icons.queued_station,       theme->rows.background            );
    init_pair( THEME_QUEUED_INV_FOCUSED,                    theme->icons.queued_station,       theme->rows.selected_focused_bg   );
    init_pair( THEME_QUEUED_INV_UNFOCUSED,                  theme->icons.queued_station,       theme->rows.selected_unfocused_bg );
    init_pair( THEME_ICON_OFF,                              theme->icons.playback_off,         theme->foreground                 );
    init_pair( THEME_ICON_ON,                               theme->icons.playback_on,          theme->foreground                 );
    init_pair( THEME_FIELD_DFLT,                            theme->foreground,                 theme->background                 );
    init_pair( THEME_FIELD_INVALID,                         theme->field.invalid_fg,           theme->background                 );
    init_pair( THEME_BUTTON_DFLT,                           theme->button.foreground,          theme->button.background          );
    init_pair( THEME_BUTTON_VALID,                          theme->button.validated_fg,        theme->button.background          );
    init_pair( THEME_BUTTON_INVALID,                        theme->button.invalid_fg,          theme->button.background          );
}

/**
 * Initiates the theming attributes
 * @param theme Theme to use
 */
static void ctune_UI_Theme_init( ctune_UI_Theme_t * theme ) {
    if( !is_initialised ) {
        start_color();

        if( theme == NULL ) {
            CTUNE_LOG( CTUNE_LOG_WARNING,
                       "[ctune_UI_Theme_init( %p )] NULL theme: using default values.",
                       theme
            );

            ctune_UI_Theme_t default_theme = ctune_ColourTheme.init( CTUNE_UITHEME_DEFAULT );
            ctune_UI_Theme_set( &default_theme );

        } else {
            ctune_UI_Theme_set( theme );
        }

        CTUNE_LOG( CTUNE_LOG_MSG, "[ctune_UI_Theme_init( %p )] Theming initialised.", theme )
        is_initialised = true;
    }
}

/**
 * Get the color for a UI item
 * @param item UI item
 * @return Color
 */
static int ctune_UI_Theme_color( ctune_UI_ThemeItem_e item ) {
    if( ( sizeof( attributes ) / sizeof( attributes[0] ) ) <= item ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_Theme_attribute( %i )] UI theme item not implemented.", item );
        return COLOR_PAIR( 0 );
    }

    return attributes[item];
}

/**
 * Constructor
 */
const struct ctune_UI_Theme_Instance ctune_UI_Theme = {
    .init  = &ctune_UI_Theme_init,
    .color = &ctune_UI_Theme_color,
};