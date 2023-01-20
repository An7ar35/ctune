#include "RadioBrowserFilter.h"

#include "../logger/Logger.h"

/**
 * Default struct constructor for ctune_RadioBrowserFilter
 * @return ctune_RadioBrowserFilter with all field initialized to default
 */
static struct ctune_RadioBrowserFilter ctune_RadioBrowserFilter_init() {
    return (struct ctune_RadioBrowserFilter) {
        .name           = NULL,
        .nameExact      = false,
        .country        = NULL,
        .countryExact   = false,
        .countrycode    = { [0] = ' ', [1] = ' ', [2] = '\0' },
        .state          = NULL,
        .stateExact     = false,
        .language       = NULL,
        .languageExact  = false,
        .tag            = NULL,
        .tagExact       = false,
        .tagList        = StrList.init(),
        .codec          = NULL,
        .bitrateMin     = 0,
        .bitrateMax     = CTUNE_RADIOBROSWERFILTER_BITRATE_MAX_DFLT,
        .order          = STATION_ATTR_NONE,
        .reverse        = false,
        .offset         = 0,
        .limit          = CTUNE_RADIOBROSWERFILTER_LIMIT_DFLT,
        .station_src    = CTUNE_STATIONSRC_RADIOBROWSER,
    };
}

/**
 * Copies a RadioBrowserFilter DTO into another
 * @param from Origin DTO
 * @param dest Destination DTO (assumed to be already allocated)
 */
void ctune_RadioBrowserFilter_copy( const ctune_RadioBrowserFilter_t * from, ctune_RadioBrowserFilter_t * dest ) {
    if( from == dest )
        return; //pointer to the same object

    ctune_RadioBrowserFilter.freeContent( dest );

    dest->nameExact      = from->nameExact;
    dest->countryExact   = from->countryExact;
    dest->stateExact     = from->stateExact;
    dest->languageExact  = from->languageExact;
    dest->tagExact       = from->tagExact;
    dest->countrycode[0] = from->countrycode[0];
    dest->countrycode[1] = from->countrycode[1];
    dest->bitrateMin     = from->bitrateMin;
    dest->bitrateMax     = from->bitrateMax;
    dest->order          = from->order;
    dest->reverse        = from->reverse;
    dest->offset         = from->offset;
    dest->limit          = from->limit;
    dest->station_src    = from->station_src;

    if( from->name )
        dest->name = strdup( from->name );
    if( from->country )
        dest->country = strdup( from->country );
    if( from->state )
        dest->state = strdup( from->state );
    if( from->language )
        dest->language = strdup( from->language );
    if( from->tag )
        dest->tag = strdup( from->tag );
    if( from->codec )
        dest->codec = strdup( from->codec );

    if( !StrList.empty( &from->tagList ) ) {
        struct StrListNode * curr = from->tagList._front;
        while( curr != NULL ) {
            StrList.insert_back( &dest->tagList, curr->data );
            curr = curr->next;
        }
    }
}

/**
 * Converts a filter struct into a fully formed query parameter string ready to be appended to a path
 * @param filter Filter
 * @param str    String to append the formed filter query to (if any)
 */
static void ctune_RadioBrowserFilter_parameteriseFields( const ctune_RadioBrowserFilter_t * filter, String_t * str ) {
    CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_RadioBrowserFilter_parameteriseFields( %p, %p )]", filter, str );

    if( filter == NULL )
        return; //EARLY RETURN

    struct String query = String.init();

    if( filter->name ) {
        String_t encoded = String.init();

        String.append_back( &query, "name=" );
        ctune_encodeURI( filter->name, &encoded );
        String.append_back( &query, encoded._raw );

        String.free( &encoded );

        if( filter->nameExact )
            String.append_back( &query, "&nameExact=true" );
    }

    if( filter->country ) {
        if( !String.empty( &query ) )
            String.append_back( &query, "&" );

        String_t encoded = String.init();

        String.append_back( &query, "country=" );
        ctune_encodeURI( filter->country, &encoded );
        String.append_back( &query, encoded._raw );

        String.free( &encoded );

        if( filter->countryExact )
            String.append_back( &query, "&countryExact=true" );
    }

    if( filter->countrycode[0] != ' ' && filter->countrycode[1] != ' ' ) {
        if( !String.empty( &query ) )
            String.append_back( &query, "&" );

        String.append_back( &query, "countrycode=" );
        String.append_back( &query, filter->countrycode );
    }

    if( filter->state ) {
        if( !String.empty( &query ) )
            String.append_back( &query, "&" );

        String_t encoded = String.init();

        String.append_back( &query, "state=" );
        ctune_encodeURI( filter->state, &encoded );
        String.append_back( &query, encoded._raw );

        String.free( &encoded );

        if( filter->stateExact )
            String.append_back( &query, filter->state );
    }

    if( filter->language ) {
        if( !String.empty( &query ) )
            String.append_back( &query, "&" );

        String_t encoded = String.init();

        String.append_back( &query, "language=" );
        ctune_encodeURI( filter->language, &encoded );
        String.append_back( &query, encoded._raw );

        String.free( &encoded );

        if( filter->languageExact )
            String.append_back( &query, "&languageExact=true" );
    }

    if( filter->tag ) {
        if( !String.empty( &query ) )
            String.append_back( &query, "&" );

        String_t encoded = String.init();

        String.append_back( &query, "tag=" );
        ctune_encodeURI( filter->tag, &encoded );
        String.append_back( &query, encoded._raw );

        String.free( &encoded );

        if( filter->tagExact )
            String.append_back( &query, "&tagExact=true" );
    }

    if( !StrList.empty( &filter->tagList ) ) {
        if( !String.empty( &query ) )
            String.append_back( &query, "&" );

        String_t             encoded = String.init();
        struct StrListNode * curr    = filter->tagList._front;

        String.append_back( &query, "tagList=" );

        while( curr ) {
            ctune_encodeURI( curr->data, &encoded );

            curr = curr->next;

            if( curr )
                String.append_back( &encoded, "," );
        }

        String.append_back( &query, encoded._raw );
        String.free( &encoded );
    }

    if( filter->codec ) {
        if( !String.empty( &query ) )
            String.append_back( &query, "&" );

        String_t encoded = String.init();

        String.append_back( &query, "codec=" );
        ctune_encodeURI( filter->codec, &encoded );
        String.append_back( &query, encoded._raw );

        String.free( &encoded );
    }

    if( filter->bitrateMin > 0 ) {
        if( !String.empty( &query ) )
            String.append_back( &query, "&" );

        String.append_back( &query, "bitrateMin=" );
        struct String n = String.init();
        ctune_utos( filter->bitrateMin, &n );
        String.append_back( &query, n._raw );
    }

    if( filter->bitrateMax != CTUNE_RADIOBROSWERFILTER_BITRATE_MAX_DFLT ) {
        if( !String.empty( &query ) )
            String.append_back( &query, "&" );

        String.append_back( &query, "bitrateMax=" );
        struct String n = String.init();
        ctune_utos( filter->bitrateMax, &n );
        String.append_back( &query, n._raw );
    }

    if( filter->order != STATION_ATTR_NONE ) {
        if( !String.empty( &query ) )
            String.append_back( &query, "&" );

        String.append_back( &query, "order=" );
        String.append_back( &query, ctune_StationAttr.str( filter->order ) );
    }

    if( filter->reverse ) {
        if( !String.empty( &query ) )
            String.append_back( &query, "&" );

        String.append_back( &query, "reverse=true" );
    }

    if( filter->offset > 0 ) {
        if( !String.empty( &query ) )
            String.append_back( &query, "&" );

        String.append_back( &query, "offset=" );
        struct String n = String.init();
        ctune_utos( filter->offset, &n );
        String.append_back( &query, n._raw );
    }

    if( filter->limit > 0 ) {
        if( !String.empty( &query ) )
            String.append_back( &query, "&" );

        String.append_back( &query, "limit=" );
        struct String n = String.init();
        ctune_utos( filter->limit, &n );
        String.append_back( &query, n._raw );
        String.free( &n );
    }

    if( !String.empty( &query ) ) {
        String.append_back( str, "?" );
        String.append_back( str, query._raw );
    }

    String.free( &query );

    CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_RadioBrowserFilter_parameteriseFields( %p, %p )] END", filter, str );
}

/**
 * De-allocate all the char * inside a filter struct
 * @param filter Filter
 */
static void ctune_RadioBrowserFilter_freeContent( ctune_RadioBrowserFilter_t * filter ) {
    if( filter == NULL )
        return; //EARLY RETURN

    free( filter->name );
    filter->name = NULL;

    free( filter->country );
    filter->country = NULL;

    free( filter->state );
    filter->state = NULL;

    free( filter->language );
    filter->language = NULL;

    free( filter->tag );
    filter->tag = NULL;

    if( !StrList.empty( &filter->tagList ) )
        StrList.free_strlist( &filter->tagList );

    free( filter->codec );
    filter->codec = NULL;
}

static void ctune_RadioBrowserFilter_set_name( ctune_RadioBrowserFilter_t * filter, char * str_ptr ) {
    if( filter != NULL ) {
        if( filter->name )
            free( filter->name );

        filter->name = str_ptr;
    }
}

static void ctune_RadioBrowserFilter_set_exactNameToggle( ctune_RadioBrowserFilter_t * filter, bool state ) {
    if( filter != NULL ) {
        filter->nameExact = state;
    }
}

static void ctune_RadioBrowserFilter_set_country( ctune_RadioBrowserFilter_t * filter, char * str_ptr ) {
    if( filter != NULL ) {
        if( filter->country )
            free( filter->country );

        filter->country = str_ptr;
    }
}

static void ctune_RadioBrowserFilter_set_exactCountryToggle( ctune_RadioBrowserFilter_t * filter, bool state ) {
    if( filter != NULL ) {
        filter->countryExact = state;
    }
}

static void ctune_RadioBrowserFilter_set_countryCode( ctune_RadioBrowserFilter_t * filter, const char * cc_str ) {
    if( filter != NULL ) {
        if( cc_str != NULL ) {
            if( strlen( cc_str ) >= 2 ) {
                filter->countrycode[0] = cc_str[0];
                filter->countrycode[1] = cc_str[1];
                filter->countrycode[2] = '\0';

            } else {
                filter->countrycode[0] = ' ';
                filter->countrycode[1] = ' ';
                filter->countrycode[2] = '\0';

                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_RadioBrowserFilter_set_countryCode( %p, %p )] String passed is too short - CC set to blank.",
                           filter, cc_str
                );
            }
        }
    }
}

static void ctune_RadioBrowserFilter_set_state( ctune_RadioBrowserFilter_t * filter, char * str_ptr ) {
    if( filter != NULL ) {
        if( filter->state )
            free( filter->state );

        filter->state = str_ptr;
    }
}

static void ctune_RadioBrowserFilter_set_exactStateToggle( ctune_RadioBrowserFilter_t * filter, bool state ) {
    if( filter != NULL ) {
        filter->stateExact = state;
    }
}

static void ctune_RadioBrowserFilter_set_language( ctune_RadioBrowserFilter_t * filter, char * str_ptr ) {
    if( filter != NULL ) {
        if( filter->language )
            free( filter->language );

        filter->language = str_ptr;
    }
}

static void ctune_RadioBrowserFilter_set_exactLanguageToggle( ctune_RadioBrowserFilter_t * filter, bool state ) {
    if( filter != NULL ) {
        filter->languageExact = state;
    }
}

static void ctune_RadioBrowserFilter_set_tag( ctune_RadioBrowserFilter_t * filter, char * str_ptr ) {
    if( filter != NULL ) {
        if( filter->tag )
            free( filter->tag );

        filter->tag = str_ptr;
    }
}

static void ctune_RadioBrowserFilter_set_exactTagToggle( ctune_RadioBrowserFilter_t * filter, bool state ) {
    if( filter != NULL ) {
        filter->tagExact = state;
    }
}

static void ctune_RadioBrowserFilter_set_codec( ctune_RadioBrowserFilter_t * filter, char * str_ptr ) {
    if( filter != NULL ) {
        if( filter->codec )
            free( filter->codec );

        filter->codec = str_ptr;
    }
}

static void ctune_RadioBrowserFilter_set_bitrate( ctune_RadioBrowserFilter_t * filter, ulong min, ulong max ) {
    if( filter != NULL ) {
        filter->bitrateMin = min;
        filter->bitrateMax = max;
    }
}

static void ctune_RadioBrowserFilter_set_ordering( ctune_RadioBrowserFilter_t * filter, ctune_StationAttr_e order ) {
    if( filter != NULL ) {
        filter->order = order;
    }
}

static void ctune_RadioBrowserFilter_set_reverseToggle( ctune_RadioBrowserFilter_t * filter, bool state ) {
    if( filter != NULL ) {
        filter->reverse = state;
    }
}

static void ctune_RadioBrowserFilter_set_resultOffset( ctune_RadioBrowserFilter_t * filter, size_t offset ) {
    if( filter != NULL ) {
        filter->offset = offset;
    }
}

static void ctune_RadioBrowserFilter_set_resultLimit( ctune_RadioBrowserFilter_t * filter, size_t limit ) {
    if( filter != NULL ) {
        filter->limit = limit;
    }
}

static void ctune_RadioBrowserFilter_set_stationSource( ctune_RadioBrowserFilter_t * filter, ctune_StationSrc_e src ) {
    if( filter != NULL ) {
        filter->station_src = src;
    }
}

static const char * ctune_RadioBrowserFilter_get_name( const ctune_RadioBrowserFilter_t * filter ) {
    if( filter == NULL )
        return NULL;
    return filter->name;
}

static bool ctune_RadioBrowserFilter_get_exactNameToggle( const ctune_RadioBrowserFilter_t * filter ) {
    if( filter == NULL )
        return false;
    return filter->nameExact;
}

static const char * ctune_RadioBrowserFilter_get_country( const ctune_RadioBrowserFilter_t * filter ) {
    if( filter == NULL )
        return NULL;
    return filter->country;
}

static bool ctune_RadioBrowserFilter_get_exactCountryToggle( const ctune_RadioBrowserFilter_t * filter ) {
    if( filter == NULL )
        return false;
    return filter->countryExact;
}

static const char * ctune_RadioBrowserFilter_get_countryCode( const ctune_RadioBrowserFilter_t * filter ) {
    if( filter == NULL )
        return NULL;
    return filter->countrycode;
}

static const char * ctune_RadioBrowserFilter_get_state( const ctune_RadioBrowserFilter_t * filter ) {
    if( filter == NULL )
        return NULL;
    return filter->state;
}

static bool ctune_RadioBrowserFilter_get_exactStateToggle( const ctune_RadioBrowserFilter_t * filter ) {
    if( filter == NULL )
        return false;
    return filter->stateExact;
}

static const char * ctune_RadioBrowserFilter_get_language( const ctune_RadioBrowserFilter_t * filter ) {
    if( filter == NULL )
        return NULL;
    return filter->language;
}

static bool ctune_RadioBrowserFilter_get_exactLanguageToggle( const ctune_RadioBrowserFilter_t * filter ) {
    if( filter == NULL )
        return false;
    return filter->languageExact;
}

static const char * ctune_RadioBrowserFilter_get_tag( const ctune_RadioBrowserFilter_t * filter ) {
    if( filter == NULL )
        return NULL;
    return filter->tag;
}

static bool ctune_RadioBrowserFilter_get_exactTagToggle( const ctune_RadioBrowserFilter_t * filter ) {
    if( filter == NULL )
        return false;
    return filter->tagExact;
}

static StrList_t * ctune_RadioBrowserFilter_get_tagList( ctune_RadioBrowserFilter_t * filter ) {
    if( filter == NULL )
        return NULL;
    return &filter->tagList;
}

static const char * ctune_RadioBrowserFilter_get_codec( const ctune_RadioBrowserFilter_t * filter ) {
    if( filter == NULL )
        return NULL;
    return filter->codec;
}

static ulong ctune_RadioBrowserFilter_get_bitrateMin( const ctune_RadioBrowserFilter_t * filter ) {
    if( filter == NULL )
        return 0;
    return filter->bitrateMin;
}

static ulong ctune_RadioBrowserFilter_get_bitrateMax( const ctune_RadioBrowserFilter_t * filter ) {
    if( filter == NULL )
        return CTUNE_RADIOBROSWERFILTER_BITRATE_MAX_DFLT;
    return filter->bitrateMax;
}

static ctune_StationAttr_e ctune_RadioBrowserFilter_get_ordering( const ctune_RadioBrowserFilter_t * filter ) {
    if( filter == NULL )
        return STATION_ATTR_NONE;
    return filter->order;
}

static bool ctune_RadioBrowserFilter_get_reverseToggle( const ctune_RadioBrowserFilter_t * filter) {
    if( filter == NULL )
        return false;
    return filter->reverse;
}

static size_t ctune_RadioBrowserFilter_get_resultOffset( const ctune_RadioBrowserFilter_t * filter ) {
    if( filter == NULL )
        return 0;
    return filter->offset;
}

static size_t ctune_RadioBrowserFilter_get_resultLimit( const ctune_RadioBrowserFilter_t * filter ) {
    if( filter == NULL )
        return 0;
    return filter->limit;
}

static ctune_StationSrc_e ctune_RadioBrowserFilter_get_stationSource( ctune_RadioBrowserFilter_t * filter ) {
    if( filter == NULL )
        return CTUNE_STATIONSRC_LOCAL;
    return filter->station_src;
}


/**
 * Namespace constructor
 */
const struct ctune_RadioBrowserFilter_Namespace ctune_RadioBrowserFilter = {
    .init               = &ctune_RadioBrowserFilter_init,
    .copy               = &ctune_RadioBrowserFilter_copy,
    .parameteriseFields = &ctune_RadioBrowserFilter_parameteriseFields,
    .freeContent        = &ctune_RadioBrowserFilter_freeContent,
    .set = {
        .name                = &ctune_RadioBrowserFilter_set_name,
        .exactNameToggle     = &ctune_RadioBrowserFilter_set_exactNameToggle,
        .country             = &ctune_RadioBrowserFilter_set_country,
        .exactCountryToggle  = &ctune_RadioBrowserFilter_set_exactCountryToggle,
        .countryCode         = &ctune_RadioBrowserFilter_set_countryCode,
        .state               = &ctune_RadioBrowserFilter_set_state,
        .exactStateToggle    = &ctune_RadioBrowserFilter_set_exactStateToggle,
        .language            = &ctune_RadioBrowserFilter_set_language,
        .exactLanguageToggle = &ctune_RadioBrowserFilter_set_exactLanguageToggle,
        .tag                 = &ctune_RadioBrowserFilter_set_tag,
        .exactTagToggle      = &ctune_RadioBrowserFilter_set_exactTagToggle,
        .codec               = &ctune_RadioBrowserFilter_set_codec,
        .bitrate             = &ctune_RadioBrowserFilter_set_bitrate,
        .ordering            = &ctune_RadioBrowserFilter_set_ordering,
        .reverseToggle       = &ctune_RadioBrowserFilter_set_reverseToggle,
        .resultOffset        = &ctune_RadioBrowserFilter_set_resultOffset,
        .resultLimit         = &ctune_RadioBrowserFilter_set_resultLimit,
        .stationSource       = &ctune_RadioBrowserFilter_set_stationSource,
    },
    .get = {
        .name                = &ctune_RadioBrowserFilter_get_name,
        .exactNameToggle     = &ctune_RadioBrowserFilter_get_exactNameToggle,
        .country             = &ctune_RadioBrowserFilter_get_country,
        .exactCountryToggle  = &ctune_RadioBrowserFilter_get_exactCountryToggle,
        .countryCode         = &ctune_RadioBrowserFilter_get_countryCode,
        .state               = &ctune_RadioBrowserFilter_get_state,
        .exactStateToggle    = &ctune_RadioBrowserFilter_get_exactStateToggle,
        .language            = &ctune_RadioBrowserFilter_get_language,
        .exactLanguageToggle = &ctune_RadioBrowserFilter_get_exactLanguageToggle,
        .tag                 = &ctune_RadioBrowserFilter_get_tag,
        .exactTagToggle      = &ctune_RadioBrowserFilter_get_exactTagToggle,
        .tagList             = &ctune_RadioBrowserFilter_get_tagList,
        .codec               = &ctune_RadioBrowserFilter_get_codec,
        .bitrateMin          = &ctune_RadioBrowserFilter_get_bitrateMin,
        .bitrateMax          = &ctune_RadioBrowserFilter_get_bitrateMax,
        .ordering            = &ctune_RadioBrowserFilter_get_ordering,
        .reverseToggle       = &ctune_RadioBrowserFilter_get_reverseToggle,
        .resultOffset        = &ctune_RadioBrowserFilter_get_resultOffset,
        .resultLimit         = &ctune_RadioBrowserFilter_get_resultLimit,
        .stationSource       = &ctune_RadioBrowserFilter_get_stationSource,
    },
};
