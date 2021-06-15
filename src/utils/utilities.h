#ifndef CTUNE_UTILITIES_H
#define CTUNE_UTILITIES_H

#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <stdint.h>

#include "../logger/Logger.h"
#include "../ctune_err.h"
#include "../datastructure/String.h"
#include "../datastructure/StrList.h"

/**
 * Encodes a string using percent-encoding
 * @param src     Source string to encode
 * @param encoded Destination container String_t object to append the encoded string to
 */
extern void ctune_encodeURI( const char * src, String_t * encoded );

/**
 * Converts a `ulong` to a String
 * @param n   Unsigned number to convert to string
 * @param str String to append the number in
 */
extern void ctune_utos( ulong n, struct String * str );

/**
 * Converts a `long` to a String
 * @param n   Signed long number to convert to string
 * @param str String to append the number in
 */
extern void ctune_ltos( long n, struct String * str );

/**
 * Converts a `double` to a String
 * @param d   Double float number to convert to string
 * @param str String to append the number in
 */
extern void ctune_ftos( double d, struct String * str );

/**
 * Cast a unsigned long into an integer
 * @param u Unsigned long
 * @param i Signed Integer
 * @return Success
 */
extern bool ctune_utoi( unsigned long u, int * i );

/**
 * Converts a string expressing a boolean value as a boolean type
 * @param str Boolean string (case-insensitive and can have surrounding space)
 * @param b   Boolean value container
 * @return Success
 */
extern bool ctune_stob( const char * str, bool * b );

/**
 * Finds the largest value
 * @param lhs Unsigned integer A
 * @param rhs Unsigned integer B
 * @return Largest value
 */
extern unsigned long ctune_max_ul( unsigned long lhs, unsigned long rhs );

/**
 * Converts a string representing a hexadecimal number into a decimal integer
 * @param hex Hexadecimal string
 * @param dec Pointer to integer where to store the result
 * @return Success
 */
extern bool ctune_hex2dec( const char * hex, u_int64_t * dec );

/**
 * Creates a substring
 * @param str Source string
 * @param pos Starting position on the source string
 * @param len Length of the substring
 * @return Allocated substring (or NULL failed)
 */
extern char * ctune_substr( const char * str, size_t pos, size_t len );

/**
 * Trims surrounding whitespace off a string
 * @param src Source string
 * @return Allocated string containing a copy of the source string without the surrounding whitespace (or NULL: empty/whitespace or error)
 */
extern char * ctune_trimspace( const char * src );

/**
 * Splits and packs each values of a comma separated string value-list into a StrList
 * @param css  Comma separated value string (surrounding spaces will be trimmed)
 * @param list StrList_t object to put all the values into
 * @return Number of values parsed
 */
extern size_t ctune_splitcss( const char * css, StrList_t * list );

/**
 * Hashes a string using the FNV-1 hash algorithm (https://en.wikipedia.org/wiki/Fowler_Noll_Vo_hash)
 * @param str String to hash
 * @return Hash (64bit unsigned integer)
 */
extern uint64_t ctune_fnvHash( const char * str );

/**
 * Inverses a comparison results
 * @param i Comparison result
 * @return Inverse of result
 */
extern int ctune_inverseComparison( int i );

/**
 * Validates a UUID string
 * @param uuid UUID (must not have surrounding space)
 * @return Valid state
 */
extern bool ctune_validateUUID( const char * uuid );

/**
 * Generates a local UUID
 * @param uuid Storage for generated UUID
 * @return Success
 */
extern bool ctune_generateUUID( String_t * uuid );

/**
 * Checks the equivalence of strings
 * @param lhs Pointer to first string
 * @param rhs Pointer to second string
 * @return Equivalence inc. if pointers are both NULL
 */
extern bool ctune_streq( const char * lhs, const char * rhs );

/**
 * Converts a string to uppercase (dumb conversion)
 * @param str Pointer to string
 * @return Success
 */
extern bool ctune_strupr( char * str );

/**
 * Converts a string to lowercase (dumb conversion)
 * @param str Pointer to string
 * @return Success
 */
extern bool ctune_strlwr( char * str );

/**
 * Creates a ISO8601 UTC timestamp
 * @param ts String to append timestamp to
 * @return Success
 */
extern bool ctune_timestampISO8601( String_t * ts );

/**
 * Creates a local timestamp
 * @param ts  String to append timestamp to
 * @param fmt Format
 * @return Success
 */
extern bool ctune_timestampLocal( String_t * ts, const char * fmt );

/**
 * Auto chooses between a given string and a fallback based on whether or not the primary string is NULL or not
 * @param str Pointer to string
 * @param alt Alternative string to return if primary is NULL (if fallback is NULL then an empty "" string will be chosen)
 * @return Primary string, fallback string or empty string if both are NULL
 */
extern const char * ctune_fallbackStr( const char * str, const char * alt );

#endif //CTUNE_UTILITIES_H
