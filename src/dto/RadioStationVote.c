#include "RadioStationVote.h"

#include <string.h>

/**
 * Initialise fields in the struct
 * @param stats RadioStationVote DTO pointer
 */
static void ctune_RadioStationVote_init( void * rsv ) {
    ( (struct ctune_RadioStationVote *) rsv )->ok      = NULL;
    ( (struct ctune_RadioStationVote *) rsv )->message = NULL;
}

/**
 * Frees the content of a RadioStationVote DTO
 * @param stats RadioStationVote DTO
 */
static void ctune_StationRadioStationVote_freeContent( void * rsv ) {
    if( rsv == NULL )
        return; //EARLY RETURN

    ctune_RadioStationVote_t * o = (struct ctune_RadioStationVote *) rsv;

    if( o->ok ) {
        free( o->ok );
        o->ok = NULL;
    }

    if( o->message ) {
        free( o->message );
        o->message = NULL;
    }
}

/**
 * Prints a RadioStationVote
 * @param out   Output
 * @param stats RadioStationVote instance
 */
static void ctune_StationRadioStationVote_print( FILE * out, const struct ctune_RadioStationVote * rsv ) {
    fprintf( out, "OK ....: %s\n", rsv->ok );
    fprintf( out, "Message: %s",   rsv->message );
}

/**
 * Gets a field by its name string
 * @param rsi ClickCounter_t object
 * @param api_name Name string
 * @return Field
 */
inline static ctune_Field_t ctune_StationRadioStationVote_getField( struct ctune_RadioStationVote *rsv, const char *api_name ) {
    if( strcmp( api_name, "ok" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsv->ok, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "message" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsv->message, ._type = CTUNE_FIELD_CHAR_PTR };

    } else {
        return (ctune_Field_t) { ._field = NULL, ._type = CTUNE_FIELD_UNKNOWN };
    }
}

/**
 * Namespace constructor
 */
const struct ctune_RadioStationVote_Namespace ctune_RadioStationVote = {
    .init        = &ctune_RadioStationVote_init,
    .freeContent = &ctune_StationRadioStationVote_freeContent,
    .print       = &ctune_StationRadioStationVote_print,
    .getField    = &ctune_StationRadioStationVote_getField,
};