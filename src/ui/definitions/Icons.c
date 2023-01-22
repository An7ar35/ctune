#include "Icons.h"

#include "../../logger/Logger.h"

struct IconSet {
    const char * ascii;
    const char * unicode;
};

static const struct IconSet icons[CTUNE_UI_ICON_COUNT] = {
    [CTUNE_UI_ICON_UP_ARROW     ] = { .ascii = "-", .unicode = "▲" },
    [CTUNE_UI_ICON_DOWN_ARROW   ] = { .ascii = "+", .unicode = "▼" },
    [CTUNE_UI_ICON_LEFT_ARROW   ] = { .ascii = "<", .unicode = "◄" },
    [CTUNE_UI_ICON_RIGHT_ARROW  ] = { .ascii = ">", .unicode = "►" },
    [CTUNE_UI_ICON_STOPPED      ] = { .ascii = ".", .unicode = "■" },
    [CTUNE_UI_ICON_PLAYING      ] = { .ascii = ">", .unicode = "▶" },
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


const struct ctune_UI_Icons_Instance ctune_UI_Icons = {
    .setUnicode = &ctune_UI_Icons_setUnicode,
    .icon       = &ctune_UI_Icons_icon,
};