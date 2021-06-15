#include "NewRadioStation.h"

#include "../logger/Logger.h"

/**
 * Initialise fields in the struct
 * @param stats NewRadioStation DTO pointer
 */
static void ctune_NewRadioStation_init( struct ctune_NewRadioStation * nrs ) {
    nrs->send.name        = NULL;
    nrs->send.url         = NULL;
    nrs->send.homepage    = NULL;
    nrs->send.favicon     = NULL;
    nrs->send.country     = NULL;
    nrs->send.countrycode = NULL;
    nrs->send.state       = NULL;
    nrs->send.language    = NULL;
    nrs->send.tags        = StrList.init();
    nrs->received.ok      = NULL;
    nrs->received.message = NULL;
    nrs->received.uuid    = NULL;
}

/**
 * Frees the content of a NewRadioStation DTO
 * @param cat_item NewRadioStation DTO
 */
static void ctune_NewRadioStation_freeContent( struct ctune_NewRadioStation * nrs ) {
    if( nrs == NULL )
        return; //EARLY RETURN

    if( nrs->send.name ) {
        free( nrs->send.name );
        nrs->send.name = NULL;
    }

    if( nrs->send.url ) {
        free( nrs->send.url );
        nrs->send.url = NULL;
    }

    if( nrs->send.homepage ) {
        free( nrs->send.homepage );
        nrs->send.homepage = NULL;
    }

    if( nrs->send.favicon ) {
        free( nrs->send.favicon );
        nrs->send.favicon = NULL;
    }

    if( nrs->send.country ) {
        free( nrs->send.country );
        nrs->send.country = NULL;
    }

    if( nrs->send.countrycode ) {
        free( nrs->send.countrycode );
        nrs->send.countrycode = NULL;
    }

    if( nrs->send.state ) {
        free( nrs->send.state );
        nrs->send.state = NULL;
    }

    if( nrs->send.language ) {
        free( nrs->send.language );
        nrs->send.language = NULL;
    }

    StrList.free_strlist( &nrs->send.tags );

    if( nrs->received.ok ) {
        free( nrs->received.ok );
        nrs->received.ok = NULL;
    }

    if( nrs->received.message ) {
        free( nrs->received.message );
        nrs->received.message = NULL;
    }

    if( nrs->received.uuid ) {
        free( nrs->received.uuid );
        nrs->received.uuid = NULL;
    }
}

/**
 * Prints a NewRadioStation.send struct
 * @param out Output
 * @param nrs NewRadioStation instance
 */
static void ctune_NewRadioStation_printSend( FILE * out, const struct ctune_NewRadioStation * nrs ) {
    fprintf( out, "Name ........: %s\n", nrs->send.name );
    fprintf( out, "URL .........: %s\n", nrs->send.url );
    fprintf( out, "Homepage ....: %s\n", nrs->send.homepage );
    fprintf( out, "FavIcon .....: %s\n", nrs->send.favicon );
    fprintf( out, "Country .....: %s\n", nrs->send.country );
    fprintf( out, "Country Code : %s\n", nrs->send.countrycode );
    fprintf( out, "State .......: %s\n", nrs->send.state );
    fprintf( out, "Language ....: %s\n", nrs->send.language );
    fprintf( out, "Tags ........: " );

    struct StrListNode * curr = nrs->send.tags._front;

    while( curr ) {
        fprintf( out, "%s\n", curr->data );
        curr = curr->next;
        if( curr )
            fprintf( out, ", " );
    }
}

/**
 * Prints a NewRadioStation.received struct
 * @param out Output
 * @param nrs NewRadioStation instance
 */
static void ctune_NewRadioStation_printRcv( FILE * out, const struct ctune_NewRadioStation * nrs ) {
    fprintf( out, "OK ..........: %s\n", nrs->received.ok );
    fprintf( out, "Message .....: %s\n", nrs->received.message );
    fprintf( out, "UUID ........: %s",   nrs->received.uuid );
 }

/**
 * Converts a the fields in the NewRadioStation.send struct into a fully formed query parameter string ready to be appended to a path
 * @param nrs NewRadioStation struct
 * @param str String to append the formed filter query to (if any)
 * @return Number of filter parsed
 */
static void ctune_NewRadioStation_parameteriseSendFields( const struct ctune_NewRadioStation * nrs, struct String * str ) {
    CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_NewRadioStation_parameteriseSendFields( %p, %p )]", nrs, str );
    //TODO Although not in use atm might be a good idea when/if this functionality is implemented in the UI to URI encode some/all strings
    struct String query = String.init();

    if( nrs->send.name ) {
        String.append_back( &query, "name=" );
        String.append_back( &query, nrs->send.name );
    }

    if( nrs->send.url ) {
        if( !String.empty( &query ) )
            String.append_back( &query, "&" );

        String.append_back( &query, "url=" );
        String.append_back( &query, nrs->send.url );
    }

    if( nrs->send.homepage ) {
        if( !String.empty( &query ) )
            String.append_back( &query, "&" );

        String.append_back( &query, "homepage=" );
        String.append_back( &query, nrs->send.homepage );
    }

    if( nrs->send.favicon ) {
        if( !String.empty( &query ) )
            String.append_back( &query, "&" );

        String.append_back( &query, "favicon=" );
        String.append_back( &query, nrs->send.favicon );
    }

    if( nrs->send.country ) {
        if( !String.empty( &query ) )
            String.append_back( &query, "&" );

        String.append_back( &query, "country=" );
        String.append_back( &query, nrs->send.country );
    }

    if( nrs->send.countrycode ) {
        if( !String.empty( &query ) )
            String.append_back( &query, "&" );

        String.append_back( &query, "countrycode=" );
        String.append_back( &query, nrs->send.countrycode );
    }

    if( nrs->send.state ) {
        if( !String.empty( &query ) )
            String.append_back( &query, "&" );

        String.append_back( &query, "state=" );
        String.append_back( &query, nrs->send.state );
    }

    if( nrs->send.language ) {
        if( !String.empty( &query ) )
            String.append_back( &query, "&" );

        String.append_back( &query, "language=" );
        String.append_back( &query, nrs->send.language );
    }

    if( !StrList.empty( &nrs->send.tags ) ) {
        if( !String.empty( &query ) )
            String.append_back( &query, "&" );

        struct StrListNode * curr = nrs->send.tags._front;

        String.append_back( &query, "tags=" );

        while( curr ) {
            String.append_back( &query, curr->data );

            curr = curr->next;

            if( curr )
                String.append_back( &query, "," );
        }
    }

    if( !String.empty( &query ) ) {
        String.append_back( str, "?" );
        String.append_back( str, query._raw );
    }

    String.free( &query );
}

/**
 * Validates `.send` fields
 * @param nrs NewRadioStation struct
 * @return Valid state
 */
static bool ctune_NewRadioStation_validateSendFields( const struct ctune_NewRadioStation * nrs ) { //TODO ?
    //mandatory - 400 chars max
    if( nrs->send.name == NULL || strlen( nrs->send.name ) == 0 || strlen( nrs->send.name ) > 400 )
        return false;

    //mandatory
    if( nrs->send.url == NULL || strlen( nrs->send.url ) < 1 ) //TODO check URL
        return false;

    if( nrs->send.homepage ) {} //TODO check for valid URL
    if( nrs->send.favicon ) {} //TODO check for valid URL
    if( nrs->send.country ) {} //TODO check?
    if( nrs->send.countrycode ) {} //TODO check against code standard
    if( nrs->send.state ) {} //TODO check?
    if( nrs->send.language ) {} //TODO check?
    if( !StrList.empty( &nrs->send.tags ) ) {} //TODO check?

    return true;
}

/**
 * Namespace constructor
 */
const struct ctune_NewRadioStation_Namespace ctune_NewRadioStation = {
    .init                   = &ctune_NewRadioStation_init,
    .freeContent            = &ctune_NewRadioStation_freeContent,
    .printSend              = &ctune_NewRadioStation_printSend,
    .printRcv               = &ctune_NewRadioStation_printRcv,
    .parameteriseSendFields = &ctune_NewRadioStation_parameteriseSendFields,
    .validateSendFields     = &ctune_NewRadioStation_validateSendFields,
};