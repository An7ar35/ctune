#ifndef CTUNE_UI_DEFINITIONS_KEYDESCRIPTION_H
#define CTUNE_UI_DEFINITIONS_KEYDESCRIPTION_H

#include "../enum/InputKeyID.h"

#define CTUNE_KEY_TAB             9
#define CTUNE_KEY_RETURN         10
#define CTUNE_KEY_ESC            27
#define CTUNE_KEY_SPACE          32
#define CTUNE_KEY_BACKSPACE     127
#define CTUNE_KEY_ALT_BACKSPACE 263
#define CTUNE_KEY_STAB          353

/**
 * Container for keyboard key properties
 * @param ncurses_key NCurses key integer
 * @param text        Display text of the key
 */
typedef struct ctune_UI_KeyDescription {
    int          ncurses_key;
    const char * text;
} ctune_UI_KeyDescription_t;

/**
 * KeyDescription namespace
 */
extern const struct ctune_UI_KeyDescription_Namespace {
    /**
     * Get a key's display text
     * @param keyID ctune_UI_InputKey_e enum type
     * @return Pointer to key's description string (NULL if error)
     */
    const char * (* getKeyText)( ctune_UI_InputKey_e keyID );

    /**
     * Gets the raw ncurses key associated with the KeyID
     * @param keyID ctune_UI_InputKey_e enum type
     * @return NCurses key integer
     */
    int (* getKeyRaw)( ctune_UI_InputKey_e keyID );

    /**
     * Get the key ID from an ncurses key
     * @param ncurses_key NCurses key
     * @return ctune_UI_InputKey_e enum type
     */
    ctune_UI_InputKey_e (* getKeyID)( int ncurses_key );

} ctune_UI_KeyDescription;

#endif //CTUNE_UI_DEFINITIONS_KEYDESCRIPTION_H
