#ifndef CTUNE_DTO_STATIONCLICKCOUNTER_H
#define CTUNE_DTO_STATIONCLICKCOUNTER_H

#include <stdlib.h>
#include <stdio.h>

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

} ctune_ClickCounter;

#endif //CTUNE_DTO_STATIONCLICKCOUNTER_H
