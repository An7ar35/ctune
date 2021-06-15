#include "StationSrc.h"

#include <assert.h>

/**
 * Gets corresponding string for a radio station source
 * @param e Station source enum type
 * @return String
 */
const char * ctune_StationSrc_str( ctune_StationSrc_e e ) {
    static const char * arr[CTUNE_STATIONSRC_COUNT] = {
        [CTUNE_STATIONSRC_LOCAL       ] = "local",
        [CTUNE_STATIONSRC_RADIOBROWSER] = "radiobrowser",
    };

    assert( (int) e >= 0 && (int) e < CTUNE_STATIONSRC_COUNT );
    return arr[ (int) e ];
}

/**
 * Namespace constructor
 */
const struct ctune_StationSrc_Namespace ctune_StationSrc = {
    .str = &ctune_StationSrc_str,
};