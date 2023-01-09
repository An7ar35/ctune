#include "KeyDescription.h"

#include <ncurses.h>
#include <sys/ioctl.h>

/**
 * cTune keys to NCurses keys/descriptions
 * -
 * Note: before adding a new key a corresponding entry must be added to the KeyboardKeyID enum type
 *       before it can be used.
 */
const ctune_UI_KeyDescription_t keys[CTUNE_UI_KEYBOARD_COUNT] = {
    [CTUNE_UI_KEYBOARD_ERR           ] = { .ncurses_key = ERR,                     .text = "Error" },
    [CTUNE_UI_KEYBOARD_ESC           ] = { .ncurses_key = CTUNE_KEY_ESC,           .text = "Esc" },
    [CTUNE_UI_KEYBOARD_F1            ] = { .ncurses_key = KEY_F( 1 ),              .text = "F1" },
    [CTUNE_UI_KEYBOARD_F2            ] = { .ncurses_key = KEY_F( 2 ),              .text = "F2" },
    [CTUNE_UI_KEYBOARD_F3            ] = { .ncurses_key = KEY_F( 3 ),              .text = "F3" },
    [CTUNE_UI_KEYBOARD_F4            ] = { .ncurses_key = KEY_F( 4 ),              .text = "F4" },
    [CTUNE_UI_KEYBOARD_F5            ] = { .ncurses_key = KEY_F( 5 ),              .text = "F5" },
    [CTUNE_UI_KEYBOARD_F6            ] = { .ncurses_key = KEY_F( 6 ),              .text = "F6" },
    [CTUNE_UI_KEYBOARD_F7            ] = { .ncurses_key = KEY_F( 7 ),              .text = "F7" },
    [CTUNE_UI_KEYBOARD_F8            ] = { .ncurses_key = KEY_F( 8 ),              .text = "F8" },
    [CTUNE_UI_KEYBOARD_F9            ] = { .ncurses_key = KEY_F( 9 ),              .text = "F9" },
    [CTUNE_UI_KEYBOARD_F10           ] = { .ncurses_key = KEY_F( 10 ),             .text = "F10" },
    [CTUNE_UI_KEYBOARD_F11           ] = { .ncurses_key = KEY_F( 11 ),             .text = "F11" },
    [CTUNE_UI_KEYBOARD_F12           ] = { .ncurses_key = KEY_F( 12 ),             .text = "F12" },

    [CTUNE_UI_KEYBOARD_RETURN        ] = { .ncurses_key = CTUNE_KEY_RETURN,        .text = "Enter" },
    [CTUNE_UI_KEYBOARD_TAB           ] = { .ncurses_key = CTUNE_KEY_TAB,           .text = "Tab" },
    [CTUNE_UI_KEYBOARD_STAB          ] = { .ncurses_key = CTUNE_KEY_STAB,          .text = "Shift + Tab" },
    [CTUNE_UI_KEYBOARD_BACKSPACE     ] = { .ncurses_key = CTUNE_KEY_BACKSPACE,     .text = "Backspace" },
    [CTUNE_UI_KEYBOARD_ALT_BACKSPACE ] = { .ncurses_key = CTUNE_KEY_ALT_BACKSPACE, .text = "Backspace" },
    [CTUNE_UI_KEYBOARD_SPACE         ] = { .ncurses_key = CTUNE_KEY_SPACE,         .text = "Space" },
    [CTUNE_UI_KEYBOARD_DELCHAR       ] = { .ncurses_key = KEY_DC,                  .text = "Delete char" },
    [CTUNE_UI_KEYBOARD_SDELCHAR      ] = { .ncurses_key = KEY_SDC,                 .text = "Shift + Delete char" },
    [CTUNE_UI_KEYBOARD_DELLINE       ] = { .ncurses_key = KEY_DL,                  .text = "Delete line" },
    [CTUNE_UI_KEYBOARD_SDELLINE      ] = { .ncurses_key = KEY_SDL,                 .text = "Shift + Delete line" },
    [CTUNE_UI_KEYBOARD_INSLINE       ] = { .ncurses_key = KEY_IL,                  .text = "Insert line" },
    [CTUNE_UI_KEYBOARD_INSCHAR       ] = { .ncurses_key = KEY_IC,                  .text = "Insert char" },
    [CTUNE_UI_KEYBOARD_SINSCHAR      ] = { .ncurses_key = KEY_SIC,                 .text = "Shift + Insert line" },
    [CTUNE_UI_KEYBOARD_HOME          ] = { .ncurses_key = KEY_HOME,                .text = "Home" },
    [CTUNE_UI_KEYBOARD_SHOME         ] = { .ncurses_key = KEY_SHOME,               .text = "Shift + Home" },
    [CTUNE_UI_KEYBOARD_END           ] = { .ncurses_key = KEY_END,                 .text = "End" },
    [CTUNE_UI_KEYBOARD_HOMEDOWN      ] = { .ncurses_key = KEY_LL,                  .text = "Home-down" },
    [CTUNE_UI_KEYBOARD_UPPERLEFT     ] = { .ncurses_key = KEY_A1,                  .text = "Keypad upper-left" },
    [CTUNE_UI_KEYBOARD_UPPERRIGHT    ] = { .ncurses_key = KEY_A3,                  .text = "Keypad upper-right" },
    [CTUNE_UI_KEYBOARD_CENTER        ] = { .ncurses_key = KEY_B2,                  .text = "Keypad centre" },
    [CTUNE_UI_KEYBOARD_LOWERLEFT     ] = { .ncurses_key = KEY_C1,                  .text = "Keypad lower-left" },
    [CTUNE_UI_KEYBOARD_LOWERRIGHT    ] = { .ncurses_key = KEY_C3,                  .text = "Keypad lower-right" },
    [CTUNE_UI_KEYBOARD_PAGEUP        ] = { .ncurses_key = KEY_PPAGE,               .text = "Page Up" },
    [CTUNE_UI_KEYBOARD_PAGEDOWN      ] = { .ncurses_key = KEY_NPAGE,               .text = "Page Down" },
    [CTUNE_UI_KEYBOARD_UP            ] = { .ncurses_key = KEY_UP,                  .text = "Up" },
    [CTUNE_UI_KEYBOARD_DOWN          ] = { .ncurses_key = KEY_DOWN,                .text = "Down" },
    [CTUNE_UI_KEYBOARD_LEFT          ] = { .ncurses_key = KEY_LEFT,                .text = "Left" },
    [CTUNE_UI_KEYBOARD_RIGHT         ] = { .ncurses_key = KEY_RIGHT,               .text = "Right" },
    [CTUNE_UI_KEYBOARD_SLEFT         ] = { .ncurses_key = KEY_SLEFT,               .text = "Shift + Left" },
    [CTUNE_UI_KEYBOARD_SRIGHT        ] = { .ncurses_key = KEY_SRIGHT,              .text = "Shift + Right" },
    [CTUNE_UI_KEYBOARD_NEXT          ] = { .ncurses_key = KEY_NEXT,                .text = "Next" },
    [CTUNE_UI_KEYBOARD_PREVIOUS      ] = { .ncurses_key = KEY_PREVIOUS,            .text = "Previous" },
    [CTUNE_UI_KEYBOARD_SNEXT         ] = { .ncurses_key = KEY_SNEXT,               .text = "Shift + Next" },
    [CTUNE_UI_KEYBOARD_SPREVIOUS     ] = { .ncurses_key = KEY_SPREVIOUS,           .text = "Shift + Previous" },
    [CTUNE_UI_KEYBOARD_EXIT          ] = { .ncurses_key = KEY_EXIT,                .text = "Exit" },
    [CTUNE_UI_KEYBOARD_SEXIT         ] = { .ncurses_key = KEY_SEXIT,               .text = "Shift + Exit" },
    [CTUNE_UI_KEYBOARD_FIND          ] = { .ncurses_key = KEY_FIND,                .text = "Find" },
    [CTUNE_UI_KEYBOARD_SFIND         ] = { .ncurses_key = KEY_SFIND,               .text = "Shift + Find" },
    [CTUNE_UI_KEYBOARD_HELP          ] = { .ncurses_key = KEY_HELP,                .text = "Help" },
    [CTUNE_UI_KEYBOARD_SHELP         ] = { .ncurses_key = KEY_SHELP,               .text = "Shift + Help" },
    [CTUNE_UI_KEYBOARD_OPTIONS       ] = { .ncurses_key = KEY_OPTIONS,             .text = "Options" },

    [CTUNE_UI_KEYBOARD_1             ] = { .ncurses_key = '1',                     .text = "1" },
    [CTUNE_UI_KEYBOARD_2             ] = { .ncurses_key = '2',                     .text = "2" },
    [CTUNE_UI_KEYBOARD_3             ] = { .ncurses_key = '3',                     .text = "3" },
    [CTUNE_UI_KEYBOARD_4             ] = { .ncurses_key = '4',                     .text = "4" },
    [CTUNE_UI_KEYBOARD_5             ] = { .ncurses_key = '5',                     .text = "5" },
    [CTUNE_UI_KEYBOARD_6             ] = { .ncurses_key = '6',                     .text = "6" },
    [CTUNE_UI_KEYBOARD_7             ] = { .ncurses_key = '7',                     .text = "7" },
    [CTUNE_UI_KEYBOARD_8             ] = { .ncurses_key = '8',                     .text = "8" },
    [CTUNE_UI_KEYBOARD_9             ] = { .ncurses_key = '9',                     .text = "9" },
    [CTUNE_UI_KEYBOARD_0             ] = { .ncurses_key = '0',                     .text = "0" },
    [CTUNE_UI_KEYBOARD_PLUS          ] = { .ncurses_key = '+',                     .text = "+" },
    [CTUNE_UI_KEYBOARD_MINUS         ] = { .ncurses_key = '-',                     .text = "-" },

    [CTUNE_UI_KEYBOARD_E             ] = { .ncurses_key = 'e',                     .text = "E" },
    [CTUNE_UI_KEYBOARD_F             ] = { .ncurses_key = 'f',                     .text = "F" },
    [CTUNE_UI_KEYBOARD_I             ] = { .ncurses_key = 'i',                     .text = "I" },
    [CTUNE_UI_KEYBOARD_N             ] = { .ncurses_key = 'n',                     .text = "N" },
    [CTUNE_UI_KEYBOARD_O             ] = { .ncurses_key = 'o',                     .text = "O" },
    [CTUNE_UI_KEYBOARD_P             ] = { .ncurses_key = 'p',                     .text = "P" },
    [CTUNE_UI_KEYBOARD_Q             ] = { .ncurses_key = 'q',                     .text = "Q" },
    [CTUNE_UI_KEYBOARD_R             ] = { .ncurses_key = 'r',                     .text = "R" },
    [CTUNE_UI_KEYBOARD_S             ] = { .ncurses_key = 's',                     .text = "S" },
    [CTUNE_UI_KEYBOARD_X             ] = { .ncurses_key = 'x',                     .text = "X" },

    [CTUNE_UI_KEYBOARD_CTRL_D        ] = { .ncurses_key = CTRL( 'd' ),             .text = "Ctrl + D" },
    [CTUNE_UI_KEYBOARD_CTRL_F        ] = { .ncurses_key = CTRL( 'f' ),             .text = "Ctrl + F" },
    [CTUNE_UI_KEYBOARD_CTRL_I        ] = { .ncurses_key = CTRL( 'i' ),             .text = "Ctrl + I" },

    [CTUNE_UI_KEYBOARD_RESIZE        ] = { .ncurses_key = KEY_RESIZE,              .text = "Resize" },
};

/**
 * Get a key's display text
 * @param keyID ctune_UI_KeyboardKey_e enum type
 * @return Pointer to key's description string (NULL if error)
 */
static const char * ctune_UI_KeyDescription_getKeyText( ctune_UI_KeyboardKey_e keyID ) {
    if( keyID == CTUNE_UI_KEYBOARD_NONE ) {
        return "";

    } else if( keyID > 0 && keyID < CTUNE_UI_KEYBOARD_COUNT ) {
        return keys[ keyID ].text;

    } else {
        return "N/A";
    }
}

/**
 * Gets the raw ncurses key associated with the KeyID
 * @param keyID ctune_UI_KeyboardKey_e enum type
 * @return NCurses key integer (-1 on error)
 */
static int ctune_UI_KeyDescription_getKeyRaw( ctune_UI_KeyboardKey_e keyID ) {
    if( keyID > 0 && keyID < CTUNE_UI_KEYBOARD_COUNT )
        return keys[keyID].ncurses_key;
    return -1;
}

/**
 * Get the key ID from an ncurses key
 * @param ncurses_key NCurses key
 * @return ctune_UI_KeyboardKey_e enum type (`CTUNE_UI_KEYBOARD_NONE` if not found)
 */
static ctune_UI_KeyboardKey_e ctune_UI_KeyDescription_getKeyID( int ncurses_key ) {
    for( int i = 1; i < CTUNE_UI_KEYBOARD_COUNT; ++i ) {
        if( keys[i].ncurses_key == ncurses_key )
            return i;
    }

    return CTUNE_UI_KEYBOARD_NONE;
}

/**
 * Namespace
 */
const struct ctune_UI_KeyDescription_Namespace ctune_UI_KeyDescription = {
    .getKeyID   = &ctune_UI_KeyDescription_getKeyID,
    .getKeyRaw  = &ctune_UI_KeyDescription_getKeyRaw,
    .getKeyText = &ctune_UI_KeyDescription_getKeyText,
};