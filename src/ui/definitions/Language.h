#ifndef CTUNE_LANGUAGE_H
#define CTUNE_LANGUAGE_H

#include <string.h>

#include "../enum/TextID.h"

/**
 * Language string access for UI display
 *
 * Note: the implementation could be modified to load language-specific
 * strings for the various UI element at some point in the future.
 * Perhaps for a future release if I can be bothered..? /EAD
 */
extern const struct ctune_UI_Language_Instance {
    /**
     * Gets the string for a piece of display text
     * @param text_id Text ID enum
     * @return String associated with given enum
     */
    const char * (* text)( ctune_UI_TextID_e text_id );

} ctune_UI_Language;

#endif //CTUNE_LANGUAGE_H
