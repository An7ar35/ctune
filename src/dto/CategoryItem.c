#include "CategoryItem.h"

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
 * Namespace constructor
 */
const struct ctune_CategoryItem_Namespace ctune_CategoryItem = {
    .init        = &ctune_CategoryItem_init,
    .freeContent = &ctune_CategoryItem_freeContent,
    .print       = &ctune_CategoryItem_print,
};