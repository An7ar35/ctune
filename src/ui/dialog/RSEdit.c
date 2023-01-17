#include "RSEdit.h"

#include <errno.h>
#include <string.h>
#include <regex.h>

#include "../definitions/KeyBinding.h"
#include "../definitions/Theme.h"
#include "ContextHelp.h"
#include "../../dto/RadioBrowserFilter.h" //for the max bitrate macro ("CTUNE_RADIOBROWSERFILTER_BITRATE_MAX_DFLT")
#include "../Resizer.h"

typedef enum {
    LABEL_UUID = 0,
    LABEL_NAME,
    LABEL_URL,
    LABEL_RESOLVED_URL,
    LABEL_HOMEPAGE,
    LABEL_TAGS,
    LABEL_COUNTRY,
    LABEL_COUNTRY_CODE,
    LABEL_STATE,
    LABEL_LANGUAGE,
    LABEL_LANGUAGE_CODES,
    LABEL_CODEC,
    LABEL_BITRATE,
    LABEL_BITRATE_UNIT,
    LABEL_COORDINATES,
    FIELD_UUID,

    LABEL_COUNT,
} RSEdit_Label_e;

typedef enum {
    INPUT_NAME = LABEL_COUNT,
    INPUT_URL,
    INPUT_RESOLVED_URL,
    INPUT_HOMEPAGE,
    INPUT_TAGS,
    INPUT_COUNTRY,
    INPUT_COUNTRY_CODE,
    INPUT_STATE,
    INPUT_LANGUAGE,
    INPUT_LANGUAGE_CODES,
    INPUT_CODEC,
    INPUT_BITRATE,
    BUTTON_AUTODETECT,
    INPUT_COORDINATE_LAT,
    INPUT_COORDINATE_LONG,
    BUTTON_CANCEL,
    BUTTON_SAVE,

    FIELD_LAST, //NULL field ( needs this for array check in `panel.h`)
    FIELD_COUNT,
} RSEdit_Input_e;

/**
 * Private constant variables used across all instances
 */
static const struct {
    int row_height;
    int row_pos[FIELD_COUNT];

} private = {
    .row_height = 1,
    .row_pos    = {
        [LABEL_UUID           ] =  0, [FIELD_UUID           ] =  0,
        [LABEL_NAME           ] =  2, [INPUT_NAME           ] =  2,
        [LABEL_URL            ] =  4, [INPUT_URL            ] =  4,
        [LABEL_RESOLVED_URL   ] =  6, [INPUT_RESOLVED_URL   ] =  6,
        [LABEL_HOMEPAGE       ] =  8, [INPUT_HOMEPAGE       ] =  8,
        [LABEL_TAGS           ] = 10, [INPUT_TAGS           ] = 10,
        [LABEL_COUNTRY        ] = 12, [INPUT_COUNTRY        ] = 12,
        [LABEL_COUNTRY_CODE   ] = 14, [INPUT_COUNTRY_CODE   ] = 14,
        [LABEL_STATE          ] = 16, [INPUT_STATE          ] = 16,
        [LABEL_LANGUAGE       ] = 18, [INPUT_LANGUAGE       ] = 18,
        [LABEL_LANGUAGE_CODES ] = 20, [INPUT_LANGUAGE_CODES ] = 20,
        [LABEL_CODEC          ] = 22, [INPUT_CODEC          ] = 22,
        [BUTTON_AUTODETECT    ] = 23,
        [LABEL_BITRATE        ] = 24, [INPUT_BITRATE        ] = 24, [LABEL_BITRATE_UNIT   ] = 24,
        [LABEL_COORDINATES    ] = 26, [INPUT_COORDINATE_LAT ] = 26, [INPUT_COORDINATE_LONG] = 26,
        [BUTTON_CANCEL        ] = 28, [BUTTON_SAVE          ] = 28,
        [FIELD_LAST           ] = 28,
    },
};

/**
 * [PRIVATE] Scroll form when given field is out-of-view
 * @param rsedit Pointer to a ctune_UI_RSEdit_t object
 * @param field  Field to base scrolling on (currently selected?)
 */
static void ctune_UI_RSEdit_autoScroll( ctune_UI_RSEdit_t * rsedit, const FIELD * field ) {
    if( field == NULL )
        return; //EARLY RETURN

    int field_pos_y = 0;
    int field_pos_x = 0;

    if( field_info( field, NULL, NULL, &field_pos_y, &field_pos_x, NULL, NULL ) != E_OK ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_RSEdit_autoScroll( %p, %p )] Failed to get info for field." );
        return; //EARLY RETURN
    }

    ctune_UI_Dialog.autoScroll( &rsedit->dialog, field_pos_y, field_pos_x );
}

/**
 * [PRIVATE] Check if field is a button
 * @param rsedit Pointer to a ctune_UI_RSEdit_t object
 * @param field  Form field
 * @return Button field status
 */
static bool ctune_UI_RSEdit_isButton( const ctune_UI_RSEdit_t * rsedit, const FIELD * field ) {
    if( field == NULL )
        return false; //EARLY RETURN

    return ( field == rsedit->cache.fields[ BUTTON_AUTODETECT ]
          || field == rsedit->cache.fields[ BUTTON_CANCEL     ]
          || field == rsedit->cache.fields[ BUTTON_SAVE       ] );
}

/**
 * [PRIVATE] Highlights input field where the cursor is at
 * @param rsedit     Pointer to a ctune_UI_RSEdit_t object]
 * @param curr_field Current field
 */
static void ctune_UI_RSEdit_highlightCurrField( ctune_UI_RSEdit_t * rsedit, FIELD * curr_field ) {
    curs_set( ctune_UI_RSEdit_isButton( rsedit, curr_field ) ? 0 : 1 );

    for( int i = 0; i < LABEL_COUNT; ++i ) {
        set_field_back( rsedit->cache.fields[ i ], A_NORMAL );
    }

    for( int i = LABEL_COUNT; i < FIELD_COUNT; ++i ) {
        if( rsedit->cache.fields[ i ] == curr_field ) {
            set_field_back( rsedit->cache.fields[ i ], A_REVERSE );
        } else {
            if( ctune_UI_RSEdit_isButton( rsedit, rsedit->cache.fields[ i ] ) )
                set_field_back( rsedit->cache.fields[ i ], A_NORMAL );
            else
                set_field_back( rsedit->cache.fields[ i ], A_UNDERLINE );
        }
    }
}

/**
 * [PRIVATE]
 * @param rsedit Pointer to a ctune_UI_RSEdit_t object
 * @param field  Form field
 * @return Auto-detect button field status
 */
static bool ctune_UI_RSEdit_autodetectButton( const ctune_UI_RSEdit_t * rsedit, const FIELD * field ) {
    return ( field == rsedit->cache.fields[ BUTTON_AUTODETECT ] );
}

/**
 * [PRIVATE] Performs the auto-detect action for the associated button
 * @param rsedit Pointer to a ctune_UI_RSEdit_t object
 * @param rsi    Pointer to cached RadioStationInfo_t object
 * @param field  Form field
 */
static void ctune_UI_RSEdit_autodetectStreamProperties( ctune_UI_RSEdit_t * rsedit, ctune_RadioStationInfo_t * rsi, FIELD * field ) {
    if( field_status( rsedit->cache.fields[ INPUT_URL ] ) )
        ctune_RadioStationInfo.set.stationURL( rsi, ctune_trimspace( field_buffer( rsedit->cache.fields[ INPUT_URL ], 0 ) ) );

    if( field_status( rsedit->cache.fields[ INPUT_RESOLVED_URL ] ) )
        ctune_RadioStationInfo.set.resolvedURL( rsi, ctune_trimspace( field_buffer( rsedit->cache.fields[ INPUT_RESOLVED_URL ], 0 ) ) );

    const bool has_url          = ( ctune_RadioStationInfo.get.stationURL( rsi )  != NULL && strlen( ctune_RadioStationInfo.get.stationURL( rsi ) )  > 0 );
    const bool has_url_resolved = ( ctune_RadioStationInfo.get.resolvedURL( rsi ) != NULL && strlen( ctune_RadioStationInfo.get.resolvedURL( rsi ) ) > 0 );

    if( !has_url_resolved && !has_url ) { //i.e.: no URLs
        set_field_back( field, ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_DFLT    ) );
        set_field_fore( field, ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_INVALID ) );

        set_current_field( rsedit->form, rsedit->cache.fields[ INPUT_RESOLVED_URL ] );
        ctune_UI_RSEdit_highlightCurrField( rsedit, current_field( rsedit->form ) );

        return; //EARLY RETURN
    }

    String_t     codec     = String.init();
    String_t     bitrate   = String.init();
    String_t     timestamp = String.init();
    const char * url       = ( has_url_resolved ? ctune_RadioStationInfo.get.resolvedURL( rsi )
                                                : ctune_RadioStationInfo.get.stationURL( rsi ) );

    if( rsedit->cb.validateURL( url ) ) {
        ctune_RadioStationInfo.clearCheckTimestamps( rsi );
        ctune_timestampISO8601( &timestamp );

        if( rsedit->cb.testStream( url, &codec, &rsi->bitrate ) ) {
            if( rsi->codec != NULL ) {
                free( rsi->codec );
                rsi->codec = NULL;
            }

            set_field_back( field, ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_DFLT  ) );
            set_field_fore( field, ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_VALID ) );

            ctune_strupr( codec._raw );
            set_field_buffer( rsedit->cache.fields[ INPUT_CODEC ], 0, ( codec._raw == NULL ? "" : codec._raw ) );

            if( rsi->bitrate > 0 ) { //Bitrate field (ulong)
                ctune_utos( rsi->bitrate, &bitrate );
                set_field_buffer( rsedit->cache.fields[ INPUT_BITRATE ], 0, bitrate._raw );
            }

            ctune_RadioStationInfo.set.lastCheckOK( rsi, true );
            ctune_RadioStationInfo.set.lastCheckOkTS( rsi, strdup( timestamp._raw ) );

        } else {
            set_field_back( field, ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_DFLT    ) );
            set_field_fore( field, ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_INVALID ) );
        }

        ctune_RadioStationInfo.set.lastCheckTS( rsi, strdup( timestamp._raw ) );
        ctune_RadioStationInfo.set.lastLocalCheckTS( rsi, strdup( timestamp._raw ) );

    } else {
        set_current_field( rsedit->form, rsedit->cache.fields[ ( has_url_resolved ? INPUT_RESOLVED_URL : INPUT_URL ) ] );
        ctune_UI_RSEdit_highlightCurrField( rsedit, current_field( rsedit->form ) );
    }

    String.free( &timestamp );
    String.free( &bitrate );
    String.free( &codec );
}

/**
 * [PRIVATE] Changes the 'last change time' ISO8601 timestamp to now
 * @param rsi Pointer to RadioStationInfo_t object to set the change timestamp
 */
static void ctune_UI_RSEdit_setChangeTimestamp( ctune_RadioStationInfo_t * rsi ) {
    String_t ts = String.init();
    ctune_timestampISO8601( &ts );
    ctune_RadioStationInfo.set.lastChangeTS( rsi, strdup( ts._raw ) );
    String.free( &ts );
}

/**
 * [PRIVATE] Check if given field is a form exit
 * @param rsedit    Pointer to a ctune_UI_RSEdit_t object
 * @param field     FIELD pointer
 * @param exit_type Variable pointer to set the exit type of the field
 * @return Exit state
 */
static bool ctune_UI_RSEdit_isExitState( const ctune_UI_RSEdit_t * rsedit, const FIELD * field, ctune_FormExit_e * exit_type ) {
    if( ctune_UI_RSEdit_isButton( rsedit, field ) ) {
        if( field == rsedit->cache.fields[ BUTTON_CANCEL ] ) {
            *exit_type = CTUNE_UI_FORM_CANCEL;
            return true;
        }

        if( field == rsedit->cache.fields[ BUTTON_SAVE ] ) {
            *exit_type = CTUNE_UI_FORM_SUBMIT;
            return true;
        }
    }

    return false;
}

/**
 * [PRIVATE] Checks field values
 * @param rsedit Pointer to ctune_UI_RSEdit_t object
 * @return Validation state
 */
static bool ctune_UI_RSEdit_validate( ctune_UI_RSEdit_t * rsedit, const ctune_RadioStationInfo_t * rsi ) {
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_RSEdit_validateFieldValues( %p )] Checking field values...", rsedit );

    bool error_state = false;

    bool no_url          = ( ctune_RadioStationInfo.get.stationURL( rsi )  == NULL || strlen( ctune_RadioStationInfo.get.stationURL( rsi ) )  == 0 );
    bool no_url_resolved = ( ctune_RadioStationInfo.get.resolvedURL( rsi ) == NULL || strlen( ctune_RadioStationInfo.get.resolvedURL( rsi ) ) == 0 );

    if( no_url && no_url_resolved ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_RSEdit_validate( %p, %p )] Failed validation: URL",
                   rsedit, rsi
        );

        error_state = true;
        set_current_field( rsedit->form, rsedit->cache.fields[ INPUT_RESOLVED_URL ] );

    } else {
        if( !no_url && !rsedit->cb.validateURL( rsi->url ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_RSEdit_validate( %p, %p )] Failed URL validation: %s",
                       rsedit, rsi, rsi->url
            );

            error_state = true;
            set_current_field( rsedit->form, rsedit->cache.fields[ INPUT_URL ] );

        } else if( !no_url_resolved && !rsedit->cb.validateURL( rsi->url_resolved ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_RSEdit_validate( %p, %p )] Failed resolved URL validation: %s",
                       rsedit, rsi, rsi->url
            );

            error_state = true;
            set_current_field( rsedit->form, rsedit->cache.fields[ INPUT_RESOLVED_URL ] );
        }
    }

    if( ctune_RadioStationInfo.get.stationName( rsi ) == NULL || strlen( ctune_RadioStationInfo.get.stationName( rsi ) ) == 0 ) { //required field
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_RSEdit_validate( %p, %p )] Failed name validation: empty/NULL",
                   rsedit, rsi
        );

        error_state = true;
        set_current_field( rsedit->form, rsedit->cache.fields[ INPUT_NAME ] );
    }

    ctune_UI_RSEdit_highlightCurrField( rsedit, current_field( rsedit->form ) );

    return !( error_state );
}

/**
 * [PRIVATE] Packs all the field values into the cached RSI object (as LOCAL station)
 * @param rsi Pointer to ctune_UI_RSEdit_t object
 * @param
 * @return Validated state
 */
static bool ctune_UI_RSEdit_packFieldValues( ctune_UI_RSEdit_t * rsedit, ctune_RadioStationInfo_t * rsi ) {
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_RSEdit_packFieldValues( %p, %p )] Packing field values...", rsedit, rsi );

    bool error_state = false;

    ctune_RadioStationInfo.set.stationSource( rsi, CTUNE_STATIONSRC_LOCAL );

    if( field_status( rsedit->cache.fields[ INPUT_NAME ] ) ) //required field
        ctune_RadioStationInfo.set.stationName( rsi, ctune_trimspace( field_buffer( rsedit->cache.fields[ INPUT_NAME ], 0 ) ) );

    if( field_status( rsedit->cache.fields[ INPUT_URL ] ) ) //required field *
        ctune_RadioStationInfo.set.stationURL( rsi, ctune_trimspace( field_buffer( rsedit->cache.fields[ INPUT_URL ], 0 ) ) );

    if( field_status( rsedit->cache.fields[ INPUT_RESOLVED_URL ] ) ) //required field *
        ctune_RadioStationInfo.set.resolvedURL( rsi, ctune_trimspace( field_buffer( rsedit->cache.fields[ INPUT_RESOLVED_URL ], 0 ) ) );

    if( field_status( rsedit->cache.fields[ INPUT_HOMEPAGE ] ) )
        ctune_RadioStationInfo.set.homepage( rsi, ctune_trimspace( field_buffer( rsedit->cache.fields[ INPUT_HOMEPAGE ], 0 ) ) );

    if( field_status( rsedit->cache.fields[ INPUT_TAGS ] ) ) {
        StrList_t tag_list = StrList.init();
        String_t  tmp      = String.init();

        size_t val_count = ctune_splitcss( field_buffer( rsedit->cache.fields[ INPUT_TAGS ], 0 ), &tag_list );

        CTUNE_LOG( CTUNE_LOG_DEBUG,
                   "[ctune_UI_RSFind_packFieldValues( %p, %p )] Found %i values in field 'INPUT_TAGS': \"%s\"",
                   rsedit, rsi, val_count, field_buffer( rsedit->cache.fields[ INPUT_TAGS ], 0 )
        );

        if( val_count == 0 ) {
            ctune_RadioStationInfo.set.tags( rsi, NULL );

        } else if( val_count == 1 ) {
            ctune_RadioStationInfo.set.tags( rsi, strdup( StrList.at( &tag_list, 0 )->data ) );

            if( rsi->tags == NULL ) {
                error_state = true;

                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_UI_RSFind_packFieldValues( %p, %p )] Failed strdup of tag.",
                           rsedit, rsi
                );
            }

        } else { //val_count > 1
            size_t ret_count = StrList.stringify( &tag_list, &tmp, ',' );

            if( ret_count != val_count ) {
                error_state = true;

                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_UI_RSFind_packFieldValues( %p, %p )] Error parsing all tags (%lu/%lu).",
                           rsedit, rsi, ret_count, val_count
                );
            }

            ctune_RadioStationInfo.set.tags( rsi, strdup( tmp._raw ) );

            if( rsi->tags == NULL ) {
                error_state = true;

                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_UI_RSFind_packFieldValues( %p, %p )] Failed strdup of tags.",
                           rsedit, rsi
                );
            };
        }

        String.free( &tmp );
        StrList.free_strlist( &tag_list );
    }

    if( field_status( rsedit->cache.fields[ INPUT_COUNTRY ] ) )
        ctune_RadioStationInfo.set.country( rsi, ctune_trimspace( field_buffer( rsedit->cache.fields[ INPUT_COUNTRY ], 0 ) ) );

    if( field_status( rsedit->cache.fields[ INPUT_COUNTRY_CODE ] ) ) {
        char * cc = ctune_trimspace( field_buffer( rsedit->cache.fields[ INPUT_COUNTRY_CODE ], 0 ) );

        if( cc != NULL ) {
            ctune_RadioStationInfo.set.countryCode_ISO3166_1( rsi, cc );
            free( cc );
        }
    }

    if( field_status( rsedit->cache.fields[ INPUT_STATE ] ) )
        ctune_RadioStationInfo.set.state( rsi, ctune_trimspace( field_buffer( rsedit->cache.fields[ INPUT_STATE ], 0 ) ) );

    if( field_status( rsedit->cache.fields[ INPUT_LANGUAGE ] ) )
        ctune_RadioStationInfo.set.language( rsi, ctune_trimspace( field_buffer( rsedit->cache.fields[ INPUT_LANGUAGE ], 0 ) ) );

    if( field_status( rsedit->cache.fields[ INPUT_LANGUAGE_CODES ] ) ) {
        StrList_t list = StrList.init();
        String_t  tmp  = String.init();

        size_t val_count = ctune_splitcss( field_buffer( rsedit->cache.fields[ INPUT_LANGUAGE_CODES ], 0 ), &list );

        CTUNE_LOG( CTUNE_LOG_DEBUG,
                   "[ctune_UI_RSFind_packFieldValues( %p, %p )] Found %i values in field 'INPUT_LANGUAGE_CODES': \"%s\"",
                   rsedit, rsi, val_count, field_buffer( rsedit->cache.fields[ INPUT_LANGUAGE_CODES ], 0 )
        );

        if( val_count == 0 ) {
            ctune_RadioStationInfo.set.language( rsi, NULL );

        } else if( val_count == 1 ) {
            ctune_RadioStationInfo.set.language( rsi, strdup( StrList.at( &list, 0 )->data ) );

            if( rsi->language_codes == NULL ) {
                error_state = true;

                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_UI_RSFind_packFieldValues( %p, %p )] Failed strdup of language code.",
                           rsedit, rsi
                );
            }

        } else if( val_count > 0 ) {
            size_t ret_count = StrList.stringify( &list, &tmp, ',' );

            if( ret_count != val_count ) {
                error_state = true;

                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_UI_RSFind_packFieldValues( %p, %p )] Error parsing all language codes (%lu/%lu).",
                           rsedit, rsi, ret_count, val_count
                );
            }

            ctune_RadioStationInfo.set.language( rsi, strdup( tmp._raw ) );

            if( rsi->tags == NULL ) {
                error_state = true;

                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_UI_RSFind_packFieldValues( %p, %p )] Failed strdup of language codes.",
                           rsedit, rsi
                );
            };
        }

        String.free( &tmp );
        StrList.free_strlist( &list );
    }

    if( field_status( rsedit->cache.fields[ INPUT_CODEC ] ) )
        ctune_RadioStationInfo.set.codec( rsi, ctune_trimspace( field_buffer( rsedit->cache.fields[ INPUT_CODEC ], 0 ) ) );

    if( field_status( rsedit->cache.fields[ INPUT_BITRATE ] ) ) {
        ulong bitrate = strtoul( field_buffer( rsedit->cache.fields[ INPUT_BITRATE ], 0 ), NULL, 10 );
        ctune_RadioStationInfo.set.bitrate( rsi, bitrate );
    }

    if( field_status( rsedit->cache.fields[ INPUT_COORDINATE_LAT ] ) || field_status( rsedit->cache.fields[ INPUT_COORDINATE_LONG ] ) ) {
        char * latitude_str  = ctune_trimspace( field_buffer( rsedit->cache.fields[ INPUT_COORDINATE_LAT  ], 0 ) );
        char * longitude_str = ctune_trimspace( field_buffer( rsedit->cache.fields[ INPUT_COORDINATE_LONG ], 0 ) );
        double latitude_val  = 0.0;
        double longitude_val = 0.0;

        if( latitude_str != NULL ) {
            latitude_val = strtod( latitude_str, NULL );

            if( errno == ERANGE ) {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_UI_RSFind_packFieldValues( %p, %p )] Failed string->`double` conversion of latitude ('%s').",
                           rsedit, rsi, latitude_str
                );

            }

            free( latitude_str );
        }

        if( longitude_str != NULL ) {
            longitude_val = strtod( longitude_str, NULL );

            if( errno == ERANGE ) {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_UI_RSFind_packFieldValues( %p, %p )] Failed string->`double` conversion of longitude ('%s').",
                           rsedit, rsi, longitude_str
                );

            }

            free( longitude_str );
        }

        ctune_RadioStationInfo.set.geoCoordinates( rsi, latitude_val, longitude_val );
    }

    //check required fields are present
    if( !ctune_UI_RSEdit_validate( rsedit, &rsedit->cache.station ) )
        error_state = true;

    if( error_state ) {
        CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_RSEdit_packFieldValues( %p, %p )] Field validation failed.", rsedit, rsi );
    } else {
        CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_RSEdit_packFieldValues( %p, %p )] Field values packed.", rsedit, rsi );
    }

    return !( error_state );
}

/**
 * [PRIVATE] Initialises cached filter and fields
 * @return Success
 */
static void ctune_UI_RSEdit_initFields( ctune_UI_RSEdit_t * rsedit ) {
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_RSEdit_initFields( %p )] Initialising form fields...", rsedit );

    const ctune_RadioStationInfo_t * rsi = &rsedit->cache.station; //shortcut ptr

    set_field_buffer( rsedit->cache.fields[ LABEL_UUID           ], 0, rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_STATION_UUID ) );
    set_field_buffer( rsedit->cache.fields[ LABEL_NAME           ], 0, rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_STATION_NAME ) );
    set_field_buffer( rsedit->cache.fields[ LABEL_URL            ], 0, rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_URL ) );
    set_field_buffer( rsedit->cache.fields[ LABEL_RESOLVED_URL   ], 0, rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_URL_RESOLVED ) );
    set_field_buffer( rsedit->cache.fields[ LABEL_HOMEPAGE       ], 0, rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_URL_HOMEPAGE ) );
    set_field_buffer( rsedit->cache.fields[ LABEL_TAGS           ], 0, rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_TAGS ) );
    set_field_buffer( rsedit->cache.fields[ LABEL_COUNTRY        ], 0, rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_COUNTRY ) );
    set_field_buffer( rsedit->cache.fields[ LABEL_COUNTRY_CODE   ], 0, rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_COUNTRY_CODE ) );
    set_field_buffer( rsedit->cache.fields[ LABEL_STATE          ], 0, rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_STATE ) );
    set_field_buffer( rsedit->cache.fields[ LABEL_LANGUAGE       ], 0, rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_LANGUAGE ) );
    set_field_buffer( rsedit->cache.fields[ LABEL_LANGUAGE_CODES ], 0, rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_LANGUAGE_CODES ) );
    set_field_buffer( rsedit->cache.fields[ LABEL_CODEC          ], 0, rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_CODEC ) );
    set_field_buffer( rsedit->cache.fields[ LABEL_BITRATE        ], 0, rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_BITRATE ) );
    set_field_buffer( rsedit->cache.fields[ LABEL_BITRATE_UNIT   ], 0, rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_BITRATE_UNIT_LONG ) );
    set_field_buffer( rsedit->cache.fields[ LABEL_COORDINATES    ], 0, rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_GEO_COORDS ) );

    set_field_buffer( rsedit->cache.fields[ FIELD_UUID           ], 0, ctune_fallbackStr( ctune_RadioStationInfo.get.stationUUID( rsi ), "" ) );
    set_field_buffer( rsedit->cache.fields[ INPUT_NAME           ], 0, ctune_fallbackStr( ctune_RadioStationInfo.get.stationName( rsi ), "" ) );
    set_field_buffer( rsedit->cache.fields[ INPUT_URL            ], 0, ctune_fallbackStr( ctune_RadioStationInfo.get.stationURL( rsi ), "" ) );
    set_field_buffer( rsedit->cache.fields[ INPUT_RESOLVED_URL   ], 0, ctune_fallbackStr( ctune_RadioStationInfo.get.resolvedURL( rsi ), "" ) );
    set_field_buffer( rsedit->cache.fields[ INPUT_HOMEPAGE       ], 0, ctune_fallbackStr( ctune_RadioStationInfo.get.homepage( rsi ), "" ) );
    set_field_buffer( rsedit->cache.fields[ INPUT_TAGS           ], 0, ctune_fallbackStr( ctune_RadioStationInfo.get.tags( rsi ), "" ) );
    set_field_buffer( rsedit->cache.fields[ INPUT_COUNTRY        ], 0, ctune_fallbackStr( ctune_RadioStationInfo.get.country( rsi ), "" ) );
    set_field_buffer( rsedit->cache.fields[ INPUT_COUNTRY_CODE   ], 0, ctune_fallbackStr( ctune_RadioStationInfo.get.countryCode_ISO3166_1( rsi ), "" ) );
    set_field_buffer( rsedit->cache.fields[ INPUT_STATE          ], 0, ctune_fallbackStr( ctune_RadioStationInfo.get.state( rsi ), "" ) );
    set_field_buffer( rsedit->cache.fields[ INPUT_LANGUAGE       ], 0, ctune_fallbackStr( ctune_RadioStationInfo.get.language( rsi ), "" ) );
    set_field_buffer( rsedit->cache.fields[ INPUT_LANGUAGE_CODES ], 0, ctune_fallbackStr( ctune_RadioStationInfo.get.languageCodes( rsi ), "" ) );
    set_field_buffer( rsedit->cache.fields[ INPUT_CODEC          ], 0, ctune_fallbackStr( ctune_RadioStationInfo.get.codec( rsi ), "" ) );

    { //Bitrate field (ulong)
        String_t bitrate = String.init();
        ctune_utos( ctune_RadioStationInfo.get.bitrate( rsi ), &bitrate );
        set_field_buffer( rsedit->cache.fields[ INPUT_BITRATE ], 0, bitrate._raw );
        String.free( &bitrate );
    }

    { //geo coordinates field (double, double)
        String_t latitude  = String.init();
        String_t longitude = String.init();
        ctune_ftos( ctune_RadioStationInfo.get.geoLatitude( rsi ), &latitude );
        ctune_ftos( ctune_RadioStationInfo.get.geoLongitude( rsi ), &longitude );
        set_field_buffer( rsedit->cache.fields[ INPUT_COORDINATE_LAT  ], 0, latitude._raw  );
        set_field_buffer( rsedit->cache.fields[ INPUT_COORDINATE_LONG ], 0, longitude._raw );
        String.free( &latitude  );
        String.free( &longitude );
    }

    set_field_buffer( rsedit->cache.fields[ BUTTON_AUTODETECT ], 0, rsedit->cb.getDisplayText( CTUNE_UI_TEXT_BUTTON_AUTODETECT_STREAM ) );
    set_field_buffer( rsedit->cache.fields[ BUTTON_CANCEL     ], 0, rsedit->cb.getDisplayText( CTUNE_UI_TEXT_BUTTON_CANCEL ) );
    set_field_buffer( rsedit->cache.fields[ BUTTON_SAVE       ], 0, rsedit->cb.getDisplayText( CTUNE_UI_TEXT_BUTTON_SAVE ) );

    set_field_back( rsedit->cache.fields[ BUTTON_AUTODETECT ], ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_DFLT ) );
    set_field_fore( rsedit->cache.fields[ BUTTON_AUTODETECT ], ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_DFLT ) );
    set_field_back( rsedit->cache.fields[ BUTTON_CANCEL     ], ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_DFLT ) );
    set_field_fore( rsedit->cache.fields[ BUTTON_CANCEL     ], ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_DFLT ) );
    set_field_back( rsedit->cache.fields[ BUTTON_SAVE       ], ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_DFLT ) );
    set_field_fore( rsedit->cache.fields[ BUTTON_SAVE       ], ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_DFLT ) );
}

/**
 * [PRIVATE] Clears all input fields on the form and move back to first field
 */
static void ctune_UI_RSEdit_clearAllFields( ctune_UI_RSEdit_t * rsedit ) {
    for( int i = LABEL_COUNT; i < FIELD_COUNT; ++i ) {
        set_current_field( rsedit->form, rsedit->cache.fields[ i ] );
        form_driver( rsedit->form, REQ_CLR_FIELD );
    }

    ctune_UI_RSEdit_initFields( rsedit );

    set_current_field( rsedit->form, rsedit->cache.fields[LABEL_COUNT] );
}

/**
 * [PRIVATE] Creates the fields (Called once and cached for the remainder of the runtime)
 * @return Success
 */
static bool ctune_UI_RSEdit_createFields( ctune_UI_RSEdit_t * rsedit ) {
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_RSEdit_createFields( %p )] Creating form fields...", rsedit );

//    .--------------[ FORM ]------------------.
//    |                                        |
//    | Name    ______________________________ |
//    |                                        |
//    | URL     ______________________________ |
//    |                                        |
//    | Codec   ________________               |
//    |                          [Auto-detect] |
//    | Bitrate ________ Kbps                  |
//    .                                        .
//    .                                        .

    if( rsedit->cache.max_label_width == 0 ) {
        rsedit->cache.max_label_width = ctune_max_ul( rsedit->cache.max_label_width, strlen( rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_STATION_UUID ) ) );
        rsedit->cache.max_label_width = ctune_max_ul( rsedit->cache.max_label_width, strlen( rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_STATION_NAME ) ) );
        rsedit->cache.max_label_width = ctune_max_ul( rsedit->cache.max_label_width, strlen( rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_URL ) ) );
        rsedit->cache.max_label_width = ctune_max_ul( rsedit->cache.max_label_width, strlen( rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_URL_RESOLVED ) ) );
        rsedit->cache.max_label_width = ctune_max_ul( rsedit->cache.max_label_width, strlen( rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_URL_HOMEPAGE ) ) );
        rsedit->cache.max_label_width = ctune_max_ul( rsedit->cache.max_label_width, strlen( rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_TAGS ) ) );
        rsedit->cache.max_label_width = ctune_max_ul( rsedit->cache.max_label_width, strlen( rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_COUNTRY ) ) );
        rsedit->cache.max_label_width = ctune_max_ul( rsedit->cache.max_label_width, strlen( rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_COUNTRY_CODE ) ) );
        rsedit->cache.max_label_width = ctune_max_ul( rsedit->cache.max_label_width, strlen( rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_STATE ) ) );
        rsedit->cache.max_label_width = ctune_max_ul( rsedit->cache.max_label_width, strlen( rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_LANGUAGE ) ) );
        rsedit->cache.max_label_width = ctune_max_ul( rsedit->cache.max_label_width, strlen( rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_LANGUAGE_CODES ) ) );
        rsedit->cache.max_label_width = ctune_max_ul( rsedit->cache.max_label_width, strlen( rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_CODEC ) ) );
        rsedit->cache.max_label_width = ctune_max_ul( rsedit->cache.max_label_width, strlen( rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_BITRATE ) ) );
        rsedit->cache.max_label_width = ctune_max_ul( rsedit->cache.max_label_width, strlen( rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_BITRATE_UNIT_LONG ) ) );
        rsedit->cache.max_label_width = ctune_max_ul( rsedit->cache.max_label_width, strlen( rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_GEO_COORDS ) ) );
    }

    int  label_col_width = 0;
    bool err_state       = false;

    if( !ctune_utoi( rsedit->cache.max_label_width, &label_col_width ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_UI_RSEdit_createFields()] Failed cast max label width value (%lu) to integer.",
                   rsedit->cache.max_label_width, strerror( errno )
        );

        err_state = true;
    }

    const int label_col               = 0;
    const int field_col               = (int) rsedit->cache.max_label_width + 2;
    const int autodetect_button_width = (int) strlen( rsedit->cb.getDisplayText( CTUNE_UI_TEXT_BUTTON_AUTODETECT_STREAM ) );

    const int cc_field_width          = 2; //(country code)

    const int bitrate_field_width     = 10; //old = 8
    const int bitrate_unit_width      = (int) strlen( rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_BITRATE_UNIT_LONG ) );
    const int bitrate_unit_col        = ( field_col + bitrate_field_width + 1 );
    const int bitrate_block_width     = ( bitrate_field_width + 1 + bitrate_unit_width );

    const int codec_field_width      = 32;

    const int autodetect_button_col  = ( field_col + ( codec_field_width > bitrate_block_width ? codec_field_width : bitrate_block_width ) + 2 );

    const int std_field_min_width    = ( ( bitrate_block_width > codec_field_width ? bitrate_block_width : codec_field_width ) + 1 + autodetect_button_width );
    const int std_field_width        = ( std_field_min_width < 50 ? 50 : std_field_min_width ); //i.e.: at least a UUID size for width
    const int latitude_field_col     = ( field_col + bitrate_block_width + 2 );

    const int form_width             = field_col + std_field_width;

    //[ Fields ]                                               rows                cols                 y                                         x
    rsedit->cache.fields[ LABEL_UUID            ] = new_field( private.row_height, label_col_width,     private.row_pos[ LABEL_UUID            ], label_col, 0, 0 );
    rsedit->cache.fields[ LABEL_NAME            ] = new_field( private.row_height, label_col_width,     private.row_pos[ LABEL_NAME            ], label_col, 0, 0 );
    rsedit->cache.fields[ LABEL_URL             ] = new_field( private.row_height, label_col_width,     private.row_pos[ LABEL_URL             ], label_col, 0, 0 );
    rsedit->cache.fields[ LABEL_RESOLVED_URL    ] = new_field( private.row_height, label_col_width,     private.row_pos[ LABEL_RESOLVED_URL    ], label_col, 0, 0 );
    rsedit->cache.fields[ LABEL_HOMEPAGE        ] = new_field( private.row_height, label_col_width,     private.row_pos[ LABEL_HOMEPAGE        ], label_col, 0, 0 );
    rsedit->cache.fields[ LABEL_TAGS            ] = new_field( private.row_height, label_col_width,     private.row_pos[ LABEL_TAGS            ], label_col, 0, 0 );
    rsedit->cache.fields[ LABEL_COUNTRY         ] = new_field( private.row_height, label_col_width,     private.row_pos[ LABEL_COUNTRY         ], label_col, 0, 0 );
    rsedit->cache.fields[ LABEL_COUNTRY_CODE    ] = new_field( private.row_height, label_col_width,     private.row_pos[ LABEL_COUNTRY_CODE    ], label_col, 0, 0 );
    rsedit->cache.fields[ LABEL_STATE           ] = new_field( private.row_height, label_col_width,     private.row_pos[ LABEL_STATE           ], label_col, 0, 0 );
    rsedit->cache.fields[ LABEL_LANGUAGE        ] = new_field( private.row_height, label_col_width,     private.row_pos[ LABEL_LANGUAGE        ], label_col, 0, 0 );
    rsedit->cache.fields[ LABEL_LANGUAGE_CODES  ] = new_field( private.row_height, label_col_width,     private.row_pos[ LABEL_LANGUAGE_CODES  ], label_col, 0, 0 );
    rsedit->cache.fields[ LABEL_CODEC           ] = new_field( private.row_height, label_col_width,     private.row_pos[ LABEL_CODEC           ], label_col, 0, 0 );
    rsedit->cache.fields[ LABEL_BITRATE         ] = new_field( private.row_height, label_col_width,     private.row_pos[ LABEL_BITRATE         ], label_col, 0, 0 );
    rsedit->cache.fields[ LABEL_BITRATE_UNIT    ] = new_field( private.row_height, bitrate_unit_width,  private.row_pos[ LABEL_BITRATE_UNIT    ], bitrate_unit_col, 0, 0 );
    rsedit->cache.fields[ LABEL_COORDINATES     ] = new_field( private.row_height, label_col_width,     private.row_pos[ LABEL_COORDINATES     ], label_col, 0, 0 );
    rsedit->cache.fields[ FIELD_UUID            ] = new_field( private.row_height, std_field_width,     private.row_pos[ FIELD_UUID            ], field_col, 0, 0 );
    rsedit->cache.fields[ INPUT_NAME            ] = new_field( private.row_height, std_field_width,     private.row_pos[ INPUT_NAME            ], field_col, 0, 0 );
    rsedit->cache.fields[ INPUT_URL             ] = new_field( private.row_height, std_field_width,     private.row_pos[ INPUT_URL             ], field_col, 0, 0 );
    rsedit->cache.fields[ INPUT_RESOLVED_URL    ] = new_field( private.row_height, std_field_width,     private.row_pos[ INPUT_RESOLVED_URL    ], field_col, 0, 0 );
    rsedit->cache.fields[ INPUT_HOMEPAGE        ] = new_field( private.row_height, std_field_width,     private.row_pos[ INPUT_HOMEPAGE        ], field_col, 0, 0 );
    rsedit->cache.fields[ INPUT_TAGS            ] = new_field( private.row_height, std_field_width,     private.row_pos[ INPUT_TAGS            ], field_col, 0, 0 );
    rsedit->cache.fields[ INPUT_COUNTRY         ] = new_field( private.row_height, std_field_width,     private.row_pos[ INPUT_COUNTRY         ], field_col, 0, 0 );
    rsedit->cache.fields[ INPUT_COUNTRY_CODE    ] = new_field( private.row_height, cc_field_width,      private.row_pos[ INPUT_COUNTRY_CODE    ], field_col, 0, 0 );
    rsedit->cache.fields[ INPUT_STATE           ] = new_field( private.row_height, std_field_width,     private.row_pos[ INPUT_STATE           ], field_col, 0, 0 );
    rsedit->cache.fields[ INPUT_LANGUAGE        ] = new_field( private.row_height, std_field_width,     private.row_pos[ INPUT_LANGUAGE        ], field_col, 0, 0 );
    rsedit->cache.fields[ INPUT_LANGUAGE_CODES  ] = new_field( private.row_height, std_field_width,     private.row_pos[ INPUT_LANGUAGE_CODES  ], field_col, 0, 0 );
//    rsedit->cache.fields[ INPUT_CODEC           ] = new_field( private.row_height, col2_sml_width,    private.row_pos[ INPUT_CODEC           ], col2, 0, 0 );
    rsedit->cache.fields[ INPUT_CODEC           ] = new_field( private.row_height, codec_field_width,   private.row_pos[ INPUT_CODEC           ], field_col, 0, 0 );
    rsedit->cache.fields[ INPUT_BITRATE         ] = new_field( private.row_height, bitrate_field_width, private.row_pos[ INPUT_BITRATE         ], field_col, 0, 0 );
    rsedit->cache.fields[ INPUT_COORDINATE_LAT  ] = new_field( private.row_height, bitrate_block_width, private.row_pos[ INPUT_COORDINATE_LAT  ], field_col, 0, 0 );
    rsedit->cache.fields[ INPUT_COORDINATE_LONG ] = new_field( private.row_height, bitrate_block_width, private.row_pos[ INPUT_COORDINATE_LONG ], latitude_field_col, 0, 0 );

    const int button_separation     = 6;
    const int max_button_width      = (int) ctune_max_ul( strlen( rsedit->cb.getDisplayText( CTUNE_UI_TEXT_BUTTON_SAVE ) ),
                                                          strlen( rsedit->cb.getDisplayText( CTUNE_UI_TEXT_BUTTON_CANCEL ) ) );
    const int button_line_ln        = max_button_width + button_separation + max_button_width;
    int       button_line_pad       = 0;

    if( button_line_ln < form_width ) {
        button_line_pad = ( ( form_width - button_line_ln ) / 2 ) + 1;
    }
    //[ Buttons ]                                          rows                cols                     y                                     x
    rsedit->cache.fields[ BUTTON_AUTODETECT ] = new_field( private.row_height, autodetect_button_width, private.row_pos[ BUTTON_AUTODETECT ], autodetect_button_col, 0, 0 );
    rsedit->cache.fields[ BUTTON_CANCEL     ] = new_field( private.row_height, max_button_width,        private.row_pos[ BUTTON_CANCEL     ], button_line_pad, 0, 0 );
    rsedit->cache.fields[ BUTTON_SAVE       ] = new_field( private.row_height, max_button_width,        private.row_pos[ BUTTON_SAVE       ], ( button_line_pad + max_button_width + button_separation ), 0, 0 );
    rsedit->cache.fields[ FIELD_LAST        ] = NULL;

    for( int i = 0; i < FIELD_LAST; ++i ) {
        if( rsedit->cache.fields[i] == NULL ) {
            CTUNE_LOG( CTUNE_LOG_FATAL,
                       "[ctune_UI_RSEdit_createFields()] Failed to create Field (#%i): %s",
                       i, strerror( errno )
            );
            err_state = true;
        }
    }

    if( err_state )
        return false;

    set_field_opts( rsedit->cache.fields[ LABEL_UUID            ], O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    set_field_opts( rsedit->cache.fields[ LABEL_NAME            ], O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    set_field_opts( rsedit->cache.fields[ LABEL_URL             ], O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    set_field_opts( rsedit->cache.fields[ LABEL_RESOLVED_URL    ], O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    set_field_opts( rsedit->cache.fields[ LABEL_TAGS            ], O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    set_field_opts( rsedit->cache.fields[ LABEL_HOMEPAGE        ], O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    set_field_opts( rsedit->cache.fields[ LABEL_COUNTRY         ], O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    set_field_opts( rsedit->cache.fields[ LABEL_COUNTRY_CODE    ], O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    set_field_opts( rsedit->cache.fields[ LABEL_STATE           ], O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    set_field_opts( rsedit->cache.fields[ LABEL_LANGUAGE        ], O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    set_field_opts( rsedit->cache.fields[ LABEL_LANGUAGE_CODES  ], O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    set_field_opts( rsedit->cache.fields[ LABEL_CODEC           ], O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    set_field_opts( rsedit->cache.fields[ LABEL_BITRATE         ], O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    set_field_opts( rsedit->cache.fields[ LABEL_BITRATE_UNIT    ], O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    set_field_opts( rsedit->cache.fields[ LABEL_COORDINATES     ], O_VISIBLE | O_PUBLIC | O_AUTOSKIP );

    set_field_opts( rsedit->cache.fields[ FIELD_UUID            ], O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    set_field_opts( rsedit->cache.fields[ INPUT_NAME            ], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE );
    set_field_opts( rsedit->cache.fields[ INPUT_URL             ], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE );
    set_field_opts( rsedit->cache.fields[ INPUT_RESOLVED_URL    ], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE );
    set_field_opts( rsedit->cache.fields[ INPUT_HOMEPAGE        ], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE );
    set_field_opts( rsedit->cache.fields[ INPUT_TAGS            ], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE );
    set_field_opts( rsedit->cache.fields[ INPUT_COUNTRY         ], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE );
    set_field_opts( rsedit->cache.fields[ INPUT_COUNTRY_CODE    ], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE | O_INPUT_LIMIT | O_NULLOK | O_STATIC );
    set_field_type( rsedit->cache.fields[ INPUT_COUNTRY_CODE    ], TYPE_ALPHA, 2 );
    set_field_opts( rsedit->cache.fields[ INPUT_STATE           ], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE );
    set_field_opts( rsedit->cache.fields[ INPUT_LANGUAGE        ], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE );
    set_field_opts( rsedit->cache.fields[ INPUT_CODEC           ], O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    set_field_opts( rsedit->cache.fields[ INPUT_BITRATE         ], O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    set_field_type( rsedit->cache.fields[ INPUT_BITRATE         ], TYPE_INTEGER, 0, 0, CTUNE_RADIOBROSWERFILTER_BITRATE_MAX_DFLT );
    set_field_type( rsedit->cache.fields[ INPUT_COORDINATE_LAT  ], TYPE_NUMERIC, 6, 0, 0 );
    set_field_type( rsedit->cache.fields[ INPUT_COORDINATE_LONG ], TYPE_NUMERIC, 6, 0, 0 );

    set_field_opts( rsedit->cache.fields[ BUTTON_AUTODETECT     ], O_VISIBLE | O_PUBLIC | O_ACTIVE | O_STATIC );
    set_field_opts( rsedit->cache.fields[ BUTTON_CANCEL         ], O_VISIBLE | O_PUBLIC | O_ACTIVE | O_STATIC );
    set_field_opts( rsedit->cache.fields[ BUTTON_SAVE           ], O_VISIBLE | O_PUBLIC | O_ACTIVE | O_STATIC );

    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_RSEdit_createFields()] Fields created." );
    return true;
}

/**
 * [PRIVATE] Initialises the Form
 * @return Success
 */
static bool ctune_UI_RSEdit_initForm( ctune_UI_RSEdit_t * rsedit ) {
    if( rsedit->form != NULL ) {
        unpost_form( rsedit->form );
        free_form( rsedit->form );
        rsedit->form = NULL;
    }

    if( rsedit->cache.max_label_width == 0 ) {
        if( !ctune_UI_RSEdit_createFields( rsedit ) ) {
            CTUNE_LOG( CTUNE_LOG_FATAL,
                       "[ctune_UI_RSEdit_initForm()] Failed to create fields for the form."
            );

            return false; //EARLY RETURN
        }
    }

    ctune_UI_RSEdit_initFields( rsedit );

    if( rsedit->form != NULL ) {
        free_form( rsedit->form );
        rsedit->form = NULL;
    }

    if( ( rsedit->form = new_form( rsedit->cache.fields ) ) == NULL ) {
        switch( errno ) {
            case E_BAD_ARGUMENT:
                CTUNE_LOG( CTUNE_LOG_FATAL,
                           "[ctune_UI_RSEdit_initForm()] "
                           "Failed to create Form: Routine detected an incorrect or out-of-range argument."
                );
                break;

            case E_CONNECTED:
                CTUNE_LOG( CTUNE_LOG_FATAL,
                           "[ctune_UI_RSEdit_initForm()] "
                           "Failed to create Form: The field is already connected to a form."
                );
                break;

            case E_SYSTEM_ERROR:
                CTUNE_LOG( CTUNE_LOG_FATAL,
                           "[ctune_UI_RSEdit_initForm()] "
                           "Failed to create Form: System error occurred, e.g., malloc failure."
                );
                break;

            default:
                CTUNE_LOG( CTUNE_LOG_FATAL,
                           "[ctune_UI_RSEdit_initForm()] "
                           "Failed to create Form: unknown error."
                );
        }

        return false; //EARLY RETURN
    }

    scale_form( rsedit->form, &rsedit->form_dimension.rows, &rsedit->form_dimension.cols );

    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_UI_RSEdit_initForm()] Form size calculated as: rows = %i, cols = %i (desc field size = %i)",
               rsedit->form_dimension.rows, rsedit->form_dimension.cols, rsedit->cache.max_label_width
    );

    return true;
}

/**
 * Creates a base ctune_UI_RSEdit_t object
 * @param parent         Pointer to size property of the parent window
 * @param getDisplayText Callback method to get text strings for the display
 * @param generateUUID   Callback method to create a unique UUID for new stations
 * @param testStream     Callback method to test and get codec/bitrate from a stream URL
 * @param validateURL    Callback method to check the validity of a URL string
 * @return Basic un-initialised ctune_UI_RSEdit_t object
 */
static ctune_UI_RSEdit_t ctune_UI_RSEdit_create( const WindowProperty_t * parent,
                                                 const char * (* getDisplayText)( ctune_UI_TextID_e ),
                                                 bool         (* generateUUID)( String_t * ),
                                                 bool         (* testStream)( const char *, String_t *, ulong * ),
                                                 bool         (* validateURL)( const char * ) ) {
    return (ctune_UI_RSEdit_t) {
        .initialised    = false,
        .screen_size    = parent,
        .margins        = { 1, 1, 1, 1 },
        .dialog         = ctune_UI_Dialog.init(),
        .form_dimension = { 0, 0, 0, 0 },
        .form           = NULL,
        .cache          = { .max_label_width = 0, },
        .cb             = { .getDisplayText = getDisplayText, .generateUUID = generateUUID, .testStream = testStream, .validateURL = validateURL },
    };
}

/**
 * Initialises RSFind (mostly checks base values are OK)
 * @param rsedit Pointer to a ctune_UI_RSFind_t object
 * @return Success
 */
static bool ctune_UI_RSEdit_init( ctune_UI_RSEdit_t * rsedit ) {
    bool error_state = false;

    if( rsedit->initialised == true ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_RSEdit_init( %p )] RSFind has already been initialised!",
                   rsedit
        );

        error_state = true;
        goto end;
    }

    if( CTUNE_UI_DIALOG_RSEDIT_FIELD_COUNT != FIELD_COUNT ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_UI_RSEdit_init( %p )] "
                   "Field count macro value does not match number of fields in enum. Check src code!",
                   rsedit
        );

        error_state = true;
        goto end;
    }

    if( rsedit->screen_size == NULL ) {
        CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_UI_RSEdit_init( %p )] Pointer to screen size is NULL.", rsedit );
        error_state = true;
    }

    if( rsedit->cb.getDisplayText == NULL  ) {
        CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_UI_RSEdit_init( %p )] Callback methods not set: 'getDisplayText'", rsedit );
        error_state = true;
    }

    if( rsedit->cb.generateUUID == NULL ) {
        CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_UI_RSEdit_init( %p )] Callback methods not set: 'generateUUID'", rsedit );
        error_state = true;
    }

    if( rsedit->cb.testStream == NULL ) {
        CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_UI_RSEdit_init( %p )] Callback methods not set: 'testStream'", rsedit );
        error_state = true;
    }

    if( rsedit->cb.validateURL == NULL ) {
        CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_UI_RSEdit_init( %p )] Callback methods not set: 'validateURL'", rsedit );
        error_state = true;
    }

    ctune_UI_Dialog.setAutoScrollOffset( &rsedit->dialog, 2, 22 );

    //pattern from https://gist.github.com/gruber/249502
    int ret = regcomp( &rsedit->cache.url_regex,
                       "^((?:[a-z][\\w-]+:(?:\\/{1,3}|[a-z0-9%])|www\\d{0,3}[.]|[a-z0-9.\\-]+[.][a-z]{2,4}\\/)"
                       "(?:[^\\s()<>]+|\\(([^\\s()<>]+|(\\([^\\s()<>]+\\)))*\\))+"
                       "(?:\\(([^\\s()<>]+|(\\([^\\s()<>]+\\)))*\\)|[^\\s`!()\\[\\]{};:'\".,<>?«»“”‘’]))",
                       0 );

    if( ret ) {
        CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_UI_RSEdit_init( %p )] Failed to compile regular expression (ret=%i)", rsedit, ret );
        error_state = true;
    }

    if( !error_state ) {
        ctune_RadioStationInfo.init( &rsedit->cache.station );
        rsedit->initialised = true;
    }

    end:
        return !( error_state );
}

/**
 * Get the initialised state of the instance
 * @param rsedit Pointer to a ctune_UI_RSEdit_t object
 * @return Initialised state
 */
static bool ctune_UI_RSEdit_isInitialised( ctune_UI_RSEdit_t * rsedit ) {
    return rsedit->initialised;
}

/**
 * Loads a radio station into the form
 * @param rsedit Pointer to a ctune_UI_RSEdit_t object
 * @param rsi    Pointer to RSI to edit
 */
static void ctune_UI_RSEdit_loadStation( ctune_UI_RSEdit_t * rsedit, const ctune_RadioStationInfo_t * rsi ) {
    ctune_RadioStationInfo.freeContent( &rsedit->cache.station );
    ctune_RadioStationInfo.mincopy( rsi, &rsedit->cache.station );
}

/**
 * Sets up the form for a new radio station
 * @param rsedit Pointer to a ctune_UI_RSEdit_t object
 * @return Success
 */
static bool ctune_UI_RSEdit_newStation( ctune_UI_RSEdit_t * rsedit ) {
    bool                       error_state = false;
    String_t                   uuid        = String.init();
    ctune_RadioStationInfo_t * cached_rsi  = &rsedit->cache.station; //shortcut pointer

    ctune_RadioStationInfo.freeContent( cached_rsi );
    ctune_RadioStationInfo.init( cached_rsi );
    ctune_RadioStationInfo.set.stationSource( cached_rsi, CTUNE_STATIONSRC_LOCAL );

    if( rsedit->cb.generateUUID( &uuid ) ){
        ctune_RadioStationInfo.set.stationUUID( cached_rsi, strdup( uuid._raw ) );

        if( ctune_RadioStationInfo.get.stationUUID( cached_rsi ) == NULL ) {
            CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_RSEdit_newStation( %p )] Failed to allocate/copy UUID to station object.", rsedit );
            error_state = true;
        };

    } else {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_RSEdit_newStation( %p )] Failed to get a UUID for station.", rsedit );
        error_state = true;
    }

    String.free( &uuid );
    return !( error_state );
}

/**
 * Copy a radio station into the form with a new generated local UUID
 * @param rsedit Pointer to a ctune_UI_RSEdit_t object
 * @param rsi    Pointer to RSI to copy as new local station
 * @return Success
 */
bool ctune_UI_RSEdit_copyStation( ctune_UI_RSEdit_t * rsedit, const ctune_RadioStationInfo_t * rsi ) {
    bool                       error_state = false;
    String_t                   uuid        = String.init();
    ctune_RadioStationInfo_t * cached_rsi  = &rsedit->cache.station; //shortcut pointer

    ctune_RadioStationInfo.freeContent(cached_rsi );
    ctune_RadioStationInfo.copy( rsi, cached_rsi );
    ctune_RadioStationInfo.set.stationSource( cached_rsi, CTUNE_STATIONSRC_LOCAL );

    if( rsedit->cb.generateUUID( &uuid ) ){
        ctune_RadioStationInfo.set.stationUUID( cached_rsi, strdup( uuid._raw ) );

        if( ctune_RadioStationInfo.get.stationUUID( cached_rsi ) == NULL ) {
            CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_RSEdit_copyStation( %p, %p )] Failed to allocate/copy UUID to station object.", rsedit, rsi );
            error_state = true;
        };

    } else {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_RSEdit_copyStation( %p, %p )] Failed to get a UUID for station.", rsedit, rsi );
        error_state = true;
    }

    String.free( &uuid );
    return !( error_state );
}

/**
 * Create and show a populated window with the find form
 * @param rsedit Pointer to a ctune_UI_RSEdit_t object
 * @return Success
 */
static bool ctune_UI_RSEdit_show( ctune_UI_RSEdit_t * rsedit ) {
    if( !rsedit->initialised ) {
        ctune_err.set( CTUNE_ERR_UI );
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_RSEdit_show( %p )] RSEdit not initialised prior.", rsedit );
        return false; //EARLY RETURN
    }

    if( !ctune_UI_RSEdit_initForm( rsedit ) ) {
        ctune_err.set( CTUNE_ERR_UI );
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_RSEdit_show( %p )] Could not initialise form.", rsedit );
        return false; //EARLY RETURN
    }

    ctune_UI_Dialog.free( &rsedit->dialog );

    ctune_UI_Dialog.createScrollWin( &rsedit->dialog,
                                     rsedit->form_dimension.rows,
                                     rsedit->form_dimension.cols );

    ctune_UI_Dialog.createBorderWin( &rsedit->dialog,
                                     rsedit->screen_size,
                                     rsedit->cb.getDisplayText( CTUNE_UI_TEXT_WIN_TITLE_RSEDIT ),
                                     &rsedit->margins );

    set_form_win( rsedit->form, rsedit->dialog.border_win.window );
    set_form_sub( rsedit->form, rsedit->dialog.canvas.pad );
    post_form( rsedit->form );

    ctune_UI_Dialog.show( &rsedit->dialog );
    ctune_UI_Dialog.refreshView( &rsedit->dialog );

    ctune_UI_Resizer.push( ctune_UI_RSEdit.resize, rsedit );

    doupdate();
    return true;
}

/**
 * Resize the dialog
 * @param rsedit Pointer to a ctune_UI_RSEdit_t object
 */
static void ctune_UI_RSEdit_resize( void * rsedit ) {
    CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_UI_RSEdit_resize( %p )] Resize event called.", rsedit );

    if( rsedit == NULL ) {
        ctune_err.set( CTUNE_ERR_UI );
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_RSEdit_resize( %p )] RSEdit is NULL.", rsedit );
        return;
    }

    ctune_UI_RSEdit_t * rse_dialog = rsedit;

    if( !rse_dialog->initialised ) {
        ctune_err.set( CTUNE_ERR_UI );
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_RSEdit_resize( %p )] RSEdit is not initialised.", rse_dialog );
        return; //EARLY RETURN
    }

    ctune_UI_Dialog.scrollHome( &rse_dialog->dialog );

    ctune_UI_Dialog.createBorderWin( &rse_dialog->dialog,
                                     rse_dialog->screen_size,
                                     rse_dialog->cb.getDisplayText( CTUNE_UI_TEXT_WIN_TITLE_RSFIND ),
                                     &rse_dialog->margins );

    set_form_win( rse_dialog->form, rse_dialog->dialog.border_win.window );
    set_form_sub( rse_dialog->form, rse_dialog->dialog.canvas.pad );
    post_form( rse_dialog->form );

    ctune_UI_Dialog.show( &rse_dialog->dialog );
    ctune_UI_Dialog.refreshView( &rse_dialog->dialog );
}

/**
 * Pass keyboard input to the form
 * @param rsedit Pointer to a ctune_UI_RSEdit_t object
 * @return Form exit state
 */
static ctune_FormExit_e ctune_UI_RSEdit_captureInput( ctune_UI_RSEdit_t * rsedit ) {
    keypad( rsedit->dialog.canvas.pad, TRUE );
    bool             exit       = false;
    ctune_FormExit_e exit_state = CTUNE_UI_FORM_ESC;
    int  character;

    form_driver( rsedit->form, REQ_FIRST_FIELD );
    ctune_UI_RSEdit_highlightCurrField( rsedit, current_field( rsedit->form ) );
    ctune_UI_ScrollWin.refreshView( &rsedit->dialog.canvas );

    while( !exit ) {
        character = wgetch( rsedit->dialog.canvas.pad );

        switch( ctune_UI_KeyBinding.getAction( CTUNE_UI_CTX_RSEDIT, character ) ) {
            case CTUNE_UI_ACTION_ERR   : //fallthrough
            case CTUNE_UI_ACTION_RESIZE: break;

            case CTUNE_UI_ACTION_HELP: { //Contextual help
                ctune_UI_ContextHelp.show( CTUNE_UI_CTX_RSEDIT );
                ctune_UI_ContextHelp.captureInput();
            } break;

            case CTUNE_UI_ACTION_ESC: {
                exit_state = CTUNE_UI_FORM_ESC;
                exit = true;
            } break;

            case CTUNE_UI_ACTION_FIELD_BEGIN: {
                form_driver( rsedit->form, REQ_BEG_FIELD );
            } break;

            case CTUNE_UI_ACTION_FIELD_END: {
                form_driver( rsedit->form, REQ_END_FIELD );
            } break;

            case CTUNE_UI_ACTION_FIELD_FIRST: {
                set_current_field( rsedit->form, rsedit->cache.fields[ LABEL_COUNT ] );
                ctune_UI_RSEdit_highlightCurrField( rsedit, current_field( rsedit->form ) );
                ctune_UI_Dialog.scrollTop( &rsedit->dialog );
                ctune_UI_RSEdit_autoScroll( rsedit, current_field( rsedit->form ) );
            } break;

            case CTUNE_UI_ACTION_FIELD_LAST: {
                set_current_field( rsedit->form, rsedit->cache.fields[ ( FIELD_LAST - 1 ) ] );
                ctune_UI_RSEdit_highlightCurrField( rsedit, current_field( rsedit->form ) );
                ctune_UI_Dialog.scrollBottom( &rsedit->dialog );
                ctune_UI_RSEdit_autoScroll( rsedit, current_field( rsedit->form ) );
            } break;

            case CTUNE_UI_ACTION_FIELD_PREV: {
                form_driver( rsedit->form, REQ_PREV_FIELD );
                ctune_UI_RSEdit_highlightCurrField( rsedit, current_field( rsedit->form ) );
                form_driver( rsedit->form, REQ_END_LINE );
                ctune_UI_RSEdit_autoScroll( rsedit, current_field( rsedit->form ) );
            } break;

            case CTUNE_UI_ACTION_FIELD_NEXT: {
                form_driver( rsedit->form, REQ_NEXT_FIELD );
                ctune_UI_RSEdit_highlightCurrField( rsedit, current_field( rsedit->form ) );
                form_driver( rsedit->form, REQ_END_LINE );
                ctune_UI_RSEdit_autoScroll( rsedit, current_field( rsedit->form ) );
            } break;

            case CTUNE_UI_ACTION_GO_LEFT: {
                form_driver( rsedit->form, REQ_LEFT_CHAR );
            } break;

            case CTUNE_UI_ACTION_GO_RIGHT: {
                form_driver( rsedit->form, REQ_RIGHT_CHAR );
            } break;

            case CTUNE_UI_ACTION_TRIGGER: {
                if( ctune_UI_RSEdit_autodetectButton( rsedit, current_field( rsedit->form ) ) ) {
                    ctune_UI_RSEdit_autodetectStreamProperties( rsedit, &rsedit->cache.station, current_field( rsedit->form ) );

                } else if( ( exit = ctune_UI_RSEdit_isExitState( rsedit, current_field( rsedit->form ), &exit_state ) ) ) {
                    if( exit_state == CTUNE_UI_FORM_SUBMIT ) {
                        if( ctune_UI_RSEdit_packFieldValues( rsedit, &rsedit->cache.station ) ) {
                            ctune_UI_RSEdit_setChangeTimestamp( &rsedit->cache.station );
                        } else {
                            exit = false;
                        }
                    }

                } else {
                    form_driver( rsedit->form, REQ_NEXT_FIELD );
                    ctune_UI_RSEdit_highlightCurrField( rsedit, current_field( rsedit->form ) );
                    form_driver( rsedit->form, REQ_END_LINE );
                }
            } break;

            case CTUNE_UI_ACTION_TOGGLE_ALT: { //'space'
                if( ctune_UI_RSEdit_autodetectButton( rsedit, current_field( rsedit->form ) ) ) {
                    ctune_UI_RSEdit_autodetectStreamProperties( rsedit, &rsedit->cache.station, current_field( rsedit->form ) );

                } else if( ( exit = ctune_UI_RSEdit_isExitState( rsedit, current_field( rsedit->form ), &exit_state ) ) ) {
                    if( exit_state == CTUNE_UI_FORM_SUBMIT ) {
                        if( ctune_UI_RSEdit_packFieldValues( rsedit, &rsedit->cache.station ) ) {
                            ctune_UI_RSEdit_setChangeTimestamp( &rsedit->cache.station );
                        } else {
                            exit = false;
                        }
                    }

                } else {
                    form_driver( rsedit->form, character );
                }
            } break;

            case CTUNE_UI_ACTION_CLEAR_ALL: {
                ctune_UI_RSEdit_clearAllFields( rsedit );
                ctune_UI_RSEdit_highlightCurrField( rsedit, current_field( rsedit->form ) );
            } break;

            case CTUNE_UI_ACTION_CLEAR_SELECTED: {
                form_driver( rsedit->form, REQ_CLR_FIELD );
            } break;

            case CTUNE_UI_ACTION_DEL_PREV: {
                form_driver( rsedit->form, REQ_DEL_PREV );
            } break;

            case CTUNE_UI_ACTION_DEL_NEXT: {
                form_driver( rsedit->form, REQ_DEL_CHAR );
            } break;

            default: {
                form_driver( rsedit->form, character );
            } break;
        }

        ctune_UI_ScrollWin.refreshView( &rsedit->dialog.canvas );
    }

    ctune_UI_Dialog.hide( &rsedit->dialog );
    ctune_UI_Resizer.pop();

    return ( exit_state );
}

/**
 * Gets the internal filter object
 * @param rsedit Pointer to a ctune_UI_RSEdit_t object
 * @return Pointer to internal RadioStationInfo_t DTO
 */
static ctune_RadioStationInfo_t * ctune_UI_RSEdit_getStation( ctune_UI_RSEdit_t * rsedit ) {
    return &rsedit->cache.station;
}


/**
 * De-allocates the form and its fields
 * @param rsedit Pointer to a ctune_UI_RSEdit_t object
 */
static void ctune_UI_RSEdit_free( ctune_UI_RSEdit_t * rsedit ) {
    unpost_form( rsedit->form );
    free_form( rsedit->form );

    for( int i = 0; i < FIELD_COUNT; i++ )
        free_field( rsedit->cache.fields[i] );

    regfree( &rsedit->cache.url_regex );

    ctune_UI_Dialog.free( &rsedit->dialog );
    ctune_RadioStationInfo.freeContent( &rsedit->cache.station );
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_RSEdit_free( %p )] RSEdit freed.", rsedit );
}


/**
 * Namespace constructor
 */
const struct ctune_UI_RSEdit_Namespace ctune_UI_RSEdit = {
    .create        = &ctune_UI_RSEdit_create,
    .init          = &ctune_UI_RSEdit_init,
    .isInitialised = &ctune_UI_RSEdit_isInitialised,
    .newStation    = &ctune_UI_RSEdit_newStation,
    .copyStation   = &ctune_UI_RSEdit_copyStation,
    .loadStation   = &ctune_UI_RSEdit_loadStation,
    .show          = &ctune_UI_RSEdit_show,
    .resize        = &ctune_UI_RSEdit_resize,
    .captureInput  = &ctune_UI_RSEdit_captureInput,
    .getStation    = &ctune_UI_RSEdit_getStation,
    .free          = &ctune_UI_RSEdit_free,
};