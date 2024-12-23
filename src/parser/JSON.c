#include "JSON.h"

#include <errno.h>
#include <json-c/json.h>
#include <json-c/json_visit.h>

#include "logger/src/Logger.h"
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
    if( val == NULL ) {
        CTUNE_LOG( CTUNE_LOG_DEBUG,
                   "[ctune_parser_JSON_packField_str( %s, %p, %p )] Value is NULL - replacing by empty string.",
                   key, val, target
        );

        *target = malloc(  1 * sizeof( char ) );
        (*target)[0] = '\0';

        return true; //EARLY RETURN
    }

    const size_t length = strlen( val );

    if( length == 0 ) {
        *target = malloc(  1 * sizeof( char ) );
        (*target)[0] = '\0';

        return true; //EARLY RETURN
    }

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

    if( strcmp( val, "false" ) == 0 ) {
        *target = false;

    } else if( strcmp( val, "true" ) == 0 ) {
        *target = true;

    } else {
        long tmp = strtol( val, NULL, 10 );

        if( ( tmp == 0 && strcmp( val, "0" ) != 0 ) || errno == ERANGE ) {
            //last ditch attempt to get a boolean (done as a last resort as slow)
            if( !ctune_stob( val, target ) ) {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_parser_JSON_packField_bool( \"%s\", \"%s\", %p )] "
                           "Failed string->`bool` conversion.",
                           key, val, target
                );

                return false; //EARLY RETURN
            }


        } else {
            *target = ( tmp != 0 );
        }
    }

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
 * [PRIVATE] Converts and packs a value into a field
 * @param key Key
 * @param val Value
 * @param type Field type
 * @param target Target field
 * @return Success of operation
 */
static bool ctune_parser_JSON_packField( const char * key, const char * val, ctune_FieldType_e type, void * target ) {
    switch( type ) {
        case CTUNE_FIELD_BOOLEAN:
            return ctune_parser_JSON_packField_bool( key, val, target );
        case CTUNE_FIELD_SIGNED_LONG:
            return ctune_parser_JSON_packField_long( key, val, target );
        case CTUNE_FIELD_UNSIGNED_LONG:
            return ctune_parser_JSON_packField_ulong( key, val, target );
        case CTUNE_FIELD_DOUBLE:
            return ctune_parser_JSON_packField_double( key, val, target );
        case CTUNE_FIELD_CHAR_PTR:
            return ctune_parser_JSON_packField_str( key, val, target );
        case CTUNE_FIELD_STRLIST:
            return( StrList.insert_back( target, val ) != NULL );
        case CTUNE_FIELD_ENUM_STATIONSRC:
            return ctune_parser_JSON_packField_enum( key, val,
                                                     (int) CTUNE_STATIONSRC_LOCAL,
                                                     ((int) CTUNE_STATIONSRC_COUNT) - 1,
                                                     (int *) target );

        case CTUNE_FIELD_STRING: //fallthrough
        default: {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_parser_JSON_packField( \"%s\", \"%s\", %i, %p )] "
                       "Field type (%i) not implemented.",
                       key, val, (int) type, target
            );

            return false;
        }
    }
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
    
    ctune_Field_t field = ctune_ServerStats.getField( stats, key );

    if( !ctune_parser_JSON_packField( key, val, field._type, field._field ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_packServerStats( %p, \"%s\", \"%s\" )] "
                   "Key (type: %i) not recognised (unexpected change in src json?).",
                   stats, key, val, (int) field._type
        );

        return false;
    }

    return true;
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

    ctune_Field_t field = ctune_ServerConfig.getField( cfg, key );

    if( !ctune_parser_JSON_packField( key, val, field._type, field._field ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_packServerConfig( %p, \"%s\", \"%s\" )] "
                   "Key (type: %i) not recognised (unexpected change in src json?).",
                   cfg, key, val, (int) field._type
        );

        return false;
    }

    return true;
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

    ctune_Field_t field = ctune_RadioStationInfo.getField( rsi, key );

    if( !ctune_parser_JSON_packField( key, val, field._type, field._field ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_packStationInfo( %p, \"%s\", \"%s\" )] "
                   "Key (type: %i) not recognised (unexpected change in src json?): @dev -> check remote API docs for recent changes.",
                   rsi, key, val, (int) field._type
        );

        return false;
    }

    return true;
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

    ctune_Field_t field = ctune_CategoryItem.getField( cat_item, key );

    if( !ctune_parser_JSON_packField( key, val, field._type, field._field ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_packCategoryItem( %p, \"%s\", \"%s\" )] "
                   "Key (type: %i) not recognised (unexpected change in src json?).",
                   cat_item, key, val, (int) field._type
        );

        return false;
    }

    return true;
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

    ctune_Field_t field = ctune_ClickCounter.getField( clk_counter, key );

    if( !ctune_parser_JSON_packField( key, val, field._type, field._field ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_packClickCounter( %p, \"%s\", \"%s\" )] "
                   "Key (type: %i) not recognised (unexpected change in src json?).",
                   clk_counter, key, val, (int) field._type
        );

        return false;
    }

    return true;
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

    ctune_Field_t field = ctune_RadioStationVote.getField( vote_state, key );

    if( !ctune_parser_JSON_packField( key, val, field._type, field._field ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_packRadioStationVote( %p, \"%s\", \"%s\" )] "
                   "Key (type: %i) not recognised (unexpected change in src json?).",
                   vote_state, key, val, (int) field._type
        );

        return false;
    }

    return true;
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

    ctune_Field_t field = ctune_NewRadioStation.getReceiveField( new_station, key );

    if( !ctune_parser_JSON_packField( key, val, field._type, field._field ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_parser_JSON_packNewRadioStationRcv( %p, \"%s\", \"%s\" )] "
                   "Key (type: %i) not recognised (unexpected change in src json?).",
                   new_station, key, val, (int) field._type
        );

        return false;
    }

    return true;
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
static bool ctune_parser_JSON_parseToServerStats( const struct String * raw_str, struct ctune_ServerStats * stats ) {
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

            ctune_err.set( CTUNE_ERR_PARSE_UNKNOWN_KEY );
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
static bool ctune_parser_JSON_parseToServerConfig( const struct String * raw_str, struct ctune_ServerConfig * cfg ) {
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

                    ctune_err.set( CTUNE_ERR_PARSE_UNKNOWN_KEY );
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

                ctune_err.set( CTUNE_ERR_PARSE_UNKNOWN_KEY );
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

                ctune_err.set( CTUNE_ERR_PARSE_UNKNOWN_KEY );
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

                ctune_err.set( CTUNE_ERR_PARSE_UNKNOWN_KEY );
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
static bool ctune_parser_JSON_parseToCategoryItemList( const struct String * raw_str, struct Vector * category_items ) {
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

                ctune_err.set( CTUNE_ERR_PARSE_UNKNOWN_KEY );
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

            ctune_err.set( CTUNE_ERR_PARSE_UNKNOWN_KEY );
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
static bool ctune_parser_JSON_parseToRadioStationVote( const struct String * raw_str, struct ctune_RadioStationVote * vote_state ) {
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

            ctune_err.set( CTUNE_ERR_PARSE_UNKNOWN_KEY );
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

            ctune_err.set( CTUNE_ERR_PARSE_UNKNOWN_KEY );
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
        int err[38] = { 0 };

        CTUNE_LOG( CTUNE_LOG_DEBUG,
                   "[ctune_parser_JSON_parseRadioStationListToJSON( %p, %p )] "
                   "Parsing favourite station: %lu/%lu",
                   stations, json_str, (i + 1), Vector.size( stations )
        );

        const ctune_RadioStationInfo_t * rsi     = Vector.at( (Vector_t *) stations, i );
        json_object                    * station = json_object_new_object();

        err[ 0] = json_object_object_add( station, "changeuuid", json_object_new_string( ( ctune_RadioStationInfo.get.changeUUID( rsi ) != NULL ? ctune_RadioStationInfo.get.changeUUID( rsi ) : "" ) ) );
        err[ 1] = json_object_object_add( station, "stationuuid", json_object_new_string( ( ctune_RadioStationInfo.get.stationUUID( rsi ) != NULL ? ctune_RadioStationInfo.get.stationUUID( rsi ) : "" ) ) );
        err[ 2] = json_object_object_add( station, "serveruuid", json_object_new_string( ( ctune_RadioStationInfo.get.serverUUID( rsi ) != NULL ? ctune_RadioStationInfo.get.serverUUID( rsi ) : "" ) ) );
        err[ 3] = json_object_object_add( station, "name", json_object_new_string( ( ctune_RadioStationInfo.get.stationName( rsi ) != NULL ? ctune_RadioStationInfo.get.stationName( rsi ) : "" ) ) );
        err[ 4] = json_object_object_add( station, "url", json_object_new_string( ( ctune_RadioStationInfo.get.stationURL( rsi ) != NULL ? ctune_RadioStationInfo.get.stationURL( rsi ) : "" ) ) );
        err[ 5] = json_object_object_add( station, "url_resolved", json_object_new_string( ( ctune_RadioStationInfo.get.resolvedURL( rsi ) != NULL ? ctune_RadioStationInfo.get.resolvedURL( rsi ) : "" ) ) );
        err[ 6] = json_object_object_add( station, "homepage", json_object_new_string( ( ctune_RadioStationInfo.get.homepage( rsi ) != NULL ? ctune_RadioStationInfo.get.homepage( rsi ) : "" ) ) );
        err[ 7] = json_object_object_add( station, "favicon", json_object_new_string( ( ctune_RadioStationInfo.get.faviconURL( rsi ) != NULL ? ctune_RadioStationInfo.get.faviconURL( rsi ) : "" ) ) );
        err[ 8] = json_object_object_add( station, "tags", json_object_new_string( ( ctune_RadioStationInfo.get.tags( rsi ) != NULL ? ctune_RadioStationInfo.get.tags( rsi ) : "" ) ) );
        err[ 9] = json_object_object_add( station, "country", json_object_new_string( ( ctune_RadioStationInfo.get.country( rsi ) != NULL ? ctune_RadioStationInfo.get.country( rsi ) : "" ) ) );
        err[10] = json_object_object_add( station, "countrycode", json_object_new_string( ( ctune_RadioStationInfo.get.countryCode_ISO3166_1( rsi ) != NULL ? ctune_RadioStationInfo.get.countryCode_ISO3166_1( rsi ) : "" ) ) );
        err[11] = json_object_object_add( station, "iso_3166_2", json_object_new_string( ( ctune_RadioStationInfo.get.countryCode_ISO3166_2( rsi ) != NULL ? ctune_RadioStationInfo.get.countryCode_ISO3166_2( rsi ) : "" ) ) );
        err[12] = json_object_object_add( station, "state", json_object_new_string( ( ctune_RadioStationInfo.get.state( rsi ) != NULL ? ctune_RadioStationInfo.get.state( rsi ) : "" )) );
        err[13] = json_object_object_add( station, "language", json_object_new_string( ( ctune_RadioStationInfo.get.language( rsi ) != NULL ? ctune_RadioStationInfo.get.language( rsi ) : "" )) );
        err[14] = json_object_object_add( station, "languagecodes", json_object_new_string( ( ctune_RadioStationInfo.get.languageCodes( rsi ) != NULL ? ctune_RadioStationInfo.get.languageCodes( rsi ) : "" )) );
        err[15] = json_object_object_add( station, "votes", json_object_new_uint64( ctune_RadioStationInfo.get.votes( rsi ) ) );
        err[16] = json_object_object_add( station, "lastchangetime", json_object_new_string( ( rsi->last_change_time != NULL ? rsi->last_change_time : "" ) ) );
        err[17] = json_object_object_add( station, "lastchangetime_iso8601", json_object_new_string( ( ctune_RadioStationInfo.get.lastChangeTS( rsi ) != NULL ? ctune_RadioStationInfo.get.lastChangeTS( rsi ) : "" )) );
        err[18] = json_object_object_add( station, "codec", json_object_new_string( ( ctune_RadioStationInfo.get.codec( rsi ) != NULL ? ctune_RadioStationInfo.get.codec( rsi ) : "" )) );
        err[19] = json_object_object_add( station, "bitrate", json_object_new_uint64( ctune_RadioStationInfo.get.bitrate( rsi ) ) );
        err[20] = json_object_object_add( station, "hls", json_object_new_int( ctune_RadioStationInfo.get.hls( rsi ) ) );
        err[21] = json_object_object_add( station, "lastcheckok", json_object_new_int( ctune_RadioStationInfo.get.lastCheckOK( rsi ) ) );
        err[22] = json_object_object_add( station, "lastchecktime", json_object_new_string( ( rsi->last_check_time != NULL ? rsi->last_check_time : "" )) );
        err[23] = json_object_object_add( station, "lastchecktime_iso8601", json_object_new_string( ( ctune_RadioStationInfo.get.lastCheckTS( rsi ) != NULL ? ctune_RadioStationInfo.get.lastCheckTS( rsi ) : "" )) );
        err[24] = json_object_object_add( station, "lastcheckoktime", json_object_new_string( ( rsi->last_check_ok_time != NULL ? rsi->last_check_ok_time : "" )) );
        err[25] = json_object_object_add( station, "lastcheckoktime_iso8601", json_object_new_string( ( ctune_RadioStationInfo.get.lastCheckOkTS( rsi ) != NULL ? ctune_RadioStationInfo.get.lastCheckOkTS( rsi ) : "" )) );
        err[26] = json_object_object_add( station, "lastlocalchecktime", json_object_new_string( ( rsi->last_local_check_time != NULL ? rsi->last_local_check_time : "" )) );
        err[27] = json_object_object_add( station, "lastlocalchecktime_iso8601", json_object_new_string( ( ctune_RadioStationInfo.get.lastLocalCheckTS( rsi ) != NULL ? ctune_RadioStationInfo.get.lastLocalCheckTS( rsi ) : "" )) );
        err[28] = json_object_object_add( station, "clicktimestamp", json_object_new_string( ( rsi->click_timestamp != NULL ? rsi->click_timestamp : "" )) );
        err[29] = json_object_object_add( station, "clicktimestamp_iso8601", json_object_new_string( ( ctune_RadioStationInfo.get.clickTS( rsi ) != NULL ? ctune_RadioStationInfo.get.clickTS( rsi ) : "" )) );
        err[30] = json_object_object_add( station, "clickcount", json_object_new_uint64( ctune_RadioStationInfo.get.clickCount( rsi ) ) );
        err[31] = json_object_object_add( station, "clicktrend", json_object_new_int64( ctune_RadioStationInfo.get.clickTrend( rsi ) ) );
        err[32] = json_object_object_add( station, "ssl_error", json_object_new_int64( ctune_RadioStationInfo.get.sslErrCode( rsi ) ) );
        err[33] = json_object_object_add( station, "geo_lat", json_object_new_double( ctune_RadioStationInfo.get.geoLatitude( rsi ) ) );
        err[34] = json_object_object_add( station, "geo_long", json_object_new_double( ctune_RadioStationInfo.get.geoLongitude( rsi ) ) );
        err[35] = json_object_object_add( station, "geo_distance", json_object_new_double( ctune_RadioStationInfo.get.geoDistance( rsi ) ) );
        err[36] = json_object_object_add( station, "has_extended_info", json_object_new_int( ctune_RadioStationInfo.get.hasExtendedInfo( rsi ) ) );
        err[37] = json_object_object_add( station, "station_src", json_object_new_int( ctune_RadioStationInfo.get.stationSource( rsi ) ) );
        
        json_object_array_add( array, station );

        for( int err_i = 0; err_i < 37; ++err_i ) {
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
