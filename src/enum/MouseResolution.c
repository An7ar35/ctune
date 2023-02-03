#include "MouseResolution.h"

#include <ncurses.h>

#include "../logger/Logger.h"

static const int values[CTUNE_MOUSERESOLUTION_COUNT] = {
    [CTUNE_MOUSERESOLUTION_DEFAULT         ] =    0,
    [CTUNE_MOUSERESOLUTION_RELAXED         ] =  250,
    [CTUNE_MOUSERESOLUTION_SOBER_HIPPY     ] =  350,
    [CTUNE_MOUSERESOLUTION_STONED_HIPPY    ] =  500,
    [CTUNE_MOUSERESOLUTION_SLOTH           ] =  650,
    [CTUNE_MOUSERESOLUTION_SUPPER_SLOTH    ] =  800,
    [CTUNE_MOUSERESOLUTION_DEATH_CHALLENGED] = 1000, //1s
};

static const char * strings[CTUNE_MOUSERESOLUTION_COUNT] = {
    [CTUNE_MOUSERESOLUTION_DEFAULT         ] = "DEFAULT",
    [CTUNE_MOUSERESOLUTION_RELAXED         ] = "RELAXED",
    [CTUNE_MOUSERESOLUTION_SOBER_HIPPY     ] = "SOBER_HIPPY",
    [CTUNE_MOUSERESOLUTION_STONED_HIPPY    ] = "STONED_HIPPY",
    [CTUNE_MOUSERESOLUTION_SLOTH           ] = "SLOTH",
    [CTUNE_MOUSERESOLUTION_SUPPER_SLOTH    ] = "SUPER_SLOTH",
    [CTUNE_MOUSERESOLUTION_DEATH_CHALLENGED] = "DEATH_CHALLENGED",
    [CTUNE_MOUSERESOLUTION_CUSTOM          ] = "CUSTOM",
};


/**
 * Gets the resolution value
 * @param resolution_e Resolution enum
 * @return Value in milliseconds (-1 on "not set", 0 on "custom", ncurses value for "default")
 */
static int ctune_MouseResolution_value( ctune_MouseResolution_e resolution_e ) {
    switch( resolution_e ) {
        case CTUNE_MOUSERESOLUTION_DEFAULT         : return mouseinterval( -1 );
        case CTUNE_MOUSERESOLUTION_RELAXED         :
        case CTUNE_MOUSERESOLUTION_SOBER_HIPPY     :
        case CTUNE_MOUSERESOLUTION_STONED_HIPPY    :
        case CTUNE_MOUSERESOLUTION_SLOTH           :
        case CTUNE_MOUSERESOLUTION_SUPPER_SLOTH    : //fallthrough
        case CTUNE_MOUSERESOLUTION_DEATH_CHALLENGED: return values[resolution_e];
        case CTUNE_MOUSERESOLUTION_CUSTOM          : return 0;

        case CTUNE_MOUSERESOLUTION_NOT_SET         : //fallthrough
        default                                    : {
            CTUNE_LOG( CTUNE_LOG_WARNING,
                       "[ctune_MouseResolution_value( %i )] Invalid mouse resolution: '%s'",
                       resolution_e, ctune_MouseResolution.str( resolution_e )
            );

            return mouseinterval( -1 );
        }
    }
}

/**
 * Gets the resolution enum
 * @param ms Value in milliseconds (positive value)
 * @return Resolution enum (when not matched: "custom")
 */
static ctune_MouseResolution_e ctune_MouseResolution_resolution( int ms ) {
    for( int i = 0; i < CTUNE_MOUSERESOLUTION_CUSTOM; ++i ) {
        if( ms == values[i] ) {
            return i; //EARLY RETURN
        }
    }

    return CTUNE_MOUSERESOLUTION_CUSTOM;
}

/**
 * Gets the string representation of the enum
 * @param res MouseResolution enum
 * @return String representation
 */
static const char * ctune_MouseResolution_str( ctune_MouseResolution_e res ) {
    if( res == -1 ) {
        return "NOT_SET";
    } else if( res >= 0 && res < CTUNE_MOUSERESOLUTION_COUNT ) {
        return strings[res];
    } else {
        return "INVALID";
    }
}

/**
 * Namespace declaration
 */
const struct ctune_MouseResolution_Namespace ctune_MouseResolution = {
    .resolution = &ctune_MouseResolution_resolution,
    .value      = &ctune_MouseResolution_value,
    .str        = &ctune_MouseResolution_str,
};