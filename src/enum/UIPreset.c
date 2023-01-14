#include "UIPreset.h"

#include <assert.h>
#include <string.h>

static const char * THEME_NAMES[CTUNE_UIPRESET_COUNT] = {
    [CTUNE_UIPRESET_DEFAULT  ] = "default",
    [CTUNE_UIPRESET_HACKERMAN] = "hackerman",
    [CTUNE_UIPRESET_REDZONE  ] = "red-zone",
    [CTUNE_UIPRESET_DEEPBLUE ] = "deep-blue",
    //<- insert new template entries here
    [CTUNE_UIPRESET_CUSTOM   ] = "custom",
};

/**
 * Gets the list of UI theme presets
 * @return Pointer to first item in list
 */
static const char ** ctune_UITheme_presetList( void ) {
    return &THEME_NAMES[0];
}

/**
 * Gets string for enum
 * @param e UITheme enum
 * @return String
 */
static const char * ctune_UITheme_str( ctune_UIPreset_e e ) {
    assert( (int) e >= 0 && (int) e < CTUNE_UIPRESET_COUNT );
    return THEME_NAMES[ (int) e ];
}

/**
 * Convert a UITheme name string to its enum value
 * @param str String to convert
 * @return Associated enum or `CTUNE_UIPRESET_UNKNOWN`
 */
static ctune_UIPreset_e ctune_UITheme_toEnum( const char * str ) {
    for( ctune_UIPreset_e i = 0; i < CTUNE_UIPRESET_COUNT; ++i ) {
        if( strcmp( str, THEME_NAMES[i] ) == 0 ) {
            return i;
        }
    }

    return CTUNE_UIPRESET_UNKNOWN;
}

/**
 * Namespace constructor
 */
const struct ctune_UIPreset_Namespace ctune_UIPreset = {
    .presetList = &ctune_UITheme_presetList,
    .str        = &ctune_UITheme_str,
    .toEnum     = &ctune_UITheme_toEnum,
};