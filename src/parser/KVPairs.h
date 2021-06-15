#ifndef CTUNE_PARSER_KVPAIRS_H
#define CTUNE_PARSER_KVPAIRS_H

#include <stdbool.h>

#include "../datastructure/String.h"
#include "../datastructure/HashMap.h"

extern const struct ctune_Parser_KVPairs_Namespace {
    /**
     * Parses a key-value pair string into separate String_t objects with any surrounding space trimmed out
     * @param str       Key-Value pair string
     * @param delimiter Key-Value pair delimiter (e.g.: '=')
     * @param key       Container for the key (assumed to be initialised)
     * @param value     Container for the value (assumed to be initialised)
     * @return Success
     */
    bool (* parse)( const char * str, const char delimiter, String_t * key, String_t * value );

    /**
     * Validate/Parse a UUID
     * @param src    Source value to parse
     * @param target Target String_t to parse the validated value into
     * @return Success
     */
    bool (* validateUUID)( const String_t * src, String_t * target );

    /**
     * Validate/Parse an integer
     * @param src    Source value to parse
     * @param target Target int value to parse the validated value into
     * @return Success
     */
    bool (* validateInteger)( const String_t * src, int * target );

    /**
     * Validate/Parse a boolean value
     * @param src    Source value to parse
     * @param target Target bool type to parse the validated value into
     * @return Success
     */
    bool (* validateBoolean)( const String_t * src, bool * target );

    /**
     * Validate/Parse a colour
     * @param src    Source value to parse
     * @param target Target short value to parse the validated value into
     * @return Success
     */
    bool (* validateColour)( const String_t * src, short * target );

    /**
     * Validate/Parse a colour pair ("{$FOREGROUND,$BACKGROUND}")
     * @param src       Source value string to parse
     * @param target_fg Target short value to parse the validated foreground value into
     * @param target_bg Target short value to parse the validated background value into
     * @return Success
     */
    bool (* validateColourPair)( const String_t * src, short * target_fg, short * target_bg );

    /**
     * Validates a string against a list of acceptable inputs
     * @param src            Source value string to check
     * @param list           String array containing acceptable values
     * @param list_ln        String array length
     * @param case_sensitive Flag for case sensitivity
     * @return Index of list matching value
     */
    int (* validateString)( const String_t * src, const char * list[], int list_ln, bool case_sensitive );

} ctune_Parser_KVPairs;

#endif //CTUNE_PARSER_KVPAIRS_H
