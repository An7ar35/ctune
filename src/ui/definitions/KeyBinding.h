#ifndef CTUNE_UI_KEYBINDING_H
#define CTUNE_UI_KEYBINDING_H

#include <stdbool.h>

#include "../datastructure/KeyBind.h"
#include "../datastructure/KeyInfo.h"
#include "../enum/InputKeyID.h"

typedef enum ctune_UI_KeyBinding_KeyEntryType {
    CTUNE_UI_KEYBINDING_TYPE_NORMAL,  //standard key-binding entry
    CTUNE_UI_KEYBINDING_TYPE_HIDDEN,  //for display formatting: hidden entry
    CTUNE_UI_KEYBINDING_TYPE_HEADING, //for display formatting: heading entry
    CTUNE_UI_KEYBINDING_TYPE_EMPTY    //for for display formatting: empty line
} ctune_UI_KeyEntryType_e;

/**
 * Binding between an contextual action, its description and key(s)
 * @param entry_type  Type of entry (key binding, display heading/spacing for the context help)
 * @param action      ActionID enum
 * @param description TextID for the action's description
 * @param key_binding Linked list of keys associated with the action
 */
typedef struct ctune_UI_KeyBinding {
    ctune_UI_KeyEntryType_e entry_type;
    ctune_UI_ActionID_e     action;
    ctune_UI_TextID_e       description;
    ctune_UI_InputKey_e     key;

} ctune_UI_KeyBinding_t;

/**
 * KeyBinding namespace
 */
extern const struct ctune_UI_KeyBinding_Instance {
    /**
     * Initialises the key bindings and loads into cache
     * @return Success
     */
    bool (* init)( void );

    /**
     * Gets action bound to an ncurses key
     * @param ctx         Action's context
     * @param ncurses_key Raw ncurses key
     * @return Action bound to key
     */
    ctune_UI_ActionID_e (* getAction)( ctune_UI_Context_e ctx, int ncurses_key );

    /**
     * Get a key's display text
     * @param keyID ctune_UI_InputKey_e enum type
     * @return Pointer to key's description string (NULL if error)
     */
    const char * (* getKeyText)( ctune_UI_InputKey_e keyID );

    /**
     * Iterates through the binding entries for a context calling the callback method for each
     * @param ctx      Context to process entries for
     * @param userdata Pointer to some userdata that will be passed to the callback (optional)
     * @param callback Method called for each entries
     * @return Success
     */
    bool (* processEntries)( ctune_UI_Context_e ctx, void * userdata, void (* callback)( const ctune_UI_KeyBinding_t * binding, void * userdata ) );

} ctune_UI_KeyBinding;

#endif //CTUNE_UI_KEYBINDING_H