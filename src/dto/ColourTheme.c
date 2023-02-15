#include "ColourTheme.h"

#include <stdlib.h>

#include "logger/src/Logger.h"

#define NUMBER_OF_COLOURS   8
#define COLOUR_CODE_BLACK   0b0000
#define COLOUR_CODE_RED     0b0001
#define COLOUR_CODE_GREEN   0b0010
#define COLOUR_CODE_YELLOW  0b0011
#define COLOUR_CODE_BLUE    0b0100
#define COLOUR_CODE_MAGENTA 0b0101
#define COLOUR_CODE_CYAN    0b0110
#define COLOUR_CODE_WHITE   0b0111

/**
 * Initiate a colour theme object with default values
 * @param theme Theme preset enum
 * @return Initiated ColourTheme
 */
struct ctune_ColourTheme ctune_ColourTheme_init( ctune_UIPreset_e theme ) {
    switch( theme ) {
        case CTUNE_UIPRESET_DEFAULT: {
            return (struct ctune_ColourTheme) {
                .background = ctune_ColourTheme.colour.BLACK,
                .foreground = ctune_ColourTheme.colour.WHITE,

                .rows = {
                    .background            = ctune_ColourTheme.colour.BLACK,
                    .foreground            = ctune_ColourTheme.colour.WHITE,
                    .selected_focused_bg   = ctune_ColourTheme.colour.BLUE,
                    .selected_focused_fg   = ctune_ColourTheme.colour.WHITE,
                    .selected_unfocused_bg = ctune_ColourTheme.colour.WHITE,
                    .selected_unfocused_fg = ctune_ColourTheme.colour.BLACK,
                    .favourite_local_fg    = ctune_ColourTheme.colour.MAGENTA,
                    .favourite_remote_fg   = ctune_ColourTheme.colour.YELLOW,
                    .broken_fg             = ctune_ColourTheme.colour.RED,
                },

                .icons = {
                    .playback_on    = ctune_ColourTheme.colour.GREEN,
                    .playback_off   = ctune_ColourTheme.colour.RED,
                    .playback_rec   = ctune_ColourTheme.colour.YELLOW,
                    .queued_station = ctune_ColourTheme.colour.CYAN,
                },

                .field = {
                    .invalid_fg = ctune_ColourTheme.colour.RED,
                },

                .button = {
                    .background   = ctune_ColourTheme.colour.BLACK,
                    .foreground   = ctune_ColourTheme.colour.WHITE,
                    .invalid_fg   = ctune_ColourTheme.colour.RED,
                    .validated_fg = ctune_ColourTheme.colour.GREEN,
                },
            };
        } break;

        case CTUNE_UIPRESET_HACKERMAN: {
            return (struct ctune_ColourTheme) {
                .background = ctune_ColourTheme.colour.BLACK,
                .foreground = ctune_ColourTheme.colour.GREEN,

                .rows = {
                    .background            = ctune_ColourTheme.colour.BLACK,
                    .foreground            = ctune_ColourTheme.colour.GREEN,
                    .selected_focused_bg   = ctune_ColourTheme.colour.GREEN,
                    .selected_focused_fg   = ctune_ColourTheme.colour.WHITE,
                    .selected_unfocused_bg = ctune_ColourTheme.colour.WHITE,
                    .selected_unfocused_fg = ctune_ColourTheme.colour.BLACK,
                    .favourite_local_fg    = ctune_ColourTheme.colour.MAGENTA,
                    .favourite_remote_fg   = ctune_ColourTheme.colour.YELLOW,
                    .broken_fg             = ctune_ColourTheme.colour.RED,
                },

                .icons = {
                    .playback_on    = ctune_ColourTheme.colour.WHITE,
                    .playback_off   = ctune_ColourTheme.colour.RED,
                    .playback_rec   = ctune_ColourTheme.colour.YELLOW,
                    .queued_station = ctune_ColourTheme.colour.CYAN,
                },

                .field = {
                    .invalid_fg = ctune_ColourTheme.colour.RED,
                },

                .button = {
                    .background   = ctune_ColourTheme.colour.BLACK,
                    .foreground   = ctune_ColourTheme.colour.CYAN,
                    .invalid_fg   = ctune_ColourTheme.colour.RED,
                    .validated_fg = ctune_ColourTheme.colour.GREEN,
                },
            };
        } break;

        case CTUNE_UIPRESET_REDZONE: {
            return (struct ctune_ColourTheme) {
                .background = ctune_ColourTheme.colour.BLACK,
                .foreground = ctune_ColourTheme.colour.RED,

                .rows = {
                    .background            = ctune_ColourTheme.colour.BLACK,
                    .foreground            = ctune_ColourTheme.colour.WHITE,
                    .selected_focused_bg   = ctune_ColourTheme.colour.RED,
                    .selected_focused_fg   = ctune_ColourTheme.colour.WHITE,
                    .selected_unfocused_bg = ctune_ColourTheme.colour.WHITE,
                    .selected_unfocused_fg = ctune_ColourTheme.colour.BLACK,
                    .favourite_local_fg    = ctune_ColourTheme.colour.MAGENTA,
                    .favourite_remote_fg   = ctune_ColourTheme.colour.YELLOW,
                    .broken_fg             = ctune_ColourTheme.colour.RED,
                },

                .icons = {
                    .playback_on    = ctune_ColourTheme.colour.GREEN,
                    .playback_off   = ctune_ColourTheme.colour.WHITE,
                    .playback_rec   = ctune_ColourTheme.colour.WHITE,
                    .queued_station = ctune_ColourTheme.colour.CYAN,
                },

                .field = {
                    .invalid_fg = ctune_ColourTheme.colour.RED,
                },

                .button = {
                    .background   = ctune_ColourTheme.colour.BLACK,
                    .foreground   = ctune_ColourTheme.colour.YELLOW,
                    .invalid_fg   = ctune_ColourTheme.colour.RED,
                    .validated_fg = ctune_ColourTheme.colour.GREEN,
                },
            };
        } break;

        case CTUNE_UIPRESET_DEEPBLUE: {
            return (struct ctune_ColourTheme) {
                .background = ctune_ColourTheme.colour.BLACK,
                .foreground = ctune_ColourTheme.colour.BLUE,

                .rows = {
                    .background            = ctune_ColourTheme.colour.BLACK,
                    .foreground            = ctune_ColourTheme.colour.WHITE,
                    .selected_focused_bg   = ctune_ColourTheme.colour.BLUE,
                    .selected_focused_fg   = ctune_ColourTheme.colour.WHITE,
                    .selected_unfocused_bg = ctune_ColourTheme.colour.WHITE,
                    .selected_unfocused_fg = ctune_ColourTheme.colour.BLACK,
                    .favourite_local_fg    = ctune_ColourTheme.colour.MAGENTA,
                    .favourite_remote_fg   = ctune_ColourTheme.colour.YELLOW,
                    .broken_fg             = ctune_ColourTheme.colour.RED,
                },

                .icons = {
                    .playback_on    = ctune_ColourTheme.colour.GREEN,
                    .playback_off   = ctune_ColourTheme.colour.RED,
                    .playback_rec   = ctune_ColourTheme.colour.YELLOW,
                    .queued_station = ctune_ColourTheme.colour.CYAN,
                },

                .field = {
                    .invalid_fg = ctune_ColourTheme.colour.RED,
                },

                .button = {
                    .background   = ctune_ColourTheme.colour.BLACK,
                    .foreground   = ctune_ColourTheme.colour.BLUE,
                    .invalid_fg   = ctune_ColourTheme.colour.RED,
                    .validated_fg = ctune_ColourTheme.colour.GREEN,
                },
            };
        } break;

        case CTUNE_UIPRESET_COUNT: //fallthrough
        default: {
            CTUNE_LOG( CTUNE_LOG_ERROR,
               "[ctune_ColourTheme_init(..)] "
               "Invalid or unknown UITheme - reverting to default."
            );

            return ctune_ColourTheme_init( CTUNE_UIPRESET_DEFAULT );
        } break;
    }
}

/**
 * Gets the string representation of a colour code
 * @param colour    Colour code (0-8)
 * @param uppercase Uppercase flag
 * @return String or NULL if colour code is not valid
 */
const char * ctune_ColourTheme_str( short colour, bool uppercase ) {
    static const char * str_map[8][2] = {
        [COLOUR_CODE_BLACK  ][0] = "Black",   [COLOUR_CODE_BLACK  ][1] = "BLACK",
        [COLOUR_CODE_RED    ][0] = "Red",     [COLOUR_CODE_RED    ][1] = "RED",
        [COLOUR_CODE_GREEN  ][0] = "Green",   [COLOUR_CODE_GREEN  ][1] = "GREEN",
        [COLOUR_CODE_YELLOW ][0] = "Yellow",  [COLOUR_CODE_YELLOW ][1] = "YELLOW",
        [COLOUR_CODE_BLUE   ][0] = "Blue",    [COLOUR_CODE_BLUE   ][1] = "BLUE",
        [COLOUR_CODE_MAGENTA][0] = "Magenta", [COLOUR_CODE_MAGENTA][1] = "MAGENTA",
        [COLOUR_CODE_CYAN   ][0] = "Cyan",    [COLOUR_CODE_CYAN   ][1] = "CYAN",
        [COLOUR_CODE_WHITE  ][0] = "White",   [COLOUR_CODE_WHITE  ][1] = "WHITE",
    };

    if( colour < 0 || colour >= 8 ) {
        return NULL;
    }

    return str_map[ colour ][ ( uppercase == 0 ? 0 : 1 ) ];
}

/**
 * Get a pointer to a list of available colours
 * @return Pointer to list of colours
 */
const short * ctune_ColourTheme_colourList( void ) {
    static const short list[NUMBER_OF_COLOURS] = {
        COLOUR_CODE_BLACK,
        COLOUR_CODE_RED,
        COLOUR_CODE_GREEN,
        COLOUR_CODE_YELLOW,
        COLOUR_CODE_BLUE,
        COLOUR_CODE_MAGENTA,
        COLOUR_CODE_CYAN,
        COLOUR_CODE_WHITE,
    };

    return &list[0];
}

/**
 * Namespace constructor
 */
const struct ctune_ColourTheme_Namespace ctune_ColourTheme = {
    .colour = {
        .BLACK   = COLOUR_CODE_BLACK,
        .RED     = COLOUR_CODE_RED,
        .GREEN   = COLOUR_CODE_GREEN,
        .YELLOW  = COLOUR_CODE_YELLOW,
        .BLUE    = COLOUR_CODE_BLUE,
        .MAGENTA = COLOUR_CODE_MAGENTA,
        .CYAN    = COLOUR_CODE_CYAN,
        .WHITE   = COLOUR_CODE_WHITE,
        .count   = NUMBER_OF_COLOURS,
    },

    .init       = &ctune_ColourTheme_init,
    .str        = &ctune_ColourTheme_str,
    .colourList = &ctune_ColourTheme_colourList,
};