#ifndef CTUNE_ENUM_UIPRESET_H
#define CTUNE_ENUM_UIPRESET_H

#include <stdbool.h>

typedef enum {
    CTUNE_UIPRESET_UNKNOWN = -1,
    CTUNE_UIPRESET_DEFAULT =  0,
    CTUNE_UIPRESET_HACKERMAN,
    CTUNE_UIPRESET_REDZONE,
    CTUNE_UIPRESET_DEEPBLUE,
    //<- insert new template enums here
    CTUNE_UIPRESET_CUSTOM,
    CTUNE_UIPRESET_COUNT
} ctune_UIPreset_e;

typedef struct {
    ctune_UIPreset_e id;
    const char *     name;
} ctune_UIPreset_t;

extern const struct ctune_UIPreset_Namespace {
    /**
     * Gets the list of UI theme presets
     * @return Pointer to first item in list
     */
    const char ** (* presetList)( void );

    /**
     * Gets string for enum
     * @param e UITheme enum
     * @return String
     */
    const char * (* str)( ctune_UIPreset_e e );

    /**
     * Convert a UITheme name string to its enum value
     * @param str String to convert
     * @return Associated enum or `CTUNE_UIPRESET_UNKNOWN`
     */
    ctune_UIPreset_e (* toEnum)( const char * str );

} ctune_UIPreset;

#endif //CTUNE_ENUM_UIPRESET_H
