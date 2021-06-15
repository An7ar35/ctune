#ifndef CTUNE_DTO_RADIOBROWSERFILTER_H
#define CTUNE_DTO_RADIOBROWSERFILTER_H

#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#include "../datastructure/String.h"
#include "../datastructure/StrList.h"
#include "../enum/StationAttribute.h"
#include "../enum/StationSrc.h"
#include "../utils/utilities.h"

#define CTUNE_RADIOBROSWERFILTER_BITRATE_MAX_DFLT      1000000
#define CTUNE_RADIOBROSWERFILTER_BITRATE_MAX_DFLT_STR "1000000"
#define CTUNE_RADIOBROSWERFILTER_LIMIT_DFLT               1000

/**
 * Container for all filtering attributes
 */
typedef struct ctune_RadioBrowserFilter {
    char *              name;            //name of the station
    bool                nameExact;       //true: only exact matches, otherwise all matches
    char *              country;         //country of the station
    bool                countryExact;    //true: only exact matches, otherwise all matches
    char                countrycode[3];  //2-digit country code of the station (see ISO 3166-1 alpha-2: https://en.wikipedia.org/wiki/ISO_3166-1_alpha-2)
    char *              state;           //state of the station
    bool                stateExact;      //true: only exact matches, otherwise all matches
    char *              language;        //language of the station
    bool                languageExact;   //true: only exact matches, otherwise all matches
    char *              tag;             //a tag of the station
    bool                tagExact;        //true: only exact matches, otherwise all matches
    struct StrList      tagList;         //a comma-separated list of tags. All tags in list have to match.
    char *              codec;           //codec of the station
    ulong               bitrateMin;      //minimum of kbps for bitrate field of stations in result
    ulong               bitrateMax;      //maximum of kbps for bitrate field of stations in result
    ctune_StationAttr_e order;           //name of the attribute the result list will be sorted by
    bool                reverse;         //reverse the result list if set to true
    size_t              offset;          //starting value of the result list from the database. For example, if you want to do paging on the server side
    size_t              limit;           //number of returned data-rows (stations) starting with offset

    //internal
    ctune_StationSrc_e  station_src; //unused at the moment but in case there are more than 1 remote source in the future

} ctune_RadioBrowserFilter_t;


extern const struct ctune_RadioBrowserFilter_Namespace {
    /**
     * Default struct constructor for ctune_RadioBrowserFilter
     * @return ctune_RadioBrowserFilter with all field initialized to default
     */
    ctune_RadioBrowserFilter_t (* init)( void );

    /**
     * Copies a RadioBrowserFilter DTO into another
     * @param from Origin DTO
     * @param dest Destination DTO (assumed to be already allocated)
     */
    void (* copy)( const ctune_RadioBrowserFilter_t * from, ctune_RadioBrowserFilter_t * dest );

    /**
     * Converts a filter struct into a fully formed query parameter string ready to be appended to a path
     * @param filter Filter
     * @param str    String to append the formed filter query to (if any)
     */
    void (* parameteriseFields)( const ctune_RadioBrowserFilter_t * filter, String_t * str );

    /**
     * De-allocate all the char * inside a filter struct
     * @param filter Filter
     */
    void (* freeContent)( ctune_RadioBrowserFilter_t * filter );

    /**
     * Setters
     * -
     * Note: string vars take a pointer (`str_ptr`) to their replacements so make sure that's been
     * allocated specially for that purpose and cannot be freed elsewhere. If the internal pointer is
     * used, its resources will be freed prior to setting the new pointer. The only exception is
     * `countryCode` where the first 2 characters of the string passed are just copied into the
     * internal array.
     */
    struct {
        void (* name)( ctune_RadioBrowserFilter_t * filter, char * str_ptr );
        void (* exactNameToggle)( ctune_RadioBrowserFilter_t * filter, bool state );
        void (* country)( ctune_RadioBrowserFilter_t * filter, char * str_ptr );
        void (* exactCountryToggle)( ctune_RadioBrowserFilter_t * filter, bool state );
        void (* countryCode)( ctune_RadioBrowserFilter_t * filter, const char * cc_str );
        void (* state)( ctune_RadioBrowserFilter_t * filter, char * str_ptr );
        void (* exactStateToggle)( ctune_RadioBrowserFilter_t * filter, bool state );
        void (* language)( ctune_RadioBrowserFilter_t * filter, char * str_ptr );
        void (* exactLanguageToggle)( ctune_RadioBrowserFilter_t * filter, bool state );
        void (* tag)( ctune_RadioBrowserFilter_t * filter, char * str_ptr );
        void (* exactTagToggle)( ctune_RadioBrowserFilter_t * filter, bool state );
        void (* codec)( ctune_RadioBrowserFilter_t * filter, char * str_ptr );
        void (* bitrate)( ctune_RadioBrowserFilter_t * filter, ulong min, ulong max );
        void (* ordering)( ctune_RadioBrowserFilter_t * filter, ctune_StationAttr_e order );
        void (* reverseToggle)( ctune_RadioBrowserFilter_t * filter, bool state );
        void (* resultOffset)( ctune_RadioBrowserFilter_t * filter, size_t offset );
        void (* resultLimit)( ctune_RadioBrowserFilter_t * filter, size_t limit );
        void (* stationSource)( ctune_RadioBrowserFilter_t * filter, ctune_StationSrc_e src );
    } set;

    /**
     * Getters
     */
    struct {
        const char * (* name)( const ctune_RadioBrowserFilter_t * filter );
        bool (* exactNameToggle)( const ctune_RadioBrowserFilter_t * filter );
        const char * (* country)( const ctune_RadioBrowserFilter_t * filter );
        bool (* exactCountryToggle)( const ctune_RadioBrowserFilter_t * filter );
        const char * (* countryCode)( const ctune_RadioBrowserFilter_t * filter );
        const char * (* state)( const ctune_RadioBrowserFilter_t * filter );
        bool (* exactStateToggle)( const ctune_RadioBrowserFilter_t * filter );
        const char * (* language)( const ctune_RadioBrowserFilter_t * filter );
        bool (* exactLanguageToggle)( const ctune_RadioBrowserFilter_t * filter );
        const char * (* tag)( const ctune_RadioBrowserFilter_t * filter );
        bool (* exactTagToggle)( const ctune_RadioBrowserFilter_t * filter );
        StrList_t * (* tagList)( ctune_RadioBrowserFilter_t * filter );
        const char * (* codec)( const ctune_RadioBrowserFilter_t * filter );
        ulong (* bitrateMin)( const ctune_RadioBrowserFilter_t * filter );
        ulong (* bitrateMax)( const ctune_RadioBrowserFilter_t * filter );
        ctune_StationAttr_e (* ordering)( const ctune_RadioBrowserFilter_t * filter );
        bool (* reverseToggle)( const ctune_RadioBrowserFilter_t * filter);
        size_t (* resultOffset)( const ctune_RadioBrowserFilter_t * filter );
        size_t (* resultLimit)( const ctune_RadioBrowserFilter_t * filter );
        ctune_StationSrc_e (* stationSource)( ctune_RadioBrowserFilter_t * filter );
    } get;

} ctune_RadioBrowserFilter;



#endif //CTUNE_DTO_RADIOBROWSERFILTER_H
