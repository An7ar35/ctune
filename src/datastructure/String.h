#ifndef CTUNE_DATASTRUCTURE_STRING_H
#define CTUNE_DATASTRUCTURE_STRING_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

/**
 * Wrapper to make growing a string buffer easier by hiding the on-the-fly memory allocation logic
 */
typedef struct String {
    char   * _raw;
    size_t   _length;
} String_t;

extern const struct ctune_String_Namespace {
    /**
     * Initializer
     * @return An initialized String
     */
    struct String (* init)();

    /**
     * Initialises an allocated String pointer
     * @param string Allocated String_t pointer
     */
    void (* initString)( void * string );

    /**
     * Initializes String with content
     * @param self String instance
     * @param str  String content
     * @return Success
     */
    bool (* set)( struct String * self, const char * );
    
    /**
     * Initializes String with a copy of the content of another String
     * @param self String instance
     * @param str  String to copy from
     * @return Success
     */
    bool (* copy)( struct String * self, const struct String * str );

    /**
     * Frees the memory used internally
     * @param self String instance
     */
    void (* free)( void * self );

    /**
     * Returns the length of the string, in terms of bytes.
     * @param self String instance
     * @return Length of the string
     */
    size_t (* length)( const struct String * self );

    /**
     * Gets the number of code points for a UTF-8 string
     * @param str Null terminated UTF-8 formatted string
     * @return Code point length
     */
    size_t (* u8strlen)( const char * str );

    /**
     * Gets the number of UTF-8 characters
     * @param str String instance
     * @return UTF-8 length of the string
     */
    size_t (* u8length)( const struct String * self );

    /**
     * Test if string is empty
     * @param self String instance
     * @return Empty state
     */
    bool (* empty)( const struct String * self );

    /**
     * Appends a c-string to a String
     * @param self String instance
     * @param str  C-style string to append to String
     * @return Success of operation
     */
    bool (* append_back)( struct String * self, const char * str );

    /**
     * Appends a c-string at the front of a String
     * @param self String instance
     * @param str  C-string to append
     * @return Success of operation
     */
    bool (* append_front)( struct String * self, const char * str );

    /**
     * Prints String content
     * @param self String instance
     * @param out  Stream output
     */
    void (* print)( const struct String * self, FILE * out );

    /**
     * Prints String content with a newline appended at the end
     * @param self String instance
     * @param out  Stream output
     */
    void (* println)( const struct String * self, FILE * out );

} String;

#endif //CTUNE_DATASTRUCTURE_STRING_H
