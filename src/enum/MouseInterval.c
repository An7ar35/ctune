#include "MouseInterval.h"

#include <ncurses.h>

#include "logger/src/Logger.h"

static const int values[CTUNE_MOUSEINTERVAL_COUNT] = {
    [CTUNE_MOUSEINTERVAL_FAST   ] =  83,
    [CTUNE_MOUSEINTERVAL_DEFAULT] = 166,
    [CTUNE_MOUSEINTERVAL_RELAXED] = 250,
    [CTUNE_MOUSEINTERVAL_HIPPY  ] = 333,
    [CTUNE_MOUSEINTERVAL_SLOTH  ] = 500,
};

static const char * strings[CTUNE_MOUSEINTERVAL_COUNT] = {
    [CTUNE_MOUSEINTERVAL_FAST   ] = "FAST",
    [CTUNE_MOUSEINTERVAL_DEFAULT] = "DEFAULT",
    [CTUNE_MOUSEINTERVAL_RELAXED] = "RELAXED",
    [CTUNE_MOUSEINTERVAL_HIPPY  ] = "HIPPY",
    [CTUNE_MOUSEINTERVAL_SLOTH  ] = "SLOTH",
};


/**
 * Gets the resolution value
 * @param resolution_e Resolution enum
 * @return Value in milliseconds (-1 on "not set", 0 on "custom", ncurses value for "default")
 */
static int ctune_MouseInterval_value( ctune_MouseInterval_e resolution_e ) {
    switch( resolution_e ) {
        case CTUNE_MOUSEINTERVAL_FAST   :
        case CTUNE_MOUSEINTERVAL_DEFAULT:
        case CTUNE_MOUSEINTERVAL_RELAXED:
        case CTUNE_MOUSEINTERVAL_HIPPY  : //fallthrough
        case CTUNE_MOUSEINTERVAL_SLOTH  : return values[resolution_e];
        default                           : {
            CTUNE_LOG( CTUNE_LOG_WARNING,
                       "[ctune_MouseInterval_value( %i )] Invalid mouse resolution: '%s'",
                       resolution_e, ctune_MouseInterval.str( resolution_e )
            );

            return values[CTUNE_MOUSEINTERVAL_DEFAULT];
        }
    }
}

/**
 * Gets the string representation of the enum
 * @param res MouseResolution enum
 * @return String representation
 */
static const char * ctune_MouseInterval_str( ctune_MouseInterval_e res ) {
    if( res < CTUNE_MOUSEINTERVAL_COUNT ) {
        return strings[res];
    } else {
        return "INVALID";
    }
}

/**
 * Namespace declaration
 */
const struct ctune_MouseInterval_Namespace ctune_MouseInterval = {
    .value = &ctune_MouseInterval_value,
    .str   = &ctune_MouseInterval_str,
};