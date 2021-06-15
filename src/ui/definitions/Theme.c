#include "Theme.h"

#include <ncurses.h>

#include "../../logger/Logger.h"

/*
 * Default theme variables
 */
static const ctune_UI_Theme_t default_theme = {
    .background = COLOR_BLACK,
    .foreground = COLOR_WHITE,

    .rows = {
        .background            = COLOR_BLACK,
        .foreground            = COLOR_WHITE,
        .selected_focused_bg   = COLOR_BLUE,
        .selected_focused_fg   = COLOR_WHITE,
        .selected_unfocused_bg = COLOR_WHITE,
        .selected_unfocused_fg = COLOR_BLACK,
        .favourite_local_fg    = COLOR_MAGENTA,
        .favourite_remote_fg   = COLOR_YELLOW,
    },

    .icons = {
        .playback_on    = COLOR_GREEN,
        .playback_off   = COLOR_RED,
        .queued_station = COLOR_CYAN,
    },

    .field = {
        .invalid_fg = COLOR_RED,
    },

    .button = {
        .background   = COLOR_BLACK,
        .foreground   = COLOR_WHITE,
        .invalid_fg   = COLOR_RED,
        .validated_fg = COLOR_GREEN,
    },
};

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
 * Initiates the theming attributes
 * @param theme Theme to use
 */
static void ctune_UI_Theme_init( ctune_UI_Theme_t * theme ) {
    if( !is_initialised ) {
        start_color();

        if( theme == NULL )
            CTUNE_LOG( CTUNE_LOG_WARNING, "[ctune_UI_Theme_init( %p )] NULL theme: using default values.", theme );

        const ctune_UI_Theme_t * t = ( theme != NULL ? theme : &default_theme );

        //         theme item                                   foreground                     background
        init_pair( THEME_STD,                                   t->foreground,                 t->background                 );
        init_pair( THEME_STD_INV,                               t->background,                 t->foreground                 );
        init_pair( THEME_ROW,                                   t->rows.foreground,            t->rows.background            );
        init_pair( THEME_ROW_SELECTED_FOCUSED,                  t->rows.selected_focused_fg,   t->rows.selected_focused_bg   );
        init_pair( THEME_ROW_SELECTED_UNFOCUSED,                t->rows.selected_unfocused_fg, t->rows.selected_unfocused_bg );
        init_pair( THEME_ROW_LOCAL_FAV_TXT,                     t->rows.favourite_local_fg,    t->rows.background            );
        init_pair( THEME_ROW_LOCAL_FAV_TXT_SELECTED_FOCUSED,    t->rows.favourite_local_fg,    t->rows.selected_focused_bg   );
        init_pair( THEME_ROW_LOCAL_FAV_TXT_SELECTED_UNFOCUSED,  t->rows.favourite_local_fg,    t->rows.selected_unfocused_bg );
        init_pair( THEME_ROW_REMOTE_FAV_TXT,                    t->rows.favourite_remote_fg,   t->rows.background            );
        init_pair( THEME_ROW_REMOTE_FAV_TXT_SELECTED_FOCUSED,   t->rows.favourite_remote_fg,   t->rows.selected_focused_bg   );
        init_pair( THEME_ROW_REMOTE_FAV_TXT_SELECTED_UNFOCUSED, t->rows.favourite_remote_fg,   t->rows.selected_unfocused_bg );
        init_pair( THEME_QUEUED,                                t->icons.queued_station,       t->rows.background            );
        init_pair( THEME_QUEUED_FAV,                            t->icons.queued_station,       t->rows.background            );
        init_pair( THEME_QUEUED_INV_FOCUSED,                    t->icons.queued_station,       t->rows.selected_focused_bg   );
        init_pair( THEME_QUEUED_INV_UNFOCUSED,                  t->icons.queued_station,       t->rows.selected_unfocused_bg );
        init_pair( THEME_ICON_OFF,                              t->icons.playback_off,         t->foreground                 );
        init_pair( THEME_ICON_ON,                               t->icons.playback_on,          t->foreground                 );
        init_pair( THEME_FIELD_DFLT,                            t->foreground,                 t->background                 );
        init_pair( THEME_FIELD_INVALID,                         t->field.invalid_fg,           t->background                 );
        init_pair( THEME_BUTTON_DFLT,                           t->button.foreground,          t->button.background          );
        init_pair( THEME_BUTTON_VALID,                          t->button.validated_fg,        t->button.background          );
        init_pair( THEME_BUTTON_INVALID,                        t->button.invalid_fg,          t->button.background          );

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