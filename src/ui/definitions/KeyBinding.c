#include "KeyBinding.h"

#include <string.h>
#include <ncurses.h>

#include "KeyDescription.h"
#include "../../logger/Logger.h"

/**
 * [PRIVATE] Key mapping cache between ncurses key and the associated action (int -> ctune_UI_ActionID_e)
 */
static ctune_UI_ActionID_e keymap_cache[CTUNE_UI_CTX_COUNT][KEY_MAX] = { { CTUNE_UI_ACTION_NONE } };

/**
 * [PRIVATE] Generic key bindings
 */
const ctune_UI_KeyBinding_t bindings_main[] = {
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_HIDDEN, .action = CTUNE_UI_ACTION_RESIZE, .description = CTUNE_UI_TEXT_BLANK, .key = CTUNE_UI_KEYBOARD_RESIZE },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_GO_BACK, .description = CTUNE_UI_TEXT_HELP_ESC, .key = CTUNE_UI_KEYBOARD_ESC },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_HELP, .description = CTUNE_UI_TEXT_HELP_KEY, .key = CTUNE_UI_KEYBOARD_F1 },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_QUIT, .description = CTUNE_UI_TEXT_HELP_QUIT, .key = CTUNE_UI_KEYBOARD_Q },
};

/**
 * [PRIVATE] Key bindings for the favourites tab
 */
const ctune_UI_KeyBinding_t bindings_fav_tab[] = {
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_HIDDEN, .action = CTUNE_UI_ACTION_RESIZE, .description = CTUNE_UI_TEXT_BLANK, .key = CTUNE_UI_KEYBOARD_RESIZE },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_HIDDEN, .action = CTUNE_UI_ACTION_GO_LEFT, .description = CTUNE_UI_TEXT_BLANK, .key = CTUNE_UI_KEYBOARD_LEFT },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_GO_BACK, .description = CTUNE_UI_TEXT_HELP_ESC, .key = CTUNE_UI_KEYBOARD_ESC },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_HELP, .description = CTUNE_UI_TEXT_HELP_KEY, .key = CTUNE_UI_KEYBOARD_F1 },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_TAB1, .description = CTUNE_UI_TEXT_HELP_CHANGE_TAB_1, .key = CTUNE_UI_KEYBOARD_1 },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_TAB2, .description = CTUNE_UI_TEXT_HELP_CHANGE_TAB_2, .key = CTUNE_UI_KEYBOARD_2 },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_TAB3, .description = CTUNE_UI_TEXT_HELP_CHANGE_TAB_3, .key = CTUNE_UI_KEYBOARD_3 },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_PLAY, .description = CTUNE_UI_TEXT_HELP_PLAYBACK_START, .key = CTUNE_UI_KEYBOARD_P },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_TRIGGER, .description = CTUNE_UI_TEXT_HELP_PLAYBACK_START, .key = CTUNE_UI_KEYBOARD_RETURN },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_RESUME, .description = CTUNE_UI_TEXT_HELP_PLAYBACK_RESUME, .key = CTUNE_UI_KEYBOARD_R },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_STOP, .description = CTUNE_UI_TEXT_HELP_PLAYBACK_STOP, .key = CTUNE_UI_KEYBOARD_S },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_QUIT, .description = CTUNE_UI_TEXT_HELP_QUIT, .key = CTUNE_UI_KEYBOARD_Q },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_VOLUP, .description = CTUNE_UI_TEXT_HELP_VOL_UP, .key = CTUNE_UI_KEYBOARD_PLUS },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_VOLDOWN, .description = CTUNE_UI_TEXT_HELP_VOL_DOWN, .key = CTUNE_UI_KEYBOARD_MINUS },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_FAV, .description = CTUNE_UI_TEXT_HELP_TOGGLE_FAV, .key = CTUNE_UI_KEYBOARD_F },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_RSI_QUEUED, .description = CTUNE_UI_TEXT_HELP_OPEN_RSINFO_QUEUED, .key = CTUNE_UI_KEYBOARD_CTRL_I },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_FIND, .description = CTUNE_UI_TEXT_HELP_OPEN_RSFIND_FORM, .key = CTUNE_UI_KEYBOARD_CTRL_F },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_NEW, .description = CTUNE_UI_TEXT_HELP_OPEN_RSEDIT_FORM_NEW, .key = CTUNE_UI_KEYBOARD_N },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_EDIT, .description = CTUNE_UI_TEXT_HELP_OPEN_RSEDIT_FORM_EDIT, .key = CTUNE_UI_KEYBOARD_E },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_OPTIONS, .description = CTUNE_UI_TEXT_HELP_OPEN_OPTIONS, .key = CTUNE_UI_KEYBOARD_O },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_EMPTY, .action = CTUNE_UI_ACTION_NONE, .description = CTUNE_UI_TEXT_BLANK, .key = CTUNE_UI_KEYBOARD_NONE },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_RSI_SELECTED, .description = CTUNE_UI_TEXT_HELP_OPEN_RSINFO_SELECTED, .key = CTUNE_UI_KEYBOARD_I },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_GO_RIGHT, .description = CTUNE_UI_TEXT_HELP_OPEN_RSINFO_SELECTED, .key = CTUNE_UI_KEYBOARD_RIGHT },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_SELECT_PREV, .description = CTUNE_UI_TEXT_HELP_PREV_ENTRY, .key = CTUNE_UI_KEYBOARD_UP },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_SELECT_NEXT, .description = CTUNE_UI_TEXT_HELP_NEXT_ENTRY, .key = CTUNE_UI_KEYBOARD_DOWN },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_PAGE_UP, .description = CTUNE_UI_TEXT_HELP_PREV_ENTRY_PAGE, .key = CTUNE_UI_KEYBOARD_PAGEUP },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_PAGE_DOWN, .description = CTUNE_UI_TEXT_HELP_NEXT_ENTRY_PAGE, .key = CTUNE_UI_KEYBOARD_PAGEDOWN },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_SELECT_FIRST, .description = CTUNE_UI_TEXT_HELP_FIRST_ENTRY, .key = CTUNE_UI_KEYBOARD_HOME },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_SELECT_LAST, .description = CTUNE_UI_TEXT_HELP_LAST_ENTRY, .key = CTUNE_UI_KEYBOARD_END },
};

/**
 * [PRIVATE] Key bindings for the search tab
 */
static const ctune_UI_KeyBinding_t bindings_search_tab[] = {
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_HIDDEN, .action = CTUNE_UI_ACTION_RESIZE, .description = CTUNE_UI_TEXT_BLANK, .key = CTUNE_UI_KEYBOARD_RESIZE },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_HIDDEN, .action = CTUNE_UI_ACTION_GO_LEFT, .description = CTUNE_UI_TEXT_BLANK, .key = CTUNE_UI_KEYBOARD_LEFT },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_GO_BACK, .description = CTUNE_UI_TEXT_HELP_ESC, .key = CTUNE_UI_KEYBOARD_ESC },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_HELP, .description = CTUNE_UI_TEXT_HELP_KEY, .key = CTUNE_UI_KEYBOARD_F1 },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_TAB1, .description = CTUNE_UI_TEXT_HELP_CHANGE_TAB_1, .key = CTUNE_UI_KEYBOARD_1 },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_TAB2, .description = CTUNE_UI_TEXT_HELP_CHANGE_TAB_2, .key = CTUNE_UI_KEYBOARD_2 },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_TAB3, .description = CTUNE_UI_TEXT_HELP_CHANGE_TAB_3, .key = CTUNE_UI_KEYBOARD_3 },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_PLAY, .description = CTUNE_UI_TEXT_HELP_PLAYBACK_START, .key = CTUNE_UI_KEYBOARD_P },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_TRIGGER, .description = CTUNE_UI_TEXT_HELP_PLAYBACK_START, .key = CTUNE_UI_KEYBOARD_RETURN },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_RESUME, .description = CTUNE_UI_TEXT_HELP_PLAYBACK_RESUME, .key = CTUNE_UI_KEYBOARD_R },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_STOP, .description = CTUNE_UI_TEXT_HELP_PLAYBACK_STOP, .key = CTUNE_UI_KEYBOARD_S },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_QUIT, .description = CTUNE_UI_TEXT_HELP_QUIT, .key = CTUNE_UI_KEYBOARD_Q },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_VOLUP, .description = CTUNE_UI_TEXT_HELP_VOL_UP, .key = CTUNE_UI_KEYBOARD_PLUS },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_VOLDOWN, .description = CTUNE_UI_TEXT_HELP_VOL_DOWN, .key = CTUNE_UI_KEYBOARD_MINUS },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_FAV, .description = CTUNE_UI_TEXT_HELP_TOGGLE_FAV, .key = CTUNE_UI_KEYBOARD_F },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_RSI_QUEUED, .description = CTUNE_UI_TEXT_HELP_OPEN_RSINFO_QUEUED, .key = CTUNE_UI_KEYBOARD_CTRL_I },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_FIND, .description = CTUNE_UI_TEXT_HELP_OPEN_RSFIND_FORM, .key = CTUNE_UI_KEYBOARD_CTRL_F },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_OPTIONS, .description = CTUNE_UI_TEXT_HELP_OPEN_OPTIONS, .key = CTUNE_UI_KEYBOARD_O },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_EMPTY, .action = CTUNE_UI_ACTION_NONE, .description = CTUNE_UI_TEXT_BLANK, .key = CTUNE_UI_KEYBOARD_NONE },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_RSI_SELECTED, .description = CTUNE_UI_TEXT_HELP_OPEN_RSINFO_SELECTED, .key = CTUNE_UI_KEYBOARD_I },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_GO_RIGHT, .description = CTUNE_UI_TEXT_BLANK, .key = CTUNE_UI_KEYBOARD_RIGHT },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_SELECT_PREV, .description = CTUNE_UI_TEXT_HELP_PREV_ENTRY, .key = CTUNE_UI_KEYBOARD_UP },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_SELECT_NEXT, .description = CTUNE_UI_TEXT_HELP_NEXT_ENTRY, .key = CTUNE_UI_KEYBOARD_DOWN },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_PAGE_UP, .description = CTUNE_UI_TEXT_HELP_PREV_ENTRY_PAGE, .key = CTUNE_UI_KEYBOARD_PAGEUP },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_PAGE_DOWN, .description = CTUNE_UI_TEXT_HELP_NEXT_ENTRY_PAGE, .key = CTUNE_UI_KEYBOARD_PAGEDOWN },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_SELECT_FIRST, .description = CTUNE_UI_TEXT_HELP_FIRST_ENTRY, .key = CTUNE_UI_KEYBOARD_HOME },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_SELECT_LAST, .description = CTUNE_UI_TEXT_HELP_LAST_ENTRY, .key = CTUNE_UI_KEYBOARD_END },
};

/**
 * [PRIVATE] Key bindings for the browser tab
 */
static const ctune_UI_KeyBinding_t bindings_browse_tab[] = {
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_HIDDEN, .action = CTUNE_UI_ACTION_RESIZE, .description = CTUNE_UI_TEXT_BLANK, .key = CTUNE_UI_KEYBOARD_RESIZE },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_HIDDEN, .action = CTUNE_UI_ACTION_GO_LEFT, .description = CTUNE_UI_TEXT_BLANK, .key = CTUNE_UI_KEYBOARD_LEFT },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_HIDDEN, .action = CTUNE_UI_ACTION_GO_RIGHT, .description = CTUNE_UI_TEXT_BLANK, .key = CTUNE_UI_KEYBOARD_RIGHT },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_GO_BACK, .description = CTUNE_UI_TEXT_HELP_ESC, .key = CTUNE_UI_KEYBOARD_ESC },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_HELP, .description = CTUNE_UI_TEXT_HELP_KEY, .key = CTUNE_UI_KEYBOARD_F1 },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_TAB1, .description = CTUNE_UI_TEXT_HELP_CHANGE_TAB_1, .key = CTUNE_UI_KEYBOARD_1 },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_TAB2, .description = CTUNE_UI_TEXT_HELP_CHANGE_TAB_2, .key = CTUNE_UI_KEYBOARD_2 },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_TAB3, .description = CTUNE_UI_TEXT_HELP_CHANGE_TAB_3, .key = CTUNE_UI_KEYBOARD_3 },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_PLAY, .description = CTUNE_UI_TEXT_HELP_PLAYBACK_START, .key = CTUNE_UI_KEYBOARD_P },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_TRIGGER, .description = CTUNE_UI_TEXT_HELP_PLAYBACK_START, .key = CTUNE_UI_KEYBOARD_RETURN },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_RESUME, .description = CTUNE_UI_TEXT_HELP_PLAYBACK_RESUME, .key = CTUNE_UI_KEYBOARD_R },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_STOP, .description = CTUNE_UI_TEXT_HELP_PLAYBACK_STOP, .key = CTUNE_UI_KEYBOARD_S },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_QUIT, .description = CTUNE_UI_TEXT_HELP_QUIT, .key = CTUNE_UI_KEYBOARD_Q },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_VOLUP, .description = CTUNE_UI_TEXT_HELP_VOL_UP, .key = CTUNE_UI_KEYBOARD_PLUS },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_VOLDOWN, .description = CTUNE_UI_TEXT_HELP_VOL_DOWN, .key = CTUNE_UI_KEYBOARD_MINUS },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_FAV, .description = CTUNE_UI_TEXT_HELP_TOGGLE_FAV, .key = CTUNE_UI_KEYBOARD_F },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_RSI_QUEUED, .description = CTUNE_UI_TEXT_HELP_OPEN_RSINFO_QUEUED, .key = CTUNE_UI_KEYBOARD_CTRL_I },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_FIND, .description = CTUNE_UI_TEXT_HELP_OPEN_RSFIND_FORM, .key = CTUNE_UI_KEYBOARD_CTRL_F },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_OPTIONS, .description = CTUNE_UI_TEXT_HELP_OPEN_OPTIONS, .key = CTUNE_UI_KEYBOARD_O },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_EMPTY, .action = CTUNE_UI_ACTION_NONE, .description = CTUNE_UI_TEXT_BLANK, .key = CTUNE_UI_KEYBOARD_NONE },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_RSI_SELECTED, .description = CTUNE_UI_TEXT_HELP_OPEN_RSINFO_SELECTED, .key = CTUNE_UI_KEYBOARD_I },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_SELECT_PREV, .description = CTUNE_UI_TEXT_HELP_PREV_ENTRY, .key = CTUNE_UI_KEYBOARD_UP },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_SELECT_NEXT, .description = CTUNE_UI_TEXT_HELP_NEXT_ENTRY, .key = CTUNE_UI_KEYBOARD_DOWN },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_PAGE_UP, .description = CTUNE_UI_TEXT_HELP_PREV_ENTRY_PAGE, .key = CTUNE_UI_KEYBOARD_PAGEUP },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_PAGE_DOWN, .description = CTUNE_UI_TEXT_HELP_NEXT_ENTRY_PAGE, .key = CTUNE_UI_KEYBOARD_PAGEDOWN },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_SELECT_FIRST, .description = CTUNE_UI_TEXT_HELP_FIRST_ENTRY, .key = CTUNE_UI_KEYBOARD_HOME },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_SELECT_LAST, .description = CTUNE_UI_TEXT_HELP_LAST_ENTRY, .key = CTUNE_UI_KEYBOARD_END },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_FOCUS_RIGHT, .description = CTUNE_UI_TEXT_HELP_FOCUS_RIGHT, .key = CTUNE_UI_KEYBOARD_TAB },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_FOCUS_LEFT, .description = CTUNE_UI_TEXT_HELP_FOCUS_LEFT, .key = CTUNE_UI_KEYBOARD_STAB },
};

/**
 * [PRIVATE] Key bindings for the RSInfo dialog
 */
static const ctune_UI_KeyBinding_t bindings_rsinfo[] = {
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_HIDDEN, .action = CTUNE_UI_ACTION_RESIZE, .description = CTUNE_UI_TEXT_BLANK, .key = CTUNE_UI_KEYBOARD_RESIZE },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_HELP, .description = CTUNE_UI_TEXT_HELP_KEY, .key = CTUNE_UI_KEYBOARD_F1 },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_ESC, .description = CTUNE_UI_TEXT_HELP_FORM_ESC, .key = CTUNE_UI_KEYBOARD_ESC },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_SCROLL_HOME, .description = CTUNE_UI_TEXT_HELP_SCROLL_HOME, .key = CTUNE_UI_KEYBOARD_HOME },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_SCROLL_LEFT, .description = CTUNE_UI_TEXT_HELP_SCROLL_LEFT, .key = CTUNE_UI_KEYBOARD_LEFT },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_SCROLL_RIGHT, .description = CTUNE_UI_TEXT_HELP_SCROLL_RIGHT, .key = CTUNE_UI_KEYBOARD_RIGHT },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_SCROLL_UP, .description = CTUNE_UI_TEXT_HELP_SCROLL_UP, .key = CTUNE_UI_KEYBOARD_UP },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_SCROLL_DOWN, .description = CTUNE_UI_TEXT_HELP_SCROLL_DOWN, .key = CTUNE_UI_KEYBOARD_DOWN },
};

/**
 * [PRIVATE] Key bindings for the RSFind dialog
 */
static const ctune_UI_KeyBinding_t bindings_rsfind[] = {
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_HIDDEN, .action = CTUNE_UI_ACTION_RESIZE, .description = CTUNE_UI_TEXT_BLANK, .key = CTUNE_UI_KEYBOARD_RESIZE },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_HIDDEN, .action = CTUNE_UI_ACTION_GO_LEFT, .description = CTUNE_UI_TEXT_BLANK, .key = CTUNE_UI_KEYBOARD_LEFT },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_HIDDEN, .action = CTUNE_UI_ACTION_GO_RIGHT, .description = CTUNE_UI_TEXT_BLANK, .key = CTUNE_UI_KEYBOARD_RIGHT },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_HIDDEN, .action = CTUNE_UI_ACTION_DEL_PREV, .description = CTUNE_UI_TEXT_BLANK, .key = CTUNE_UI_KEYBOARD_BACKSPACE },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_HIDDEN, .action = CTUNE_UI_ACTION_DEL_PREV, .description = CTUNE_UI_TEXT_BLANK, .key = CTUNE_UI_KEYBOARD_ALT_BACKSPACE },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_HIDDEN, .action = CTUNE_UI_ACTION_DEL_NEXT, .description = CTUNE_UI_TEXT_BLANK, .key = CTUNE_UI_KEYBOARD_DELCHAR },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_ESC, .description = CTUNE_UI_TEXT_HELP_FORM_ESC, .key = CTUNE_UI_KEYBOARD_ESC },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_HELP, .description = CTUNE_UI_TEXT_HELP_KEY, .key = CTUNE_UI_KEYBOARD_F1 },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_CLEAR_ALL, .description = CTUNE_UI_TEXT_HELP_CLEAR_ALL_FIELDS, .key = CTUNE_UI_KEYBOARD_F5 },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_CLEAR_SELECTED, .description = CTUNE_UI_TEXT_HELP_CLEAR_CURR_FIELD, .key = CTUNE_UI_KEYBOARD_CTRL_D },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_FIELD_BEGIN, .description = CTUNE_UI_TEXT_HELP_FIELD_BEGIN, .key = CTUNE_UI_KEYBOARD_HOME },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_FIELD_END, .description = CTUNE_UI_TEXT_HELP_FIELD_END, .key = CTUNE_UI_KEYBOARD_END },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_FIELD_FIRST, .description = CTUNE_UI_TEXT_HELP_FIRST_FIELD, .key = CTUNE_UI_KEYBOARD_PAGEUP },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_FIELD_LAST, .description = CTUNE_UI_TEXT_HELP_LAST_FIELD, .key = CTUNE_UI_KEYBOARD_PAGEDOWN },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_FIELD_PREV, .description = CTUNE_UI_TEXT_HELP_PREV_FIELD, .key = CTUNE_UI_KEYBOARD_UP },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_HIDDEN, .action = CTUNE_UI_ACTION_FIELD_PREV, .description = CTUNE_UI_TEXT_HELP_PREV_FIELD, .key = CTUNE_UI_KEYBOARD_STAB },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_FIELD_NEXT, .description = CTUNE_UI_TEXT_HELP_NEXT_FIELD, .key = CTUNE_UI_KEYBOARD_DOWN },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_HIDDEN, .action = CTUNE_UI_ACTION_FIELD_NEXT, .description = CTUNE_UI_TEXT_HELP_NEXT_FIELD, .key = CTUNE_UI_KEYBOARD_TAB },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_TRIGGER, .description = CTUNE_UI_TEXT_HELP_FIELD_RETURN, .key = CTUNE_UI_KEYBOARD_RETURN },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_TOGGLE, .description = CTUNE_UI_TEXT_HELP_FIELD_TOGGLE, .key = CTUNE_UI_KEYBOARD_X },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_TOGGLE_ALT, .description = CTUNE_UI_TEXT_HELP_FIELD_TOGGLE_ALT, .key = CTUNE_UI_KEYBOARD_SPACE },
};

/**
 * [PRIVATE] Key bindings for the RSEdit dialog
 */
static const ctune_UI_KeyBinding_t bindings_rsedit[] = {
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_HIDDEN, .action = CTUNE_UI_ACTION_RESIZE, .description = CTUNE_UI_TEXT_BLANK, .key = CTUNE_UI_KEYBOARD_RESIZE },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_HIDDEN, .action = CTUNE_UI_ACTION_DEL_PREV, .description = CTUNE_UI_TEXT_BLANK, .key = CTUNE_UI_KEYBOARD_BACKSPACE },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_HIDDEN, .action = CTUNE_UI_ACTION_DEL_PREV, .description = CTUNE_UI_TEXT_BLANK, .key = CTUNE_UI_KEYBOARD_ALT_BACKSPACE },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_HIDDEN, .action = CTUNE_UI_ACTION_DEL_NEXT, .description = CTUNE_UI_TEXT_BLANK, .key = CTUNE_UI_KEYBOARD_DELCHAR },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_ESC, .description = CTUNE_UI_TEXT_HELP_FORM_ESC, .key = CTUNE_UI_KEYBOARD_ESC },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_HELP, .description = CTUNE_UI_TEXT_HELP_KEY, .key = CTUNE_UI_KEYBOARD_F1 },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_CLEAR_ALL, .description = CTUNE_UI_TEXT_HELP_CLEAR_ALL_FIELDS, .key = CTUNE_UI_KEYBOARD_F5 },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_CLEAR_SELECTED, .description = CTUNE_UI_TEXT_HELP_CLEAR_CURR_FIELD, .key = CTUNE_UI_KEYBOARD_CTRL_D },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_FIELD_BEGIN, .description = CTUNE_UI_TEXT_HELP_FIELD_BEGIN, .key = CTUNE_UI_KEYBOARD_HOME },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_FIELD_END, .description = CTUNE_UI_TEXT_HELP_FIELD_END, .key = CTUNE_UI_KEYBOARD_END },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_FIELD_FIRST, .description = CTUNE_UI_TEXT_HELP_FIRST_FIELD, .key = CTUNE_UI_KEYBOARD_PAGEUP },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_FIELD_LAST, .description = CTUNE_UI_TEXT_HELP_LAST_FIELD, .key = CTUNE_UI_KEYBOARD_PAGEDOWN },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_FIELD_PREV, .description = CTUNE_UI_TEXT_HELP_PREV_FIELD, .key = CTUNE_UI_KEYBOARD_UP },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_HIDDEN, .action = CTUNE_UI_ACTION_FIELD_PREV, .description = CTUNE_UI_TEXT_HELP_PREV_FIELD, .key = CTUNE_UI_KEYBOARD_STAB },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_FIELD_NEXT, .description = CTUNE_UI_TEXT_HELP_NEXT_FIELD, .key = CTUNE_UI_KEYBOARD_DOWN },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_HIDDEN, .action = CTUNE_UI_ACTION_FIELD_NEXT, .description = CTUNE_UI_TEXT_HELP_NEXT_FIELD, .key = CTUNE_UI_KEYBOARD_TAB },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_TRIGGER, .description = CTUNE_UI_TEXT_HELP_FIELD_RETURN, .key = CTUNE_UI_KEYBOARD_RETURN },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_TOGGLE_ALT, .description = CTUNE_UI_TEXT_HELP_FIELD_TOGGLE_ALT, .key = CTUNE_UI_KEYBOARD_SPACE },
};

static const ctune_UI_KeyBinding_t bindings_options_menu[] = {
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_HIDDEN, .action = CTUNE_UI_ACTION_RESIZE, .description = CTUNE_UI_TEXT_BLANK, .key = CTUNE_UI_KEYBOARD_RESIZE },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_ESC, .description = CTUNE_UI_TEXT_HELP_FORM_ESC, .key = CTUNE_UI_KEYBOARD_ESC },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_PAGE_UP, .description = CTUNE_UI_TEXT_HELP_PREV_ENTRY_PAGE, .key = CTUNE_UI_KEYBOARD_PAGEUP },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_PAGE_DOWN, .description = CTUNE_UI_TEXT_HELP_NEXT_ENTRY_PAGE, .key = CTUNE_UI_KEYBOARD_PAGEDOWN },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_SELECT_FIRST, .description = CTUNE_UI_TEXT_HELP_FIRST_ENTRY, .key = CTUNE_UI_KEYBOARD_HOME },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_SELECT_LAST, .description = CTUNE_UI_TEXT_HELP_LAST_ENTRY, .key = CTUNE_UI_KEYBOARD_END },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_SELECT_PREV, .description = CTUNE_UI_TEXT_HELP_PREV_ENTRY, .key = CTUNE_UI_KEYBOARD_UP },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_SELECT_NEXT, .description = CTUNE_UI_TEXT_HELP_NEXT_ENTRY, .key = CTUNE_UI_KEYBOARD_DOWN },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_GO_LEFT, .description = CTUNE_UI_TEXT_BLANK, .key = CTUNE_UI_KEYBOARD_LEFT },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_GO_RIGHT, .description = CTUNE_UI_TEXT_BLANK, .key = CTUNE_UI_KEYBOARD_RIGHT },
    { .entry_type = CTUNE_UI_KEYBINDING_TYPE_NORMAL, .action = CTUNE_UI_ACTION_TRIGGER, .description = CTUNE_UI_TEXT_HELP_FIELD_RETURN, .key = CTUNE_UI_KEYBOARD_RETURN },
};

/**
 * [PRIVATE] Loads a set of keybindings to the cache
 * @param ctx      Associated context
 * @param bindings Pointer to the key bindings array
 * @param length   Length of the array
 * @return Success
 */
static bool ctune_UI_KeyBinding_loadToCache( ctune_UI_Context_e ctx, const ctune_UI_KeyBinding_t * bindings, size_t length ) {
    bool   error_state = false;
    size_t count       = 0;

    for( int i = 0; i < length; ++i ) {
        if( bindings[ i ].key > CTUNE_UI_KEYBOARD_NONE && bindings[ i ].key < CTUNE_UI_KEYBOARD_COUNT ) {
            int ncurses_key = ctune_UI_KeyDescription.getKeyRaw( bindings[ i ].key );

            if( ncurses_key >= 0 ) {
                keymap_cache[ ctx ][ ncurses_key ] = bindings[ i ].action;
                ++count;

            } else {
                CTUNE_LOG( CTUNE_LOG_FATAL,
                           "[ctune_KeyBinding_loadToCache( %i, %p, %lu )] "
                           "Failed to get a ncurses key integer (key ID: %i, action ID: %i).",
                           ctx, bindings, length, bindings[ i ].key, bindings[ i ].action
                );

                error_state = true;
            }
        }
    }

    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_KeyBinding_loadToCache( %i, %p, %lu )] Loaded %lu key bindings for context %i.",
               ctx, bindings, length, count, ctx
    );

    return !( error_state );
}

/**
 * Initialises the key bindings and loads into cache
 * @return Success
 */
static bool ctune_UI_KeyBinding_init( void ) {
    bool   error_state = false;
    size_t mem_size    = 0;

    for( ctune_UI_Context_e ctx = 0; ctx < CTUNE_UI_CTX_COUNT; ++ctx ) {
        switch( ctx ) {
            case CTUNE_UI_CTX_MAIN: {
                if( !ctune_UI_KeyBinding_loadToCache( ctx, &bindings_main[0], ( sizeof( bindings_main ) / sizeof( bindings_main[0] ) ) ) )
                    error_state = true;

                mem_size += sizeof( bindings_main );
            } break;

            case CTUNE_UI_CTX_FAV_TAB: {
                if( !ctune_UI_KeyBinding_loadToCache( ctx, &bindings_fav_tab[0], ( sizeof( bindings_fav_tab ) / sizeof( bindings_fav_tab[0] ) ) ) )
                    error_state = true;

                mem_size += sizeof( bindings_fav_tab );
            } break;

            case CTUNE_UI_CTX_SEARCH_TAB: {
                if( !ctune_UI_KeyBinding_loadToCache( ctx, &bindings_search_tab[0], ( sizeof( bindings_search_tab ) / sizeof( bindings_search_tab[0] ) ) ) )
                    error_state = true;

                mem_size += sizeof( bindings_search_tab );
            } break;

            case CTUNE_UI_CTX_BROWSE_TAB: {
                if( !ctune_UI_KeyBinding_loadToCache( ctx, &bindings_browse_tab[0], ( sizeof( bindings_browse_tab ) / sizeof( bindings_browse_tab[0] ) ) ) )
                    error_state = true;

                mem_size += sizeof( bindings_browse_tab );
            } break;

            case CTUNE_UI_CTX_RSFIND: {
                if( !ctune_UI_KeyBinding_loadToCache( ctx, &bindings_rsfind[0], ( sizeof( bindings_rsfind ) / sizeof( bindings_rsfind[0] ) ) ) )
                    error_state = true;

                mem_size += sizeof( bindings_rsfind );
            } break;

            case CTUNE_UI_CTX_RSINFO: {
                if( !ctune_UI_KeyBinding_loadToCache( ctx, &bindings_rsinfo[0], ( sizeof( bindings_rsinfo ) / sizeof( bindings_rsinfo[0] ) ) ) )
                    error_state = true;

                mem_size += sizeof( bindings_rsinfo );
            } break;

            case CTUNE_UI_CTX_RSEDIT: {
                if( !ctune_UI_KeyBinding_loadToCache( ctx, &bindings_rsedit[0], ( sizeof( bindings_rsedit ) / sizeof( bindings_rsedit[0] ) ) ) )
                    error_state = true;

                mem_size += sizeof( bindings_rsedit );
            } break;

            case CTUNE_UI_CTX_OPT_MENU: {
                if( !ctune_UI_KeyBinding_loadToCache( ctx, &bindings_options_menu[0], ( sizeof( bindings_options_menu ) / sizeof( bindings_options_menu[0] ) ) ) )
                    error_state = true;

                mem_size += sizeof( bindings_options_menu );
            } break;

            default: {
                CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_UI_KeyBinding_init()] Context (%i) not implemented.", ctx );
                error_state = true;
            } break;
        }
    }

    if( !error_state ) {
        CTUNE_LOG( CTUNE_LOG_MSG,
                   "[ctune_UI_KeyBinding_init()] Key bindings initialised (cache size: %lu bytes).",
                   mem_size
        );
    }

    return !( error_state );
}

/**
 * Gets action bound to an ncurses key
 * @param ctx         Action's context
 * @param ncurses_key Raw ncurses key
 * @return Action bound to key
 */
static ctune_UI_ActionID_e ctune_UI_KeyBinding_getAction( ctune_UI_Context_e ctx, int ncurses_key ) {
    if( ncurses_key >= KEY_MAX ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_KeyBinding_getAction( %i, %i )] Key exceeds upper limit (%i)", ctx, ncurses_key, KEY_MAX );
        return CTUNE_UI_ACTION_NONE; //EARLY RETURN

    } else if( ncurses_key < 0 ) {
        return CTUNE_UI_ACTION_ERR;

    } else {
        return keymap_cache[ ctx ][ ncurses_key ];
    }
}

/**
 * Get a key's display text
 * @param keyID ctune_UI_KeyboardKey_e enum type
 * @return Pointer to key's description string (NULL if error)
 */
static const char * ctune_UI_KeyBinding_getKeyText( ctune_UI_KeyboardKey_e keyID ) {
    return ctune_UI_KeyDescription.getKeyText( keyID );
}

/**
 * Iterates through the binding entries for a context calling the callback method for each
 * @param ctx      Context to process entries for
 * @param userdata Pointer to some userdata that will be passed to the callback (optional)
 * @param callback Method called for each entries ( enum, * key, * description )
 * @return Success
 */
static bool ctune_UI_KeyBinding_processEntries( ctune_UI_Context_e ctx, void * userdata, void (* callback)( const ctune_UI_KeyBinding_t * binding, void * userdata ) ) {
    if( callback == NULL ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_UI_KeyBinding_processEntries( %i, %p )] Callback is NULL.",
                   ctx, callback
        );
        return false; //EARLY RETURN
    }

    bool                          error_state = false;
    const ctune_UI_KeyBinding_t * bindings    = NULL;
    size_t                        length      = 0;

    switch( ctx ) {
        case CTUNE_UI_CTX_MAIN: {
            bindings = &bindings_main[0];
            length   = ( sizeof( bindings_main ) / sizeof( bindings_main[0] ) );
        } break;

        case CTUNE_UI_CTX_FAV_TAB: {
            bindings = &bindings_fav_tab[0];
            length   = ( sizeof( bindings_fav_tab ) / sizeof( bindings_fav_tab[0] ) );
        } break;

        case CTUNE_UI_CTX_SEARCH_TAB: {
            bindings = &bindings_search_tab[0];
            length   = ( sizeof( bindings_search_tab ) / sizeof( bindings_search_tab[0] ) );
        } break;

        case CTUNE_UI_CTX_BROWSE_TAB: {
            bindings = &bindings_browse_tab[0];
            length   = ( sizeof( bindings_browse_tab ) / sizeof( bindings_browse_tab[0] ) );
        } break;

        case CTUNE_UI_CTX_RSFIND: {
            bindings = &bindings_rsfind[0];
            length   = ( sizeof( bindings_rsfind ) / sizeof( bindings_rsfind[0] ) );
        } break;

        case CTUNE_UI_CTX_RSINFO: {
            bindings = &bindings_rsinfo[0];
            length   = ( sizeof( bindings_rsinfo ) / sizeof( bindings_rsinfo[0] ) );
        } break;

        case CTUNE_UI_CTX_RSEDIT: {
            bindings = &bindings_rsedit[0];
            length   = ( sizeof( bindings_rsedit ) / sizeof( bindings_rsedit[0] ) );
        } break;

        case CTUNE_UI_CTX_OPT_MENU: {
            bindings = &bindings_options_menu[0];
            length   = ( sizeof( bindings_options_menu ) / sizeof( bindings_options_menu[0] ) );
        } break;

        default:
            CTUNE_LOG( CTUNE_LOG_FATAL,
                       "[ctune_UI_KeyBinding_processEntries( %i, %p )] Context (%i) not implemented.",
                       ctx, callback, ctx
            );
            error_state = true;
    }

    if( bindings != NULL && length > 0 ) {
        for( int i = 0; i < length; ++i ) {
            callback( &bindings[i], userdata );
        }
    }

    return !( error_state );
}

/**
 * Namespace constructor
 */
const struct ctune_UI_KeyBinding_Instance ctune_UI_KeyBinding = {
    .init           = ctune_UI_KeyBinding_init,
    .getAction      = &ctune_UI_KeyBinding_getAction,
    .getKeyText     = &ctune_UI_KeyBinding_getKeyText,
    .processEntries = &ctune_UI_KeyBinding_processEntries,
};