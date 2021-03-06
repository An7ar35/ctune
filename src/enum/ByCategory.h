#ifndef CTUNE_ENUM_BYCATEGORY_H
#define CTUNE_ENUM_BYCATEGORY_H

#include <assert.h>

typedef enum {
    RADIOBROWSER_STATION_BY_UUID = 0,
    RADIOBROWSER_STATION_BY_NAME,
    RADIOBROWSER_STATION_BY_NAME_EXACT,
    RADIOBROWSER_STATION_BY_CODEC,
    RADIOBROWSER_STATION_BY_CODEC_EXACT,
    RADIOBROWSER_STATION_BY_COUNTRY,
    RADIOBROWSER_STATION_BY_COUNTRY_EXACT,
    RADIOBROWSER_STATION_BY_COUNTRY_CODE_EXACT,
    RADIOBROWSER_STATION_BY_STATE,
    RADIOBROWSER_STATION_BY_STATE_EXACT,
    RADIOBROWSER_STATION_BY_LANGUAGE,
    RADIOBROWSER_STATION_BY_LANGUAGE_EXACT,
    RADIOBROWSER_STATION_BY_TAG,
    RADIOBROWSER_STATION_BY_TAG_EXACT,
    RADIOBROWSER_STATION_BY_URL,
    RADIOBROWSER_STATION_BY_CLICKS,
    RADIOBROWSER_STATION_BY_VOTES,
    RADIOBROWSER_STATION_BY_RECENT_CLICKS,
    RADIOBROWSER_STATION_BY_RECENT_ADD_OR_MOD,

    RADIOBROWSER_STATION_BY_COUNT
} ctune_ByCategory_e;


extern const struct ctune_ByCategory_Namespace {
    /**
     * Gets corresponding string for a list category type
     * @param cat ListCategory enum type
     * @return String
     */
    const char * (* str)( ctune_ByCategory_e cat );

} ctune_ByCategory;

#endif //CTUNE_ENUM_BYCATEGORY_H
