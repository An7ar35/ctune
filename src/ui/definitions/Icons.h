#ifndef CTUNE_UI_DEFINITIONS_ICONS_H
#define CTUNE_UI_DEFINITIONS_ICONS_H

#include <stdbool.h>
#include <string.h>

#include "../enum/IconID.h"

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
     * Gets the character string for a display icon
     * @param text_id Icon ID enum
     * @return Icon string associated with given enum
     */
    const char * (* icon)( ctune_UI_IconID_e icon_id );

} ctune_UI_Icons;

#endif //CTUNE_UI_DEFINITIONS_ICONS_H