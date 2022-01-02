#ifndef CTUNE_DTO_STATIONCLICKCOUNTER_H
#define CTUNE_DTO_STATIONCLICKCOUNTER_H

#include <stdlib.h>
#include <stdio.h>

#include "Field.h"

typedef struct ctune_ClickCounter {
    char * ok;
    char * message;
    char * stationuuid;
    char * name;
    char * url;

} ctune_ClickCounter_t;


extern const struct ctune_ClickCounter_Namespace {
    /**
     * Initialise fields in the struct
     * @param stats StationClickCounter DTO pointer
     */
    void (* init)( void * clk_counter );

    /**
     * Frees the content of a StationClickCounter DTO
     * @param stats StationClickCounter DTO
     */
    void (* freeContent)( void * clk_counter );

    /**
     * Prints a StationClickCounter
     * @param out   Output
     * @param stats StationClickCounter instance
     */
    void (* print)( FILE * out, const struct ctune_ClickCounter * clk_counter );

    /**
     * Gets a field by its name string
     * @param rsi ClickCounter_t object
     * @param api_name Name string
     * @return Field
     */
    ctune_Field_t (* getField)( struct ctune_ClickCounter *clk_counter, const char *api_name );

} ctune_ClickCounter;

#endif //CTUNE_DTO_STATIONCLICKCOUNTER_H
