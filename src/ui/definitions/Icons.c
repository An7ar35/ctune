#include "Icons.h"

#include "../../logger/Logger.h"

struct IconSet {
    const char * ascii;
    const char * unicode;
};

static const struct IconSet icons[CTUNE_UI_ICON_COUNT] = {
    [CTUNE_UI_ICON_VOID         ] = { .ascii = " ", .unicode = " " },
    [CTUNE_UI_ICON_VBAR_0_8TH   ] = { .ascii = " ", .unicode = " " },
    [CTUNE_UI_ICON_VBAR_1_8TH   ] = { .ascii = " ", .unicode = "▁" },
    [CTUNE_UI_ICON_VBAR_2_8TH   ] = { .ascii = " ", .unicode = "▂" },
    [CTUNE_UI_ICON_VBAR_3_8TH   ] = { .ascii = " ", .unicode = "▃" },
    [CTUNE_UI_ICON_VBAR_4_8TH   ] = { .ascii = " ", .unicode = "▄" },
    [CTUNE_UI_ICON_VBAR_5_8TH   ] = { .ascii = " ", .unicode = "▅" },
    [CTUNE_UI_ICON_VBAR_6_8TH   ] = { .ascii = " ", .unicode = "▆" },
    [CTUNE_UI_ICON_VBAR_7_8TH   ] = { .ascii = " ", .unicode = "▇" },
    [CTUNE_UI_ICON_VBAR_8_8TH   ] = { .ascii = " ", .unicode = "█" },
    [CTUNE_UI_ICON_HBAR_0_8TH   ] = { .ascii = " ", .unicode = " " },
    [CTUNE_UI_ICON_HBAR_1_8TH   ] = { .ascii = " ", .unicode = "▏" },
    [CTUNE_UI_ICON_HBAR_2_8TH   ] = { .ascii = " ", .unicode = "▎" },
    [CTUNE_UI_ICON_HBAR_3_8TH   ] = { .ascii = " ", .unicode = "▍" },
    [CTUNE_UI_ICON_HBAR_4_8TH   ] = { .ascii = " ", .unicode = "▌" },
    [CTUNE_UI_ICON_HBAR_5_8TH   ] = { .ascii = " ", .unicode = "▋" },
    [CTUNE_UI_ICON_HBAR_6_8TH   ] = { .ascii = " ", .unicode = "▊" },
    [CTUNE_UI_ICON_HBAR_7_8TH   ] = { .ascii = " ", .unicode = "▉" },
    [CTUNE_UI_ICON_HBAR_8_8TH   ] = { .ascii = " ", .unicode = "█" },
    [CTUNE_UI_ICON_SCROLL_UP    ] = { .ascii = "-", .unicode = "▲" },
    [CTUNE_UI_ICON_SCROLL_DOWN  ] = { .ascii = "+", .unicode = "▼" },
    [CTUNE_UI_ICON_SCROLL_LEFT  ] = { .ascii = "-", .unicode = "◄" },
    [CTUNE_UI_ICON_SCROLL_RIGHT ] = { .ascii = "+", .unicode = "►" },
    [CTUNE_UI_ICON_UP_ARROW     ] = { .ascii = "-", .unicode = "▲" },
    [CTUNE_UI_ICON_DOWN_ARROW   ] = { .ascii = "+", .unicode = "▼" },
    [CTUNE_UI_ICON_LEFT_ARROW   ] = { .ascii = "<", .unicode = "◄" },
    [CTUNE_UI_ICON_RIGHT_ARROW  ] = { .ascii = ">", .unicode = "►" },
    [CTUNE_UI_ICON_STOPPED      ] = { .ascii = ".", .unicode = "■" },
    [CTUNE_UI_ICON_PLAYING      ] = { .ascii = ">", .unicode = "▶" },
    [CTUNE_UI_ICON_WINCTRL_CLOSE] = { .ascii = "X", .unicode = "⛌" },
    [CTUNE_UI_ICON_VOLUME       ] = { .ascii = "Vol:", .unicode = "   ◢" },
};

static bool unicode_support = false;

/**
 * Sets the internal flag for unicode icon support
 * @param flag Unicode icon flag
 */
static void ctune_UI_Icons_setUnicode( bool flag ) {
    CTUNE_LOG( CTUNE_LOG_MSG, "[ctune_UI_Icons_setUnicode( %s )]", ( flag ? "true" : "false" ) )
    unicode_support = flag;
}

/**
 * Gets the internal flag for unicode icon support
 * @return Unicode icon flag value
 */
static bool ctune_UI_Icons_unicodeState( void ) {
    return unicode_support;
}

/**
 * Gets the character string for a display icon
 * @param text_id Icon ID enum
 * @return Icon string associated with given enum
 */
static const char * ctune_UI_Icons_icon( ctune_UI_IconID_e icon_id ) {
    if( ( sizeof( icons ) / sizeof( icons[0] ) ) <= icon_id ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_Icon_icon( %i )] ID not implemented.", icon_id );
        return "?";
    }

    return unicode_support
           ? icons[ icon_id ].unicode
           : icons[ icon_id ].ascii;
}

/**
 * Converts a percentage to a a progress "eighth" (use to work out which progress icon to use)
 * @param percent Percent value (0-100)
 * @return Progress range 0-9 value (-1 on error)
 */
static int ctune_UI_Icons_percentTo8th( int percent ) {
    static const int ranges[2][9] = {
    //   [0][1] [2] [3] [4] [5] [6] [7]  [8]
        { 0,  1, 15, 29, 43, 58, 72, 86, 100 }, //lower
        { 0, 14, 28, 42, 57, 71, 85, 99, 100 }  //upper
    };

    int left_i  = 0;
    int right_i = 8;

    while( left_i <= right_i ) {
        int mid_i = left_i + ( right_i - left_i ) / 2;

        if( percent >= ranges[0][mid_i] && percent <= ranges[1][mid_i] ) {
            return mid_i; //EARLY RETURN
        } else if( percent > ranges[0][mid_i] ){
            left_i  = ( mid_i + 1 );
        } else {
            right_i = ( mid_i - 1 );
        }
    }

    CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_Icons_percentTo8th( %i )] Failed to get range.", percent );
    return -1;
}

/**
 * Gets the character string for a progress icon
 * @param val_8th 8th value to get an icon for
 * @param icon_type Progress icon type
 * @return Icon string associated with value and type (empty on error)
 */
static const char * ctune_UI_Icons_progressIcon( int val_8th, ctune_UI_IconCategory_e icon_type ) {
    if( val_8th >= 0 && val_8th <= 8 ) {
        switch( icon_type ) {
            case CTUNE_UI_ICONCATEGORY_HPROGRESS: {
                return ctune_UI_Icons.icon( ( CTUNE_UI_ICON_HBAR_0_8TH + val_8th ) );
            } break;

            case CTUNE_UI_ICONCATEGORY_VPROGRESS: {
                return ctune_UI_Icons.icon( ( CTUNE_UI_ICON_VBAR_0_8TH + val_8th ) );
            } break;

            default: break;
        }
    }

    return ctune_UI_Icons.icon( CTUNE_UI_ICON_VOID );
}

const struct ctune_UI_Icons_Instance ctune_UI_Icons = {
    .setUnicode   = &ctune_UI_Icons_setUnicode,
    .unicodeState = &ctune_UI_Icons_unicodeState,
    .icon         = &ctune_UI_Icons_icon,
    .percentTo8th = &ctune_UI_Icons_percentTo8th,
    .progressIcon = &ctune_UI_Icons_progressIcon,
};