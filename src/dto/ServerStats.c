#include "ServerStats.h"

/**
 * Initialise fields in the struct
 * @param stats ServerStats DTO pointer
 */
static void ctune_ServerStats_init( struct ctune_ServerStats * stats ) {
    stats->supported_version = NULL;
    stats->software_version  = NULL;
    stats->status            = NULL;
    stats->stations          = 0;
    stats->stations_broken   = 0;
    stats->tags              = 0;
    stats->clicks_last_hour  = 0;
    stats->clicks_last_day   = 0;
    stats->languages         = 0;
    stats->countries         = 0;
}

/**
 * Frees the content of a ServerStats DTO
 * @param stats ServerStats DTO
 */
static void ctune_ServerStats_freeContent( struct ctune_ServerStats * stats ) {
    if( stats == NULL )
        return; //EARLY RETURN

    if( stats->supported_version ) {
        free( stats->supported_version );
        stats->supported_version = NULL;
    }

    if( stats->software_version ) {
        free( stats->software_version );
        stats->software_version = NULL;
    }

    if( stats->status ) {
        free( stats->status );
        stats->status = NULL;
    }
}

/**
 * Prints a ServerStats
 * @param out   Output
 * @param stats ServerStats instance
 */
static void ctune_ServerStats_print( FILE * out, const struct ctune_ServerStats * stats ) {
    fprintf( out, "Supported version : %s\n", stats->supported_version );
    fprintf( out, "Software version .: %s\n", stats->software_version );
    fprintf( out, "Server status ....: %s\n", stats->status );
    fprintf( out, "Stations .........: %lu\n", stats->stations );
    fprintf( out, "Stations broken ..: %lu\n", stats->stations_broken );
    fprintf( out, "Tags .............: %lu\n", stats->tags );
    fprintf( out, "Clicks last hour .: %lu\n", stats->clicks_last_hour );
    fprintf( out, "Clicks last day ..: %lu\n", stats->clicks_last_day );
    fprintf( out, "Languages ........: %lu\n", stats->languages );
    fprintf( out, "Countries ........: %lu\n", stats->countries );
}

/**
 * Namespace constructor
 */
const struct ctune_ServerStats_Namespace ctune_ServerStats = {
    .init        = &ctune_ServerStats_init,
    .freeContent = &ctune_ServerStats_freeContent,
    .print       = &ctune_ServerStats_print,
};