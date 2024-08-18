#include "Flag.h"

#include <assert.h>

/**
 * Gets corresponding string for a Flag
 * @param id Flag enum type
 * @return String
 */
static const char * ctune_Flag_str( ctune_Flag_e id ) {
    static const char * str[3] = {
        [FLAG_SET_OFF  ] = "SET_OFF",
        [FLAG_SET_ON   ] = "SET_ON",
        [FLAG_GET_VALUE] = "GET_VALUE",
    };

    assert( id >= 0 && id < 3 );
    return str[id];
}

/**
 * Namespace constructor
 */
const struct ctune_Flag_Namespace ctune_Flag = {
    .str = &ctune_Flag_str,
};