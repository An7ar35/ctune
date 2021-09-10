#ifndef CTUNE_DTO_RADIOSTATIONINFO_H
#define CTUNE_DTO_RADIOSTATIONINFO_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "../enum/StationSrc.h"
#include "../datastructure/String.h"
#include "../utils/utilities.h"

typedef int (* Comparator)( const void *, const void * );

/**
 * RadioStationInfo DTO
 */
typedef struct ctune_RadioStationInfo {
    char * change_uuid;
    char * station_uuid;
    char * name;
    char * url;
    char * url_resolved;
    char * homepage;
    char * favicon_url;
    char * tags;
    char * country;

    struct {
        char * iso3166_1; //https://en.wikipedia.org/wiki/ISO_3166-1_alpha-2
        char * iso3166_2; //https://en.wikipedia.org/wiki/ISO_3166-2
    } country_code;

    char * state;
    char * language;
    char * language_codes; //https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes
    ulong  votes;
    char * last_change_time; //FIXME depreciated (replaced by iso8601 timestamps)
    char * codec;
    ulong  bitrate;
    bool   hls;
    bool   last_check_ok;
    char * last_check_time; //FIXME depreciated (replaced by iso8601 timestamps)
    char * last_check_ok_time; //FIXME depreciated (replaced by iso8601 timestamps)
    char * last_local_check_time; //FIXME depreciated (replaced by iso8601 timestamps)
    char * click_timestamp; //FIXME depreciated (replaced by iso8601 timestamps)
    ulong  clickcount;
    long   clicktrend;
    bool   broken;
    long   ssl_error;

    struct {
        char * last_change_time;
        char * last_check_time;
        char * last_check_ok_time;
        char * last_local_check_time;
        char * click_timestamp;
    } iso8601; //https://en.wikipedia.org/wiki/ISO_8601

    struct {
        double latitude;
        double longitude;
    } geo;

    //internal cTune specific vars
    bool               is_favourite;
    ctune_StationSrc_e station_src;

} ctune_RadioStationInfo_t;

/**
 * Internal sorting attributes IDs
 */
typedef enum {
    CTUNE_RADIOSTATIONINFO_SORTBY_NONE = 0,
    CTUNE_RADIOSTATIONINFO_SORTBY_SOURCE,
    CTUNE_RADIOSTATIONINFO_SORTBY_SOURCE_DESC,
    CTUNE_RADIOSTATIONINFO_SORTBY_NAME,
    CTUNE_RADIOSTATIONINFO_SORTBY_NAME_DESC,
    CTUNE_RADIOSTATIONINFO_SORTBY_TAGS,
    CTUNE_RADIOSTATIONINFO_SORTBY_TAGS_DESC,
    CTUNE_RADIOSTATIONINFO_SORTBY_COUNTRY,
    CTUNE_RADIOSTATIONINFO_SORTBY_COUNTRY_DESC,
    CTUNE_RADIOSTATIONINFO_SORTBY_COUNTRYCODE,
    CTUNE_RADIOSTATIONINFO_SORTBY_COUNTRYCODE_DESC,
    CTUNE_RADIOSTATIONINFO_SORTBY_STATE,
    CTUNE_RADIOSTATIONINFO_SORTBY_STATE_DESC,
    CTUNE_RADIOSTATIONINFO_SORTBY_LANGUAGE,
    CTUNE_RADIOSTATIONINFO_SORTBY_LANGUAGE_DESC,
    CTUNE_RADIOSTATIONINFO_SORTBY_CODEC,
    CTUNE_RADIOSTATIONINFO_SORTBY_CODEC_DESC,
    CTUNE_RADIOSTATIONINFO_SORTBY_BITRATE,
    CTUNE_RADIOSTATIONINFO_SORTBY_BITRATE_DESC,

    CTUNE_RADIOSTATIONINFO_SORTBY_COUNT,
} ctune_RadioStationInfo_SortBy_e;

/**
 * RadioStationInfo namespace
 */
extern const struct ctune_RadioStationInfo_Namespace {
    /* Bit mask constants */
    const unsigned IS_FAV;
    const unsigned IS_QUEUED;
    const unsigned IS_LOCAL;

    /**
     * Initialize fields in the struct
     * @param rsi RadioStationInfo DTO as a void pointer
     */
    void (* init)( void * rsi );

    /**
     * Copies a RadioStationInfo DTO into another
     * @param from Origin DTO
     * @param dest Destination DTO (assumed to be already allocated)
     */
    void (* copy)( const void * from, void * dest );

    /**
     * Copies the minimal and most essential fields from a RadioStationInfo DTO into another
     * @param from Origin DTO
     * @param dest Destination DTO (assumed to be already allocated and initialised)
     */
    void (* mincopy)( const void * from, void * dest );

    /**
     * Creates an allocated duplicate version of a RadioStationInfo_t object
     * @param rsi Source
     * @return Pointer to duplicate
     */
    void * (* dup)( const void * rsi );

    /**
     * Creates an allocated minimal duplicate of a RadioStationInfo_t object
     * @param rsi Source
     * @return Pointer to duplicate
     */
    void * (* mindup)( const void * rsi );

    /**
     * Checks UUID equivalence of 2 RadioStationInfo_t objects
     * @param lhs RadioStationInfo_t object
     * @param rhs RadioStationInfo_t object
     * @return Equivalent state
     */
    bool (* sameUUID)( const void * lhs, const void * rhs );

    /**
     * Checks equivalence of 2 RadioStationInfo_t objects (all field except internal ones)
     * @param lhs RadioStationInfo_t object
     * @param rhs RadioStationInfo_t object
     * @return Equivalent state
     */
    bool (* equal)( const void * lhs, const void * rhs );

    /**
     * Creates a hash
     * @param uuid_str RadioStationInfo_t UUID string to hash
     * @return Hash
     */
    uint64_t (* hash)( const void * uuid_str );

    /**
     * Frees and set the the change timestamps to NULL
     * @param rsi RadioStationInfo DTO
     */
    void (* clearChangeTimestamps)( void * rsi );

    /**
     * Frees and sets the checking timestamps to NULL
     * @param rsi RadioStationInfo DTO
     */
    void (* clearCheckTimestamps)( void * rsi );

    /**
     * Frees the content of a RadioStationInfo DTO
     * @param rsi RadioStationInfo DTO
     */
    void (* freeContent)( void * rsi );

    /**
     * Frees a heap-allocated RadioStationInfo DTO and its content
     * @param rsi RadioStationInfo DTO
     */
    void (* free)( void * rsi );

    /**
     * Prints a RadioStationInfo
     * @param rsi RadioStationInfo instance
     * @param out Output
     */
    void (* print)( const ctune_RadioStationInfo_t * rsi, FILE * out );

    /**
     * Prints a 'lite' version of RadioStationInfo
     * @param rsi RadioStationInfo instance
     * @param out Output
     */
    void (* printLite)( const ctune_RadioStationInfo_t * rsi, FILE * out );

    /**
     * Compare two stations
     * @param lhs  Pointer to a RadioStationInfo_t object
     * @param rhs  Pointer to a RadioStationInfo_t object against
     * @param attr Attribute to compare
     * @return Result of comparison (-1: less, 0: equal, +1: greater)
     */
    int (* compareBy)( const void * lhs, const void * rhs, ctune_RadioStationInfo_SortBy_e attr );

    /**
     * Gets the comparator function pointer
     * @param attr Comparison attribute
     * @return Comparator or NULL if attribute not implemented
     */
    Comparator (* getComparator)( ctune_RadioStationInfo_SortBy_e attr );

    /**
     * Gets the string representation of a sorting attribute
     * @param attr Sorting attribute
     * @return String/NULL if no matching string is found
     */
    const char * (* sortAttrStr)( ctune_RadioStationInfo_SortBy_e attr );

    /**
     * Setters
     * -
     * Note: string vars take a pointer (`str_ptr`) to their replacements so make sure that's been
     * allocated specially for that purpose and cannot be freed elsewhere. If the internal pointer is
     * used, its resources will be freed prior to setting the new pointer. The only exception is
     * `countryCode` where the first 2 characters of the string passed are just copied.
     */
    struct {
        void (* changeUUID)( ctune_RadioStationInfo_t * rsi, char * str_ptr );
        void (* stationUUID)( ctune_RadioStationInfo_t * rsi, char * str_ptr );
        void (* stationName)( ctune_RadioStationInfo_t * rsi, char * str_ptr );
        void (* stationURL)( ctune_RadioStationInfo_t * rsi, char * str_ptr );
        void (* resolvedURL)( ctune_RadioStationInfo_t * rsi, char * str_ptr );
        void (* homepage)( ctune_RadioStationInfo_t * rsi, char * str_ptr );
        void (* faviconURL)( ctune_RadioStationInfo_t * rsi, char * str_ptr );
        void (* tags)( ctune_RadioStationInfo_t * rsi, char * str_ptr );
        void (* country)( ctune_RadioStationInfo_t * rsi, char * str_ptr );
        void (* countryCode_ISO3166_1)( ctune_RadioStationInfo_t * rsi, const char * cc_str );
        void (* countryCode_ISO3166_2)( ctune_RadioStationInfo_t * rsi, char * cc_str );
        void (* state)( ctune_RadioStationInfo_t * rsi, char * str_ptr );
        void (* language)( ctune_RadioStationInfo_t * rsi, char * str_ptr );
        void (* languageCodes)( ctune_RadioStationInfo_t * rsi, char * str_ptr );
        void (* votes)( ctune_RadioStationInfo_t * rsi, ulong votes );
        void (* lastChangeTS)( ctune_RadioStationInfo_t * rsi, char * str_ptr );
        void (* codec)( ctune_RadioStationInfo_t * rsi, char * str_ptr );
        void (* bitrate)( ctune_RadioStationInfo_t * rsi, ulong bitrate );
        void (* hls)( ctune_RadioStationInfo_t * rsi, bool state );
        void (* lastCheckOK)( ctune_RadioStationInfo_t * rsi, bool state );
        void (* lastCheckTS)( ctune_RadioStationInfo_t * rsi, char * str_ptr );
        void (* lastCheckOkTS)( ctune_RadioStationInfo_t * rsi, char * str_ptr );
        void (* lastLocalCheckTS)( ctune_RadioStationInfo_t * rsi, char * str_ptr );
        void (* clickTS)( ctune_RadioStationInfo_t * rsi, char * str_ptr );
        void (* clickCount)( ctune_RadioStationInfo_t * rsi, ulong count );
        void (* clickTrend)( ctune_RadioStationInfo_t * rsi, long trend );
        void (* broken)( ctune_RadioStationInfo_t * rsi, bool state );
        void (* sslErrCode)( ctune_RadioStationInfo_t * rsi, long ssl_err_code );
        void (* geoCoordinates)( ctune_RadioStationInfo_t * rsi, double latitude, double longitude );
        void (* favourite)( ctune_RadioStationInfo_t * rsi, bool state );
        void (* stationSource)( ctune_RadioStationInfo_t * rsi, ctune_StationSrc_e src );
    } set;

    /**
     * Getters
     */
    struct {
        const char * (* changeUUID)( const ctune_RadioStationInfo_t * rsi );
        const char * (* stationUUID)( const ctune_RadioStationInfo_t * rsi );
        const char * (* stationName)( const ctune_RadioStationInfo_t * rsi );
        const char * (* stationURL)( const ctune_RadioStationInfo_t * rsi );
        const char * (* resolvedURL)( const ctune_RadioStationInfo_t * rsi );
        const char * (* homepage)( const ctune_RadioStationInfo_t * rsi );
        const char * (* faviconURL)( const ctune_RadioStationInfo_t * rsi );
        const char * (* tags)( const ctune_RadioStationInfo_t * rsi );
        const char * (* country)( const ctune_RadioStationInfo_t * rsi );
        const char * (* countryCode)( const ctune_RadioStationInfo_t * rsi );
        const char * (* countryCode_ISO3166_1)( const ctune_RadioStationInfo_t * rsi );
        const char * (* countryCode_ISO3166_2)( const ctune_RadioStationInfo_t * rsi );
        const char * (* state)( const ctune_RadioStationInfo_t * rsi );
        const char * (* language)( const ctune_RadioStationInfo_t * rsi );
        const char * (* languageCodes)( const ctune_RadioStationInfo_t * rsi );
        ulong (* votes)( const ctune_RadioStationInfo_t * rsi );
        const char * (* lastChangeTS)( const ctune_RadioStationInfo_t * rsi );
        const char * (* codec)( const ctune_RadioStationInfo_t * rsi );
        ulong (* bitrate)( const ctune_RadioStationInfo_t * rsi );
        bool (* hls)( const ctune_RadioStationInfo_t * rsi );
        bool (* lastCheckOK)( const ctune_RadioStationInfo_t * rsi );
        const char * (* lastCheckTS)( const ctune_RadioStationInfo_t * rsi );
        const char * (* lastCheckOkTS)( const ctune_RadioStationInfo_t * rsi );
        const char * (* lastLocalCheckTS)( const ctune_RadioStationInfo_t * rsi );
        const char * (* clickTS)( const ctune_RadioStationInfo_t * rsi );
        ulong (* clickCount)( const ctune_RadioStationInfo_t * rsi );
        long (* clickTrend)( const ctune_RadioStationInfo_t * rsi );
        bool (* broken)( const ctune_RadioStationInfo_t * rsi );
        long (* sslErrCode)( const ctune_RadioStationInfo_t * rsi );
        double (* geoLatitude)( const ctune_RadioStationInfo_t * rsi );
        double (* geoLongitude)( const ctune_RadioStationInfo_t * rsi );
        bool (* favourite)( const ctune_RadioStationInfo_t * rsi );
        ctune_StationSrc_e (* stationSource)( const ctune_RadioStationInfo_t * rsi );
    } get;

} ctune_RadioStationInfo;

#endif //CTUNE_DTO_RADIOSTATIONINFO_H
