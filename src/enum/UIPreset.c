#include "UIPreset.h"

#include <assert.h>
#include <string.h>

static const char * PRESET_NAMES[CTUNE_UIPRESET_COUNT] = {
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
static const char ** ctune_UIPreset_presetList( void ) {
    return &PRESET_NAMES[0];
}

/**
 * Gets string for enum
 * @param e UITheme enum
 * @return String
 */
static const char * ctune_UIPreset_str( ctune_UIPreset_e e ) {
    assert( (int) e >= 0 && (int) e < CTUNE_UIPRESET_COUNT );
    return PRESET_NAMES[ (int) e ];
}

/**
 * Convert a UITheme name string to its enum value
 * @param str String to convert
 * @return Associated enum or `CTUNE_UIPRESET_UNKNOWN`
 */
static ctune_UIPreset_e ctune_UIPreset_toEnum( const char * str ) {
    for( ctune_UIPreset_e i = 0; i < CTUNE_UIPRESET_COUNT; ++i ) {
        if( strcmp( str, PRESET_NAMES[i] ) == 0 ) {
            return i;
        }
    }

    return CTUNE_UIPRESET_UNKNOWN;
}

/**
 * Namespace constructor
 */
const struct ctune_UIPreset_Namespace ctune_UIPreset = {
    .presetList  = &ctune_UIPreset_presetList,
    .str         = &ctune_UIPreset_str,
    .toEnum      = &ctune_UIPreset_toEnum,
};