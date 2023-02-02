#include "ClickCounter.h"

#include <string.h>

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

    free( cc->ok );
    cc->ok = NULL;

    free( cc->message );
    cc->message = NULL;

    free( cc->stationuuid );
    cc->stationuuid = NULL;

    free( cc->name );
    cc->name = NULL;

    free( cc->url );
    cc->url = NULL;
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
 * Gets a field by its name string
 * @param rsi ClickCounter_t object
 * @param api_name Name string
 * @return Field
 */
inline static ctune_Field_t ctune_StationClickCounter_getField( struct ctune_ClickCounter *clk_counter, const char *api_name ) {
    if( strcmp( api_name, "name" ) == 0 ) {
        return ( ctune_Field_t ) { ._field = &clk_counter->name, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "stationuuid" ) == 0 ) {
        return ( ctune_Field_t ) { ._field = &clk_counter->stationuuid, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "url" ) == 0 ) {
        return (ctune_Field_t){ ._field = &clk_counter->url, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "ok" ) == 0 ) {
        return (ctune_Field_t){ ._field = &clk_counter->ok, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "message" ) == 0 ) {
        return (ctune_Field_t){ ._field = &clk_counter->message, ._type = CTUNE_FIELD_CHAR_PTR };

    } else {
        return (ctune_Field_t) { ._field = NULL, ._type = CTUNE_FIELD_UNKNOWN };
    }
}

/**
 * Namespace constructor
 */
const struct ctune_ClickCounter_Namespace ctune_ClickCounter = {
    .init        = &ctune_StationClickCounter_init,
    .freeContent = &ctune_StationClickCounter_freeContent,
    .print       = &ctune_StationClickCounter_print,
    .getField    = &ctune_StationClickCounter_getField,
};