#include "String.h"

#include <stdlib.h>
#include "logger/src/Logger.h"

/**
 * Initializer
 * @return An initialized String
 */
static struct String String_init() {
    return (struct String) {
        ._raw       = NULL,
        ._length    = 0,
    };
}

/**
 * Initialises an allocated String pointer
 * @param string Allocated String_t pointer
 */
void String_initString( void * string ) {
    ( (String_t *) string )->_raw    = NULL;
    ( (String_t *) string )->_length = 0;
}

/**
 * Initializes String with content
 * @param self String instance
 * @param str  String content
 * @return Success
 */
static bool String_set( struct String * self, const char * str ) {
    if( self == NULL || str == NULL )
        return false;

    free( self->_raw );

    size_t size = strlen( str );

    if( ( self->_raw = malloc( size + 1 ) ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[String_set( %p, \"%s\" )] Failed malloc.", self, str );
        return false;
    } else {
        strcpy( self->_raw, str );
        self->_length = size;
    }

    return true;
}

/**
 * Initializes String with a copy of the content of another String
 * @param self String instance
 * @param str  String to copy from
 * @return Success
 */
static bool String_copy( struct String * self, const struct String * str ) {
    if( self == NULL || str == NULL )
        return false;
    else
        return String_set( self, str->_raw );
}

/**
 * Frees the memory used internally
 * @param self String instance
 */
static void String_free( void * self ) {
    if( self != NULL ) {
        free( ( ( String_t * ) self )->_raw );
        ( (String_t *) self )->_raw    = NULL;
        ( (String_t *) self )->_length = 0;
    }
}

/**
 * Returns the length of the string, in terms of bytes.
 * @param self String instance
 * @return Length of the string
 */
static size_t String_length( const struct String * self ) {
    return self->_length;
}

/**
 * Gets a pointer to the first character in the string
 * @param self String instance
 * @return Pointer to first character or NULL
 */
char * String_front( struct String * self ) {
    if( self && String.length( self ) > 0 ) {
        return &self->_raw[0];
    }

    return NULL;
}

/**
 * Gets a pointer to the last character in the string
 * @param self String instance
 * @return Pointer to last character or NULL
 */
char * String_back( struct String * self ) {
    if( self && String.length( self ) > 0 ) {
        return &self->_raw[ ( String.length( self ) - 1 ) ];
    }

    return NULL;
}

/**
 * Gets the number of code points for a UTF-8 string (not validated)
 * @param str Null terminated UTF-8 formatted string
 * @return Code point length
 */
static size_t String_u8strlen( const char * str ) {
    if( str == NULL )
        return 0;

    size_t n = 0;
    const char * p = &str[0];

    while( *p ) {
        n += ( *p & 0xC0 ) != 0x80; //+1 when char matches the first byte of a UTF-8 byte sequence
        ++p;
    }

    return n;
}

/**
 * Gets the number of UTF-8 code points (not validated)
 * @param str String instance
 * @return UTF-8 length of the string
 */
static size_t String_u8length( const struct String * self ) {
    if( self == NULL )
        return 0;

    return String_u8strlen( self->_raw );
}

/**
 * Test if string is empty
 * @param self String instance
 * @return Empty state
 */
static bool String_empty( const struct String * self ) {
    return ( self->_length == 0 );
}

/**
 * Appends a c-string to a String
 * @param self String instance
 * @param str  C-style string to append to String
 * @return Success of operation
 */
static bool String_append_back( struct String * self, const char * str ) {
    if( str == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[String_append_back( %p, \"%s\" )] Aborted: string to append is NULL.", self, str );
        return false;
    }

    if( self->_raw != NULL ) {
        if( ULONG_MAX - strlen( str ) < strlen( self->_raw ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR, "[String_append_back( %p, \"%s\" )] Aborted: String size overflow detected.", self, str );
            return false;
        }

        size_t final_size = strlen( self->_raw ) + strlen( str );

        self->_raw = realloc( self->_raw, final_size + 1 );

        if( self->_raw == NULL ) {
            CTUNE_LOG( CTUNE_LOG_ERROR, "[String_append_back( %p, \"%s\" )] Aborted: failed malloc.", self, str );
            return false; //EARLY RETURN
        }

        strcat( self->_raw, str );
        self->_length = final_size;

    } else { //self._raw == NULL
        String_set( self, str );
    }

    return true;
}

/**
 * Appends a c-string at the front of a String
 * @param self String instance
 * @param str  C-string to append
 * @return Success of operation
 */
static bool String_append_front( struct String * self, const char * str ) {
    if( str == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[String_append_front( %p, \"%s\" )] Aborted: string to append is NULL.", self, str );
        return false;
    }

    if( self->_raw != NULL ) {
        if( ULONG_MAX - strlen( str ) < strlen( self->_raw ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR, "[String_append_front( %p, \"%s\" )] Aborted: String size overflow detected.", self, str );
            return false;
        }

        size_t final_size = strlen( self->_raw ) + strlen( str );
        char * final_str  = NULL;

        if( ( final_str = malloc( final_size + 1 ) ) == NULL ) {
            CTUNE_LOG( CTUNE_LOG_ERROR, "[String_append_front( %p, \"%s\" )] Aborted: failed malloc.", self, str );
            return false;
        }

        strcpy( final_str, str );
        strcat( final_str, self->_raw );

        free( self->_raw );

        self->_raw    = final_str;
        self->_length = final_size;

    } else { //self._raw == NULL
        String_set( self, str );
    }

    return true;
}

/**
 * Prints String content
 * @param self String instance
 * @param out  Stream output
 */
static void String_print( const struct String * self, FILE * out ) {
    fprintf( out, "%s", self->_raw );
}

/**
 * Prints String content with a newline appended at the end
 * @param self String instance
 * @param out  Stream output
 */
static void String_println( const struct String * self, FILE * out ) {
    fprintf( out, "%s\n", self->_raw );
}

/**
 * Constructor
 */
const struct ctune_String_Namespace String = {
    .init           = &String_init,
    .initString     = &String_initString,
    .set            = &String_set,
    .copy           = &String_copy,
    .free           = &String_free,
    .length         = &String_length,
    .front          = &String_front,
    .back           = &String_back,
    .u8strlen       = &String_u8strlen,
    .u8length       = &String_u8length,
    .empty          = &String_empty,
    .append_back    = &String_append_back,
    .append_front   = &String_append_front,
    .print          = &String_print,
    .println        = &String_println,
};