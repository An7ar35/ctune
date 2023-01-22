#ifndef CTUNE_UI_DEFINITIONS_ICONS_H
#define CTUNE_UI_DEFINITIONS_ICONS_H

#include <stdbool.h>
#include <string.h>

#include "../enum/IconID.h"

typedef enum {
    CTUNE_UI_ICONCATEGORY_HPROGRESS,
    CTUNE_UI_ICONCATEGORY_VPROGRESS,
} ctune_UI_IconCategory_e;

/**
 * Icon string access for UI display
 */
extern const struct ctune_UI_Icons_Instance {
    /**
     * Sets the internal flag for unicode icon support
     * @param flag Unicode icon flag
     */
    void (* setUnicode)( bool flag );

    /**
     * Gets the internal flag for unicode icon support
     * @return Unicode icon flag value
     */
    bool (* unicodeState)( void );

    /**
     * Gets the character string for a display icon
     * @param text_id Icon ID enum
     * @return Icon string associated with given enum
     */
    const char * (* icon)( ctune_UI_IconID_e icon_id );

    /**
     * Converts a percentage to a a progress "eighth" (use to work out which progress icon to use)
     * @param percent Percent value (0-100)
     * @return Progress range 0-9 value (-1 on error)
     */
    int (* percentTo8th)( int percent );

    /**
     * Gets the character string for a progress icon
     * @param val_8th 8th value to get an icon for
     * @param icon_type Progress icon type
     * @return Icon string associated with value and type (empty on error)
     */
    const char * (* progressIcon)( int val_8th, ctune_UI_IconCategory_e icon_type );

} ctune_UI_Icons;

#endif //CTUNE_UI_DEFINITIONS_ICONS_H