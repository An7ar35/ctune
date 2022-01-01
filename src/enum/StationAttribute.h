#ifndef CTUNE_ENUM_BYATTRIBUTE_H
#define CTUNE_ENUM_BYATTRIBUTE_H

/**
 * RadioBrowser/Remote sorting attributes
 */
typedef enum {
    STATION_ATTR_NONE            =  0,
    STATION_ATTR_NAME            =  1,
    STATION_ATTR_URL             =  2,
    STATION_ATTR_HOMEPAGE        =  3,
    STATION_ATTR_FAVICON         =  4,
    STATION_ATTR_TAGS            =  5,
    STATION_ATTR_COUNTRY         =  6,
    STATION_ATTR_STATE           =  7,
    STATION_ATTR_LANGUAGE        =  8,
    STATION_ATTR_VOTES           =  9,
    STATION_ATTR_CODEC           = 10,
    STATION_ATTR_BITRATE         = 11,
    STATION_ATTR_LASTCHECKOK     = 12,
    STATION_ATTR_LASTCHECKTIME   = 13,
    STATION_ATTR_CLICKTIMESTAMP  = 14,
    STATION_ATTR_CLICKCOUNT      = 15,
    STATION_ATTR_CLICKTREND      = 16,
    STATION_ATTR_RANDOM          = 17,
    STATION_ATTR_STATIONCOUNT    = 18,
    STATION_ATTR_CHANGETIMESTAMP = 19,

    STATION_ATTR_COUNT
} ctune_StationAttr_e;

extern const struct ctune_StationAttribute_Namespace {
    /**
     * Gets corresponding string for a radio station attribute type
     * @param cat Radio station Attribute enum type
     * @return String
     */
    const char * (* str)( ctune_StationAttr_e cat );

} ctune_StationAttr;

#endif //CTUNE_ENUM_BYATTRIBUTE_H
