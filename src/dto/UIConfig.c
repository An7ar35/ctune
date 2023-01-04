#include "UIConfig.h"

#include <stdlib.h>

/**
 * Copies the content of a UIConfig object into another
 * @param from Origin of content to copy from
 * @param to   Destination to copy to
 * @return Success
 */
static bool ctune_UIConfig_copy( const ctune_UIConfig_t * from, ctune_UIConfig_t * to ) {
    if( from == NULL || to == NULL )
        return false;

    to->fav_tab    = from->fav_tab;
    to->search_tab = from->search_tab;
    to->browse_tab = from->browse_tab;
    to->theme      = from->theme;

    return true;
}

/**
 * Get/Set "Favourites" tab theming
 * @param cfg Pointer to ctune_UIConfig_t object
 * @param flag Flag action
 * @return Property value after operation
 */
static bool ctune_UIConfig_FavTab_theming( ctune_UIConfig_t * cfg, ctune_Flag_e flag ) {
    switch( flag ) {
        case FLAG_SET_OFF: return ( cfg->fav_tab.theme_favourites = false );
        case FLAG_SET_ON : return ( cfg->fav_tab.theme_favourites = true );
        default          : return cfg->fav_tab.theme_favourites;
    }
}

/**
 * Get/Set "Favourites" tab's large row property
 * @param cfg Pointer to ctune_UIConfig_t object
 * @param flag Flag action
 * @return Property value after operation
 */
static bool ctune_UIConfig_FavTab_largeRowSize( ctune_UIConfig_t * cfg, ctune_Flag_e flag ) {
    switch( flag ) {
        case FLAG_SET_OFF: return ( cfg->fav_tab.large_rows = false );
        case FLAG_SET_ON : return ( cfg->fav_tab.large_rows = true );
        default          : return cfg->fav_tab.large_rows;
    }
}

/**
 * Get/Set "Search" tab's large row property
 * @param cfg Pointer to ctune_UIConfig_t object
 * @param flag Flag action
 * @return Property value after operation
 */
static bool ctune_UIConfig_SearchTab_largeRowSize( ctune_UIConfig_t * cfg, ctune_Flag_e flag ) {
    switch( flag ) {
        case FLAG_SET_OFF: return ( cfg->search_tab.large_rows = false );
        case FLAG_SET_ON : return ( cfg->search_tab.large_rows = true );
        default          : return cfg->search_tab.large_rows;
    }
}

/**
 * Get/Set "Browser" tab's large row property
 * @param cfg Pointer to ctune_UIConfig_t object
 * @param flag Flag action
 * @return Property value after operation
 */
static bool ctune_UIConfig_BrowseTab_largeRowSize( ctune_UIConfig_t * cfg, ctune_Flag_e flag ) {
    switch( flag ) {
        case FLAG_SET_OFF: return ( cfg->browse_tab.large_rows = false );
        case FLAG_SET_ON : return ( cfg->browse_tab.large_rows = true );
        default          : return cfg->browse_tab.large_rows;
    }
}

/**
 * Namespace constructor
 */
const struct ctune_UIConfig_Namespace ctune_UIConfig = {
    .copy = &ctune_UIConfig_copy,

    .fav_tab = {
        .theming      = &ctune_UIConfig_FavTab_theming,
        .largeRowSize = &ctune_UIConfig_FavTab_largeRowSize,
    },

    .search_tab = {
        .largeRowSize = &ctune_UIConfig_SearchTab_largeRowSize,
    },

    .browse_tab = {
        .largeRowSize = &ctune_UIConfig_BrowseTab_largeRowSize,
    },
};