#include "ByCategory.h"

/**
 * Gets corresponding string for a list category type
 * @param cat ListCategory enum type
 * @return String
 */
static const char * ctune_ByCategory_str( ctune_ByCategory_e cat ) {
    static const char * arr[RADIOBROWSER_STATION_BY_COUNT] = {
        "byuuid", "byname", "bynameexact", "bycodec", "bycodecexact",
        "bycountry", "bycountryexact", "bycountrycodeexact", "bystate", "bystateexact",
        "bylanguage", "bylanguageexact", "bytag", "bytagexact", "byurl",
        "topclick", "topvote", "lastclick", "lastchange"
    };

    assert( (int) cat >= 0 && (int) cat < RADIOBROWSER_STATION_BY_COUNT );
    return arr[ (int) cat ];
}

/**
 * Namespace constructor
 */
const struct ctune_ByCategory_Namespace ctune_ByCategory = {
    .str = &ctune_ByCategory_str,
};