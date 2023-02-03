#include "UIConfig.h"

#include <stdlib.h>

/**
 * Initialise fields in the struct
 * @return Initialised ctune_UIConfig_t object
 */
static ctune_UIConfig_t ctune_UIConfig_create( void ) {
    return (ctune_UIConfig_t) {
        .unicode_icons        = false,
        .mouse = {
           .enabled           = false,
           .interval_preset   = CTUNE_MOUSEINTERVAL_DEFAULT,
        },
        .fav_tab = {
            .large_rows       = true,
            .theme_favourites = true,
            .custom_theming   = false,
        },
        .browse_tab = {
            .large_rows       = true,
        },
        .search_tab = {
            .large_rows       = false,
        },
        .theme = {
            .preset           = CTUNE_UIPRESET_DEFAULT,
            .preset_pallet    = ctune_ColourTheme.init( CTUNE_UIPRESET_DEFAULT ),
            .custom_pallet    = ctune_ColourTheme.init( CTUNE_UIPRESET_DEFAULT ),
        },
    };
}

/**
 * Copies the content of a UIConfig object into another
 * @param from Origin of content to copy from
 * @param to   Destination to copy to
 * @return Success
 */
static bool ctune_UIConfig_copy( const ctune_UIConfig_t * from, ctune_UIConfig_t * to ) {
    if( from == NULL || to == NULL ) {
        return false;
    }

    to->mouse.enabled         = from->mouse.enabled;
    to->mouse.interval_preset = from->mouse.interval_preset;
    to->unicode_icons         = from->unicode_icons;
    to->fav_tab               = from->fav_tab;
    to->search_tab            = from->search_tab;
    to->browse_tab            = from->browse_tab;
    to->theme.preset          = from->theme.preset;
    to->theme.preset_pallet   = from->theme.preset_pallet;
    to->theme.custom_pallet   = from->theme.custom_pallet;

    return true;
}

/**
 * Gets the unicode icon flag property
 * @param cfg  Pointer to ctune_UIConfig_t object
 * @param flag Flag action
 * @return Flag state
 */
static bool ctune_UIConfig_unicodeIcons( ctune_UIConfig_t * cfg, ctune_Flag_e flag ) {
    if( cfg ) {
        switch( flag ) {
            case FLAG_SET_OFF: return ( cfg->unicode_icons = false );
            case FLAG_SET_ON : return ( cfg->unicode_icons = true );
            default          : return cfg->unicode_icons;
        }
    }

    return false;
}


/**
 * Set/Gets the mouse flag
 * @param cfg  Pointer to ctune_UIConfig_t object
 * @param flag Flag action
 * @return Property value after operation
 */
static bool ctune_UIConfig_mouse_enabled( ctune_UIConfig_t * cfg, ctune_Flag_e flag ) {
    if( cfg ) {
        switch( flag ) {
            case FLAG_SET_OFF: return ( cfg->mouse.enabled = false );
            case FLAG_SET_ON : return ( cfg->mouse.enabled = true );
            default          : return cfg->mouse.enabled;
        }
    }

    return false;
}

/**
 * Gets the mouse interval value
 * @param cfg Pointer to ctune_UIConfig_t object
 * @param Mouse Interval value in milliseconds between press and release to be recognised as a click
 */
static int ctune_UIConfig_mouse_clickIntervalResolution( ctune_UIConfig_t * cfg ) {
    if( cfg ) {
        return ctune_MouseInterval.value( cfg->mouse.interval_preset );
    }

    return -1;
}

/**
 * Gets the associated enum of the interval value stored
 * @param cfg Pointer to ctune_UIConfig_t object
 * @return MouseResolution enum
 */
static ctune_MouseInterval_e ctune_UIConfig_mouse_clickIntervalPreset( ctune_UIConfig_t * cfg ) {
    if( cfg ) {
        return cfg->mouse.interval_preset; //EARLY RETURN
    }

    return CTUNE_MOUSEINTERVAL_DEFAULT;
}

/**
 * Sets the mouse interval preset
 * @param cfg Pointer to ctune_UIConfig_t object
 * @param res MouseResolution preset
 * @param Success
 */
static bool ctune_UIConfig_mouse_setResolutionPreset( ctune_UIConfig_t * cfg, ctune_MouseInterval_e res ) {
    if( cfg && res < CTUNE_MOUSEINTERVAL_COUNT ) {
        cfg->mouse.interval_preset = res;
        return true; //EARLY RETURN
    }

    return false;
}

/**
 * Gets the current preset
 * @param cfg Pointer to ctune_UIConfig_t object
 * @return Theme preset
 */
static ctune_UIPreset_e ctune_UIConfig_theming_currentPreset( ctune_UIConfig_t * cfg ) {
    if( cfg ) {
        return cfg->theme.preset;
    }

    return CTUNE_UIPRESET_UNKNOWN;
}

/**
 * Gets currently active theme pallet
 * @param cfg Pointer to ctune_UIConfig_t object
 * @return Pointer to active theme pallet or NULL if the pointer to the config is NULL
 */
static struct ctune_ColourTheme * ctune_UIConfig_theming_getCurrentThemePallet( ctune_UIConfig_t * cfg ) {
    if( cfg ) {
        if( cfg->theme.preset == CTUNE_UIPRESET_CUSTOM ) {
            return &cfg->theme.custom_pallet; //EARLY RETURN
        }

        if( cfg->fav_tab.custom_theming ) {
            cfg->theme.preset_pallet.rows.favourite_local_fg  = cfg->theme.custom_pallet.rows.favourite_local_fg;
            cfg->theme.preset_pallet.rows.favourite_remote_fg = cfg->theme.custom_pallet.rows.favourite_remote_fg;
        }

        return &cfg->theme.preset_pallet;
    }

    return NULL;
}

/**
 * Sets the current active theme pallet
 * @param cfg Pointer to ctune_UIConfig_t object
 * @param preset ctune_UIPreset_e preset
 * @return Success
 */
static bool ctune_UIConfig_theming_setPreset( ctune_UIConfig_t * cfg, ctune_UIPreset_e preset ) {
    if( cfg && preset >= 0 && preset < CTUNE_UIPRESET_COUNT ) {
        cfg->theme.preset = preset;

        if( preset != CTUNE_UIPRESET_CUSTOM ) {
            cfg->theme.preset_pallet = ctune_ColourTheme.init( preset );

            if( cfg->fav_tab.custom_theming ) {
                cfg->theme.preset_pallet.rows.favourite_local_fg  = cfg->theme.custom_pallet.rows.favourite_local_fg;
                cfg->theme.preset_pallet.rows.favourite_remote_fg = cfg->theme.custom_pallet.rows.favourite_remote_fg;
            }
        }

        return true;
    }

    return false;
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
 * Get/Set "Favourites" tab custom theming for the station sources
 * @param cfg Pointer to ctune_UIConfig_t object
 * @param flag Flag action
 * @return Property value after operation
 */
static bool ctune_UIConfig_FavTab_customTheming( ctune_UIConfig_t * cfg, ctune_Flag_e flag ) {
    switch( flag ) {
        case FLAG_SET_OFF: return ( cfg->fav_tab.custom_theming = false );
        case FLAG_SET_ON : return ( cfg->fav_tab.custom_theming = true );
        default          : return cfg->fav_tab.custom_theming;
    }
}

/**
 * Get the custom colouring used for a station source in the "Favourites" tab
 * @param cfg         Pointer to ctune_UIConfig_t object
 * @param station_src ctune_StationSrc_e enum value
 * @return Colour value
 */
static short ctune_UIConfig_FavTab_getCustomThemingColour( ctune_UIConfig_t * cfg, ctune_StationSrc_e station_src ) {
    switch( station_src ) {
        case CTUNE_STATIONSRC_LOCAL       : { return cfg->theme.custom_pallet.rows.favourite_local_fg;  } break;
        case CTUNE_STATIONSRC_RADIOBROWSER: //fallthrough
        default                           : { return cfg->theme.custom_pallet.rows.favourite_remote_fg; } break;
    }
}

/**
 * Set the custom colouring used for a station source in the "Favourites" tab
 * @param cfg         Pointer to ctune_UIConfig_t object
 * @param station_src ctune_StationSrc_e enum value
 * @param colour      Colour code
 */
static void ctune_UIConfig_FavTab_setCustomThemingColour( ctune_UIConfig_t * cfg, ctune_StationSrc_e station_src, short colour_code ) {
    switch( station_src ) {
        case CTUNE_STATIONSRC_LOCAL       : { cfg->theme.custom_pallet.rows.favourite_local_fg = colour_code;  } break;
        case CTUNE_STATIONSRC_RADIOBROWSER: //fallthrough
        default                           : { cfg->theme.custom_pallet.rows.favourite_remote_fg = colour_code; } break;
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
    .create       = &ctune_UIConfig_create,
    .copy         = &ctune_UIConfig_copy,
    .unicodeIcons = &ctune_UIConfig_unicodeIcons,

    .mouse = {
        .enabled                 = &ctune_UIConfig_mouse_enabled,
        .clickIntervalResolution = &ctune_UIConfig_mouse_clickIntervalResolution,
        .clickIntervalPreset     = &ctune_UIConfig_mouse_clickIntervalPreset,
        .setResolutionPreset     = &ctune_UIConfig_mouse_setResolutionPreset,
    },
    .theming = {
        .currentPreset           = &ctune_UIConfig_theming_currentPreset,
        .getCurrentThemePallet   = &ctune_UIConfig_theming_getCurrentThemePallet,
        .setPreset               = &ctune_UIConfig_theming_setPreset,
    },

    .fav_tab = {
        .theming                 = &ctune_UIConfig_FavTab_theming,
        .customTheming           = &ctune_UIConfig_FavTab_customTheming,
        .getCustomThemingColour  = &ctune_UIConfig_FavTab_getCustomThemingColour,
        .setCustomThemingColour  = &ctune_UIConfig_FavTab_setCustomThemingColour,
        .largeRowSize            = &ctune_UIConfig_FavTab_largeRowSize,
    },

    .search_tab = {
        .largeRowSize            = &ctune_UIConfig_SearchTab_largeRowSize,
    },

    .browse_tab = {
        .largeRowSize            = &ctune_UIConfig_BrowseTab_largeRowSize,
    },
};