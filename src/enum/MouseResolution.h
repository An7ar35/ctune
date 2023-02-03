#ifndef CTUNE_ENUM_MOUSERESOLUTION_H
#define CTUNE_ENUM_MOUSERESOLUTION_H

typedef enum {
    CTUNE_MOUSERESOLUTION_NOT_SET = -1,
    CTUNE_MOUSERESOLUTION_DEFAULT =  0,  //default ncurse
    CTUNE_MOUSERESOLUTION_RELAXED,
    CTUNE_MOUSERESOLUTION_SOBER_HIPPY,
    CTUNE_MOUSERESOLUTION_STONED_HIPPY,
    CTUNE_MOUSERESOLUTION_SLOTH,
    CTUNE_MOUSERESOLUTION_SUPPER_SLOTH,
    CTUNE_MOUSERESOLUTION_DEATH_CHALLENGED,

    CTUNE_MOUSERESOLUTION_CUSTOM,
    CTUNE_MOUSERESOLUTION_COUNT
} ctune_MouseResolution_e;

/**
 * MouseResolution namespace
 */
const extern struct ctune_MouseResolution_Namespace {
    /**
     * Gets the resolution value
     * @param resolution_e Resolution enum
     * @return Value in milliseconds
     */
    int (* value)( ctune_MouseResolution_e resolution_e );

    /**
     * Gets the resolution enum
     * @param ms Value in milliseconds
     * @return Resolution enum
     */
    ctune_MouseResolution_e (* resolution)( int ms );

    /**
     * Gets the string representation of the enum
     * @param res MouseResolution enum
     * @return String representation
     */
    const char * (* str)( ctune_MouseResolution_e res );

} ctune_MouseResolution;

#endif //CTUNE_ENUM_MOUSERESOLUTION_H
