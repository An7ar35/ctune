#include "CategoryItem.h"

#include <string.h>

/**
 * Initialise fields in the struct
 * @param cat_item CategoryItem DTO as a void pointer
 */
static void ctune_CategoryItem_init( void * cat_item ) {
    ( (struct ctune_CategoryItem *) cat_item )->name         = NULL;
    ( (struct ctune_CategoryItem *) cat_item )->stationcount = 0;
    ( (struct ctune_CategoryItem *) cat_item )->country      = NULL;
}

/**
 * Frees the content of a CategoryItem DTO
 * @param cat_item CategoryItem DTO
 */
static void ctune_CategoryItem_freeContent( void * cat_item ) {
    if( cat_item == NULL )
        return; //EARLY RETURN

    ctune_CategoryItem_t * ci = (struct ctune_CategoryItem *) cat_item;

    if( ci->name ) {
        free( ci->name );
        ci->name = NULL;
    }

    if( ci->country ) {
        free( ci->country );
        ci->country = NULL;
    }
}

/**
 * Prints a CategoryItem
 * @param out      Output
 * @param cat_item CategoryItem instance
 */
static void ctune_CategoryItem_print( FILE * out, const struct ctune_CategoryItem * cat_item ) {
    fprintf( out, "{ name: \"%s\", stationcount: %lu", cat_item->name, cat_item->stationcount );
    if( cat_item->country )
        fprintf( out, ", country: \"%s\" }", cat_item->country );
    else
        fprintf( out, " }" );
}

/**
 * Gets a field by its name string
 * @param rsi CategoryItem object
 * @param api_name Name string
 * @return Field
 */
inline static ctune_Field_t ctune_ServerStats_getField( struct ctune_CategoryItem *cat_item, const char *api_name ) {
    if( strcmp( api_name, "name" ) == 0 ) {
        return (ctune_Field_t){ ._field = &cat_item->name, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if(strcmp( api_name, "stationcount" ) == 0 ) {
        return (ctune_Field_t){ ._field = &cat_item->stationcount, ._type = CTUNE_FIELD_UNSIGNED_LONG };

    } else if(strcmp( api_name, "country" ) == 0 ) {
        return (ctune_Field_t){ ._field = &cat_item->country, ._type = CTUNE_FIELD_CHAR_PTR };
        //INFO (28 Sept 2020): The RadioBrowser API returns "Array" when there are multiple countries with the same state name
    } else {
        return (ctune_Field_t) { ._field = NULL, ._type = CTUNE_FIELD_UNKNOWN };
    }
}

/**
 * Namespace constructor
 */
const struct ctune_CategoryItem_Namespace ctune_CategoryItem = {
    .init        = &ctune_CategoryItem_init,
    .freeContent = &ctune_CategoryItem_freeContent,
    .print       = &ctune_CategoryItem_print,
    .getField    = &ctune_ServerStats_getField,
};