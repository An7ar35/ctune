#ifndef CTUNE_ENUM_MOUSEINTERVAL_H
#define CTUNE_ENUM_MOUSEINTERVAL_H

typedef enum {
    CTUNE_MOUSEINTERVAL_FAST    = 0,
    CTUNE_MOUSEINTERVAL_DEFAULT = 1, //default ncurse (166ms)
    CTUNE_MOUSEINTERVAL_RELAXED,
    CTUNE_MOUSEINTERVAL_HIPPY,
    CTUNE_MOUSEINTERVAL_SLOTH,

    CTUNE_MOUSEINTERVAL_COUNT
} ctune_MouseInterval_e;

/**
 * MouseResolution namespace
 */
const extern struct ctune_MouseInterval_Namespace {
    /**
     * Gets the resolution value
     * @param resolution_e Resolution enum
     * @return Value in milliseconds
     */
    int (* value)( ctune_MouseInterval_e resolution_e );

    /**
     * Gets the string representation of the enum
     * @param res MouseResolution enum
     * @return String representation
     */
    const char * (* str)( ctune_MouseInterval_e res );

} ctune_MouseInterval;

#endif //CTUNE_ENUM_MOUSEINTERVAL_H
