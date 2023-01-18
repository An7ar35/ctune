#include "KVPairs.h"

#include <stdlib.h>
#include <errno.h>

#include "../logger/Logger.h"
#include "../datastructure/StrList.h"
#include "../utils/utilities.h"
#include "../dto/ColourTheme.h"

/**
 * Parses a key-value pair string into separate String_t objects with any surrounding space trimmed out
 * @param str       Key-Value pair string
 * @param delimiter Key-Value pair delimiter (e.g.: '=')
 * @param key       Container for the key (assumed to be initialised)
 * @param value     Container for the value (assumed to be initialised)
 * @return Success
 */
static bool ctune_Parser_KVPairs_parseLine( const char * str, const char delimiter, String_t * key, String_t * value ) {
    size_t length = 0;

    if( str == NULL || key == NULL || value == NULL || ( length = strlen( str ) ) == 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Parser_KVPairs_parseLine( %p, \'%c\', %p, %p )] NULL arg(s) or empty k/v string passed.",
                   str, delimiter, key, value
        );

        return false; //EARLY RETURN
    }

    size_t i           = 0;
    bool   error_state = false;

    for( ; ( i < length && str[i] != delimiter ); ++i );

    size_t key_ln      = i;
    char * key_raw     = ctune_substr( str, 0, key_ln );
    char * key_trimmed = ctune_trimspace( key_raw );

    size_t val_ln      = ( i < length ? ( length - ( i + 1 ) ) : 0 );
    char * val_raw     = ctune_substr( str, ( i + 1), val_ln );
    char * val_trimmed = ctune_trimspace( val_raw );

    if( key_trimmed == NULL || !String.set( key, key_trimmed ) )
        error_state =  true;

    if( val_trimmed == NULL || !String.set( value, val_trimmed ) )
        error_state = true;

    if( error_state ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Parser_KVPairs_parseLine( %p, \'%c\', %p, %p )] Key value pair could not be parsed (K=\"%s\", V=\"%s\").",
                   str, delimiter, key, value, ( key_trimmed != NULL ? key_trimmed : "(NULL)" ), ( val_trimmed != NULL ? val_trimmed : "(NULL)" )
        );
    }

    if( key_raw != NULL )
        free( key_raw );
    if( key_trimmed != NULL )
        free( key_trimmed );
    if( val_raw != NULL )
        free( val_raw );
    if( val_trimmed != NULL )
        free( val_trimmed );

    return !( error_state );
}

/**
 * Validate/Parse a UUID
 * @param src    Source value to parse
 * @param target Target String_t to parse the validated value into
 * @return Success
 */
static bool ctune_Parser_KVPairs_parseUUID( const String_t * src, String_t * target ) {
    bool error_state = false;

    if( ctune_validateUUID( src->_raw ) ) {
        if( !String.copy( target, src ) ) {
            error_state = true;

            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_Parser_KVPairs_parseUUID( %p, %p )] Failed to copy UUID: \"%s\".",
                       src, target, src->_raw
            );
        }

    } else {
        error_state = true;

        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Parser_KVPairs_parseUUID( %p, %p )] Invalid UUID: \"%s\".",
                   src, target, src->_raw
        );
    }

    return !( error_state );
}

/**
 * Validate/Parse an integer
 * @param src    Source value to parse
 * @param target Target int value to parse the validated value into
 * @return Success
 */
static bool ctune_Parser_KVPairs_parseInteger( const String_t * src, int * target ) {
    bool error_state = false;

    if( src == NULL || target == NULL )
        return false; //EARLY RETURN

    for( size_t i = 0; i < strlen( src->_raw ); ++i ) {
        if( !isalnum( src->_raw[i] ) ) {
            error_state = true;
            break;
        }
    }

    if( !error_state ) {
        char * end_ptr;
        long   result = strtol( src->_raw, &end_ptr, 10 );

        if( result == 0 && errno == EINVAL ) {
            error_state = true;

            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_Parser_KVPairs_parseInteger( %p, %p )] Failed to convert to long: \"%s\".",
                       src, target, src->_raw
            );

        } else if( ( result == LONG_MIN || result == LONG_MAX ) && errno == ERANGE ) {
            error_state = true;

            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_Parser_KVPairs_parseInteger( %p, %p )] Failed to convert to long (too big): \"%s\".",
                       src, target, src->_raw
            );

        } else {
            if( result < INT_MIN || result > INT_MAX ) {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_Parser_KVPairs_parseInteger( %p, %p )] Failed to convert long to integer: \"%s\".",
                           src, target, src->_raw
                );

                error_state = true;
            }

            *target = (int) result;
        }
    }

    return !( error_state );
}

/**
 * Validate/Parse a boolean value
 * @param src    Source value to parse
 * @param target Target bool type to parse the validated value into
 * @return Success
 */
static bool ctune_Parser_KVPairs_parseBoolean( const String_t * src, bool * target ) {
    if( !ctune_stob( src->_raw, target ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Parser_KVPairs_parseBoolean( %p, %p )] Failed to copy boolean value: \"%s\".",
                   src, target, src->_raw
        );

        return false;
    }

    return true;
}

/**
 * Validate/Parse a colour
 * @param src    Source value to parse
 * @param target Target short value to parse the validated value into
 * @return Success
 */
static bool ctune_Parser_KVPairs_validateColour( const String_t * src, short * target ) {
    if( src == NULL || target == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Parser_KVPairs_validateColour( %p, %p )] arg(s) = NULL.",
                   src, target
        );

        return false; //EARLY RETURN
    }

    char * copy = strdup( src->_raw );

    if( copy == NULL )
        return false; //EARLY RETURN

    char * p = &copy[0];

    for( ; *p; ++p )
        *p = toupper( *p );

    bool error_state = false;

    if( strcmp( copy, "BLACK" ) == 0 ) {
        (*target) = ctune_ColourTheme.colour.BLACK;
    } else if( strcmp( copy, "RED" ) == 0 ) {
        (*target) = ctune_ColourTheme.colour.RED;
    } else if( strcmp( copy, "GREEN" ) == 0 ) {
        (*target) = ctune_ColourTheme.colour.GREEN;
    } else if( strcmp( copy, "YELLOW" ) == 0 ) {
        (*target) = ctune_ColourTheme.colour.YELLOW;
    } else if( strcmp( copy, "BLUE" ) == 0 ) {
        (*target) = ctune_ColourTheme.colour.BLUE;
    } else if( strcmp( copy, "MAGENTA" ) == 0 ) {
        (*target) = ctune_ColourTheme.colour.MAGENTA;
    } else if( strcmp( copy, "CYAN" ) == 0 ) {
        (*target) = ctune_ColourTheme.colour.CYAN;
    } else if( strcmp( copy, "WHITE" ) == 0 ) {
        (*target) = ctune_ColourTheme.colour.WHITE;
    } else {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Parser_KVPairs_validateColour( %p, %p )] Colour '%s' not recognised.",
                   src, target, src->_raw
        );

        error_state = true;
    }

    free( copy );
    return !( error_state );
}

/**
 * Validate/Parse a colour pair ("{$FOREGROUND,$BACKGROUND}")
 * @param src       Source value string to parse
 * @param target_fg Target short value to parse the validated foreground value into
 * @param target_bg Target short value to parse the validated background value into
 * @return Success
 */
static bool ctune_Parser_KVPairs_validateColourPair( const String_t * src, short * target_fg, short * target_bg ) {
    if( src == NULL || target_fg == NULL || target_bg == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Parser_KVPairs_validateColourPair( %p, %p, %p )] arg(s) = NULL.",
                   src, target_fg, target_bg
        );

        return false; //EARLY RETURN
    }

    if( src->_length == 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Parser_KVPairs_validateColourPair( %p, %p, %p )] Source string is empty.",
                   src, target_fg, target_bg
        );

        return false; //EARLY RETURN
    }

    bool      error_state  = false;
    StrList_t list         = StrList.init();
    size_t    begin        = 0;
    size_t    end          = ( src->_length > 0 ? src->_length - 1 : 0 );
    char    * copy         = NULL;
    size_t    colour_count = 0;

    //find indices past any first sets of brackets
    while( !isalpha( src->_raw[ begin ] ) && (begin++) < src->_length );
    while( !isalpha( src->_raw[ end   ] ) && (end--)   > 0                    );

    if( end < begin ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Parser_KVPairs_validateColourPair( %p, %p, %p )] "
                   "Failed to find colour substring indices in \"%s\".",
                   src, target_fg, target_bg, src->_raw
        );

        error_state = true;
        goto end;
    }

    copy         = ctune_substr( src->_raw, begin, ( end - begin ) + 1 );
    colour_count = ctune_splitcss( copy, &list );

    //parse colour(s)
    if( colour_count == 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Parser_KVPairs_validateColourPair( %p, %p, %p )] "
                   "No string(s) found.",
                   src, target_fg, target_bg
        );

        error_state = true;
        goto end;
    }

    if( colour_count > 2 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Parser_KVPairs_validateColourPair( %p, %p, %p )] Too many sub-strings found (%lu) in \"%s\"",
                   src, target_fg, target_bg, colour_count, src->_raw
        );

        error_state = true;
        goto end;
    }

    //colour_count == 1 || colour_count == 2
    for( size_t i = 0; i < 2; ++i ) {
        short * val_ptr = ( i == 0 ? target_fg : target_bg );

        const struct StrListNode * node   = StrList.at( &list, i );
        String_t                   colour = String.init();

        if( !String.set( &colour, node->data ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_Parser_KVPairs_validateColourPair( %p, %p, %p )] Failed to set StrList (%lu) node data as String_t.",
                       src, target_fg, target_bg, i
            );

            error_state = true;
        }

        if( !ctune_Parser_KVPairs_validateColour( &colour, val_ptr ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_Parser_KVPairs_validateColourPair( %p, %p, %p )] "
                       "Failed to parse string (%lu) as a colour: \"%s\"",
                       src, target_fg, target_bg, i, colour._raw
            );

            error_state = true;
        }

        String.free( &colour );
    }

    if( colour_count == 1 ) {
        CTUNE_LOG( CTUNE_LOG_WARNING,
                   "[ctune_Parser_KVPairs_validateColourPair( %p, %p, %p )] "
                   "Only 1 colour found - parsing as both FG and BG.",
                   src, target_fg, target_bg
        );

        (*target_bg) = (*target_fg);
    }

    end:
        if( copy != NULL )
            free( copy );
        StrList.free_strlist( &list );
        return !( error_state );
}

/**
 * Validates a string against a list of acceptable inputs
 * @param src            Source value string to check
 * @param list           String array containing acceptable values (in lower case if case sensitivity in OFF)
 * @param list_ln        String array length
 * @param case_sensitive Flag for case sensitivity
 * @return Index of list matching value (-1 on error/failure)
 */
static int ctune_Parser_KVPairs_validateString( const String_t * src, const char * list[], int list_ln, bool case_sensitive ) {
    if( src == NULL || list == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Parser_KVPairs_validateString( %p, %p, %d, '%s' )] NULL arg(s)",
                   src, list, list_ln, ( case_sensitive ? "case sensitive" : "case insensitive" )
        );

        return -1;
    }

    if( String.empty( src ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Parser_KVPairs_validateString( %p, %p, %d, '%s' )] Source string is empty.",
                   src, list, list_ln, ( case_sensitive ? "case sensitive" : "case insensitive" )
        );

        return -1;
    }

    int  list_index  = 0;
    bool match_found = false;

    while( !match_found && list_index < list_ln ) {
        if( case_sensitive ) {
            if( strcmp( src->_raw, list[ list_index ] ) == 0 )
                match_found = true;

        } else {
            char *val = strdup( src->_raw );
            ctune_strlwr( val );

            if( strcmp( src->_raw, list[ list_index ] ) == 0 )
                match_found = true;

            free( val );
        }

        if( !match_found )
            ++list_index;
    }

    return ( match_found ? list_index : -1 );
}

/**
 * Namespace constructor
 */
const struct ctune_Parser_KVPairs_Namespace ctune_Parser_KVPairs = {
    .parse              = &ctune_Parser_KVPairs_parseLine,
    .validateUUID       = &ctune_Parser_KVPairs_parseUUID,
    .validateInteger    = &ctune_Parser_KVPairs_parseInteger,
    .validateBoolean    = &ctune_Parser_KVPairs_parseBoolean,
    .validateColour     = &ctune_Parser_KVPairs_validateColour,
    .validateColourPair = &ctune_Parser_KVPairs_validateColourPair,
    .validateString     = &ctune_Parser_KVPairs_validateString,

};
