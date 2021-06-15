#include "utilities.h"

#include <uuid/uuid.h>
#include <time.h>

/**
 * Encodes a string using percent-encoding
 * @param src     Source string to encode
 * @param encoded Destination container String_t object to append the encoded string to
 */
void ctune_encodeURI( const char * src, String_t * encoded ) {
    size_t src_ln  = strlen( src );
    size_t buff_ln = src_ln * 4;

    char buffer[ buff_ln ];
    char * out = &buffer[0];

    for( size_t i = 0; i < src_ln; ++i ) {
        char in = src[i];

        if( ( in >= 32 && in <= 33 ) || // ' ', '!'
            ( in >= 35 && in <= 44 ) || // '#', '$', '%', '&', ''', '(', ')', '*', '+', ','
            ( in == 47 )             || // '/'
            ( in >= 58 && in <= 64 ) || // ':', ';', '<', '=', '>', '?', '@'
            ( in >= 91 && in <= 93 ) )  // '\', '[', ']'
        {
            char q = ( in / 16 );
            char r = ( in % 16 );

            *(out++) = '%';
            *(out++) = q + 48;                                     //+48 (0) for char on quotient;
            *(out++) = ( r < 10 ) ? ( r + 48 ) : ( r % 10 ) + 65; //+48 (0) for 0-9 and +65 (A) for 10-15

        } else { //append char to buffer "as-is"
            *(out++) = in;
        }
    }

    *out = '\0';

    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_encodeURI( \"%s\", %p )] URI encoded => \"%s\"", src, encoded, buffer );

    String.append_back( encoded, buffer );
}

/**
 * Converts a `ulong` to a String
 * @param n   Unsigned number to convert to string
 * @param str String to append the number in
 */
void ctune_utos( ulong n, struct String * str ) {
    if( n == 0 ) {
        String.append_back( str, "0" );
        return;
    }

    struct String tmp     = String.init();
    char          buff[2] = "\0";

    while( n > 0 ) {
        buff[0]  = '0';
        buff[0] += n % 10;
        String.append_front( &tmp, &buff[0] );
        n /= 10;
    }

    String.append_back( str, tmp._raw );
    String.free( &tmp );
}

/**
 * Converts a `long` to a String
 * @param n   Signed long number to convert to string
 * @param str String to append the number in
 */
void ctune_ltos( long n, struct String * str ) {
    if( n == 0 ) {
        String.append_back( str, "0" );
        return;
    }

    char buff[256] = "\0";

    sprintf( buff, "%ld", n );

    String.append_back( str, buff );
}

/**
 * Converts a `double` to a String
 * @param d   Double float number to convert to string
 * @param str String to append the number in
 */
void ctune_ftos( double d, struct String * str ) {
    char buff[256] = "\0";
    sprintf( buff, "%f", d );

    String.append_back( str, buff );
}

/**
 * Cast a unsigned long into an integer
 * @param u Unsigned long
 * @param i Signed Integer
 * @return Success
 */
bool ctune_utoi( unsigned long u, int * i ) {
    if( u <= INT_MAX ) {
        *i = ( int ) u;
        return true;
    }

    return false;
}

/**
 * Converts a string expressing a boolean value as a boolean type
 * @param str Boolean string (case-insensitive and can have surrounding space)
 * @param b   Boolean value container
 * @return Success
 */
bool ctune_stob( const char * str, bool * b ) {
    if( str == NULL || b == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_stob( %p, %p )] arg(s) = NULL." );
        return false; //EARLY RETURN
    }

    char * copy = strdup( str );

    if( copy == NULL )
        return false; //EARLY RETURN

    char * p = &copy[0];

    for( ; *p; ++p )
        *p = tolower( *p );

    bool error_state = false;

    if( strcmp( "true", copy ) == 0 || strcmp( "1", copy ) == 0 ) {
        *b = true;
    } else if( strcmp( "false", copy ) == 0 || strcmp( "0", copy ) == 0 ) {
        *b = false;
    } else {
        error_state = true;
    }

    free( copy );
    return !( error_state );
}

/**
 * Finds the largest value
 * @param lhs Unsigned integer A
 * @param rhs Unsigned integer B
 * @return Largest value
 */
unsigned long ctune_max_ul( unsigned long lhs, unsigned long rhs ) {
    return ( lhs >= rhs ? lhs : rhs );
}

/**
 * Converts a string representing a hexadecimal number into a decimal integer
 * @param hex Hexadecimal string
 * @param dec Pointer to integer where to store the result
 * @return Success
 */
bool ctune_hex2dec( const char * hex, u_int64_t * dec ) {
    if( dec == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_hex2dec( \"%s\", %p )] Target uint64 pointer is NULL.", hex, dec );
        return false; //EARLY RETURN
    }

    size_t length = strlen( hex );

    for( size_t i = 0; i < length; ++i ) {
        u_int64_t val = 0;

        if( hex[ i ] >= '0' && hex[ i ] <= '9' ) {
            val = hex[ i ] - 48;
        } else if( hex[ i ] >= 'a' && hex[ i ] <= 'f' ) {
            val = hex[ i ] - 97 + 10;
        } else if( hex[ i ] >= 'A' && hex[ i ] <= 'F' ) {
            val = hex[ i ] - 65 + 10;
        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_hex2dec( \"%s\", %p )] Invalid HEX char in string ('%c').", hex, dec, hex[ i ] );
            return false; //EARLY RETURN
        }

        u_int64_t res = val * pow( 16, length );

        if( ( UINT64_MAX - *dec ) < res ) {
            CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_hex2dec( \"%s\", %p )] Integer overflow detected.", hex, dec );
            return false; //EARLY RETURN
        }

        *dec += res;
        --length;
    }

    return true;
}

/**
 * Creates a substring
 * @param str Source string
 * @param pos Starting position on the source string
 * @param len Length of the substring
 * @return Allocated substring (or NULL failed)
 */
char * ctune_substr( const char * str, size_t pos, size_t len ) {
    size_t src_ln = strlen( str );

    if( pos >= src_ln )
        return NULL; //EARLY RETURN

    if( ( pos + len ) >= src_ln )
        len = src_ln - pos; //in this case just substr 'till end of str

    char * substr = malloc( sizeof( char ) * len + 1 );

    if( substr != NULL ) {
        for( size_t i = 0; i < len; ++i )
            substr[ i ] = str[ ( pos + i ) ];

        substr[ len ] = '\0';
    }

    return substr;
}

/**
 * Trims surrounding whitespace off a string
 * @param src Source string
 * @return Allocated string containing a copy of the source string without the surrounding whitespace (or NULL: empty/whitespace or error)
 */
char * ctune_trimspace( const char * src ) {
    if( src == NULL )
        return NULL; //EARLY RETURN

    size_t length = strlen( src );
    size_t from   = 0;
    size_t to     = length;

    while( from < length && isspace( src[from] ) ) {
        ++from;
    }

    while( to > 0 && isspace( src[ (to - 1) ] ) ) {
        --to;
    }

    if( length == 0 || to < from ) //empty string or all whitespace
        return NULL; //EARLY RETURN

    return ctune_substr( src, from, ( to - from ) );
}

/**
 * Splits and packs each values of a comma separated string value-list into a StrList
 * @param css  Comma separated value string (surrounding spaces will be trimmed)
 * @param list StrList_t object to put all the values into
 * @return Number of values parsed
 */
size_t ctune_splitcss( const char * css, StrList_t * list ) {
    const size_t csv_ln = strlen( css );
    size_t       i      = 0;
    size_t       from   = 0;
    size_t       to     = 0;

    while( i < csv_ln ) {
        while( to < csv_ln && css[i] != ',' ) {
            ++to;
            ++i;
        }

        char * tmp = ctune_substr( css, from, ( to - from ) );
        char * val = ctune_trimspace( tmp );

        if( val != NULL ) {
            CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_splitcss( \"%s\", %p )] Adding \"%s\" to StrList_t", css, list, val );
            StrList.insert_back( list, val );
        }

        if( tmp != NULL )
            free( tmp );
        if( val != NULL )
            free( val );


        ++i;
        from = to = i;
    }

    return StrList.size( list );
}

/**
 * Hashes a string using the FNV-1 hash algorithm (https://en.wikipedia.org/wiki/Fowler_Noll_Vo_hash)
 * @param str String to hash
 * @return Hash (64bit unsigned integer)
 */
uint64_t ctune_fnvHash( const char * str ) {
    static const uint64_t fnv_offset = 14695981039346656037UL;
    static const uint64_t fnv_prime  =        1099511628211UL;

    size_t length = strlen( str );
    uint64_t hash = fnv_offset;

    for( size_t i = 0; i < length; ++i ) {
        uint64_t byte = (uint64_t) str[i];
        hash *= fnv_prime;
        hash ^= byte;
    }

    return hash;
}

/**
 * Inverses a comparison results
 * @param i Comparison result
 * @return Inverse of result
 */
int ctune_inverseComparison( int i ) {
    return ( i > 0 ? -1 : ( i < 0 ? +1 : 0 ) );
}

/**
 * Validates a UUID string
 * @param uuid UUID (must not have surrounding space)
 * @return Valid state
 */
bool ctune_validateUUID( const char * uuid ) {
    //beg: |0       |9   |14  |19  |24
    //     xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx  (length = 36)
    //end:       7|  12|  17|  22|          35|

    if( strlen( uuid ) != 36 )
        return false; //EARLY RETURN

    for( size_t i = 0; i < 36; ++i ) {
        if( i == 8 || i == 13 || i == 18 || i == 23 ) {
            if( !( uuid[i] == '-' ) )
                return false; //EARLY RETURN

        } else {
            bool is_hex_char = ( uuid[i] >= 48 && uuid[i] <=  57 )  //0-9
                            || ( uuid[i] >= 65 && uuid[i] <=  70 )  //A-F
                            || ( uuid[i] >= 97 && uuid[i] <= 102 ); //a-f

            if( !is_hex_char )
                return false; //EARLY RETURN;
        }
    }

    return true;
}

/**
 * Generates a local UUID (uses the linux UUID library)
 * @param uuid Storage for generated UUID
 * @return Success
 */
bool ctune_generateUUID( String_t * uuid ) {
    uuid_t bin_uuid;
    char * str_uuid    = malloc( 37 );
    bool   error_state = false;

    uuid_generate_random( bin_uuid );
    uuid_unparse_lower( bin_uuid, str_uuid );

    if( str_uuid == NULL || !String.set( uuid, str_uuid ) )
        error_state = true;

    free( str_uuid );

    return !( error_state );
}

/**
 * Checks the equivalence of strings
 * @param lhs Pointer to first string
 * @param rhs Pointer to second string
 * @return Equivalence inc. if pointers are both NULL
 */
bool ctune_streq( const char * lhs, const char * rhs ) {
    if( lhs != NULL && rhs != NULL )
        return ( strcmp( lhs, rhs ) == 0 );

    if( lhs == NULL && rhs == NULL )
        return true;

    return false;
}

/**
 * Converts a string to uppercase (dumb conversion)
 * @param str Pointer to string
 * @return Success
 */
bool ctune_strupr( char * str ) {
    if( str == NULL )
        return false; //EARLY RETURN

    char * p = &str[0];

    for( ; *p; ++p )
        *p = toupper( *p );

    return true;
}

/**
 * Converts a string to lowercase (dumb conversion)
 * @param str Pointer to string
 * @return Success
 */
bool ctune_strlwr( char * str ) {
    if( str == NULL )
        return false; //EARLY RETURN

    char * p = &str[0];

    for( ; *p; ++p )
        *p = tolower( *p );

    return true;
}

/**
 * Creates a ISO8601 UTC timestamp
 * @param ts String to append timestamp to
 * @return Success
 */
bool ctune_timestampISO8601( String_t * ts ) {
    static const size_t size = sizeof( char ) * 21;

    time_t now;
    char   buffer[size];

    time(&now);
    strftime( buffer, size, "%FT%TZ", gmtime( &now ) );
    buffer[20] = '\0';

    return String.append_back( ts, buffer );
}

/**
 * Creates a local timestamp
 * @param ts  String to append timestamp to
 * @param fmt Format
 * @return Success
 */
bool ctune_timestampLocal( String_t * ts, const char * fmt ) {
    time_t now;
    char   buffer[256];

    time(&now);
    if( strftime( buffer, 256, fmt, localtime( &now ) ) <= 0 )
        return false;

    return String.append_back( ts, buffer );
}

/**
 * Auto chooses between a given string and a fallback based on whether or not the primary string is NULL or not
 * @param str Pointer to string
 * @param alt Alternative string to return if primary is NULL (if fallback is NULL then an empty "" string will be chosen)
 * @return Primary string, fallback string or empty string if both are NULL
 */
const char * ctune_fallbackStr( const char * str, const char * alt ) {
    if( str == NULL )
        return ( alt != NULL ? alt : "" );
    return str;
}