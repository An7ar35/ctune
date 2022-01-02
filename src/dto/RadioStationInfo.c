#include "RadioStationInfo.h"

/**
 * Initialize fields in the struct
 * @param rsi RadioStationInfo DTO as a void pointer
 */
static void ctune_RadioStationInfo_init( void * rsi ) {
    ( (struct ctune_RadioStationInfo *) rsi )->change_uuid            = NULL;
    ( (struct ctune_RadioStationInfo *) rsi )->station_uuid           = NULL;
    ( (struct ctune_RadioStationInfo *) rsi )->server_uuid            = NULL;
    ( (struct ctune_RadioStationInfo *) rsi )->name                   = NULL;
    ( (struct ctune_RadioStationInfo *) rsi )->url                    = NULL;
    ( (struct ctune_RadioStationInfo *) rsi )->url_resolved           = NULL;
    ( (struct ctune_RadioStationInfo *) rsi )->homepage               = NULL;
    ( (struct ctune_RadioStationInfo *) rsi )->favicon_url            = NULL;
    ( (struct ctune_RadioStationInfo *) rsi )->tags                   = NULL;
    ( (struct ctune_RadioStationInfo *) rsi )->country                = NULL;
    ( (struct ctune_RadioStationInfo *) rsi )->country_code.iso3166_1 = NULL;
    ( (struct ctune_RadioStationInfo *) rsi )->country_code.iso3166_2 = NULL;
    ( (struct ctune_RadioStationInfo *) rsi )->state                  = NULL;
    ( (struct ctune_RadioStationInfo *) rsi )->language               = NULL;
    ( (struct ctune_RadioStationInfo *) rsi )->language_codes         = NULL;
    ( (struct ctune_RadioStationInfo *) rsi )->votes                  = 0;
    ( (struct ctune_RadioStationInfo *) rsi )->last_change_time       = NULL;
    ( (struct ctune_RadioStationInfo *) rsi )->codec                  = NULL;
    ( (struct ctune_RadioStationInfo *) rsi )->bitrate                = 0;
    ( (struct ctune_RadioStationInfo *) rsi )->hls                    = false;
    ( (struct ctune_RadioStationInfo *) rsi )->last_check_ok          = false;
    ( (struct ctune_RadioStationInfo *) rsi )->last_check_time        = NULL;
    ( (struct ctune_RadioStationInfo *) rsi )->last_check_ok_time     = NULL;
    ( (struct ctune_RadioStationInfo *) rsi )->last_local_check_time  = NULL;
    ( (struct ctune_RadioStationInfo *) rsi )->click_timestamp        = NULL;
    ( (struct ctune_RadioStationInfo *) rsi )->clickcount             = 0;
    ( (struct ctune_RadioStationInfo *) rsi )->clicktrend             = 0;
    ( (struct ctune_RadioStationInfo *) rsi )->broken                 = false;
    ( (struct ctune_RadioStationInfo *) rsi )->ssl_error              = 0;

    ( (struct ctune_RadioStationInfo *) rsi )->iso8601.last_change_time      = NULL;
    ( (struct ctune_RadioStationInfo *) rsi )->iso8601.last_check_time       = NULL;
    ( (struct ctune_RadioStationInfo *) rsi )->iso8601.last_check_ok_time    = NULL;
    ( (struct ctune_RadioStationInfo *) rsi )->iso8601.last_local_check_time = NULL;
    ( (struct ctune_RadioStationInfo *) rsi )->iso8601.click_timestamp       = NULL;

    ( (struct ctune_RadioStationInfo *) rsi )->geo.latitude          = 0.0;
    ( (struct ctune_RadioStationInfo *) rsi )->geo.longitude         = 0.0;

    ( (struct ctune_RadioStationInfo *) rsi )->is_favourite          = false;
    ( (struct ctune_RadioStationInfo *) rsi )->station_src           = CTUNE_STATIONSRC_LOCAL;
}

/**
 * Copies a RadioStationInfo DTO into another
 * @param lhs Pointer to origin DTO
 * @param rhs Pointer to destination DTO (assumed to be already allocated and initialised)
 */
static void ctune_RadioStationInfo_copy( const void * lhs, void * rhs ) {
    if( lhs == rhs )
        return; //pointer to the same object

    ctune_RadioStationInfo_t * from = (struct ctune_RadioStationInfo *) lhs;
    ctune_RadioStationInfo_t * dest = (struct ctune_RadioStationInfo *) rhs;

    if( from->change_uuid )
        dest->change_uuid = strdup( from->change_uuid );
    if( from->station_uuid )
        dest->station_uuid = strdup( from->station_uuid );
    if( from->server_uuid )
        dest->server_uuid = strdup( from->server_uuid );
    if( from->name )
        dest->name = strdup( from->name );
    if( from->url )
        dest->url = strdup( from->url );
    if( from->url_resolved )
        dest->url_resolved = strdup( from->url_resolved );
    if( from->homepage )
        dest->homepage = strdup( from->homepage );
    if( from->favicon_url )
        dest->favicon_url = strdup( from->favicon_url );
    if( from->tags )
        dest->tags = strdup( from->tags );
    if( from->country )
        dest->country = strdup( from->country );
    if( from->country_code.iso3166_1 )
        dest->country_code.iso3166_1 = strdup( from->country_code.iso3166_1 );
    if( from->country_code.iso3166_2 )
        dest->country_code.iso3166_2 = strdup( from->country_code.iso3166_2 );
    if( from->state )
        dest->state = strdup( from->state );
    if( from->language )
        dest->language = strdup( from->language );
    if( from->language_codes )
        dest->language_codes = strdup( from->language_codes );
    dest->votes = from->votes;
    if( from->last_change_time )
        dest->last_change_time = strdup( from->last_change_time );
    if( from->codec )
        dest->codec = strdup( from->codec );
    dest->bitrate       = from->bitrate;
    dest->hls           = from->hls;
    dest->last_check_ok = from->last_check_ok;
    if( from->last_check_time )
        dest->last_check_time = strdup( from->last_check_time );
    if( from->last_check_ok_time )
        dest->last_check_ok_time = strdup( from->last_check_ok_time );
    if( from->last_local_check_time )
        dest->last_local_check_time = strdup( from->last_local_check_time );
    if( from->click_timestamp )
        dest->click_timestamp = strdup( from->click_timestamp );
    dest->clickcount   = from->clickcount;
    dest->clicktrend   = from->clicktrend;
    dest->broken       = from->broken;
    dest->ssl_error    = from->ssl_error;

    if( from->iso8601.last_change_time )
        dest->iso8601.last_change_time = strdup( from->iso8601.last_change_time );
    if( from->iso8601.last_check_time )
        dest->iso8601.last_check_time = strdup( from->iso8601.last_check_time );
    if( from->iso8601.last_check_ok_time )
        dest->iso8601.last_check_ok_time = strdup( from->iso8601.last_check_ok_time );
    if( from->iso8601.last_local_check_time )
        dest->iso8601.last_local_check_time = strdup( from->iso8601.last_local_check_time );
    if( from->iso8601.click_timestamp )
        dest->iso8601.click_timestamp = strdup( from->iso8601.click_timestamp );

    dest->geo          = from->geo;
    dest->is_favourite = from->is_favourite;
    dest->station_src  = from->station_src;
}

/**
 * Copies the minimal and most essential fields from a RadioStationInfo DTO into another
 * @param lhs Origin DTO
 * @param rhs Destination DTO (assumed to be already allocated and initialised)
 */
static void ctune_RadioStationInfo_mincopy( const void * lhs, void * rhs ) {
    if( lhs == rhs )
        return; //pointer to the same object

    ctune_RadioStationInfo_t * from = (struct ctune_RadioStationInfo *) lhs;
    ctune_RadioStationInfo_t * dest = (struct ctune_RadioStationInfo *) rhs;

    if( from->station_uuid )
        dest->station_uuid = strdup( from->station_uuid );
    if( from->name )
        dest->name = strdup( from->name );
    if( from->url )
        dest->url = strdup( from->url );
    if( from->url_resolved )
        dest->url_resolved = strdup( from->url_resolved );
    if( from->homepage )
        dest->homepage = strdup( from->homepage );
    if( from->favicon_url )
        dest->favicon_url = strdup( from->favicon_url );
    if( from->tags )
        dest->tags = strdup( from->tags );
    if( from->country )
        dest->country = strdup( from->country );
    if( from->country_code.iso3166_1 )
        dest->country_code.iso3166_1 = strdup( from->country_code.iso3166_1 );
    if( from->country_code.iso3166_2 )
        dest->country_code.iso3166_2 = strdup( from->country_code.iso3166_2 );
    if( from->state )
        dest->state = strdup( from->state );
    if( from->language )
        dest->language = strdup( from->language );
    if( from->language_codes )
        dest->language_codes = strdup( from->language_codes );
    if( from->last_change_time )
        dest->last_change_time = strdup( from->last_change_time );
    if( from->codec )
        dest->codec = strdup( from->codec );
    dest->bitrate       = from->bitrate;
    dest->last_check_ok = from->last_check_ok;

    dest->broken       = from->broken;

    if( from->iso8601.last_change_time )
        dest->iso8601.last_change_time = strdup( from->iso8601.last_change_time );
    if( from->iso8601.last_check_time )
        dest->iso8601.last_check_time = strdup( from->iso8601.last_check_time );
    if( from->iso8601.last_check_ok_time )
        dest->iso8601.last_check_ok_time = strdup( from->iso8601.last_check_ok_time );
    if( from->iso8601.last_local_check_time )
        dest->iso8601.last_local_check_time = strdup( from->iso8601.last_local_check_time );

    dest->geo       = from->geo;

    dest->is_favourite = from->is_favourite;
    dest->station_src  = from->station_src;
}

/**
 * Creates an allocated duplicate of a RadioStationInfo_t object
 * @param rsi Source
 * @return Pointer to duplicate
 */
static void * ctune_RadioStationInfo_dup( const void * rsi ) {
    ctune_RadioStationInfo_t * dup = malloc( sizeof( ctune_RadioStationInfo_t ) );

    if( dup != NULL ) {
        ctune_RadioStationInfo_init( dup );
        ctune_RadioStationInfo_copy( rsi, dup );
    }

    return dup;
}

/**
 * Creates an allocated minimal duplicate of a RadioStationInfo_t object
 * @param rsi Source
 * @return Pointer to duplicate
 */
static void * ctune_RadioStationInfo_mindup( const void * rsi ) {
    ctune_RadioStationInfo_t * dup = malloc( sizeof( ctune_RadioStationInfo_t ) );

    if( dup != NULL ) {
        ctune_RadioStationInfo_init( dup );
        ctune_RadioStationInfo_mincopy( rsi, dup );
    }

    return dup;
}

/**
 * Checks equivalence of 2 RadioStationInfo_t objects (i.e.: `station_uuid` values are the same)
 * @param lhs RadioStationInfo_t object
 * @param rhs RadioStationInfo_t object
 * @return Equivalent state
 */
static bool ctune_RadioStationInfo_equalUUID( const void * lhs, const void * rhs ) {
    if( lhs == NULL || rhs == NULL )
        return false;

    ctune_RadioStationInfo_t * rsi_a = (struct ctune_RadioStationInfo *) lhs;
    ctune_RadioStationInfo_t * rsi_b = (struct ctune_RadioStationInfo *) rhs;

    if( rsi_a->station_uuid == NULL || rsi_b == NULL )
        return false;

    return ( strcmp( rsi_a->station_uuid, rsi_b->station_uuid ) == 0 );
}

/**
 * Checks equivalence of 2 RadioStationInfo_t objects (all field except internal ones)
 * @param lhs RadioStationInfo_t object
 * @param rhs RadioStationInfo_t object
 * @return Equivalent state
 */
static bool ctune_RadioStationInfo_equal( const void * lhs, const void * rhs ) {
    if( lhs == NULL || rhs == NULL )
        return false;

    ctune_RadioStationInfo_t * rsi_a = (struct ctune_RadioStationInfo *) lhs;
    ctune_RadioStationInfo_t * rsi_b = (struct ctune_RadioStationInfo *) rhs;

    if( !ctune_streq( rsi_a->change_uuid, rsi_b->change_uuid ) )
        return false;

    if( !ctune_streq( rsi_a->station_uuid, rsi_b->station_uuid ) )
        return false;

    if( !ctune_streq( rsi_a->server_uuid, rsi_b->server_uuid ) )
        return false;

    if( !ctune_streq( rsi_a->name, rsi_b->name ) )
        return false;

    if( !ctune_streq( rsi_a->url, rsi_b->url ) )
        return false;

    if( !ctune_streq( rsi_a->url_resolved, rsi_b->url_resolved ) )
        return false;

    if( !ctune_streq( rsi_a->favicon_url, rsi_b->favicon_url ) )
        return false;

    if( !ctune_streq( rsi_a->tags, rsi_b->tags ) )
        return false;

    if( !ctune_streq( rsi_a->country, rsi_b->country ) )
        return false;

    if( !ctune_streq( rsi_a->country_code.iso3166_1, rsi_b->country_code.iso3166_1 ) )
        return false;

    if( !ctune_streq( rsi_a->country_code.iso3166_2, rsi_b->country_code.iso3166_2 ) )
        return false;

    if( !ctune_streq( rsi_a->state, rsi_b->state ) )
        return false;

    if( !ctune_streq( rsi_a->language, rsi_b->language ) )
        return false;

    if( !ctune_streq( rsi_a->language_codes, rsi_b->language_codes ) )
        return false;

    if( rsi_a->votes != rsi_b->votes )
        return false;

    if( !ctune_streq( rsi_a->last_change_time, rsi_b->last_change_time ) )
        return false;

    if( !ctune_streq( rsi_a->codec, rsi_b->codec ) )
        return false;

    if( rsi_a->bitrate != rsi_b->bitrate )
        return false;

    if( rsi_a->hls != rsi_b->hls )
        return false;

    if( rsi_a->last_check_ok != rsi_b->last_check_ok )
        return false;

    if( !ctune_streq( rsi_a->last_check_time, rsi_b->last_check_time ) )
        return false;

    if( !ctune_streq( rsi_a->last_check_ok_time, rsi_b->last_check_ok_time ) )
        return false;

    if( !ctune_streq( rsi_a->last_local_check_time, rsi_b->last_local_check_time ) )
        return false;

    if( !ctune_streq( rsi_a->click_timestamp, rsi_b->click_timestamp ) )
        return false;

    if( rsi_a->clickcount != rsi_b->clickcount )
        return false;

    if( rsi_a->clicktrend != rsi_b->clicktrend )
        return false;

    if( !ctune_streq( rsi_a->iso8601.last_change_time, rsi_b->iso8601.last_change_time ) )
        return false;

    if( !ctune_streq( rsi_a->iso8601.last_check_time, rsi_b->iso8601.last_check_time ) )
        return false;

    if( !ctune_streq( rsi_a->iso8601.last_check_ok_time, rsi_b->iso8601.last_check_ok_time ) )
        return false;

    if( !ctune_streq( rsi_a->iso8601.last_local_check_time, rsi_b->iso8601.last_local_check_time ) )
        return false;

    if( !ctune_streq( rsi_a->iso8601.click_timestamp, rsi_b->iso8601.click_timestamp ) )
        return false;

    if( rsi_a->ssl_error != rsi_b->ssl_error )
        return false;

    if( rsi_a->geo.latitude != rsi_b->geo.latitude )
        return false;

    if( rsi_a->geo.longitude != rsi_b->geo.longitude )
        return false;

    if( rsi_a->broken != rsi_b->broken )
        return false;

    return true;
}

/**
 * Creates a hash
 * @param uuid_str RadioStationInfo_t UUID string to hash
 * @return Hash
 */
static uint64_t ctune_RadioStationInfo_hash( const void * uuid_str ) {
    if( uuid_str == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_RadioStationInfo_hash( %p )] Arg is NULL.", uuid_str );
        return 0;
    }

    return ctune_fnvHash( uuid_str );
}

/**
 * Frees and set the the change timestamps to NULL
 * @param rsi RadioStationInfo DTO
 */
static void ctune_RadioStationInfo_clearChangeTimestamps( void * rsi ) {
    if( rsi ) {
        ctune_RadioStationInfo_t * station = ( struct ctune_RadioStationInfo * ) rsi;

        if( station->last_change_time ) {
            free( station->last_change_time );
            station->last_change_time = NULL;
        }

        if( station->iso8601.last_change_time ) {
            free( station->iso8601.last_change_time );
            station->iso8601.last_change_time = NULL;
        }
    }
}

/**
 * Frees and sets all checking timestamps to NULL
 * @param rsi RadioStationInfo DTO
 */
static void ctune_RadioStationInfo_clearCheckTimestamps( void * rsi ) {
    if( rsi ) {
        ctune_RadioStationInfo_t * station = ( struct ctune_RadioStationInfo * ) rsi;

        if( station->last_check_time ) {
            free( station->last_check_time );
            station->last_check_time = NULL;
        }

        if( station->iso8601.last_check_time != NULL ) {
            free( station->iso8601.last_check_time );
            station->iso8601.last_check_time = NULL;
        }

        if( station->last_check_ok_time ) {
            free( station->last_check_ok_time );
            station->last_check_ok_time = NULL;
        }

        if( station->iso8601.last_check_ok_time != NULL ) {
            free( station->iso8601.last_check_ok_time );
            station->iso8601.last_check_ok_time = NULL;
        }

        if( station->last_local_check_time ) {
            free( station->last_local_check_time );
            station->last_local_check_time = NULL;
        }

        if( station->iso8601.last_local_check_time != NULL ) {
            free( station->iso8601.last_local_check_time );
            station->iso8601.last_local_check_time = NULL;
        }
    }
}

/**
 * Frees the content of a RadioStationInfo DTO
 * @param rsi RadioStationInfo DTO
 */
static void ctune_RadioStationInfo_freeContent( void * rsi ) {
    if( rsi == NULL )
        return;

    ctune_RadioStationInfo_t * station = (struct ctune_RadioStationInfo *) rsi;

    if( station->change_uuid ) {
        free( station->change_uuid );
        station->change_uuid = NULL;
    }

    if( station->station_uuid ) {
        free( station->station_uuid );
        station->station_uuid = NULL;
    }

    if( station->server_uuid ) {
        free( station->server_uuid );
        station->server_uuid = NULL;
    }

    if( station->name ) {
        free( station->name );
        station->name = NULL;
    }

    if( station->url ) {
        free( station->url );
        station->url = NULL;
    }

    if( station->url_resolved ) {
        free( station->url_resolved );
        station->url_resolved = NULL;
    }

    if( station->homepage ) {
        free( station->homepage );
        station->homepage = NULL;
    }
    if( station->favicon_url ) {
        free( station->favicon_url );
        station->favicon_url = NULL;
    }

    if( station->tags ) {
        free( station->tags );
        station->tags = NULL;
    }

    if( station->country ) {
        free( station->country );
        station->country = NULL;
    }

    if( station->country_code.iso3166_1 ) {
        free( station->country_code.iso3166_1 );
        station->country_code.iso3166_1 = NULL;
    }

    if( station->country_code.iso3166_2 ) {
        free( station->country_code.iso3166_2 );
        station->country_code.iso3166_2 = NULL;
    }

    if( station->state ) {
        free( station->state );
        station->state = NULL;
    }

    if( station->language ) {
        free( station->language );
        station->language = NULL;
    }

    if( station->language_codes ) {
        free( station->language_codes );
        station->language_codes = NULL;
    }

    if( station->last_change_time ) {
        free( station->last_change_time );
        station->last_change_time = NULL;
    }

    if( station->iso8601.last_change_time ) {
        free( station->iso8601.last_change_time );
        station->iso8601.last_change_time = NULL;
    }

    if( station->codec ) {
        free( station->codec );
        station->codec = NULL;
    }

    if( station->last_check_time ) {
        free( station->last_check_time );
        station->last_check_time = NULL;
    }

    if( station->iso8601.last_check_time ) {
        free( station->iso8601.last_check_time );
        station->iso8601.last_check_time = NULL;
    }

    if( station->last_check_ok_time ) {
        free( station->last_check_ok_time );
        station->last_check_ok_time = NULL;
    }

    if( station->iso8601.last_check_ok_time ) {
        free( station->iso8601.last_check_ok_time );
        station->iso8601.last_check_ok_time = NULL;
    }

    if( station->last_local_check_time ) {
        free( station->last_local_check_time );
        station->last_local_check_time = NULL;
    }

    if( station->iso8601.last_local_check_time ) {
        free( station->iso8601.last_local_check_time );
        station->iso8601.last_local_check_time = NULL;
    }

    if( station->click_timestamp ) {
        free( station->click_timestamp );
        station->click_timestamp = NULL;
    }

    if( station->iso8601.click_timestamp ) {
        free( station->iso8601.click_timestamp );
        station->iso8601.click_timestamp = NULL;
    }
}

/**
 * Frees a heap-allocated RadioStationInfo DTO and its content
 * @param rsi RadioStationInfo DTO
 */
static void ctune_RadioStationInfo_free( void * rsi ) {
    if( rsi ) {
        ctune_RadioStationInfo_freeContent( rsi );
        free( rsi );
    }
}

/**
 * Prints a RadioStationInfo
 * @param rsi RadioStationInfo instance
 * @param out Output
 */
static void ctune_RadioStationInfo_print( const ctune_RadioStationInfo_t * rsi, FILE * out ) {
    if( rsi == NULL || out == NULL )
        return; //EARLY RETURN

    fprintf( out, "change_uuid......................: %s\n", rsi->change_uuid );
    fprintf( out, "station_uuid ....................: %s\n", rsi->station_uuid );
    fprintf( out, "server_uuid .....................: %s\n", rsi->server_uuid );
    fprintf( out, "name ............................: %s\n", rsi->name );
    fprintf( out, "url .............................: %s\n", rsi->url );
    fprintf( out, "url_resolved ....................: %s\n", rsi->url_resolved );
    fprintf( out, "homepage ........................: %s\n", rsi->homepage );
    fprintf( out, "favicon_url .....................: %s\n", rsi->favicon_url );
    fprintf( out, "tags ............................: %s\n", rsi->tags );
    fprintf( out, "country .........................: %s\n", rsi->country );
    fprintf( out, "country_code (ISO 3166_1)........: %c%c\n", rsi->country_code.iso3166_1[0], rsi->country_code.iso3166_1[1] );
    fprintf( out, "country_code (ISO 3166_2)........: %c%c\n", rsi->country_code.iso3166_2[0], rsi->country_code.iso3166_2[1] );
    fprintf( out, "state ...........................: %s\n", rsi->state );
    fprintf( out, "language ........................: %s\n", rsi->language );
    fprintf( out, "language_codes...................: %s\n", rsi->language_codes );
    fprintf( out, "votes ...........................: %lu\n", rsi->votes );
    fprintf( out, "last_change_time ................: %s\n", rsi->last_change_time );
    fprintf( out, "last_change_time (ISO-8601) .....: %s\n", rsi->iso8601.last_change_time );
    fprintf( out, "codec ...........................: %s\n", rsi->codec );
    fprintf( out, "bitrate .........................: %lu\n", rsi->bitrate );
    fprintf( out, "hls .............................: %i\n", rsi->hls );
    fprintf( out, "last_check_ok ...................: %i\n", rsi->last_check_ok );
    fprintf( out, "last_check_time .................: %s\n", rsi->last_check_time );
    fprintf( out, "last_check_time (ISO-8601) ......: %s\n", rsi->iso8601.last_check_time );
    fprintf( out, "last_check_ok_time ..............: %s\n", rsi->last_check_ok_time );
    fprintf( out, "last_check_ok_time (ISO-8601) ...: %s\n", rsi->iso8601.last_check_ok_time );
    fprintf( out, "last_local_check_time ...........: %s\n", rsi->last_local_check_time );
    fprintf( out, "last_local_check_time (ISO-8601) : %s\n", rsi->iso8601.last_local_check_time );
    fprintf( out, "click_timestamp .................: %s\n", rsi->click_timestamp );
    fprintf( out, "click_timestamp (ISO-8601) ......: %s\n", rsi->iso8601.last_local_check_time );
    fprintf( out, "clickcount ......................: %lu\n", rsi->clickcount );
    fprintf( out, "clicktrend ......................: %ld\n", rsi->clicktrend );
    fprintf( out, "ssl_error .......................: %ld\n", rsi->ssl_error );
    fprintf( out, "geo coordinates .................: (%f, %f)\n", rsi->geo.latitude, rsi->geo.longitude );
    fprintf( out, "broken    .......................: %s\n", ( rsi->broken ? "1" : "0" ) );
    fprintf( out, "favourite .......................: %s\n", ( rsi->is_favourite ? "1" : "0" ) );
    fprintf( out, "station source ..................: %i", rsi->station_src );
}

/**
 * Prints a 'lite' version of RadioStationInfo
 * @param rsi RadioStationInfo instance
 * @param out Output
 */
static void ctune_RadioStationInfo_printLite( const ctune_RadioStationInfo_t * rsi, FILE * out ) {
    if( rsi == NULL || out == NULL )
        return; //EARLY RETURN

    const size_t name_ln  = ( rsi->name != NULL ? strlen( rsi->name ) : 0 );
    const size_t src_ln   = ( strlen( ctune_StationSrc.str( rsi->station_src ) ) );
    const size_t uuid_ln  = ( rsi->station_uuid != NULL ? strlen( rsi->station_uuid ) : 0 );
    size_t       total_ln = name_ln + 2 + src_ln + 3 + uuid_ln + 2;

    for( size_t i = 0; i < total_ln; ++i )
        fprintf( out, "=" );

    fprintf( out, "\n%s {%s: <%s>}\n", rsi->name, ctune_StationSrc.str( rsi->station_src ), rsi->station_uuid );

    for( size_t i = 0; i < total_ln; ++i )
        fprintf( out, "=" );
}


//==================================== SORTING METHODS =============================================
static int ctune_RadioStationInfo_compare_by_none( const void * lhs, const void * rhs ) {
    return 0;
}

static int ctune_RadioStationInfo_compare_by_name( const void * lhs, const void * rhs ) {
    const ctune_RadioStationInfo_t * a = (ctune_RadioStationInfo_t *) lhs;
    const ctune_RadioStationInfo_t * b = (ctune_RadioStationInfo_t *) rhs;

    if( a->name == NULL )
        return +1;
    if( b->name == NULL )
        return -1;

    int comp = strcmp( a->name, b->name );

    if( comp == 0 )
        return ctune_RadioStationInfo.compareBy( lhs, rhs, CTUNE_RADIOSTATIONINFO_SORTBY_BITRATE );
    else
        return comp;
}

static int ctune_RadioStationInfo_compare_by_name_r( const void * lhs, const void * rhs ) {
    const ctune_RadioStationInfo_t * a = (ctune_RadioStationInfo_t *) lhs;
    const ctune_RadioStationInfo_t * b = (ctune_RadioStationInfo_t *) rhs;

    if( a->name == NULL )
        return -1;
    if( b->name == NULL )
        return +1;

    int comp = strcmp( a->name, b->name );

    if( comp == 0 )
        return ctune_RadioStationInfo.compareBy( lhs, rhs, CTUNE_RADIOSTATIONINFO_SORTBY_BITRATE );
    else
        return comp;
}

static int ctune_RadioStationInfo_compare_by_tags( const void * lhs, const void * rhs ) {
    const ctune_RadioStationInfo_t * a = (ctune_RadioStationInfo_t *) lhs;
    const ctune_RadioStationInfo_t * b = (ctune_RadioStationInfo_t *) rhs;

    if( a->tags == NULL )
        return +1;
    else if( b->tags == NULL )
        return -1;
    else
        return strcmp( a->tags, b->tags );
}

static int ctune_RadioStationInfo_compare_by_tags_r( const void * lhs, const void * rhs ) {
    return ctune_inverseComparison( ctune_RadioStationInfo_compare_by_tags( lhs, rhs ) );
}

static int ctune_RadioStationInfo_compare_by_country( const void * lhs, const void * rhs ) {
    const ctune_RadioStationInfo_t * a = (ctune_RadioStationInfo_t *) lhs;
    const ctune_RadioStationInfo_t * b = (ctune_RadioStationInfo_t *) rhs;

    if( a->country == NULL )
        return +1;
    else if( b->country == NULL )
        return -1;
    else
        return strcmp( a->country, b->country );
}

static int ctune_RadioStationInfo_compare_by_country_r( const void * lhs, const void * rhs ) {
    return ctune_inverseComparison( ctune_RadioStationInfo_compare_by_country( lhs, rhs ) );
}

static int ctune_RadioStationInfo_compare_by_country_code( const void * lhs, const void * rhs ) {
    const ctune_RadioStationInfo_t * a = (ctune_RadioStationInfo_t *) lhs;
    const ctune_RadioStationInfo_t * b = (ctune_RadioStationInfo_t *) rhs;

    const char * lhs_cc = ( a->country_code.iso3166_2 == NULL ? a->country_code.iso3166_1 : a->country_code.iso3166_2 );
    const char * rhs_cc = ( b->country_code.iso3166_2 == NULL ? b->country_code.iso3166_1 : b->country_code.iso3166_2 );

    if( lhs_cc == NULL )
        return +1;
    else if( rhs_cc == NULL )
        return -1;
    else
        return strcmp( lhs_cc, rhs_cc );
}

static int ctune_RadioStationInfo_compare_by_country_code_r( const void * lhs, const void * rhs ) {
    return ctune_inverseComparison( ctune_RadioStationInfo_compare_by_country_code( lhs, rhs ) );
}

static int ctune_RadioStationInfo_compare_by_state( const void * lhs, const void * rhs ) {
    const ctune_RadioStationInfo_t * a = (ctune_RadioStationInfo_t *) lhs;
    const ctune_RadioStationInfo_t * b = (ctune_RadioStationInfo_t *) rhs;

    if( a->state == NULL )
        return +1;
    else if( b->state == NULL )
        return -1;
    else
        return strcmp( a->state, b->state );
}

static int ctune_RadioStationInfo_compare_by_state_r( const void * lhs, const void * rhs ) {
    return ctune_inverseComparison( ctune_RadioStationInfo_compare_by_state( lhs, rhs ) );
}

static int ctune_RadioStationInfo_compare_by_language( const void * lhs, const void * rhs ) {
    const ctune_RadioStationInfo_t * a = (ctune_RadioStationInfo_t *) lhs;
    const ctune_RadioStationInfo_t * b = (ctune_RadioStationInfo_t *) rhs;

    if( a->language_codes != NULL && b->language_codes != NULL )
        return strcmp( a->language_codes, b->language_codes );

    if( a->language != NULL && b->language != NULL )
        return strcmp( a->language, b->language );

    if( a->language_codes != NULL && b->language_codes == NULL )
        return +1;

    if( a->language_codes == NULL && b->language_codes != NULL )
        return -1;

    if( a->language == NULL && b->language != NULL )
        return +1;

    if( a->language != NULL && b->language == NULL )
        return -1;

    return 0;
}

static int ctune_RadioStationInfo_compare_by_language_r( const void * lhs, const void * rhs ) {
    return ctune_inverseComparison( ctune_RadioStationInfo_compare_by_language( lhs, rhs ) );
}

static int ctune_RadioStationInfo_compare_by_votes( const void * lhs, const void * rhs ) {
    const ctune_RadioStationInfo_t * a = (ctune_RadioStationInfo_t *) lhs;
    const ctune_RadioStationInfo_t * b = (ctune_RadioStationInfo_t *) rhs;
    return ( a->votes > b->votes ? +1 : ( a->votes < b->votes ? -1 : 0 ) );
}

static int ctune_RadioStationInfo_compare_by_votes_r( const void * lhs, const void * rhs ) {
    return ctune_inverseComparison( ctune_RadioStationInfo_compare_by_votes( lhs, rhs ) );
}

static int ctune_RadioStationInfo_compare_by_click_count( const void * lhs, const void * rhs ) {
    const ctune_RadioStationInfo_t * a = (ctune_RadioStationInfo_t *) lhs;
    const ctune_RadioStationInfo_t * b = (ctune_RadioStationInfo_t *) rhs;
    return ( a->clickcount > b->clickcount ? +1 : ( a->clickcount < b->clickcount ? -1 : 0 ) );
}

static int ctune_RadioStationInfo_compare_by_click_count_r( const void * lhs, const void * rhs ) {
    return ctune_inverseComparison( ctune_RadioStationInfo_compare_by_click_count( lhs, rhs ) );
}

static int ctune_RadioStationInfo_compare_by_click_trend( const void * lhs, const void * rhs ) {
    const ctune_RadioStationInfo_t * a = (ctune_RadioStationInfo_t *) lhs;
    const ctune_RadioStationInfo_t * b = (ctune_RadioStationInfo_t *) rhs;
    return ( a->clicktrend > b->clicktrend ? +1 : ( a->clicktrend < b->clicktrend ? -1 : 0 ) );
}

static int ctune_RadioStationInfo_compare_by_click_trend_r( const void * lhs, const void * rhs ) {
    return ctune_inverseComparison( ctune_RadioStationInfo_compare_by_click_trend( lhs, rhs ) );
}

static int ctune_RadioStationInfo_compare_by_codec( const void * lhs, const void * rhs ) {
    const ctune_RadioStationInfo_t * a = (ctune_RadioStationInfo_t *) lhs;
    const ctune_RadioStationInfo_t * b = (ctune_RadioStationInfo_t *) rhs;
    return ( a->codec > b->codec ? +1 : ( a->codec < b->codec ? -1 : 0 ) );
}

static int ctune_RadioStationInfo_compare_by_codec_r( const void * lhs, const void * rhs ) {
    return ctune_inverseComparison( ctune_RadioStationInfo_compare_by_codec( lhs, rhs ) );
}

static int ctune_RadioStationInfo_compare_by_bitrate( const void * lhs, const void * rhs ) {
    const ctune_RadioStationInfo_t * a = (ctune_RadioStationInfo_t *) lhs;
    const ctune_RadioStationInfo_t * b = (ctune_RadioStationInfo_t *) rhs;
    return ( a->bitrate == b->bitrate ? 0 : ( a->bitrate > b->bitrate ? +1 : -1 ) );
}

static int ctune_RadioStationInfo_compare_by_bitrate_r( const void * lhs, const void * rhs ) {
    return ctune_inverseComparison( ctune_RadioStationInfo_compare_by_bitrate( lhs, rhs ) );
}

static int ctune_RadioStationInfo_compare_by_source( const void * lhs, const void * rhs ) {
    const ctune_RadioStationInfo_t * a = (ctune_RadioStationInfo_t *) lhs;
    const ctune_RadioStationInfo_t * b = (ctune_RadioStationInfo_t *) rhs;
    return ( a->station_src && b->station_src
             ? ctune_RadioStationInfo.compareBy( lhs, rhs, CTUNE_RADIOSTATIONINFO_SORTBY_NAME )
             : ( a->station_src && !b->station_src ? +1 : -1 ) );
}

static int ctune_RadioStationInfo_compare_by_source_r( const void * lhs, const void * rhs ) {
    const ctune_RadioStationInfo_t * a = (ctune_RadioStationInfo_t *) lhs;
    const ctune_RadioStationInfo_t * b = (ctune_RadioStationInfo_t *) rhs;
    return ( a->station_src && b->station_src
             ? ctune_RadioStationInfo.compareBy( lhs, rhs, CTUNE_RADIOSTATIONINFO_SORTBY_NAME )
             : ( a->station_src && !b->station_src ? -1 : +1 ) );
}

//============================================ COMPARATORS =========================================

static int (* fn[CTUNE_RADIOSTATIONINFO_SORTBY_COUNT])( const void *, const void * ) = {
    [CTUNE_RADIOSTATIONINFO_SORTBY_NONE             ] = &ctune_RadioStationInfo_compare_by_none,
    [CTUNE_RADIOSTATIONINFO_SORTBY_SOURCE           ] = &ctune_RadioStationInfo_compare_by_source,
    [CTUNE_RADIOSTATIONINFO_SORTBY_SOURCE_DESC      ] = &ctune_RadioStationInfo_compare_by_source_r,
    [CTUNE_RADIOSTATIONINFO_SORTBY_NAME             ] = &ctune_RadioStationInfo_compare_by_name,
    [CTUNE_RADIOSTATIONINFO_SORTBY_NAME_DESC        ] = &ctune_RadioStationInfo_compare_by_name_r,
    [CTUNE_RADIOSTATIONINFO_SORTBY_TAGS             ] = &ctune_RadioStationInfo_compare_by_tags,
    [CTUNE_RADIOSTATIONINFO_SORTBY_TAGS_DESC        ] = &ctune_RadioStationInfo_compare_by_tags_r,
    [CTUNE_RADIOSTATIONINFO_SORTBY_COUNTRY          ] = &ctune_RadioStationInfo_compare_by_country,
    [CTUNE_RADIOSTATIONINFO_SORTBY_COUNTRY_DESC     ] = &ctune_RadioStationInfo_compare_by_country_r,
    [CTUNE_RADIOSTATIONINFO_SORTBY_COUNTRYCODE      ] = &ctune_RadioStationInfo_compare_by_country_code,
    [CTUNE_RADIOSTATIONINFO_SORTBY_COUNTRYCODE_DESC ] = &ctune_RadioStationInfo_compare_by_country_code_r,
    [CTUNE_RADIOSTATIONINFO_SORTBY_STATE            ] = &ctune_RadioStationInfo_compare_by_state,
    [CTUNE_RADIOSTATIONINFO_SORTBY_STATE_DESC       ] = &ctune_RadioStationInfo_compare_by_state_r,
    [CTUNE_RADIOSTATIONINFO_SORTBY_LANGUAGE         ] = &ctune_RadioStationInfo_compare_by_language,
    [CTUNE_RADIOSTATIONINFO_SORTBY_LANGUAGE_DESC    ] = &ctune_RadioStationInfo_compare_by_language_r,
    [CTUNE_RADIOSTATIONINFO_SORTBY_CODEC            ] = &ctune_RadioStationInfo_compare_by_codec,
    [CTUNE_RADIOSTATIONINFO_SORTBY_CODEC_DESC       ] = &ctune_RadioStationInfo_compare_by_codec_r,
    [CTUNE_RADIOSTATIONINFO_SORTBY_BITRATE          ] = &ctune_RadioStationInfo_compare_by_bitrate,
    [CTUNE_RADIOSTATIONINFO_SORTBY_BITRATE_DESC     ] = &ctune_RadioStationInfo_compare_by_bitrate_r,
};

/**
 * Compare two stations
 * @param lhs  Pointer to a RadioStationInfo_t object
 * @param rhs  Pointer to a RadioStationInfo_t object against
 * @param attr Attribute to compare
 * @return Result of comparison (-1: less, 0: equal, +1: greater)
 */
static int ctune_RadioStationInfo_compareBy( const void * lhs, const void * rhs, ctune_RadioStationInfo_SortBy_e attr ) {
    if( attr < CTUNE_RADIOSTATIONINFO_SORTBY_NONE || attr >= CTUNE_RADIOSTATIONINFO_SORTBY_COUNT ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_RadioStationInfo_compareBy( %p, %p, %i )] "
                   "Sorting attribute '%i' not recognised/implemented",
                   lhs, rhs, attr, attr
        );

        return 0; //EARLY RETURN
    }

    return fn[attr]( lhs, rhs );
}

/**
 * Gets the comparator function pointer
 * @param attr Comparison attribute
 * @return Comparator or NULL if attribute not implemented
 */
static Comparator ctune_RadioStationInfo_getComparator( ctune_RadioStationInfo_SortBy_e attr ) {
    if( attr < CTUNE_RADIOSTATIONINFO_SORTBY_NONE || attr >= CTUNE_RADIOSTATIONINFO_SORTBY_COUNT ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_RadioStationInfo_getComparator( %i )] "
                   "Sorting attribute '%i' not recognised/implemented",
                   attr, attr
        );

        return NULL; //EARLY RETURN
    }

    return fn[attr];
}

/**
 * Gets the string representation of a sorting attribute
 * @param attr Sorting attribute
 * @return String/NULL if no matching string is found
 */
static const char * ctune_RadioStationInfo_sortAttrStr( ctune_RadioStationInfo_SortBy_e attr ) {
    static const char * arr[CTUNE_RADIOSTATIONINFO_SORTBY_COUNT] = {
        [CTUNE_RADIOSTATIONINFO_SORTBY_NONE             ] = "none",
        [CTUNE_RADIOSTATIONINFO_SORTBY_SOURCE           ] = "source ascending",
        [CTUNE_RADIOSTATIONINFO_SORTBY_SOURCE_DESC      ] = "source descending",
        [CTUNE_RADIOSTATIONINFO_SORTBY_NAME             ] = "name ascending",
        [CTUNE_RADIOSTATIONINFO_SORTBY_NAME_DESC        ] = "name descending",
        [CTUNE_RADIOSTATIONINFO_SORTBY_TAGS             ] = "tags ascending",
        [CTUNE_RADIOSTATIONINFO_SORTBY_TAGS_DESC        ] = "tags descending",
        [CTUNE_RADIOSTATIONINFO_SORTBY_COUNTRY          ] = "country ascending",
        [CTUNE_RADIOSTATIONINFO_SORTBY_COUNTRY_DESC     ] = "country descending",
        [CTUNE_RADIOSTATIONINFO_SORTBY_COUNTRYCODE      ] = "country code ascending",
        [CTUNE_RADIOSTATIONINFO_SORTBY_COUNTRYCODE_DESC ] = "country code",
        [CTUNE_RADIOSTATIONINFO_SORTBY_STATE            ] = "state ascending",
        [CTUNE_RADIOSTATIONINFO_SORTBY_STATE_DESC       ] = "state descending",
        [CTUNE_RADIOSTATIONINFO_SORTBY_LANGUAGE         ] = "language ascending",
        [CTUNE_RADIOSTATIONINFO_SORTBY_LANGUAGE_DESC    ] = "language descending",
        [CTUNE_RADIOSTATIONINFO_SORTBY_CODEC            ] = "codec ascending",
        [CTUNE_RADIOSTATIONINFO_SORTBY_CODEC_DESC       ] = "codec descending",
        [CTUNE_RADIOSTATIONINFO_SORTBY_BITRATE          ] = "bitrate ascending",
        [CTUNE_RADIOSTATIONINFO_SORTBY_BITRATE_DESC     ] = "bitrate",
    };

    if( attr < 0 || attr >= CTUNE_RADIOSTATIONINFO_SORTBY_COUNT ) {
        return "'No description string found'";
    }

    return arr[ (int) attr ];
}

static void ctune_RadioStationInfo_set_changeUUID( ctune_RadioStationInfo_t * rsi, char * str_ptr ) {
    if( rsi != NULL ) {
        if( rsi->change_uuid )
            free( rsi->change_uuid );

        rsi->change_uuid = str_ptr;
    }
}

static void ctune_RadioStationInfo_set_stationUUID( ctune_RadioStationInfo_t * rsi, char * str_ptr ) {
    if( rsi != NULL ) {
        if( rsi->station_uuid )
            free( rsi->station_uuid );

        rsi->station_uuid = str_ptr;
    }
}

static void ctune_RadioStationInfo_set_serverUUID( ctune_RadioStationInfo_t * rsi, char * str_ptr ) {
    if( rsi != NULL ) {
        if( rsi->server_uuid )
            free( rsi->server_uuid );

        rsi->server_uuid = str_ptr;
    }
}

static void ctune_RadioStationInfo_set_stationName( ctune_RadioStationInfo_t * rsi, char * str_ptr ) {
    if( rsi != NULL ) {
        if( rsi->name )
            free( rsi->name );

        rsi->name = str_ptr;
    }
}

static void ctune_RadioStationInfo_set_stationURL( ctune_RadioStationInfo_t * rsi, char * str_ptr ) {
    if( rsi != NULL ) {
        if( rsi->url )
            free( rsi->url );

        rsi->url = str_ptr;
    }
}

static void ctune_RadioStationInfo_set_resolvedURL( ctune_RadioStationInfo_t * rsi, char * str_ptr ) {
    if( rsi != NULL ) {
        if( rsi->url_resolved )
            free( rsi->url_resolved );

        rsi->url_resolved = str_ptr;
    }
}

static void ctune_RadioStationInfo_set_homepage( ctune_RadioStationInfo_t * rsi, char * str_ptr ) {
    if( rsi != NULL ) {
        if( rsi->homepage )
            free( rsi->homepage );

        rsi->homepage = str_ptr;
    }
}

static void ctune_RadioStationInfo_set_faviconURL( ctune_RadioStationInfo_t * rsi, char * str_ptr ) {
    if( rsi != NULL ) {
        if( rsi->favicon_url )
            free( rsi->favicon_url );

        rsi->favicon_url = str_ptr;
    }
}

static void ctune_RadioStationInfo_set_tags( ctune_RadioStationInfo_t * rsi, char * str_ptr ) {
    if( rsi != NULL ) {
        if( rsi->tags )
            free( rsi->tags );

        rsi->tags = str_ptr;
    }
}

static void ctune_RadioStationInfo_set_country( ctune_RadioStationInfo_t * rsi, char * str_ptr ) {
    if( rsi != NULL ) {
        if( rsi->country )
            free( rsi->country );

        rsi->country = str_ptr;
    }
}

static void ctune_RadioStationInfo_set_countryCode_ISO3166_1( ctune_RadioStationInfo_t * rsi, const char * cc_str ) {
    if( rsi != NULL ) {
        if( rsi->country_code.iso3166_1 ) {
            free( rsi->country_code.iso3166_1 );
            rsi->country_code.iso3166_1 = NULL;
        }

        if( cc_str != NULL ) {
            if( strlen( cc_str ) >= 2 ) {
                rsi->country_code.iso3166_1 = malloc( sizeof( char ) * 3 );

                if( rsi->country_code.iso3166_1 != NULL ) {
                    rsi->country_code.iso3166_1[ 0 ] = cc_str[ 0 ];
                    rsi->country_code.iso3166_1[ 1 ] = cc_str[ 1 ];
                    rsi->country_code.iso3166_1[ 2 ] = '\0';

                } else {
                    CTUNE_LOG( CTUNE_LOG_ERROR,
                               "[ctune_RadioStationInfo_set_countryCode_ISO3166_1( %p, %p )] Failed to allocate memory for 'country_code'.",
                               rsi, cc_str
                    );
                }

            } else {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_RadioStationInfo_set_countryCode_ISO3166_1( %p, %p )] String passed is too short - nothing copied.",
                           rsi, cc_str
                );
            }
        }
    }
}

static void ctune_RadioStationInfo_set_countryCode_ISO3166_2( ctune_RadioStationInfo_t * rsi, char * cc_str ) {
    if( rsi != NULL ) {
        if( rsi->country_code.iso3166_2 )
            free( rsi->country_code.iso3166_2 );

        rsi->country_code.iso3166_2 = cc_str;
    }
}

static void ctune_RadioStationInfo_set_state( ctune_RadioStationInfo_t * rsi, char * str_ptr ) {
    if( rsi != NULL ) {
        if( rsi->state )
            free( rsi->state );

        rsi->state = str_ptr;
    }
}

static void ctune_RadioStationInfo_set_language( ctune_RadioStationInfo_t * rsi, char * str_ptr ) {
    if( rsi != NULL ) {
        if( rsi->language )
            free( rsi->language );

        rsi->language = str_ptr;
    }
}

static void ctune_RadioStationInfo_set_languageCodes( ctune_RadioStationInfo_t * rsi, char * str_ptr ) {
    if( rsi != NULL ) {
        if( rsi->language_codes )
            free( rsi->language_codes );

        rsi->language_codes = str_ptr;
    }
}

static void ctune_RadioStationInfo_set_votes( ctune_RadioStationInfo_t * rsi, ulong votes ) {
    if( rsi != NULL ) {

        rsi->votes = votes;
    }
}

static void ctune_RadioStationInfo_set_lastChangeTS( ctune_RadioStationInfo_t * rsi, char * str_ptr ) {
    if( rsi != NULL ) {
        if( rsi->iso8601.last_change_time )
            free( rsi->iso8601.last_change_time );

        rsi->iso8601.last_change_time = str_ptr;
    }
}

static void ctune_RadioStationInfo_set_codec( ctune_RadioStationInfo_t * rsi, char * str_ptr ) {
    if( rsi != NULL ) {
        if( rsi->codec )
            free( rsi->codec );

        rsi->codec = str_ptr;
    }
}

static void ctune_RadioStationInfo_set_bitrate( ctune_RadioStationInfo_t * rsi, ulong bitrate ) {
    if( rsi != NULL )
        rsi->bitrate = bitrate;
}

static void ctune_RadioStationInfo_set_hls( ctune_RadioStationInfo_t * rsi, bool state ) {
    if( rsi != NULL )
        rsi->hls = state;
}

static void ctune_RadioStationInfo_set_lastCheckOk( ctune_RadioStationInfo_t * rsi, bool state ) {
    if( rsi != NULL )
        rsi->last_check_ok = state;
}

static void ctune_RadioStationInfo_set_lastCheckTS( ctune_RadioStationInfo_t * rsi, char * str_ptr ) {
    if( rsi != NULL ) {
        if( rsi->iso8601.last_check_time )
            free( rsi->iso8601.last_check_time );

        rsi->iso8601.last_check_time = str_ptr;
    }
}

static void ctune_RadioStationInfo_set_lastCheckOkTS( ctune_RadioStationInfo_t * rsi, char * str_ptr ) {
    if( rsi != NULL ) {
        if( rsi->iso8601.last_check_ok_time )
            free( rsi->iso8601.last_check_ok_time );

        rsi->iso8601.last_check_ok_time = str_ptr;
    }
}

static void ctune_RadioStationInfo_set_lastLocalCheckTS( ctune_RadioStationInfo_t * rsi, char * str_ptr ) {
    if( rsi != NULL ) {
        if( rsi->iso8601.last_local_check_time )
            free( rsi->iso8601.last_local_check_time );

        rsi->iso8601.last_local_check_time = str_ptr;
    }
}

static void ctune_RadioStationInfo_set_clickTS( ctune_RadioStationInfo_t * rsi, char * str_ptr ) {
    if( rsi != NULL ) {
        if( rsi->iso8601.click_timestamp )
            free( rsi->iso8601.click_timestamp );

        rsi->iso8601.click_timestamp = str_ptr;
    }
}

static void ctune_RadioStationInfo_set_clickCount( ctune_RadioStationInfo_t * rsi, ulong count ) {
    if( rsi != NULL )
        rsi->clickcount = count;
}

static void ctune_RadioStationInfo_set_clickTrend( ctune_RadioStationInfo_t * rsi, long trend ) {
    if( rsi != NULL )
        rsi->clicktrend = trend;
}

static void ctune_RadioStationInfo_set_broken( ctune_RadioStationInfo_t * rsi, bool state ) {
    if( rsi != NULL )
        rsi->broken = state;
}

static void ctune_RadioStationInfo_set_sslErrCode( ctune_RadioStationInfo_t * rsi, long ssl_err_code ) {
    if( rsi != NULL )
        rsi->ssl_error = ssl_err_code;
}

static void ctune_RadioStationInfo_set_geoCoordinates( ctune_RadioStationInfo_t * rsi, double latitude, double longitude ) {
    if( rsi != NULL ) {
        rsi->geo.latitude  = latitude;
        rsi->geo.longitude = longitude;
    }
}

static void ctune_RadioStationInfo_set_extendedInfoFlag( ctune_RadioStationInfo_t * rsi, bool state ) {
    if( rsi != NULL )
        rsi->has_extended_info = state;
}

static void ctune_RadioStationInfo_set_favourite( ctune_RadioStationInfo_t * rsi, bool state ) {
    if( rsi != NULL )
        rsi->is_favourite = state;
}

static void ctune_RadioStationInfo_set_stationSource( ctune_RadioStationInfo_t * rsi, ctune_StationSrc_e src ) {
    if( rsi != NULL )
        rsi->station_src = src;
}

static const char * ctune_RadioStationInfo_get_changeUUID( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return NULL;
    return rsi->change_uuid;
}

static const char * ctune_RadioStationInfo_get_stationUUID( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return NULL;
    return rsi->station_uuid;
}

static const char * ctune_RadioStationInfo_get_serverUUID( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return NULL;
    return rsi->server_uuid;
}

static const char * ctune_RadioStationInfo_get_stationName( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return NULL;
    return rsi->name;
}

static const char * ctune_RadioStationInfo_get_stationURL( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return NULL;
    return rsi->url;
}

static const char * ctune_RadioStationInfo_get_resolvedURL( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return NULL;
    return rsi->url_resolved;
}

static const char * ctune_RadioStationInfo_get_homepage( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return NULL;
    return rsi->homepage;
}

static const char * ctune_RadioStationInfo_get_faviconURL( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return NULL;
    return rsi->favicon_url;
}

static const char * ctune_RadioStationInfo_get_tags( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return NULL;
    return rsi->tags;
}

static const char * ctune_RadioStationInfo_get_country( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return NULL;
    return rsi->country;
}

static const char * ctune_RadioStationInfo_get_countryCode( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return NULL;
    if( rsi->country_code.iso3166_2 != NULL )
        return rsi->country_code.iso3166_2;
    else
        return rsi->country_code.iso3166_1;
}

static const char * ctune_RadioStationInfo_get_countryCode_ISO3166_1( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return NULL;
    return rsi->country_code.iso3166_1;
}

static const char * ctune_RadioStationInfo_get_countryCode_ISO3166_2( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return NULL;
    return rsi->country_code.iso3166_2;
}

static const char * ctune_RadioStationInfo_get_state( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return NULL;
    return rsi->state;
}

static const char * ctune_RadioStationInfo_get_language( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return NULL;
    return rsi->language;
}

static const char * ctune_RadioStationInfo_get_languageCodes( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return NULL;
    return rsi->language_codes;
}

static ulong ctune_RadioStationInfo_get_votes( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return 0;
    return rsi->votes;
}

static const char * ctune_RadioStationInfo_get_lastChangeTS( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return NULL;
    return rsi->iso8601.last_change_time;
}

static const char * ctune_RadioStationInfo_get_codec( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return NULL;
    return rsi->codec;
}

static ulong ctune_RadioStationInfo_get_bitrate( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return 0;
    return rsi->bitrate;
}

static bool ctune_RadioStationInfo_get_hls( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return false;
    return rsi->hls;
}

static bool ctune_RadioStationInfo_get_lastCheckOK( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return false;
    return rsi->last_check_ok;
}

static const char * ctune_RadioStationInfo_get_lastCheckTS( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return NULL;
    return rsi->iso8601.last_check_time;
}

static const char * ctune_RadioStationInfo_get_lastCheckOkTS( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return NULL;
    return rsi->iso8601.last_check_ok_time;
}

static const char * ctune_RadioStationInfo_get_lastLocalCheckTS( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return NULL;
    return rsi->iso8601.last_local_check_time;
}

static const char * ctune_RadioStationInfo_get_clickTS( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return NULL;
    return rsi->iso8601.click_timestamp;
}

static ulong ctune_RadioStationInfo_get_clickCount( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return 0;
    return rsi->clickcount;
}

static long ctune_RadioStationInfo_get_clickTrend( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return 0;
    return rsi->clicktrend;
}

static bool ctune_RadioStationInfo_get_broken( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return false;
    return rsi->broken;
}

static long ctune_RadioStationInfo_get_sslErrCode( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return 0;
    return rsi->ssl_error;
}

static double ctune_RadioStationInfo_get_geoLatitude( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return .0;
    return rsi->geo.latitude;
}

static double ctune_RadioStationInfo_get_geoLongitude( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return .0;
    return rsi->geo.longitude;
}

static bool ctune_RadioStationInfo_get_hasExtendedInfo( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return false;
    return rsi->has_extended_info;
}

static bool ctune_RadioStationInfo_get_favourite( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return false;
    return rsi->is_favourite;
}

static ctune_StationSrc_e ctune_RadioStationInfo_get_stationSource( const ctune_RadioStationInfo_t * rsi ) {
    if( rsi == NULL )
        return CTUNE_STATIONSRC_LOCAL;
    return rsi->station_src;
}

/**
 * Gets a field by its name string
 * @param rsi RadioStationInfo_t object
 * @param api_name Name string
 * @return Field
 */
inline static ctune_Field_t ctune_RadioStationInfo_getField( ctune_RadioStationInfo_t * rsi, const char * api_name ) {
    if( strcmp( api_name, "bitrate" ) == 0 ) {
        return ( ctune_Field_t ) { ._field = &rsi->bitrate, ._type = CTUNE_FIELD_UNSIGNED_LONG };

    } else if( strcmp( api_name, "changeuuid" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->change_uuid, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "stationuuid" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->station_uuid, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "serveruuid" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->server_uuid, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "clickcount" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->clickcount, ._type = CTUNE_FIELD_UNSIGNED_LONG };

    } else if( strcmp( api_name, "clicktimestamp" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->click_timestamp, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "clicktimestamp_iso8601" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->iso8601.click_timestamp, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "clicktrend" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->clicktrend, ._type = CTUNE_FIELD_SIGNED_LONG };

    } else if( strcmp( api_name, "codec" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->codec, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "country" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->country, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "countrycode" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->country_code.iso3166_1, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "iso_3166_2" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->country_code.iso3166_2, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "favicon" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->favicon_url, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "hls" ) == 0 ) {
        return ( ctune_Field_t ) { ._field = &rsi->hls, ._type = CTUNE_FIELD_BOOLEAN };

    } else if( strcmp( api_name, "homepage" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->homepage, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "language" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->language, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "languagecodes" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->language_codes, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "lastchangetime" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->last_change_time, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "lastchangetime_iso8601" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->iso8601.last_change_time, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "lastcheckok" ) == 0 ) {
        return ( ctune_Field_t ) { ._field = &rsi->last_check_ok, ._type = CTUNE_FIELD_BOOLEAN };

    } else if( strcmp( api_name, "lastchecktime" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->last_check_time, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "lastchecktime_iso8601" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->iso8601.last_check_time, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "lastcheckoktime" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->last_check_ok_time, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "lastcheckoktime_iso8601" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->iso8601.last_check_ok_time, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "lastlocalchecktime" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->last_local_check_time, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "lastlocalchecktime_iso8601" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->iso8601.last_local_check_time, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "name" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->name, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "state" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->state, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "stationuuid" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->station_uuid, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "tags" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->tags, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "url" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->url, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "url_resolved" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->url_resolved, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "votes" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->votes, ._type = CTUNE_FIELD_UNSIGNED_LONG };

    } else if( strcmp( api_name, "ssl_error" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->ssl_error, ._type = CTUNE_FIELD_SIGNED_LONG };

    } else if( strcmp( api_name, "geo_lat" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->geo.latitude, ._type = CTUNE_FIELD_DOUBLE };

    } else if( strcmp( api_name, "geo_long" ) == 0 ) {
        return (ctune_Field_t){ ._field = &rsi->geo.longitude, ._type = CTUNE_FIELD_DOUBLE };

    } else if( strcmp( api_name, "has_extended_info" ) == 0 ) {
        return ( ctune_Field_t ) { ._field = &rsi->has_extended_info, ._type = CTUNE_FIELD_BOOLEAN };

    } else if( strcmp( api_name, "station_src" ) == 0 ) {
        return ( ctune_Field_t ) { ._field = &rsi->station_src, ._type = CTUNE_FIELD_ENUM_STATIONSRC };

    } else {
        return (ctune_Field_t){ ._field = NULL, ._type = CTUNE_FIELD_UNKNOWN };
    }
}

/**
 * Namespace constructor
 */
const struct ctune_RadioStationInfo_Namespace ctune_RadioStationInfo = {
    .IS_FAV                = 0b0001,
    .IS_QUEUED             = 0b0010,
    .IS_LOCAL              = 0b0100,

    .init                  = &ctune_RadioStationInfo_init,
    .copy                  = &ctune_RadioStationInfo_copy,
    .mincopy               = &ctune_RadioStationInfo_mincopy,
    .dup                   = &ctune_RadioStationInfo_dup,
    .mindup                = &ctune_RadioStationInfo_mindup,
    .sameUUID              = &ctune_RadioStationInfo_equalUUID,
    .equal                 = &ctune_RadioStationInfo_equal,
    .hash                  = &ctune_RadioStationInfo_hash,
    .clearChangeTimestamps = &ctune_RadioStationInfo_clearChangeTimestamps,
    .clearCheckTimestamps  = &ctune_RadioStationInfo_clearCheckTimestamps,
    .freeContent           = &ctune_RadioStationInfo_freeContent,
    .free                  = &ctune_RadioStationInfo_free,
    .print                 = &ctune_RadioStationInfo_print,
    .printLite             = &ctune_RadioStationInfo_printLite,
    .compareBy             = &ctune_RadioStationInfo_compareBy,
    .getComparator         = &ctune_RadioStationInfo_getComparator,
    .sortAttrStr           = &ctune_RadioStationInfo_sortAttrStr,

    .set = {
        .changeUUID            = &ctune_RadioStationInfo_set_changeUUID,
        .stationUUID           = &ctune_RadioStationInfo_set_stationUUID,
        .serverUUID            = &ctune_RadioStationInfo_set_serverUUID,
        .stationName           = &ctune_RadioStationInfo_set_stationName,
        .stationURL            = &ctune_RadioStationInfo_set_stationURL,
        .resolvedURL           = &ctune_RadioStationInfo_set_resolvedURL,
        .homepage              = &ctune_RadioStationInfo_set_homepage,
        .faviconURL            = &ctune_RadioStationInfo_set_faviconURL,
        .tags                  = &ctune_RadioStationInfo_set_tags,
        .country               = &ctune_RadioStationInfo_set_country,
        .countryCode_ISO3166_1 = &ctune_RadioStationInfo_set_countryCode_ISO3166_1,
        .countryCode_ISO3166_2 = &ctune_RadioStationInfo_set_countryCode_ISO3166_2,
        .state                 = &ctune_RadioStationInfo_set_state,
        .language              = &ctune_RadioStationInfo_set_language,
        .languageCodes         = &ctune_RadioStationInfo_set_languageCodes,
        .votes                 = &ctune_RadioStationInfo_set_votes,
        .lastChangeTS          = &ctune_RadioStationInfo_set_lastChangeTS,
        .codec                 = &ctune_RadioStationInfo_set_codec,
        .bitrate               = &ctune_RadioStationInfo_set_bitrate,
        .hls                   = &ctune_RadioStationInfo_set_hls,
        .lastCheckOK           = &ctune_RadioStationInfo_set_lastCheckOk,
        .lastCheckTS           = &ctune_RadioStationInfo_set_lastCheckTS,
        .lastCheckOkTS         = &ctune_RadioStationInfo_set_lastCheckOkTS,
        .lastLocalCheckTS      = &ctune_RadioStationInfo_set_lastLocalCheckTS,
        .clickTS               = &ctune_RadioStationInfo_set_clickTS,
        .clickCount            = &ctune_RadioStationInfo_set_clickCount,
        .clickTrend            = &ctune_RadioStationInfo_set_clickTrend,
        .broken                = &ctune_RadioStationInfo_set_broken,
        .sslErrCode            = &ctune_RadioStationInfo_set_sslErrCode,
        .geoCoordinates        = &ctune_RadioStationInfo_set_geoCoordinates,
        .extendedInfoFlag      = &ctune_RadioStationInfo_set_extendedInfoFlag,
        .favourite             = &ctune_RadioStationInfo_set_favourite,
        .stationSource         = &ctune_RadioStationInfo_set_stationSource,
    },

    .get = {
        .changeUUID            = &ctune_RadioStationInfo_get_changeUUID,
        .stationUUID           = &ctune_RadioStationInfo_get_stationUUID,
        .serverUUID            = &ctune_RadioStationInfo_get_serverUUID,
        .stationName           = &ctune_RadioStationInfo_get_stationName,
        .stationURL            = &ctune_RadioStationInfo_get_stationURL,
        .resolvedURL           = &ctune_RadioStationInfo_get_resolvedURL,
        .homepage              = &ctune_RadioStationInfo_get_homepage,
        .faviconURL            = &ctune_RadioStationInfo_get_faviconURL,
        .tags                  = &ctune_RadioStationInfo_get_tags,
        .country               = &ctune_RadioStationInfo_get_country,
        .countryCode           = &ctune_RadioStationInfo_get_countryCode,
        .countryCode_ISO3166_1 = &ctune_RadioStationInfo_get_countryCode_ISO3166_1,
        .countryCode_ISO3166_2 = &ctune_RadioStationInfo_get_countryCode_ISO3166_2,
        .state                 = &ctune_RadioStationInfo_get_state,
        .language              = &ctune_RadioStationInfo_get_language,
        .languageCodes         = &ctune_RadioStationInfo_get_languageCodes,
        .votes                 = &ctune_RadioStationInfo_get_votes,
        .lastChangeTS          = &ctune_RadioStationInfo_get_lastChangeTS,
        .codec                 = &ctune_RadioStationInfo_get_codec,
        .bitrate               = &ctune_RadioStationInfo_get_bitrate,
        .hls                   = &ctune_RadioStationInfo_get_hls,
        .lastCheckOK           = &ctune_RadioStationInfo_get_lastCheckOK,
        .lastCheckTS           = &ctune_RadioStationInfo_get_lastCheckTS,
        .lastCheckOkTS         = &ctune_RadioStationInfo_get_lastCheckOkTS,
        .lastLocalCheckTS      = &ctune_RadioStationInfo_get_lastLocalCheckTS,
        .clickTS               = &ctune_RadioStationInfo_get_clickTS,
        .clickCount            = &ctune_RadioStationInfo_get_clickCount,
        .clickTrend            = &ctune_RadioStationInfo_get_clickTrend,
        .broken                = &ctune_RadioStationInfo_get_broken,
        .sslErrCode            = &ctune_RadioStationInfo_get_sslErrCode,
        .geoLatitude           = &ctune_RadioStationInfo_get_geoLatitude,
        .geoLongitude          = &ctune_RadioStationInfo_get_geoLongitude,
        .hasExtendedInfo       = &ctune_RadioStationInfo_get_hasExtendedInfo,
        .favourite             = &ctune_RadioStationInfo_get_favourite,
        .stationSource         = &ctune_RadioStationInfo_get_stationSource,
    },

    .getField = &ctune_RadioStationInfo_getField,
};