#include "ColourTheme.h"

#include "stdlib.h"

#define COLOUR_COUNT 8

//same as ncurses colour codes
static const short CTUNE_COLOUR_BLACK   = 0b000;
static const short CTUNE_COLOUR_RED     = 0b001;
static const short CTUNE_COLOUR_GREEN   = 0b010;
static const short CTUNE_COLOUR_YELLOW  = 0b011;
static const short CTUNE_COLOUR_BLUE    = 0b100;
static const short CTUNE_COLOUR_MAGENTA = 0b101;
static const short CTUNE_COLOUR_CYAN    = 0b110;
static const short CTUNE_COLOUR_WHITE   = 0b111;

static const char * str_map[COLOUR_COUNT] = {
    [0b000] = "BLACK",
    [0b001] = "RED",
    [0b010] = "GREEN",
    [0b011] = "YELLOW",
    [0b100] = "BLUE",
    [0b101] = "MAGENTA",
    [0b110] = "CYAN",
    [0b111] = "WHITE",
};

/**
 * Initiate a colour theme object with default values
 * @return Initiated ColourTheme
 */
struct ctune_ColourTheme ctune_ColourTheme_init( void ) {
    return (struct ctune_ColourTheme) {
        .background = CTUNE_COLOUR_BLACK,
        .foreground = CTUNE_COLOUR_WHITE,

        .rows = {
            .background            = CTUNE_COLOUR_BLACK,
            .foreground            = CTUNE_COLOUR_WHITE,
            .selected_focused_bg   = CTUNE_COLOUR_BLUE,
            .selected_focused_fg   = CTUNE_COLOUR_WHITE,
            .selected_unfocused_bg = CTUNE_COLOUR_WHITE,
            .selected_unfocused_fg = CTUNE_COLOUR_BLACK,
            .favourite_local_fg    = CTUNE_COLOUR_MAGENTA,
            .favourite_remote_fg   = CTUNE_COLOUR_YELLOW,
            .broken_fg             = CTUNE_COLOUR_RED,
        },

        .icons = {
            .playback_on    = CTUNE_COLOUR_GREEN,
            .playback_off   = CTUNE_COLOUR_RED,
            .queued_station = CTUNE_COLOUR_CYAN,
        },

        .field = {
            .invalid_fg = CTUNE_COLOUR_RED,
        },

        .button = {
            .background   = CTUNE_COLOUR_BLACK,
            .foreground   = CTUNE_COLOUR_WHITE,
            .invalid_fg   = CTUNE_COLOUR_RED,
            .validated_fg = CTUNE_COLOUR_GREEN,
        },
    };
}

/**
 * Gets the string representation of a colour code
 * @param colour Colour code (0-8)
 * @return String or NULL if colour code is not valid
 */
const char * ctune_ColourTheme_str( short colour ) {
    if( colour < 0 || colour >= COLOUR_COUNT )
        return NULL;

    return str_map[ colour ];
}

/**
 * Namespace constructor
 */
const struct ctune_ColourTheme_Namespace ctune_ColourTheme = {
    .init = &ctune_ColourTheme_init,
    .str  = &ctune_ColourTheme_str,
};