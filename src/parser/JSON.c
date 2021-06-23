#include "JSON.h"

#include <errno.h>
#include <json-c/json.h>
#include <json-c/json_visit.h>

#include "../logger/Logger.h"
#include "../datastructure/StrList.h"
#include "../dto/RadioStationInfo.h"
#include "../dto/CategoryItem.h"

//=====================================================================================================================================================================
//--------------------------------------------------------------- PRIVATE ---------------------------------------------------------------------------------------------
//=====================================================================================================================================================================

/**
 * [PRIVATE] Copies a string to a `char *` field
 * @param key    Name of the field (used for debug msg purposes)
 * @param val    Value of field as a string (if in quotes they will be omitted on copy to target container)
 * @param target Target container
 * @return Success of operation
 */
static bool ctune_parser_JSON_packField_str( const char * key, const char * val, char ** target ) {
    if( val == NULL )
        return true; //EARLY RETURN

    const size_t length = strlen( val );

    if( length == 0 )
        return true; //EARLY RETURN

    if( length > 1 && val[0] == '\"' && val[( length - 1 )] == '\"' ) { //i.e. in quotes ".."

        size_t substr_length = length - 2;
        size_t offset        = 1;

        *target = malloc( ( substr_length + 1 ) * sizeof( char ) );

        if( *target == NULL ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_parser_JSON_packField_str( \"%s\", \"%s\", %p )] "
                       "Error copying sub-string->`char *`: failed malloc.",
                       key, val, *target
            );
            return false;
        }

        strncpy( *target, &val[ offset ], substr_length );
        (*target)[substr_length] = '\0';

    } else { //copy as-is
        *target = malloc( ( length + 1 ) * sizeof( char ) );

        if( *target == NULL ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_parser_JSON_packField_str( \"%s\", \"%s\", %p )] "
                       "Error copying string->`char *`: failed malloc.",
                       key, val, *target
            );
            return false;
        }

        memcpy( *target, val, length );
        (*target)[length] = '\0';
    }

    return true;
}

/**
 * [PRIVATE] Converts and packs a string value into a `bool` field
 * @param key    Name of the field (used for debug msg purposes)
 * @param val    Value of field as a string
 * @param target Target container
 * @return Success of operation
 */
static bool ctune_parser_JSON_packField_bool( const char * key, const char * val, bool * target ) {
    if( val == NULL ) {
        CTUNE_LOG( CTUNE_LOG_WARNING,
                   "[ctune_parser_JSON_packField_bool( \"%s\", NULL, %p )] NULL value.",
                   key, target );

        return true; //EARLY RETURN
    }

    long tmp = strtol( val, NULL, 10 );

    if( ( tmp == 0 && strcmp( val, "0" ) != 0 ) || errno == ERANGE ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_packField_bool( \"%s\", \"%s\", %p )] "
                   "Failed string->`bool` conversion.",
                   key, val, target
        );

        return false;
    }

    *target = ( tmp != 0 );

    return true;
}

/**
 * [PRIVATE] Converts and packs a string value into a `long` field
 * @param key    Name of the field (used for debug msg purposes)
 * @param val    Value of field as a string
 * @param target Target container
 * @return Success of operation
 */
static bool ctune_parser_JSON_packField_long( const char * key, const char * val, long * target ) {
    if( val == NULL ) {
        CTUNE_LOG( CTUNE_LOG_WARNING,
                   "[ctune_parser_JSON_packField_long( \"%s\", NULL, %p )] NULL value.",
                   key, target );

        return true; //EARLY RETURN
    }

    *target = strtol( val, NULL, 10 );

    if( ( *target == 0 && strcmp( val, "0" ) != 0 ) || errno == ERANGE ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_packField_long( \"%s\", \"%s\", %ld )] "
                   "Failed string->`long` conversion.",
                   key, val, *target
        );

        return false;
    }

    return true;
}

/**
 * [PRIVATE] Converts and packs a string value into an unsigned `long` field
 * @param key    Name of the field (used for debug msg purposes)
 * @param val    Value of field as a string
 * @param target Target container
 * @return Success of operation
 */
static bool ctune_parser_JSON_packField_ulong( const char * key, const char * val, ulong * target ) {
    if( val == NULL ) {
        CTUNE_LOG( CTUNE_LOG_WARNING,
                   "[ctune_parser_JSON_packField_ulong( \"%s\", NULL, %p )] NULL value.",
                   key, target );

        return true; //EARLY RETURN
    }

    *target = strtoul( val, NULL, 10 );

    if( ( *target == 0 && strcmp( val, "0" ) != 0 ) || errno == ERANGE ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_packField_ulong( \"%s\", \"%s\", %lu )] "
                   "Failed string->`ulong` conversion.",
                   key, val, *target
        );

        return false;
    }

    return true;
}

/**
 * [PRIVATE] Converts and packs a string value into a 'double' field
 * @param key    Name of the field (used for debug msg purposes)
 * @param val    Value of field as a string
 * @param target Target container
 * @return Success of operation
 */
static bool ctune_parser_JSON_packField_double( const char * key, const char * val, double * target ) {
    if( val == NULL ) {
        CTUNE_LOG( CTUNE_LOG_WARNING,
                   "[ctune_parser_JSON_packField_double( \"%s\", NULL, %p )] NULL value.",
                   key, target );

        return true; //EARLY RETURN
    }

    *target = strtod( val, NULL );

    if( errno == ERANGE ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_packField_double( \"%s\", \"%s\", %f )] "
                   "Failed string->`double` conversion.",
                   key, val, *target
        );

        return false;
    }

    return true;
}

/**
 * [PRIVATE] Converts and packs a string value into an unsigned `long` field
 * @param key      Name of the field (used for debug msg purposes)
 * @param val      Value of field as a string
 * @param enum_min Smallest integer value for enum range
 * @param enum_max Largest integer value for enum range
 * @param target   Target container
 * @return Success of operation
 */
static bool ctune_parser_JSON_packField_enum( const char * key, const char * val, int enum_min, int enum_max, int * target ) {
    ulong l = strtoul( val, NULL, 10 );

    if( ( l == 0 && strcmp( val, "0" ) != 0 ) || errno == ERANGE ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_packField_enum( \"%s\", \"%s\", %i, %i, %p )] "
                   "Failed string->`ulong` conversion.",
                   key, val, enum_min, enum_max, target
        );

        return false; //EARLY RETURN
    }

    if( !ctune_utoi( l, target ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_packField_enum( \"%s\", \"%s\", %i, %i, %p )] "
                   "Integer overflow detected (%lu).",
                   key, val, enum_min, enum_max, target, l
        );

        return false; //EARLY RETURN
    }

    if( l < enum_min || l > enum_max ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_packField_enum( \"%s\", \"%s\", %i, %i, %p )] "
                   "Integer '%i' not in enum range [%i-%i]",
                   key, val, enum_min, enum_max, target, *target, enum_min, enum_max
        );

        return false; //EARLY RETURN
    }

    return true;
}

/**
 * [PRIVATE] Packs key-value pairs into a ServerStats struct
 * @param stats ServerStats object
 * @param key   Key string
 * @param val   Value string
 * @return Success
 */
static bool ctune_parser_JSON_packServerStats( struct ctune_ServerStats * stats, const char * key, const char * val ) {
    if( stats == NULL ) { //ERROR CONTROL
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_packServerStats( %p, \"%s\", \"%s\" )] "
                   "ServerStats pointer is NULL.",
                   stats, key, val
        );

        return false;
    }

    if( strcmp( key, "supported_version" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &stats->supported_version );

    if( strcmp( key, "software_version" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &stats->software_version );

    if( strcmp( key, "status" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &stats->status );

    if( strcmp( key, "stations" ) == 0 )
        return ctune_parser_JSON_packField_ulong( key, val, &stats->stations );

    if( strcmp( key, "stations_broken" ) == 0 )
        return ctune_parser_JSON_packField_ulong( key, val, &stats->stations_broken );

    if( strcmp( key, "tags" ) == 0 )
        return ctune_parser_JSON_packField_ulong( key, val, &stats->tags );

    if( strcmp( key, "clicks_last_hour" ) == 0 )
        return ctune_parser_JSON_packField_ulong( key, val, &stats->clicks_last_hour );

    if( strcmp( key, "clicks_last_day" ) == 0 )
        return ctune_parser_JSON_packField_ulong( key, val, &stats->clicks_last_day );

    if( strcmp( key, "languages" ) == 0 )
        return ctune_parser_JSON_packField_ulong( key, val, &stats->languages );

    if( strcmp( key, "countries" ) == 0 )
        return ctune_parser_JSON_packField_ulong( key, val, &stats->countries );

    CTUNE_LOG( CTUNE_LOG_ERROR,
               "[ctune_parser_JSON_packServerStats( %p, \"%s\", \"%s\" )] "
               "Key not recognised (unexpected change in src json?).",
               stats, key, val
    );

    return false;
}

/**
 * [PRIVATE] Packs key-value pairs into a ServerConfig struct
 * @param stats ServerConfig object
 * @param key   Key string
 * @param val   Value string
 * @return Success
 */
static bool ctune_parser_JSON_packServerConfig( struct ctune_ServerConfig * cfg, const char * key, const char * val ) {
    if( cfg == NULL ) { //ERROR CONTROL
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_packServerConfig( %p, \"%s\", \"%s\" )] "
                   "ServerConfig pointer is NULL.",
                   cfg, key, val
        );

        return false;
    }

    if( strcmp( key, "check_enabled" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &cfg->check_enabled );

    if( strcmp( key, "prometheus_exporter_enabled" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &cfg->prometheus_exporter_enabled );

    if( strcmp( key, "pull_servers" ) == 0 )
        return( StrList.insert_back( &cfg->pull_servers, val ) != NULL );

    if( strcmp( key, "tcp_timeout_seconds" ) == 0 )
        return ctune_parser_JSON_packField_ulong( key, val, &cfg->tcp_timeout_seconds );

    if( strcmp( key, "broken_stations_never_working_timeout_seconds" ) == 0 )
        return ctune_parser_JSON_packField_ulong( key, val, &cfg->broken_stations_never_working_timeout_seconds );

    if( strcmp( key, "broken_stations_timeout_seconds" ) == 0 )
        return ctune_parser_JSON_packField_ulong( key, val, &cfg->broken_stations_timeout_seconds );

    if( strcmp( key, "checks_timeout_seconds" ) == 0 )
        return ctune_parser_JSON_packField_ulong( key, val, &cfg->checks_timeout_seconds );

    if( strcmp( key, "click_valid_timeout_seconds" ) == 0 )
        return ctune_parser_JSON_packField_ulong( key, val, &cfg->click_valid_timeout_seconds );

    if( strcmp( key, "clicks_timeout_seconds" ) == 0 )
        return ctune_parser_JSON_packField_ulong( key, val, &cfg->clicks_timeout_seconds );

    if( strcmp( key, "mirror_pull_interval_seconds" ) == 0 )
        return ctune_parser_JSON_packField_ulong( key, val, &cfg->mirror_pull_interval_seconds );

    if( strcmp( key, "update_caches_interval_seconds" ) == 0 )
        return ctune_parser_JSON_packField_ulong( key, val, &cfg->update_caches_interval_seconds );

    if( strcmp( key, "server_name" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &cfg->server_name );

    if( strcmp( key, "check_retries" ) == 0 )
        return ctune_parser_JSON_packField_ulong( key, val, &cfg->check_retries );

    if( strcmp( key, "check_batchsize" ) == 0 )
        return ctune_parser_JSON_packField_ulong( key, val, &cfg->check_batchsize );

    if( strcmp( key, "check_pause_seconds" ) == 0 )
        return ctune_parser_JSON_packField_ulong( key, val, &cfg->check_pause_seconds );

    if( strcmp( key, "api_threads" ) == 0 )
        return ctune_parser_JSON_packField_ulong( key, val, &cfg->api_threads );

    if( strcmp( key, "cache_type" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &cfg->cache_type );

    if( strcmp( key, "cache_ttl" ) == 0 )
        return ctune_parser_JSON_packField_ulong( key, val, &cfg->cache_ttl );

    CTUNE_LOG( CTUNE_LOG_ERROR,
               "[ctune_parser_JSON_packServerConfig( %p, \"%s\", \"%s\" )] "
               "Key not recognised (unexpected change in src json?).",
               cfg, key, val
    );

    return false;
}

/**
 * [PRIVATE] Packs key-value pairs into a RadioStationInfo struct
 * @param rsi RadioStationInfo object
 * @param key Key string
 * @param val Value string
 * @return Success
 */
static bool ctune_parser_JSON_packStationInfo( struct ctune_RadioStationInfo * rsi, const char * key, const char * val ) {
    if( rsi == NULL ) { //ERROR CONTROL
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_packStationInfo( %p, \"%s\", \"%s\" )] "
                   "RadioStationInfo pointer is NULL.",
                   rsi, key, val
        );

        return false;
    }

    if( strcmp( key, "bitrate" ) == 0 )
        return ctune_parser_JSON_packField_ulong( key, val, &rsi->bitrate );

    if( strcmp( key, "changeuuid" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &rsi->change_uuid );

    if( strcmp( key, "clickcount" ) == 0 )
        return ctune_parser_JSON_packField_ulong( key, val, &rsi->clickcount );

    if( strcmp( key, "clicktimestamp" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &rsi->click_timestamp );

    if( strcmp( key, "clicktimestamp_iso8601" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &rsi->iso8601.click_timestamp );

    if( strcmp( key, "clicktrend" ) == 0 )
        return ctune_parser_JSON_packField_long( key, val, &rsi->clicktrend );

    if( strcmp( key, "codec" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &rsi->codec );

    if( strcmp( key, "country" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &rsi->country );

    if( strcmp( key, "countrycode" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &rsi->country_code );

    if( strcmp( key, "favicon" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &rsi->favicon_url );

    if( strcmp( key, "hls" ) == 0 )
        return ctune_parser_JSON_packField_bool( key, val, &rsi->hls );

    if( strcmp( key, "homepage" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &rsi->homepage );

    if( strcmp( key, "language" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &rsi->language );

    if( strcmp( key, "languagecodes" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &rsi->language_codes );

    if( strcmp( key, "lastchangetime" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &rsi->last_change_time );

    if( strcmp( key, "lastchangetime_iso8601" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &rsi->iso8601.last_change_time );

    if( strcmp( key, "lastcheckok" ) == 0 )
        return ctune_parser_JSON_packField_bool( key, val, &rsi->last_check_ok );

    if( strcmp( key, "lastchecktime" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &rsi->last_check_time );

    if( strcmp( key, "lastchecktime_iso8601" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &rsi->iso8601.last_check_time );

    if( strcmp( key, "lastcheckoktime" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &rsi->last_check_ok_time );

    if( strcmp( key, "lastcheckoktime_iso8601" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &rsi->iso8601.last_check_ok_time );

    if( strcmp( key, "lastlocalchecktime" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &rsi->last_local_check_time );

    if( strcmp( key, "lastlocalchecktime_iso8601" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &rsi->iso8601.last_local_check_time );

    if( strcmp( key, "name" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &rsi->name );

    if( strcmp( key, "state" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &rsi->state );

    if( strcmp( key, "stationuuid" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &rsi->station_uuid );

    if( strcmp( key, "tags" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &rsi->tags );

    if( strcmp( key, "url" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &rsi->url );

    if( strcmp( key, "url_resolved" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &rsi->url_resolved );

    if( strcmp( key, "votes" ) == 0 )
        return ctune_parser_JSON_packField_ulong( key, val, &rsi->votes );

    if( strcmp( key, "ssl_error" ) == 0 )
        return ctune_parser_JSON_packField_long( key, val, &rsi->ssl_error );

    if( strcmp( key, "geo_lat" ) == 0 )
        return ctune_parser_JSON_packField_double( key, val, &rsi->geo.latitude );

    if( strcmp( key, "geo_long" ) == 0 )
        return ctune_parser_JSON_packField_double( key, val, &rsi->geo.longitude );

    if( strcmp( key, "station_src" ) == 0 )
        return ctune_parser_JSON_packField_enum( key, val,
                                                 (int) CTUNE_STATIONSRC_LOCAL,
                                                 ((int) CTUNE_STATIONSRC_COUNT) - 1,
                                                 (int *) &rsi->station_src );

    CTUNE_LOG( CTUNE_LOG_ERROR,
               "[ctune_parser_JSON_packStationInfo( %p, \"%s\", \"%s\" )] "
               "Key not recognised (unexpected change in src json?): @dev -> check remote API docs for recent changes.",
               rsi, key, val
    );

    return false;
}

/**
 * [PRIVATE] Packs key-value pairs into a CategoryItem struct
 * @param cat_item CategoryItem object
 * @param key      Key string
 * @param val      Value string
 * @return Success
 */
static bool ctune_parser_JSON_packCategoryItem( struct ctune_CategoryItem * cat_item, const char * key, const char * val ) {
    if( cat_item == NULL ) { //ERROR CONTROL
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_packCategoryItem( %p, \"%s\", \"%s\" )] "
                   "CategoryItem pointer is NULL.",
                   cat_item, key, val
        );

        return false;
    }

    if( strcmp( key, "name" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &cat_item->name );

    if( strcmp( key, "stationcount" ) == 0 )
        return ctune_parser_JSON_packField_ulong( key, val, &cat_item->stationcount );

    if( strcmp( key, "country" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &cat_item->country );
        //INFO (28 Sept 2020): The RadioBrowser API returns "Array" when there are multiple countries with the same state name

    CTUNE_LOG( CTUNE_LOG_ERROR,
               "[ctune_parser_JSON_packCategoryItem( %p, \"%s\", \"%s\" )] "
               "Key not recognised (unexpected change in src json?).",
               cat_item, key, val
    );

    return false;
}

/**
 * [PRIVATE] Packs key-value pairs into a ClickCounter struct
 * @param clk_counter ClickCounter object
 * @param key         Key string
 * @param val         Value string
 * @return Success
 */
static bool ctune_parser_JSON_packClickCounter( struct ctune_ClickCounter * clk_counter, const char * key, const char * val ) {
    if( clk_counter == NULL ) { //ERROR CONTROL
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_packClickCounter( %p, \"%s\", \"%s\" )] "
                   "ClickCounter pointer is NULL.",
                   clk_counter, key, val
        );

        return false;
    }

    if( strcmp( key, "name" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &clk_counter->name );

    if( strcmp( key, "stationuuid" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &clk_counter->stationuuid );

    if( strcmp( key, "url" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &clk_counter->url );

    if( strcmp( key, "ok" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &clk_counter->ok );

    if( strcmp( key, "message" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &clk_counter->message );

    CTUNE_LOG( CTUNE_LOG_ERROR,
               "[ctune_parser_JSON_packClickCounter( %p, \"%s\", \"%s\" )] "
               "Key not recognised (unexpected change in src json?).",
               clk_counter, key, val
    );

    return false;
}

/**
 * [PRIVATE] Packs key-value pairs into a ClickCounter struct
 * @param vote_state RadioStationVote object
 * @param key        Key string
 * @param val        Value string
 * @return Success
 */
static bool ctune_parser_JSON_packRadioStationVote( struct ctune_RadioStationVote * vote_state, const char * key, const char * val ) {
    if( vote_state == NULL ) { //ERROR CONTROL
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_packRadioStationVote( %p, \"%s\", \"%s\" )] "
                   "ClickCounter pointer is NULL.",
                   vote_state, key, val
        );

        return false;
    }

    if( strcmp( key, "ok" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &vote_state->ok );

    if( strcmp( key, "message" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &vote_state->message );

    CTUNE_LOG( CTUNE_LOG_ERROR,
               "[ctune_parser_JSON_packRadioStationVote( %p, \"%s\", \"%s\" )] "
               "Key not recognised (unexpected change in src json?).",
               vote_state, key, val
    );

    return false;
}

/**
 * [PRIVATE] Packs key-value pairs into a NewRadioStation.received struct
 * @param new_station NewRadioStation object
 * @param key         Key string
 * @param val         Value string
 * @return Success
 */
static bool ctune_parser_JSON_packNewRadioStationRcv( struct ctune_NewRadioStation * new_station, const char * key, const char * val ) {
    if( new_station == NULL ) { //ERROR CONTROL
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_packNewRadioStationRcv( %p, \"%s\", \"%s\" )] "
                   "ClickCounter pointer is NULL.",
                   new_station, key, val
        );

        return false;
    }

    if( strcmp( key, "ok" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &new_station->received.ok );

    if( strcmp( key, "message" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &new_station->received.message );

    if( strcmp( key, "uuid" ) == 0 )
        return ctune_parser_JSON_packField_str( key, val, &new_station->received.uuid );

    CTUNE_LOG( CTUNE_LOG_ERROR,
               "[ctune_parser_JSON_packNewRadioStationRcv( %p, \"%s\", \"%s\" )] "
               "Key not recognised (unexpected change in src json?).",
               new_station, key, val
    );

    return false;
}

//=====================================================================================================================================================================
//--------------------------------------------------------------- PUBLIC ----------------------------------------------------------------------------------------------
//=====================================================================================================================================================================

/**
 * Parse a raw JSON formatted string into a ServerStats DTO struct
 * @param raw_str Raw JSON string
 * @param stats   ServerStats DTO instance
 * @return Success
 */
bool ctune_parser_JSON_parseToServerStats( const struct String * raw_str, struct ctune_ServerStats * stats ) {
    enum json_tokener_error err_token;
    json_object * json  = json_tokener_parse_verbose( raw_str->_raw, &err_token );

    if( err_token != json_tokener_success ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_parseToServerStats( %p, %p )] "
                   "Error parsing JSON data: %s",
                   raw_str, stats, json_tokener_error_desc( err_token )
        );

        return false;
    }

    if( json_object_get_type( json ) != json_type_object ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_parseToServerStats( %p, %p )] "
                   "Unexpected root type for JSON data: expected 'object', got '%s'.",
                   raw_str, stats, json_type_to_name( json_object_get_type( json ) )
        );

        return false;
    }

    bool                        parse_err = false;
    struct json_object_iterator it        = json_object_iter_begin( json );
    struct json_object_iterator it_end    = json_object_iter_end( json );

    while( !json_object_iter_equal( &it, &it_end ) ) {

        const char         * k     = json_object_iter_peek_name( &it );
        struct json_object * v_obj = json_object_iter_peek_value( &it );
        const char         * v     = json_object_get_string( v_obj );

        if( !ctune_parser_JSON_packServerStats( stats, k, v ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_parser_JSON_parseToServerStats( %p, %p )] "
                       "Error packing json into struct: K=%s, V=%s",
                       raw_str, stats, k, v
            );

            parse_err = true;
        }

        json_object_iter_next( &it );
    }

    json_object_put( json ); //free json object

    return (!parse_err);
}

/**
 * Parse a raw JSON formatted string into a ServerConfig DTO struct
 * @param raw_str Raw JSON string
 * @param cfg     ServerConfig DTO instance
 * @return Success
 */
bool ctune_parser_JSON_parseToServerConfig( const struct String * raw_str, struct ctune_ServerConfig * cfg ) {
    enum json_tokener_error err_token;
    json_object * json  = json_tokener_parse_verbose( raw_str->_raw, &err_token );

    if( err_token != json_tokener_success ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_parseToServerConfig( %p, %p )] "
                   "Error parsing JSON data: %s",
                   raw_str, cfg, json_tokener_error_desc( err_token )
        );

        return false;
    }

    if( json_object_get_type( json ) != json_type_object ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_parseToServerConfig( %p, %p )] "
                   "Unexpected root type for JSON data: expected 'object', got '%s'.",
                   raw_str, cfg, json_type_to_name( json_object_get_type( json ) )
        );

        return false;
    }

    bool                        parse_err = false;
    struct json_object_iterator it        = json_object_iter_begin( json );
    struct json_object_iterator it_end    = json_object_iter_end( json );

    while( !json_object_iter_equal( &it, &it_end ) ) {

        const char         * k     = json_object_iter_peek_name( &it );
        struct json_object * v_obj = json_object_iter_peek_value( &it );

        if( json_object_get_type( v_obj ) == json_type_array ) {
            //INFO only the "pull_servers" key has an array of values
            struct array_list * list = json_object_get_array( v_obj );

            for( size_t i = 0; i < json_object_array_length( v_obj ); ++i ) {
                const char * arr_val = json_object_get_string( list->array[i] );

                if( !ctune_parser_JSON_packServerConfig( cfg, k, arr_val ) ) {
                    CTUNE_LOG( CTUNE_LOG_ERROR,
                               "[ctune_parser_JSON_parseToServerConfig( %p, %p )] "
                               "Error packing json array item into struct: K=%s, V=%s",
                               raw_str, cfg, k, arr_val
                    );

                    parse_err = true;
                }
            }

        } else {
            const char *v = json_object_get_string( v_obj );

            if( !ctune_parser_JSON_packServerConfig( cfg, k, v ) ) {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_parser_JSON_parseToServerConfig( %p, %p )] "
                           "Error packing json item into struct: K=%s, V=%s",
                           raw_str, cfg, k, v
                );

                parse_err = true;
            }
        }

        json_object_iter_next( &it );
    }

    json_object_put( json ); //free json object

    return (!parse_err);
}

/**
 * Parse a raw JSON formatted string into a collection of RadioStations
 * @param raw_str        Raw JSON string
 * @param radio_stations RadioStations collection instance
 * @return Success
 */
static bool ctune_parser_JSON_parseToRadioStationList( const struct String * raw_str, struct Vector * radio_stations ) {
    enum json_tokener_error err_token;
    json_object * json = json_tokener_parse_verbose( raw_str->_raw, &err_token );

    if( err_token != json_tokener_success ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_parseToRadioStationList( %p, %p )] "
                   "Error parsing JSON data: %s",
                   raw_str, radio_stations, json_tokener_error_desc( err_token )
        );

        CTUNE_LOG( CTUNE_LOG_TRACE,
                   "[ctune_parser_JSON_parseToRadioStationList( %p, %p )] "
                   "Raw string = \n%s",
                   raw_str, radio_stations, ( raw_str != NULL ? raw_str->_raw :  "(null)" )
        );

        return false;
    }

    if( json_object_get_type( json ) != json_type_array ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_parseToRadioStationList( %p, %p )] "
                   "Unexpected root type for JSON data: expected 'array', got '%s'.",
                   raw_str, radio_stations, json_type_to_name( json_object_get_type( json ) )
        );

        return false;
    }

    bool                parse_err = false;
    struct array_list * list      = json_object_get_array( json );

    for( size_t i = 0; i < list->length; ++i ) {
        struct json_object_iterator it     = json_object_iter_begin( ( (json_object *) list->array[i] ) );
        struct json_object_iterator it_end = json_object_iter_end( ( (json_object *) list->array[i] ) );

        struct ctune_RadioStationInfo * rsi = Vector.init_back( radio_stations, ctune_RadioStationInfo.init );

        while( !json_object_iter_equal( &it, &it_end ) ) {

            const char         * k     = json_object_iter_peek_name( &it );
            struct json_object * v_obj = json_object_iter_peek_value( &it );
            const char         * v     = json_object_get_string( v_obj );

            if( !ctune_parser_JSON_packStationInfo( rsi, k, v ) ) {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_parser_JSON_parseToRadioStationList( %p, %p )] "
                           "Error packing json item into struct: K=%s, V=%s",
                          raw_str, radio_stations, k, v
                );

                parse_err = true;
            }

            json_object_iter_next( &it );
        }
    }

    json_object_put( json ); //free json object

    return (!parse_err);
}

/**
 * Parse a raw JSON formatted string into a collection of RadioStationInfo from a specified station source
 * @param raw_str        Raw JSON string
 * @param src            Radio station source to specify in each RSI objects
 * @param radio_stations RadioStationInfo collection instance
 * @return Success
 */
static bool ctune_parser_JSON_parseToRadioStationListFrom( const struct String * raw_str, ctune_StationSrc_e src, struct Vector * radio_stations ) {
    enum json_tokener_error err_token;
    json_object * json = json_tokener_parse_verbose( raw_str->_raw, &err_token );

    if( err_token != json_tokener_success ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_parseToRadioStationListFrom( %p, %i, %p )] "
                   "Error parsing JSON data: %s",
                   raw_str, src, radio_stations, json_tokener_error_desc( err_token )
        );

        CTUNE_LOG( CTUNE_LOG_TRACE,
                   "[ctune_parser_JSON_parseToRadioStationListFrom( %p, %i, %p )] "
                   "Raw string = \n%s",
                   raw_str, src, radio_stations, ( raw_str != NULL ? raw_str->_raw :  "(null)" )
        );

        return false;
    }

    if( json_object_get_type( json ) != json_type_array ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_parseToRadioStationListFrom( %p, %i, %p )] "
                   "Unexpected root type for JSON data: expected 'array', got '%s'.",
                   raw_str, src, radio_stations, json_type_to_name( json_object_get_type( json ) )
        );

        return false;
    }

    bool                parse_err = false;
    struct array_list * list      = json_object_get_array( json );

    for( size_t i = 0; i < list->length; ++i ) {
        struct json_object_iterator it     = json_object_iter_begin( ( (json_object *) list->array[i] ) );
        struct json_object_iterator it_end = json_object_iter_end( ( (json_object *) list->array[i] ) );

        struct ctune_RadioStationInfo * rsi = Vector.init_back( radio_stations, ctune_RadioStationInfo.init );

        while( !json_object_iter_equal( &it, &it_end ) ) {

            const char         * k     = json_object_iter_peek_name( &it );
            struct json_object * v_obj = json_object_iter_peek_value( &it );
            const char         * v     = json_object_get_string( v_obj );

            if( !ctune_parser_JSON_packStationInfo( rsi, k, v ) ) {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_parser_JSON_parseToRadioStationList( %p, %p )] "
                           "Error packing json item into struct: K=%s, V=%s",
                           raw_str, radio_stations, k, v
                );

                parse_err = true;
            }

            json_object_iter_next( &it );
        }

        rsi->station_src = src;
    }

    json_object_put( json ); //free json object

    return (!parse_err);
}

/**
 * Parse a raw JSON formatted string into a collection of CategoryItems
 * @param raw_str        Raw JSON string
 * @param category_items CategoryItems collection instance
 * @return Success
 */
bool ctune_parser_JSON_parseToCategoryItemList( const struct String * raw_str, struct Vector * category_items ) {
    enum json_tokener_error err_token;
    json_object * json = json_tokener_parse_verbose( raw_str->_raw, &err_token );

    if( err_token != json_tokener_success ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_parseToCategoryItemList( %p, %p )] "
                   "Error parsing JSON data: %s",
                   raw_str, category_items, json_tokener_error_desc( err_token )
        );

        return false;
    }

    if( json_object_get_type( json ) != json_type_array ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_parseToCategoryItemList( %p, %p )] "
                   "Unexpected root type for JSON data: expected 'array', got '%s'.",
                   raw_str, category_items, json_type_to_name( json_object_get_type( json ) )
        );

        return false;
    }

    bool                parse_err = false;
    struct array_list * list      = json_object_get_array( json );


    for( size_t i = 0; i < list->length; ++i ) {
        struct json_object_iterator it     = json_object_iter_begin( ( (json_object *) list->array[i] ) );
        struct json_object_iterator it_end = json_object_iter_end( ( (json_object *) list->array[i] ) );

        struct ctune_CategoryItem * cat_item = Vector.init_back( category_items, ctune_CategoryItem.init );

        while( !json_object_iter_equal( &it, &it_end ) ) {

            const char         * k     = json_object_iter_peek_name( &it );
            struct json_object * v_obj = json_object_iter_peek_value( &it );
            const char         * v     = json_object_get_string( v_obj );

            if( !ctune_parser_JSON_packCategoryItem( cat_item, k, v ) ) {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_parser_JSON_parseToCategoryItemList( %p, %p )] "
                           "Error packing json item into struct: K=%s, V=%s",
                           raw_str, category_items, k, v
                );

                parse_err = true;
            }

            json_object_iter_next( &it );
        }
    }

    json_object_put( json ); //free json object

    return (!parse_err);
}

/**
 * Parse a raw JSON formatted string into a ClickCounter DTO
 * @param raw_str     Raw JSON string
 * @param clk_counter ClickCounter DTO
 * @return Success
 */
static bool ctune_parser_JSON_parseToClickCounter( const struct String * raw_str, struct ctune_ClickCounter * clk_counter ) {
    enum json_tokener_error err_token;
    json_object * json = json_tokener_parse_verbose( raw_str->_raw, &err_token );

    if( err_token != json_tokener_success ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_parseToClickCounter( %p, %p )] "
                   "Error parsing JSON data: %s",
                   raw_str, clk_counter, json_tokener_error_desc( err_token )
        );

        return false;
    }

    if( json_object_get_type( json ) != json_type_object ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_parseToClickCounter( %p, %p )] "
                   "Unexpected root type for JSON data: expected 'object', got '%s'.",
                   raw_str, clk_counter, json_type_to_name( json_object_get_type( json ) )
        );

        return false;
    }

    bool                        parse_err = false;
    struct json_object_iterator it        = json_object_iter_begin( json );
    struct json_object_iterator it_end    = json_object_iter_end( json );

    while( !json_object_iter_equal( &it, &it_end ) ) {

        const char         * k     = json_object_iter_peek_name( &it );
        struct json_object * v_obj = json_object_iter_peek_value( &it );
        const char         * v     = json_object_get_string( v_obj );

        if( !ctune_parser_JSON_packClickCounter( clk_counter, k, v ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_parser_JSON_parseToClickCounter( %p, %p )] "
                       "Error packing json item into struct: K=%s, V=%s",
                       raw_str, clk_counter, k, v
            );

            parse_err = true;
        }

        json_object_iter_next( &it );
    }

    json_object_put( json ); //free json object

    return (!parse_err);
}

/**
 * Parse a raw JSON formatted string into a RadioStationVote DTO
 * @param raw_str    Raw JSON string
 * @param vote_state RadioStationVote DTO
 * @return Success
 */
bool ctune_parser_JSON_parseToRadioStationVote( const struct String * raw_str, struct ctune_RadioStationVote * vote_state ) {
    enum json_tokener_error err_token;
    json_object * json = json_tokener_parse_verbose( raw_str->_raw, &err_token );

    if( err_token != json_tokener_success ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_parseToRadioStationVote( %p, %p )] "
                   "Error parsing JSON data: %s",
                   raw_str, vote_state, json_tokener_error_desc( err_token )
        );

        return false;
    }

    if( json_object_get_type( json ) != json_type_object ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_parseToRadioStationVote( %p, %p )] "
                   "Unexpected root type for JSON data: expected 'object', got '%s'.",
                   raw_str, vote_state, json_type_to_name( json_object_get_type( json ) )
        );

        return false;
    }

    bool                        parse_err = false;
    struct json_object_iterator it        = json_object_iter_begin( json );
    struct json_object_iterator it_end    = json_object_iter_end( json );

    while( !json_object_iter_equal( &it, &it_end ) ) {

        const char         * k     = json_object_iter_peek_name( &it );
        struct json_object * v_obj = json_object_iter_peek_value( &it );
        const char         * v     = json_object_get_string( v_obj );

        if( !ctune_parser_JSON_packRadioStationVote( vote_state, k, v ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_parser_JSON_parseToRadioStationVote( %p, %p )] "
                       "Error packing json item into struct: K=%s, V=%s",
                       raw_str, vote_state, k, v
            );

            parse_err = true;
        }

        json_object_iter_next( &it );
    }

    json_object_put( json ); //free json object

    return (!parse_err);
}

/**
 * Parse a raw JSON formatted string into a NewRadioStation.received DTO
 * @param raw_str     Raw JSON string
 * @param new_station NewRadioStation DTO
 * @return Success
 */
static bool ctune_parser_JSON_parseToNewRadioStationRcv( const struct String * raw_str, struct ctune_NewRadioStation * new_station ) {
    enum json_tokener_error err_token;
    json_object * json = json_tokener_parse_verbose( raw_str->_raw, &err_token );

    if( err_token != json_tokener_success ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_parseToNewRadioStationRcv( %p, %p )] "
                   "Error parsing JSON data: %s",
                   raw_str, new_station, json_tokener_error_desc( err_token )
        );

        return false;
    }

    if( json_object_get_type( json ) != json_type_object ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_parseToNewRadioStationRcv( %p, %p )] "
                   "Unexpected root type for JSON data: expected 'object', got '%s'.",
                   raw_str, new_station, json_type_to_name( json_object_get_type( json ) )
        );

        return false;
    }

    bool                        parse_err = false;
    struct json_object_iterator it        = json_object_iter_begin( json );
    struct json_object_iterator it_end    = json_object_iter_end( json );

    while( !json_object_iter_equal( &it, &it_end ) ) {

        const char         * k     = json_object_iter_peek_name( &it );
        struct json_object * v_obj = json_object_iter_peek_value( &it );
        const char         * v     = json_object_get_string( v_obj );

        if( !ctune_parser_JSON_packNewRadioStationRcv( new_station, k, v ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_parser_JSON_parseToNewRadioStationRcv( %p, %p )] "
                       "Error packing json item into struct: K=%s, V=%s",
                        raw_str, new_station, k, v
            );

            parse_err = true;
        }

        json_object_iter_next( &it );
    }

    json_object_put( json ); //free json object

    return (!parse_err);
}

/**
 * JSON-ify a list of station object into a JSON string
 * @param stations Collection of RadioStationInfo_t objects
 * @param json_str Container string to store the json string into
 * @return Success
 */
static bool ctune_parser_JSON_parseRadioStationListToJSON( const struct Vector * stations, struct String * json_str ) {
    if( stations == NULL || Vector.empty( stations ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_parseRadioStationListToJSON( %p, %p )] "
                   "Vector of stations is NULL/empty.",
                   stations, json_str
        );

        return false;
    }

    if( json_str == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_parseRadioStationListToJSON( %p, %p )] "
                   "JSON String_t NULL.",
                   stations, json_str
        );

        return false;
    }

    bool          error_state = false;
    json_object * array       = json_object_new_array_ext( Vector.size( stations ) );

    for( size_t i = 0; i < Vector.size( stations ); ++i ) {
        int err[34] = { 0 };

        CTUNE_LOG( CTUNE_LOG_DEBUG,
                   "[ctune_parser_JSON_parseRadioStationListToJSON( %p, %p )] "
                   "Parsing favourite station: %lu/%lu",
                   stations, json_str, (i + 1), Vector.size( stations )
        );

        const ctune_RadioStationInfo_t * rsi     = Vector.at( (Vector_t *) stations, i );
        json_object                    * station = json_object_new_object();

        err[ 0] = json_object_object_add( station, "changeuuid", json_object_new_string( ( rsi->change_uuid != NULL ? rsi->change_uuid : "" ) ) );
        err[ 1] = json_object_object_add( station, "stationuuid", json_object_new_string( ( rsi->station_uuid != NULL ? rsi->station_uuid : "" ) ) );
        err[ 2] = json_object_object_add( station, "name", json_object_new_string( ( rsi->name != NULL ? rsi->name : "" ) ) );
        err[ 3] = json_object_object_add( station, "url", json_object_new_string( ( rsi->url != NULL ? rsi->url : "" ) ) );
        err[ 4] = json_object_object_add( station, "url_resolved", json_object_new_string( ( rsi->url_resolved != NULL ? rsi->url_resolved : "" ) ) );
        err[ 5] = json_object_object_add( station, "homepage", json_object_new_string( ( rsi->homepage != NULL ? rsi->homepage : "" ) ) );
        err[ 6] = json_object_object_add( station, "favicon", json_object_new_string( ( rsi->favicon_url != NULL ? rsi->favicon_url : "" ) ) );
        err[ 7] = json_object_object_add( station, "tags", json_object_new_string( ( rsi->tags != NULL ? rsi->tags : "" ) ) );
        err[ 8] = json_object_object_add( station, "country", json_object_new_string( ( rsi->country != NULL ? rsi->country : "" ) ) );
        err[ 9] = json_object_object_add( station, "countrycode", json_object_new_string( ( rsi->country_code != NULL ? rsi->country_code : "" )) );
        err[10] = json_object_object_add( station, "state", json_object_new_string( ( rsi->state != NULL ? rsi->state : "" )) );
        err[11] = json_object_object_add( station, "language", json_object_new_string( ( rsi->language != NULL ? rsi->language : "" )) );
        err[12] = json_object_object_add( station, "languagecodes", json_object_new_string( ( rsi->language_codes != NULL ? rsi->language_codes : "" )) );
        err[13] = json_object_object_add( station, "votes", json_object_new_uint64( rsi->votes ) );
        err[14] = json_object_object_add( station, "lastchangetime", json_object_new_string( ( rsi->last_change_time != NULL ? rsi->last_change_time : "" )) );
        err[15] = json_object_object_add( station, "lastchangetime_iso8601", json_object_new_string( ( rsi->iso8601.last_change_time != NULL ? rsi->iso8601.last_change_time : "" )) );
        err[16] = json_object_object_add( station, "codec", json_object_new_string( ( rsi->codec != NULL ? rsi->codec : "" )) );
        err[17] = json_object_object_add( station, "bitrate", json_object_new_uint64( rsi->bitrate ) );
        err[18] = json_object_object_add( station, "hls", json_object_new_int( rsi->hls ) );
        err[19] = json_object_object_add( station, "lastcheckok", json_object_new_int( rsi->last_check_ok ) );
        err[20] = json_object_object_add( station, "lastchecktime", json_object_new_string( ( rsi->last_check_time != NULL ? rsi->last_check_time : "" )) );
        err[21] = json_object_object_add( station, "lastchecktime_iso8601", json_object_new_string( ( rsi->iso8601.last_check_time != NULL ? rsi->iso8601.last_check_time : "" )) );
        err[22] = json_object_object_add( station, "lastcheckoktime", json_object_new_string( ( rsi->last_check_ok_time != NULL ? rsi->last_check_ok_time : "" )) );
        err[23] = json_object_object_add( station, "lastcheckoktime_iso8601", json_object_new_string( ( rsi->iso8601.last_check_ok_time != NULL ? rsi->iso8601.last_check_ok_time : "" )) );
        err[24] = json_object_object_add( station, "lastlocalchecktime", json_object_new_string( ( rsi->last_local_check_time != NULL ? rsi->last_local_check_time : "" )) );
        err[25] = json_object_object_add( station, "lastlocalchecktime_iso8601", json_object_new_string( ( rsi->iso8601.last_local_check_time != NULL ? rsi->iso8601.last_local_check_time : "" )) );
        err[26] = json_object_object_add( station, "clicktimestamp", json_object_new_string( ( rsi->click_timestamp != NULL ? rsi->click_timestamp : "" )) );
        err[27] = json_object_object_add( station, "clicktimestamp_iso8601", json_object_new_string( ( rsi->iso8601.click_timestamp != NULL ? rsi->iso8601.click_timestamp : "" )) );
        err[28] = json_object_object_add( station, "clickcount", json_object_new_uint64( rsi->clickcount ) );
        err[29] = json_object_object_add( station, "clicktrend", json_object_new_int64( rsi->clicktrend ) );
        err[30] = json_object_object_add( station, "ssl_error", json_object_new_int64( rsi->ssl_error ) );
        err[31] = json_object_object_add( station, "geo_lat", json_object_new_double( rsi->geo.latitude ) );
        err[32] = json_object_object_add( station, "geo_long", json_object_new_double( rsi->geo.longitude ) );
        err[33] = json_object_object_add( station, "station_src", json_object_new_int( rsi->station_src ) );

        json_object_array_add( array, station );

        for( int err_i = 0; err_i < 23; ++err_i ) {
            if( err[ err_i ] != 0 ) {
                error_state = true;

                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_parser_JSON_parseRadioStationListToJSON( %p, %p )] "
                           "Add JSON object error: RSI=%ul,%i (err: %i)",
                           stations, json_str, i, err_i, err[ err_i ]
                );
            }
        }
    }

    if( !String.set( json_str, json_object_to_json_string( array ) ) ) {
        error_state = true;
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_parseRadioStationListToJSON( %p, %p )] "
                   "Error setting JSON object string to String_t.",
                   stations, json_str
        );
    }

    json_object_put( array );

    return !( error_state );
}

const struct ctune_parser_JSON_Namespace ctune_parser_JSON = {
    .parseToServerStats          = &ctune_parser_JSON_parseToServerStats,
    .parseToServerConfig         = &ctune_parser_JSON_parseToServerConfig,
    .parseToRadioStationList     = &ctune_parser_JSON_parseToRadioStationList,
    .parseToRadioStationListFrom = &ctune_parser_JSON_parseToRadioStationListFrom,
    .parseToCategoryItemList     = &ctune_parser_JSON_parseToCategoryItemList,
    .parseToClickCounter         = &ctune_parser_JSON_parseToClickCounter,
    .parseToRadioStationVote     = &ctune_parser_JSON_parseToRadioStationVote,
    .parseToNewRadioStationRcv   = &ctune_parser_JSON_parseToNewRadioStationRcv,
    .parseRadioStationListToJSON = &ctune_parser_JSON_parseRadioStationListToJSON,
};