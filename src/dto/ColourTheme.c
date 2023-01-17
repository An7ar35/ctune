#include "ColourTheme.h"

#include <stdlib.h>

#include "../logger/Logger.h"

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
        [0b000][0] = "Black",   [0b000][1] = "BLACK",
        [0b001][0] = "Red",     [0b001][1] = "RED",
        [0b010][0] = "Green",   [0b010][1] = "GREEN",
        [0b011][0] = "Yellow",  [0b011][1] = "YELLOW",
        [0b100][0] = "Blue",    [0b100][1] = "BLUE",
        [0b101][0] = "Magenta", [0b101][1] = "MAGENTA",
        [0b110][0] = "Cyan",    [0b110][1] = "CYAN",
        [0b111][0] = "White",   [0b111][1] = "WHITE",
    };

    if( colour < 0 || colour >= 8 ) {
        return NULL;
    }

    return str_map[ colour ][ ( uppercase == 0 ? 0 : 1 ) ];
}

/**
 * Namespace constructor
 */
const struct ctune_ColourTheme_Namespace ctune_ColourTheme = {
    .colour = {
        .BLACK   = 0b000,
        .RED     = 0b001,
        .GREEN   = 0b010,
        .YELLOW  = 0b011,
        .BLUE    = 0b100,
        .MAGENTA = 0b101,
        .CYAN    = 0b110,
        .WHITE   = 0b111,
    },

    .init = &ctune_ColourTheme_init,
    .str  = &ctune_ColourTheme_str,
};