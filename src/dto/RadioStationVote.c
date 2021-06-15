#include "RadioStationVote.h"

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
 * Namespace constructor
 */
const struct ctune_RadioStationVote_Namespace ctune_RadioStationVote = {
    .init        = &ctune_RadioStationVote_init,
    .freeContent = &ctune_StationRadioStationVote_freeContent,
    .print       = &ctune_StationRadioStationVote_print,
};