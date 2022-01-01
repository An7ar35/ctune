#include "StationAttribute.h"

#include <assert.h>

/**
 * Gets corresponding string for a radio station attribute type
 * @param cat Radio station Attribute enum type
 * @return String
 */
static const char * ctune_StationAttr_str( ctune_StationAttr_e cat ) {
    static const char * arr[STATION_ATTR_COUNT] = {
        [STATION_ATTR_NONE            ] = "",
        [STATION_ATTR_NAME            ] = "name",
        [STATION_ATTR_URL             ] = "url",
        [STATION_ATTR_HOMEPAGE        ] = "homepage",
        [STATION_ATTR_FAVICON         ] = "favicon",
        [STATION_ATTR_TAGS            ] = "tags",
        [STATION_ATTR_COUNTRY         ] = "country",
        [STATION_ATTR_STATE           ] = "state",
        [STATION_ATTR_LANGUAGE        ] = "language",
        [STATION_ATTR_VOTES           ] = "votes",
        [STATION_ATTR_CODEC           ] = "codec",
        [STATION_ATTR_BITRATE         ] = "bitrate",
        [STATION_ATTR_LASTCHECKOK     ] = "lastcheckok",
        [STATION_ATTR_LASTCHECKTIME   ] = "lastchecktime",
        [STATION_ATTR_CLICKTIMESTAMP  ] = "clicktimestamp",
        [STATION_ATTR_CLICKCOUNT      ] = "clickcount",
        [STATION_ATTR_CLICKTREND      ] = "clicktrend",
        [STATION_ATTR_RANDOM          ] = "random",
        [STATION_ATTR_STATIONCOUNT    ] = "stationcount",
        [STATION_ATTR_CHANGETIMESTAMP ] = "changetimestamp",
    };

    assert( (int) cat >= 0 && (int) cat < 19 );
    return arr[ (int) cat ];
}

/**
 * Namespace constructor
 */
const struct ctune_StationAttribute_Namespace ctune_StationAttr = {
    .str = &ctune_StationAttr_str,
};