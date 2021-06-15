#ifndef CTUNE_ENUM_LISTCATEGORY_H
#define CTUNE_ENUM_LISTCATEGORY_H

#include <assert.h>

typedef enum {
    RADIOBROWSER_CATEGORY_COUNTRIES    = 0,
    RADIOBROWSER_CATEGORY_COUNTRYCODES = 1,
    RADIOBROWSER_CATEGORY_CODECS       = 2,
    RADIOBROWSER_CATEGORY_STATES       = 3,
    RADIOBROWSER_CATEGORY_LANGUAGES    = 4,
    RADIOBROWSER_CATEGORY_TAGS         = 5,

    RADIOBROWSER_CATEGORY_COUNT
} ctune_ListCategory_e;


extern const struct ctune_ListCategory_Namespace {
    /**
     * Gets corresponding string for a list category type
     * @param cat ListCategory enum type
     * @return String
     */
    const char * (* str)( ctune_ListCategory_e cat );

} ctune_ListCategory;

#endif //CTUNE_ENUM_LISTCATEGORY_H
