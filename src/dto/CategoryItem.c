#include "CategoryItem.h"

#include <string.h>

/**
 * Initialise fields in the struct
 * @param cat_item CategoryItem DTO as a void pointer
 */
static void ctune_CategoryItem_init( void * cat_item ) {
    ( (struct ctune_CategoryItem *) cat_item )->name         = NULL;
    ( (struct ctune_CategoryItem *) cat_item )->stationcount = 0;
    ( (struct ctune_CategoryItem *) cat_item )->data         = NULL;
}

/**
 * Frees the content of a CategoryItem DTO
 * @param cat_item CategoryItem DTO
 */
static void ctune_CategoryItem_freeContent( void * cat_item ) {
    if( cat_item == NULL )
        return; //EARLY RETURN

    ctune_CategoryItem_t * ci = (struct ctune_CategoryItem *) cat_item;

    free( ci->name );
    ci->name = NULL;

    free( ci->data );
    ci->data = NULL;
}

/**
 * Prints a CategoryItem
 * @param out      Output
 * @param cat_item CategoryItem instance
 */
static void ctune_CategoryItem_print( FILE * out, const struct ctune_CategoryItem * cat_item ) {
    fprintf( out, "{ name: \"%s\", stationcount: %lu", cat_item->name, cat_item->stationcount );
    if( cat_item->data )
        fprintf( out, ", data: \"%s\" }", cat_item->data );
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

    } else if( strcmp( api_name, "stationcount" ) == 0 ) {
        return (ctune_Field_t){ ._field = &cat_item->stationcount, ._type = CTUNE_FIELD_UNSIGNED_LONG };

    } else if( strcmp( api_name, "country" ) == 0 ) {
        return  ( ctune_Field_t ) { ._field = &cat_item->data, ._type = CTUNE_FIELD_CHAR_PTR };
        //INFO (28 Sept 2020): The RadioBrowser API returns "Array" when there are multiple countries with the same state name
    } else if( strcmp( api_name, "iso_639" ) == 0 ) {
        return (ctune_Field_t){ ._field = &cat_item->data, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "iso_3166_1" ) == 0 ) {
        return (ctune_Field_t){ ._field = &cat_item->data, ._type = CTUNE_FIELD_CHAR_PTR };

    } else {
        return (ctune_Field_t) { ._field = NULL, ._type = CTUNE_FIELD_UNKNOWN };
    }
}

/**
 * Compares CategoryItems based on their names
 * @param lhs Pointer to a CategoryItem object
 * @param rhs Pointer to a CategoryItem object to compare against
 * @return Result of comparison (-1: less, 0: equal, +1: greater)
 */
static int ctune_CategoryItem_compareByName( const void * lhs, const void * rhs ) {
    if( lhs && rhs ) {
        return strcmp( ( (ctune_CategoryItem_t *) lhs )->name,
                       ( (ctune_CategoryItem_t *) rhs )->name );
    }

    return ( lhs ? -1 : ( rhs ? +1 : 0 ) );
}

/**
 * Gets the name of the category
 * @param cat_item CategoryItem object
 * @return Name string
 */
static const char * ctune_CategoryItem_get_name( const struct ctune_CategoryItem * cat_item ) {
    return cat_item->name;
}

/**
 * Gets the station count of the category
 * @param cat_item CategoryItem object
 * @return Station count
 */
static ulong ctune_CategoryItem_get_stationcount( const struct ctune_CategoryItem * cat_item ) {
    return cat_item->stationcount;
}

/**
 * Gets the anonymous data store of the category
 * @param cat_item CategoryItem object
 * @return Data string
 */
static const char * ctune_CategoryItem_get_data( const struct ctune_CategoryItem * cat_item ) {
    return cat_item->data;
}

/**
 * Gets the country of the category
 * @param cat_item CategoryItem object
 * @return Country string
 */
static const char * ctune_CategoryItem_get_country( const struct ctune_CategoryItem * cat_item ) {
    return cat_item->data;
}

/**
 * Namespace constructor
 */
const struct ctune_CategoryItem_Namespace ctune_CategoryItem = {
    .init          = &ctune_CategoryItem_init,
    .freeContent   = &ctune_CategoryItem_freeContent,
    .print         = &ctune_CategoryItem_print,
    .getField      = &ctune_ServerStats_getField,
    .compareByName = &ctune_CategoryItem_compareByName,
    .get =  {
        .name         = &ctune_CategoryItem_get_name,
        .stationcount = &ctune_CategoryItem_get_stationcount,
        .data         = &ctune_CategoryItem_get_data,
        .country      = &ctune_CategoryItem_get_country,
    }
};