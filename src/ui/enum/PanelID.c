#include "PanelID.h"

#include <assert.h>

/**
 * Gets corresponding string for a Panel ID
 * @param id PanelID enum type
 * @return String
 */
static const char * ctune_UI_PanelID_str( ctune_UI_PanelID_e id ) {
    static const char * str[CTUNE_UI_PANEL_COUNT] = {
        [CTUNE_UI_PANEL_TITLE     ] = "Title",
        [CTUNE_UI_PANEL_STATUS_1  ] = "Status 1",
        [CTUNE_UI_PANEL_STATUS_2  ] = "Status 2",
        [CTUNE_UI_PANEL_STATUS_3  ] = "Status 3",
        [CTUNE_UI_PANEL_MSG_LINE  ] = "Message line",
        [CTUNE_UI_PANEL_FAVOURITES] = "Favourites",
        [CTUNE_UI_PANEL_SEARCH    ] = "Search",
        [CTUNE_UI_PANEL_BROWSER   ] = "Browser",
    };

    assert( id >= 0 && id < CTUNE_UI_PANEL_COUNT );
    return str[id];
}

/**
 * Namespace constructor
 */
const struct ctune_UI_PanelID_Namespace ctune_UI_PanelID = {
    .str = &ctune_UI_PanelID_str,
};