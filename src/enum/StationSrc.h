#ifndef CTUNE_STATIONSRC_H
#define CTUNE_STATIONSRC_H

typedef enum ctune_StationSrc {
    CTUNE_STATIONSRC_LOCAL = 0,
    CTUNE_STATIONSRC_RADIOBROWSER,
    //append new sources here/do not re-order!
    CTUNE_STATIONSRC_COUNT
} ctune_StationSrc_e;


extern const struct ctune_StationSrc_Namespace {
    /**
     * Gets corresponding string for a radio station source
     * @param e Station source enum type
     * @return String
     */
    const char * (* str)( ctune_StationSrc_e e );

} ctune_StationSrc;

#endif //CTUNE_STATIONSRC_H
