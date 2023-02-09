#ifndef CTUNE_DTO_CATEGORYITEM_H
#define CTUNE_DTO_CATEGORYITEM_H

#include <stdlib.h>
#include <stdio.h>

#include "Field.h"

typedef struct ctune_CategoryItem {
    char * name;
    ulong  stationcount;
    char * data; //multi-purpose string storage ('language': 'iso_639', 'states': 'country')

} ctune_CategoryItem_t;


extern const struct ctune_CategoryItem_Namespace {
    /**
     * Initialise fields in the struct
     * @param cat_item CategoryItem DTO as a void pointer
     */
    void (* init)( void * cat_item );

    /**
     * Frees the content of a CategoryItem DTO
     * @param cat_item CategoryItem DTO
     */
    void (* freeContent)( void * cat_item );

    /**
     * Prints a CategoryItem
     * @param out      Output
     * @param cat_item CategoryItem instance
     */
    void (* print)( FILE * out, const struct ctune_CategoryItem * cat_item );

    /**
     * Gets a field by its name string
     * @param rsi CategoryItem_t object
     * @param api_name Name string
     * @return Field
     */
    ctune_Field_t (* getField)( struct ctune_CategoryItem *cat_item, const char *api_name );

    /**
     * Compares CategoryItems based on their names
     * @param lhs Pointer to a CategoryItem object
     * @param rhs Pointer to a CategoryItem object to compare against
     * @return Result of comparison (-1: less, 0: equal, +1: greater)
     */
    int (* compareByName)( const void * lhs, const void * rhs );

    /**
     * Getters
     */
    struct {
        const char * (* name)( const ctune_CategoryItem_t * cat_item );
        ulong (* stationcount)( const ctune_CategoryItem_t * cat_item );
        const char * (* data)( const ctune_CategoryItem_t * cat_item );
        const char * (* country)( const ctune_CategoryItem_t * cat_item );
    } get;



} ctune_CategoryItem;;

#endif //CTUNE_DTO_CATEGORYITEM_H
