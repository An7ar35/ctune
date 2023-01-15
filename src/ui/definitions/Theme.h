#ifndef CTUNE_UI_DEFINITIONS_THEME_H
#define CTUNE_UI_DEFINITIONS_THEME_H

#include "../../dto/ColourTheme.h"

typedef struct ctune_ColourTheme ctune_UI_Theme_t;

typedef enum ctune_UI_ThemeItem {
    CTUNE_UI_ITEM_TITLE = 0,
    CTUNE_UI_ITEM_STATUS_BAR1,
    CTUNE_UI_ITEM_STATUS_BAR2,
    CTUNE_UI_ITEM_STATUS_BAR3,
    CTUNE_UI_ITEM_MSG_LINE,
    CTUNE_UI_ITEM_TAB_BG,
    CTUNE_UI_ITEM_TAB_CURR,
    CTUNE_UI_ITEM_DIALOG_WIN,
    CTUNE_UI_ITEM_ROW,
    CTUNE_UI_ITEM_ROW_SELECTED_FOCUSED,
    CTUNE_UI_ITEM_ROW_SELECTED_UNFOCUSED,
    CTUNE_UI_ITEM_TXT_FAV_LOCAL,
    CTUNE_UI_ITEM_TXT_SELECTED_FOCUSED_FAV_LOCAL,
    CTUNE_UI_ITEM_TXT_SELECTED_UNFOCUSED_FAV_LOCAL,
    CTUNE_UI_ITEM_TXT_FAV_REMOTE,
    CTUNE_UI_ITEM_TXT_SELECTED_FOCUSED_FAV_REMOTE,
    CTUNE_UI_ITEM_TXT_SELECTED_UNFOCUSED_FAV_REMOTE,
    CTUNE_UI_ITEM_TXT_HELP_HEADING,
    CTUNE_UI_ITEM_SCROLL_INDICATOR,
    CTUNE_UI_ITEM_PLAYBACK_ON,
    CTUNE_UI_ITEM_PLAYBACK_OFF,
    CTUNE_UI_ITEM_QUEUED,
    CTUNE_UI_ITEM_QUEUED_INV_FOCUSED,
    CTUNE_UI_ITEM_QUEUED_INV_UNFOCUSED,
    CTUNE_UI_ITEM_FIELD_DFLT,
    CTUNE_UI_ITEM_FIELD_INVALID,
    CTUNE_UI_ITEM_BUTTON_DFLT,
    CTUNE_UI_ITEM_BUTTON_VALID,
    CTUNE_UI_ITEM_BUTTON_INVALID,

    CTUNE_UI_ITEM_COUNT
} ctune_UI_ThemeItem_e;

/**
 * UI theming instance which uses a `ctune_ColourTheme` struct
 * (aliased to `ctune_UI_Theme_t`) to initialize its internal state
 */
extern const struct ctune_UI_Theme_Instance {
    /**
     * Initiates the theming attributes
     * @param theme Theme to use
     */
    void (* init)( ctune_UI_Theme_t * theme );

    /**
     * Get the color for a UI item
     * @param item UI item
     * @return Color
     */
    int (* color)( ctune_UI_ThemeItem_e item );

} ctune_UI_Theme;

#endif //CTUNE_UI_DEFINITIONS_THEME_H
