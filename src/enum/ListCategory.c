#include "ListCategory.h"

/**
 * Gets corresponding string for a list category type
 * @param cat ListCategory enum type
 * @return String
 */
static const char * ctune_ListCategory_str( ctune_ListCategory_e cat ) {
    static const char * arr[6] = {
        "countries", "countrycodes", "codecs", "states", "languages", "tags"
    };

    assert( (int) cat >= 0 && (int) cat < RADIOBROWSER_CATEGORY_COUNT );
    return arr[ (int) cat ];
}

/**
 * Namespace constructor
 */
const struct ctune_ListCategory_Namespace ctune_ListCategory = {
    .str = &ctune_ListCategory_str,
};