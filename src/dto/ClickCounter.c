#include "ClickCounter.h"

/**
 * Initialise fields in the struct
 * @param stats StationClickCounter DTO pointer
 */
static void ctune_StationClickCounter_init( void * clk_counter ) {
    ( (struct ctune_ClickCounter *) clk_counter )->ok          = NULL;
    ( (struct ctune_ClickCounter *) clk_counter )->message     = NULL;
    ( (struct ctune_ClickCounter *) clk_counter )->stationuuid = NULL;
    ( (struct ctune_ClickCounter *) clk_counter )->name        = NULL;
    ( (struct ctune_ClickCounter *) clk_counter )->url         = NULL;
}

/**
 * Frees the content of a StationClickCounter DTO
 * @param stats StationClickCounter DTO
 */
static void ctune_StationClickCounter_freeContent( void * clk_counter ) {
    if( clk_counter == NULL )
        return; //EARLY RETURN

    ctune_ClickCounter_t * cc = (struct ctune_ClickCounter *) clk_counter;

    if( cc->ok ) {
        free( cc->ok );
        cc->ok = NULL;
    }

    if( cc->message ) {
        free( cc->message );
        cc->message = NULL;
    }

    if( cc->stationuuid ) {
        free( cc->stationuuid );
        cc->stationuuid = NULL;
    }

    if( cc->name ) {
        free( cc->name );
        cc->name = NULL;
    }

    if( cc->url ) {
        free( cc->url );
        cc->url = NULL;
    }
}

/**
 * Prints a StationClickCounter
 * @param out   Output
 * @param stats StationClickCounter instance
 */
static void ctune_StationClickCounter_print( FILE * out, const struct ctune_ClickCounter * clk_counter ) {
    fprintf( out, "Name ..: %s\n", clk_counter->name );
    fprintf( out, "UUID ..: %s\n", clk_counter->stationuuid );
    fprintf( out, "URL ...: %s\n", clk_counter->url );
    fprintf( out, "OK ....: %s\n", clk_counter->ok );
    fprintf( out, "Message: %s",   clk_counter->message );
}

/**
 * Namespace constructor
 */
const struct ctune_ClickCounter_Namespace ctune_ClickCounter = {
    .init        = &ctune_StationClickCounter_init,
    .freeContent = &ctune_StationClickCounter_freeContent,
    .print       = &ctune_StationClickCounter_print,
};