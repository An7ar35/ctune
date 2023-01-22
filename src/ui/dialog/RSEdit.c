#include "RSEdit.h"

#include <errno.h>
#include <string.h>
#include <regex.h>

#include "../definitions/KeyBinding.h"
#include "../definitions/Theme.h"
#include "ContextHelp.h"
#include "../../dto/RadioBrowserFilter.h" //for the max bitrate macro ("CTUNE_RADIOBROWSERFILTER_BITRATE_MAX_DFLT")

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
 * [PRIVATE] Check if field is a button
 * @param field_id Form field ID
 * @return Button field status
 */
static bool ctune_UI_RSEdit_isButton( int field_id ) {
    return ( field_id == BUTTON_AUTODETECT
          || field_id == BUTTON_CANCEL
          || field_id == BUTTON_SAVE );
}

/**
 * [PRIVATE] Highlights input field where the cursor is at
 * @param rsedit Pointer to a ctune_UI_RSEdit_t object
 */
static void ctune_UI_RSEdit_highlightCurrField( ctune_UI_RSEdit_t * rsedit ) {
    const int  curr_field_id    = ctune_UI_Form.field.currentIndex( &rsedit->form );
    const bool curr_is_editable = !( ctune_UI_RSEdit_isButton( curr_field_id ) );

    curs_set( ( curr_is_editable ? 1 : 0 ) );

    for( int i = 0; i < LABEL_COUNT; ++i ) {
        ctune_UI_Form.field.setBackground( &rsedit->form, i, A_NORMAL );
    }

    for( int i = LABEL_COUNT; i < FIELD_COUNT; ++i ) {
        if( i == curr_field_id ) {
            ctune_UI_Form.field.setBackground( &rsedit->form, i, A_REVERSE );

        } else {
            if( ctune_UI_RSEdit_isButton( i ) ) {
                ctune_UI_Form.field.setBackground( &rsedit->form, i, A_NORMAL );
            } else {
                ctune_UI_Form.field.setBackground( &rsedit->form, i, A_UNDERLINE );
            }
        }
    }
}

/**
 * [PRIVATE] Performs the auto-detect action for the associated button (current field)
 * @param rsedit Pointer to a ctune_UI_RSEdit_t object
 * @param rsi    Pointer to cached RadioStationInfo_t object
 */
static void ctune_UI_RSEdit_autodetectStreamProperties( ctune_UI_RSEdit_t * rsedit, ctune_RadioStationInfo_t * rsi ) {
    if( ctune_UI_Form.field.status( &rsedit->form, INPUT_URL ) ) {
        char * buffer = ctune_UI_Form.field.buffer( &rsedit->form, INPUT_URL );
        ctune_RadioStationInfo.set.stationURL( rsi, ctune_trimspace( buffer ) );
    }

    if( ctune_UI_Form.field.status( &rsedit->form, INPUT_RESOLVED_URL ) ) {
        char * buffer = ctune_UI_Form.field.buffer( &rsedit->form, INPUT_RESOLVED_URL );
        ctune_RadioStationInfo.set.resolvedURL( rsi, ctune_trimspace( buffer ) );
    }

    const int  current_field_i  = ctune_UI_Form.field.currentIndex( &rsedit->form );
    const bool has_url          = ( ctune_RadioStationInfo.get.stationURL( rsi )  != NULL && strlen( ctune_RadioStationInfo.get.stationURL( rsi ) )  > 0 );
    const bool has_url_resolved = ( ctune_RadioStationInfo.get.resolvedURL( rsi ) != NULL && strlen( ctune_RadioStationInfo.get.resolvedURL( rsi ) ) > 0 );

    if( !has_url_resolved && !has_url ) { //i.e.: no URLs
        ctune_UI_Form.field.setBackground( &rsedit->form, current_field_i, ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_DFLT    ) );
        ctune_UI_Form.field.setForeground( &rsedit->form, current_field_i, ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_INVALID ) );

        ctune_UI_Form.field.setCurrent( &rsedit->form, INPUT_RESOLVED_URL );
        ctune_UI_RSEdit_highlightCurrField( rsedit );

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

            ctune_UI_Form.field.setBackground( &rsedit->form, current_field_i, ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_DFLT  ) );
            ctune_UI_Form.field.setForeground( &rsedit->form, current_field_i, ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_VALID ) );

            ctune_strupr( codec._raw );
            ctune_UI_Form.field.setBuffer( &rsedit->form, INPUT_CODEC, ( codec._raw == NULL ? "" : codec._raw ) );

            if( rsi->bitrate > 0 ) { //Bitrate field (ulong)
                ctune_utos( rsi->bitrate, &bitrate );
                ctune_UI_Form.field.setBuffer( &rsedit->form, INPUT_BITRATE, bitrate._raw );
            }

            ctune_RadioStationInfo.set.lastCheckOK( rsi, true );
            ctune_RadioStationInfo.set.lastCheckOkTS( rsi, strdup( timestamp._raw ) );

        } else {
            ctune_UI_Form.field.setBackground( &rsedit->form, current_field_i, ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_DFLT    ) );
            ctune_UI_Form.field.setForeground( &rsedit->form, current_field_i, ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_INVALID ) );
        }

        ctune_RadioStationInfo.set.lastCheckTS( rsi, strdup( timestamp._raw ) );
        ctune_RadioStationInfo.set.lastLocalCheckTS( rsi, strdup( timestamp._raw ) );

    } else {
        ctune_UI_Form.field.setCurrent( &rsedit->form, ( has_url_resolved ? INPUT_RESOLVED_URL : INPUT_URL ) );
        ctune_UI_RSEdit_highlightCurrField( rsedit );
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
 * @param field_id  Field ID
 * @param exit_type Variable pointer to set the exit type of the field
 * @return Exit state
 */
static bool ctune_UI_RSEdit_isExitState( ctune_UI_RSEdit_t * rsedit, const int field_id, ctune_FormExit_e * exit_type ) {
    if( ctune_UI_RSEdit_isButton( field_id ) ) {
        if( field_id == BUTTON_CANCEL ) {
            *exit_type = CTUNE_UI_FORM_CANCEL;
            return true;
        }

        if( field_id == BUTTON_SAVE ) {
            *exit_type = CTUNE_UI_FORM_SUBMIT;
            return true;
        }
    }

    return false;
}

/**
 * [PRIVATE] Checks field values
 * @param rsedit Pointer to ctune_UI_RSEdit_t object
 * @param rsi    Pointer to the cached RadioStationInfo_t object
 * @return Validation state
 */
static bool ctune_UI_RSEdit_validate( ctune_UI_RSEdit_t * rsedit, const ctune_RadioStationInfo_t * rsi ) {
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_RSEdit_validateFieldValues( %p, %p )] Checking field values...", rsedit, rsi );

    bool error_state     = false;
    bool no_url          = ( ctune_RadioStationInfo.get.stationURL( rsi )  == NULL || strlen( ctune_RadioStationInfo.get.stationURL( rsi ) )  == 0 );
    bool no_url_resolved = ( ctune_RadioStationInfo.get.resolvedURL( rsi ) == NULL || strlen( ctune_RadioStationInfo.get.resolvedURL( rsi ) ) == 0 );

    if( no_url && no_url_resolved ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_RSEdit_validate( %p, %p )] Failed validation: URL",
                   rsedit, rsi
        );

        error_state = true;
        ctune_UI_Form.field.setCurrent( &rsedit->form, INPUT_RESOLVED_URL );

    } else {
        if( !no_url && !rsedit->cb.validateURL( rsi->url ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_RSEdit_validate( %p, %p )] Failed URL validation: %s",
                       rsedit, rsi, rsi->url
            );

            error_state = true;
            ctune_UI_Form.field.setCurrent( &rsedit->form, INPUT_URL );

        } else if( !no_url_resolved && !rsedit->cb.validateURL( rsi->url_resolved ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_RSEdit_validate( %p, %p )] Failed resolved URL validation: %s",
                       rsedit, rsi, rsi->url
            );

            error_state = true;
            ctune_UI_Form.field.setCurrent( &rsedit->form, INPUT_RESOLVED_URL );
        }
    }

    if( ctune_RadioStationInfo.get.stationName( rsi ) == NULL || strlen( ctune_RadioStationInfo.get.stationName( rsi ) ) == 0 ) { //required field
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_RSEdit_validate( %p, %p )] Failed name validation: empty/NULL",
                   rsedit, rsi
        );

        error_state = true;
        ctune_UI_Form.field.setCurrent( &rsedit->form, INPUT_NAME );
    }

    ctune_UI_RSEdit_highlightCurrField( rsedit );

    return !( error_state );
}

/**
 * [PRIVATE] Packs all the field values into the cached RSI object (as LOCAL station)
 * @param rsedit Pointer to ctune_UI_RSEdit_t object
 * @param rsi    Pointer to the cached RadioStationInfo_t object
 * @return Validated state
 */
static bool ctune_UI_RSEdit_packFieldValues( ctune_UI_RSEdit_t * rsedit, ctune_RadioStationInfo_t * rsi ) {
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_RSEdit_packFieldValues( %p, %p )] Packing field values...", rsedit, rsi );

    bool error_state = false;

    ctune_RadioStationInfo.set.stationSource( rsi, CTUNE_STATIONSRC_LOCAL );

    if( ctune_UI_Form.field.status( &rsedit->form, INPUT_NAME ) ) { //required field
        const char * buffer = ctune_UI_Form.field.buffer( &rsedit->form, INPUT_NAME );
        ctune_RadioStationInfo.set.stationName( rsi, ctune_trimspace( buffer ) );
    }

    if( ctune_UI_Form.field.status( &rsedit->form, INPUT_URL ) ) { //required field *
        const char * buffer = ctune_UI_Form.field.buffer( &rsedit->form, INPUT_URL );
        ctune_RadioStationInfo.set.stationURL( rsi, ctune_trimspace( buffer ) );
    }

    if( ctune_UI_Form.field.status( &rsedit->form, INPUT_RESOLVED_URL ) ) { //required field *
        const char * buffer = ctune_UI_Form.field.buffer( &rsedit->form, INPUT_RESOLVED_URL );
        ctune_RadioStationInfo.set.resolvedURL( rsi, ctune_trimspace( buffer ) );
    }

    if( ctune_UI_Form.field.status( &rsedit->form, INPUT_HOMEPAGE ) ) {
        const char * buffer = ctune_UI_Form.field.buffer( &rsedit->form, INPUT_HOMEPAGE );
        ctune_RadioStationInfo.set.homepage( rsi, ctune_trimspace( buffer ) );
    }

    if( ctune_UI_Form.field.status( &rsedit->form, INPUT_TAGS ) ) {
        StrList_t    tag_list  = StrList.init();
        String_t     tmp       = String.init();
        const char * buffer    = ctune_UI_Form.field.buffer( &rsedit->form, INPUT_TAGS );
        const size_t val_count = ctune_splitcss( buffer, &tag_list );

        CTUNE_LOG( CTUNE_LOG_DEBUG,
                   "[ctune_UI_RSEdit_packFieldValues( %p, %p )] Found %i values in field 'INPUT_TAGS': \"%s\"",
                   rsedit, rsi, val_count, buffer
        );

        if( val_count == 0 ) {
            ctune_RadioStationInfo.set.tags( rsi, NULL );

        } else if( val_count == 1 ) {
            ctune_RadioStationInfo.set.tags( rsi, strdup( StrList.at( &tag_list, 0 )->data ) );

            if( rsi->tags == NULL ) {
                error_state = true;

                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_UI_RSEdit_packFieldValues( %p, %p )] Failed strdup of tag.",
                           rsedit, rsi
                );
            }

        } else { //val_count > 1
            size_t ret_count = StrList.stringify( &tag_list, &tmp, ',' );

            if( ret_count != val_count ) {
                error_state = true;

                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_UI_RSEdit_packFieldValues( %p, %p )] Error parsing all tags (%lu/%lu).",
                           rsedit, rsi, ret_count, val_count
                );
            }

            ctune_RadioStationInfo.set.tags( rsi, strdup( tmp._raw ) );

            if( rsi->tags == NULL ) {
                error_state = true;

                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_UI_RSEdit_packFieldValues( %p, %p )] Failed strdup of tags.",
                           rsedit, rsi
                );
            };
        }

        String.free( &tmp );
        StrList.free_strlist( &tag_list );
    }

    if( ctune_UI_Form.field.status( &rsedit->form, INPUT_COUNTRY ) ) {
        const char * buffer = ctune_UI_Form.field.buffer( &rsedit->form, INPUT_COUNTRY );
        ctune_RadioStationInfo.set.country( rsi, ctune_trimspace( buffer ) );
    }

    if( ctune_UI_Form.field.status( &rsedit->form, INPUT_COUNTRY_CODE ) ) {
        const char * buffer = ctune_UI_Form.field.buffer( &rsedit->form, INPUT_COUNTRY );
        char       * cc     = ctune_trimspace( buffer );

        if( cc != NULL ) {
            ctune_RadioStationInfo.set.countryCode_ISO3166_1( rsi, cc );
            free( cc );
        }
    }

    if( ctune_UI_Form.field.status( &rsedit->form, INPUT_STATE ) ) {
        const char * buffer = ctune_UI_Form.field.buffer( &rsedit->form, INPUT_STATE );
        ctune_RadioStationInfo.set.state( rsi, ctune_trimspace( buffer ) );
    }

    if( ctune_UI_Form.field.status( &rsedit->form, INPUT_LANGUAGE ) ) {
        const char * buffer = ctune_UI_Form.field.buffer( &rsedit->form, INPUT_LANGUAGE );
        ctune_RadioStationInfo.set.language( rsi, ctune_trimspace( buffer ) );
    }

    if( ctune_UI_Form.field.status( &rsedit->form, INPUT_LANGUAGE_CODES ) ) {
        StrList_t    list      = StrList.init();
        String_t     tmp       = String.init();
        const char * buffer    = ctune_UI_Form.field.buffer( &rsedit->form, INPUT_LANGUAGE_CODES );
        const size_t val_count = ctune_splitcss( buffer, &list );

        CTUNE_LOG( CTUNE_LOG_DEBUG,
                   "[ctune_UI_RSEdit_packFieldValues( %p, %p )] Found %i values in field 'INPUT_LANGUAGE_CODES': \"%s\"",
                   rsedit, rsi, val_count, buffer
        );

        if( val_count == 0 ) {
            ctune_RadioStationInfo.set.language( rsi, NULL );

        } else if( val_count == 1 ) {
            ctune_RadioStationInfo.set.language( rsi, strdup( StrList.at( &list, 0 )->data ) );

            if( rsi->language_codes == NULL ) {
                error_state = true;

                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_UI_RSEdit_packFieldValues( %p, %p )] Failed strdup of language code.",
                           rsedit, rsi
                );
            }

        } else { //val_count > 1
            size_t ret_count = StrList.stringify( &list, &tmp, ',' );

            if( ret_count != val_count ) {
                error_state = true;

                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_UI_RSEdit_packFieldValues( %p, %p )] Error parsing all language codes (%lu/%lu).",
                           rsedit, rsi, ret_count, val_count
                );
            }

            ctune_RadioStationInfo.set.language( rsi, strdup( tmp._raw ) );

            if( rsi->tags == NULL ) {
                error_state = true;

                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_UI_RSEdit_packFieldValues( %p, %p )] Failed strdup of language codes.",
                           rsedit, rsi
                );
            };
        }

        String.free( &tmp );
        StrList.free_strlist( &list );
    }

    if( ctune_UI_Form.field.status( &rsedit->form, INPUT_CODEC ) ) {
        const char * buffer = ctune_UI_Form.field.buffer( &rsedit->form, INPUT_CODEC );
        ctune_RadioStationInfo.set.codec( rsi, ctune_trimspace( buffer ) );
    }

    if( ctune_UI_Form.field.status( &rsedit->form, INPUT_BITRATE ) ) {
        const char * buffer = ctune_UI_Form.field.buffer( &rsedit->form, INPUT_BITRATE );
        const ulong bitrate = strtoul( buffer, NULL, 10 );
        ctune_RadioStationInfo.set.bitrate( rsi, bitrate );
    }

    if( ctune_UI_Form.field.status( &rsedit->form, INPUT_COORDINATE_LAT ) || ctune_UI_Form.field.status( &rsedit->form, INPUT_COORDINATE_LONG ) ) {
        char * latitude_str  = ctune_trimspace( ctune_UI_Form.field.buffer( &rsedit->form, INPUT_COORDINATE_LAT  ) );
        char * longitude_str = ctune_trimspace( ctune_UI_Form.field.buffer( &rsedit->form, INPUT_COORDINATE_LONG ) );
        double latitude_val  = 0.0;
        double longitude_val = 0.0;

        if( latitude_str != NULL ) {
            latitude_val = strtod( latitude_str, NULL );

            if( errno == ERANGE ) {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_UI_RSEdit_packFieldValues( %p, %p )] Failed string->`double` conversion of latitude ('%s').",
                           rsedit, rsi, latitude_str
                );

            }

            free( latitude_str );
        }

        if( longitude_str != NULL ) {
            longitude_val = strtod( longitude_str, NULL );

            if( errno == ERANGE ) {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_UI_RSEdit_packFieldValues( %p, %p )] Failed string->`double` conversion of longitude ('%s').",
                           rsedit, rsi, longitude_str
                );

            }

            free( longitude_str );
        }

        ctune_RadioStationInfo.set.geoCoordinates( rsi, latitude_val, longitude_val );
    }

    //check required fields are present
    if( !ctune_UI_RSEdit_validate( rsedit, &rsedit->cache.station ) ) {
        error_state = true;
    }

    if( error_state ) {
        CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_RSEdit_packFieldValues( %p, %p )] Field validation failed.", rsedit, rsi );
    } else {
        CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_RSEdit_packFieldValues( %p, %p )] Field values packed.", rsedit, rsi );
    }

    return !( error_state );
}

/**
 * [PRIVATE] Initialises cached filter and fields
 * @param rsedit Pointer to ctune_UI_RSEdit_t object
 */
static void ctune_UI_RSEdit_initFields( ctune_UI_RSEdit_t * rsedit ) {
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_RSEdit_initFields( %p )] Initialising form fields...", rsedit );

    const ctune_RadioStationInfo_t * rsi = &rsedit->cache.station; //shortcut ptr

    ctune_UI_Form.field.setBuffer( &rsedit->form, LABEL_UUID,           rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_STATION_UUID ) );
    ctune_UI_Form.field.setBuffer( &rsedit->form, LABEL_NAME,           rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_STATION_NAME ) );
    ctune_UI_Form.field.setBuffer( &rsedit->form, LABEL_URL,            rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_URL ) );
    ctune_UI_Form.field.setBuffer( &rsedit->form, LABEL_RESOLVED_URL,   rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_URL_RESOLVED ) );
    ctune_UI_Form.field.setBuffer( &rsedit->form, LABEL_HOMEPAGE,       rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_URL_HOMEPAGE ) );
    ctune_UI_Form.field.setBuffer( &rsedit->form, LABEL_TAGS,           rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_TAGS ) );
    ctune_UI_Form.field.setBuffer( &rsedit->form, LABEL_COUNTRY,        rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_COUNTRY ) );
    ctune_UI_Form.field.setBuffer( &rsedit->form, LABEL_COUNTRY_CODE,   rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_COUNTRY_CODE ) );
    ctune_UI_Form.field.setBuffer( &rsedit->form, LABEL_STATE,          rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_STATE ) );
    ctune_UI_Form.field.setBuffer( &rsedit->form, LABEL_LANGUAGE,       rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_LANGUAGE ) );
    ctune_UI_Form.field.setBuffer( &rsedit->form, LABEL_LANGUAGE_CODES, rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_LANGUAGE_CODES ) );
    ctune_UI_Form.field.setBuffer( &rsedit->form, LABEL_CODEC,          rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_CODEC ) );
    ctune_UI_Form.field.setBuffer( &rsedit->form, LABEL_BITRATE,        rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_BITRATE ) );
    ctune_UI_Form.field.setBuffer( &rsedit->form, LABEL_BITRATE_UNIT,   rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_BITRATE_UNIT_LONG ) );
    ctune_UI_Form.field.setBuffer( &rsedit->form, LABEL_COORDINATES,    rsedit->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_GEO_COORDS ) );

    ctune_UI_Form.field.setBuffer( &rsedit->form, FIELD_UUID,           ctune_fallbackStr( ctune_RadioStationInfo.get.stationUUID( rsi ), "" ) );
    ctune_UI_Form.field.setBuffer( &rsedit->form, INPUT_NAME,           ctune_fallbackStr( ctune_RadioStationInfo.get.stationName( rsi ), "" ) );
    ctune_UI_Form.field.setBuffer( &rsedit->form, INPUT_URL,            ctune_fallbackStr( ctune_RadioStationInfo.get.stationURL( rsi ), "" ) );
    ctune_UI_Form.field.setBuffer( &rsedit->form, INPUT_RESOLVED_URL,   ctune_fallbackStr( ctune_RadioStationInfo.get.resolvedURL( rsi ), "" ) );
    ctune_UI_Form.field.setBuffer( &rsedit->form, INPUT_HOMEPAGE,       ctune_fallbackStr( ctune_RadioStationInfo.get.homepage( rsi ), "" ) );
    ctune_UI_Form.field.setBuffer( &rsedit->form, INPUT_TAGS,           ctune_fallbackStr( ctune_RadioStationInfo.get.tags( rsi ), "" ) );
    ctune_UI_Form.field.setBuffer( &rsedit->form, INPUT_COUNTRY,        ctune_fallbackStr( ctune_RadioStationInfo.get.country( rsi ), "" ) );
    ctune_UI_Form.field.setBuffer( &rsedit->form, INPUT_COUNTRY_CODE,   ctune_fallbackStr( ctune_RadioStationInfo.get.countryCode_ISO3166_1( rsi ), "" ) );
    ctune_UI_Form.field.setBuffer( &rsedit->form, INPUT_STATE,          ctune_fallbackStr( ctune_RadioStationInfo.get.state( rsi ), "" ) );
    ctune_UI_Form.field.setBuffer( &rsedit->form, INPUT_LANGUAGE,       ctune_fallbackStr( ctune_RadioStationInfo.get.language( rsi ), "" ) );
    ctune_UI_Form.field.setBuffer( &rsedit->form, INPUT_LANGUAGE_CODES, ctune_fallbackStr( ctune_RadioStationInfo.get.languageCodes( rsi ), "" ) );
    ctune_UI_Form.field.setBuffer( &rsedit->form, INPUT_CODEC,          ctune_fallbackStr( ctune_RadioStationInfo.get.codec( rsi ), "" ) );

    { //Bitrate field (ulong)
        String_t bitrate = String.init();
        ctune_utos( ctune_RadioStationInfo.get.bitrate( rsi ), &bitrate );
        ctune_UI_Form.field.setBuffer( &rsedit->form, INPUT_BITRATE, bitrate._raw );
        String.free( &bitrate );
    }

    { //geo coordinates field (double, double)
        String_t latitude  = String.init();
        String_t longitude = String.init();
        ctune_ftos( ctune_RadioStationInfo.get.geoLatitude( rsi ), &latitude );
        ctune_ftos( ctune_RadioStationInfo.get.geoLongitude( rsi ), &longitude );
        ctune_UI_Form.field.setBuffer( &rsedit->form, INPUT_COORDINATE_LAT , latitude._raw  );
        ctune_UI_Form.field.setBuffer( &rsedit->form, INPUT_COORDINATE_LONG, longitude._raw );
        String.free( &latitude  );
        String.free( &longitude );
    }

    ctune_UI_Form.field.setBuffer( &rsedit->form, BUTTON_AUTODETECT, rsedit->cb.getDisplayText( CTUNE_UI_TEXT_BUTTON_AUTODETECT_STREAM ) );
    ctune_UI_Form.field.setBuffer( &rsedit->form, BUTTON_CANCEL,     rsedit->cb.getDisplayText( CTUNE_UI_TEXT_BUTTON_CANCEL ) );
    ctune_UI_Form.field.setBuffer( &rsedit->form, BUTTON_SAVE,       rsedit->cb.getDisplayText( CTUNE_UI_TEXT_BUTTON_SAVE ) );

    ctune_UI_Form.field.setBackground( &rsedit->form, BUTTON_AUTODETECT, ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_DFLT ) );
    ctune_UI_Form.field.setForeground( &rsedit->form, BUTTON_AUTODETECT, ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_DFLT ) );
    ctune_UI_Form.field.setBackground( &rsedit->form, BUTTON_CANCEL,     ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_DFLT ) );
    ctune_UI_Form.field.setForeground( &rsedit->form, BUTTON_CANCEL,     ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_DFLT ) );
    ctune_UI_Form.field.setBackground( &rsedit->form, BUTTON_SAVE,       ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_DFLT ) );
    ctune_UI_Form.field.setForeground( &rsedit->form, BUTTON_SAVE,       ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_DFLT ) );
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

    const int row_height              = 1;
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

    bool ret[FIELD_LAST]; //errors
    //Fields labels                                                                                                    rows        cols                 y   x
    ret[LABEL_UUID           ] = ctune_UI_Form.field.create( &rsedit->form, LABEL_UUID,            (WindowProperty_t){ row_height, label_col_width,      0, label_col } );
    ret[LABEL_NAME           ] = ctune_UI_Form.field.create( &rsedit->form, LABEL_NAME,            (WindowProperty_t){ row_height, label_col_width,      2, label_col } );
    ret[LABEL_URL            ] = ctune_UI_Form.field.create( &rsedit->form, LABEL_URL,             (WindowProperty_t){ row_height, label_col_width,      4, label_col } );
    ret[LABEL_RESOLVED_URL   ] = ctune_UI_Form.field.create( &rsedit->form, LABEL_RESOLVED_URL,    (WindowProperty_t){ row_height, label_col_width,      6, label_col } );
    ret[LABEL_HOMEPAGE       ] = ctune_UI_Form.field.create( &rsedit->form, LABEL_HOMEPAGE,        (WindowProperty_t){ row_height, label_col_width,      8, label_col } );
    ret[LABEL_TAGS           ] = ctune_UI_Form.field.create( &rsedit->form, LABEL_TAGS,            (WindowProperty_t){ row_height, label_col_width,     10, label_col } );
    ret[LABEL_COUNTRY        ] = ctune_UI_Form.field.create( &rsedit->form, LABEL_COUNTRY,         (WindowProperty_t){ row_height, label_col_width,     12, label_col } );
    ret[LABEL_COUNTRY_CODE   ] = ctune_UI_Form.field.create( &rsedit->form, LABEL_COUNTRY_CODE,    (WindowProperty_t){ row_height, label_col_width,     14, label_col } );
    ret[LABEL_STATE          ] = ctune_UI_Form.field.create( &rsedit->form, LABEL_STATE,           (WindowProperty_t){ row_height, label_col_width,     16, label_col } );
    ret[LABEL_LANGUAGE       ] = ctune_UI_Form.field.create( &rsedit->form, LABEL_LANGUAGE,        (WindowProperty_t){ row_height, label_col_width,     18, label_col } );
    ret[LABEL_LANGUAGE_CODES ] = ctune_UI_Form.field.create( &rsedit->form, LABEL_LANGUAGE_CODES,  (WindowProperty_t){ row_height, label_col_width,     20, label_col } );
    ret[LABEL_CODEC          ] = ctune_UI_Form.field.create( &rsedit->form, LABEL_CODEC,           (WindowProperty_t){ row_height, label_col_width,     22, label_col } );
    ret[LABEL_BITRATE        ] = ctune_UI_Form.field.create( &rsedit->form, LABEL_BITRATE,         (WindowProperty_t){ row_height, label_col_width,     24, label_col } );
    ret[LABEL_BITRATE_UNIT   ] = ctune_UI_Form.field.create( &rsedit->form, LABEL_BITRATE_UNIT,    (WindowProperty_t){ row_height, bitrate_unit_width,  24, bitrate_unit_col } );
    ret[LABEL_COORDINATES    ] = ctune_UI_Form.field.create( &rsedit->form, LABEL_COORDINATES,     (WindowProperty_t){ row_height, label_col_width,     26, label_col } );
    ret[FIELD_UUID           ] = ctune_UI_Form.field.create( &rsedit->form, FIELD_UUID,            (WindowProperty_t){ row_height, std_field_width,      0, field_col } );
    //Field inputs                                                                                                     rows        cols                y   x
    ret[INPUT_NAME           ] = ctune_UI_Form.field.create( &rsedit->form, INPUT_NAME,            (WindowProperty_t){ row_height, std_field_width,      2, field_col } );
    ret[INPUT_URL            ] = ctune_UI_Form.field.create( &rsedit->form, INPUT_URL,             (WindowProperty_t){ row_height, std_field_width,      4, field_col } );
    ret[INPUT_RESOLVED_URL   ] = ctune_UI_Form.field.create( &rsedit->form, INPUT_RESOLVED_URL,    (WindowProperty_t){ row_height, std_field_width,      6, field_col } );
    ret[INPUT_HOMEPAGE       ] = ctune_UI_Form.field.create( &rsedit->form, INPUT_HOMEPAGE,        (WindowProperty_t){ row_height, std_field_width,      8, field_col } );
    ret[INPUT_TAGS           ] = ctune_UI_Form.field.create( &rsedit->form, INPUT_TAGS,            (WindowProperty_t){ row_height, std_field_width,     10, field_col } );
    ret[INPUT_COUNTRY        ] = ctune_UI_Form.field.create( &rsedit->form, INPUT_COUNTRY,         (WindowProperty_t){ row_height, std_field_width,     12, field_col } );
    ret[INPUT_COUNTRY_CODE   ] = ctune_UI_Form.field.create( &rsedit->form, INPUT_COUNTRY_CODE,    (WindowProperty_t){ row_height, cc_field_width,      14, field_col } );
    ret[INPUT_STATE          ] = ctune_UI_Form.field.create( &rsedit->form, INPUT_STATE,           (WindowProperty_t){ row_height, std_field_width,     16, field_col } );
    ret[INPUT_LANGUAGE       ] = ctune_UI_Form.field.create( &rsedit->form, INPUT_LANGUAGE,        (WindowProperty_t){ row_height, std_field_width,     18, field_col } );
    ret[INPUT_LANGUAGE_CODES ] = ctune_UI_Form.field.create( &rsedit->form, INPUT_LANGUAGE_CODES,  (WindowProperty_t){ row_height, std_field_width,     20, field_col } );
    ret[INPUT_CODEC          ] = ctune_UI_Form.field.create( &rsedit->form, INPUT_CODEC,           (WindowProperty_t){ row_height, codec_field_width,   22, field_col } );
    ret[INPUT_BITRATE        ] = ctune_UI_Form.field.create( &rsedit->form, INPUT_BITRATE,         (WindowProperty_t){ row_height, bitrate_field_width, 24, field_col } );
    ret[INPUT_COORDINATE_LAT ] = ctune_UI_Form.field.create( &rsedit->form, INPUT_COORDINATE_LAT,  (WindowProperty_t){ row_height, bitrate_block_width, 26, field_col } );
    ret[INPUT_COORDINATE_LONG] = ctune_UI_Form.field.create( &rsedit->form, INPUT_COORDINATE_LONG, (WindowProperty_t){ row_height, bitrate_block_width, 26, latitude_field_col } );

    const int button_separation     = 6;
    const int max_button_width      = (int) ctune_max_ul( strlen( rsedit->cb.getDisplayText( CTUNE_UI_TEXT_BUTTON_SAVE ) ),
                                                          strlen( rsedit->cb.getDisplayText( CTUNE_UI_TEXT_BUTTON_CANCEL ) ) );
    const int button_line_ln        = max_button_width + button_separation + max_button_width;
    int       button_line_pad       = 0;

    if( button_line_ln < form_width ) {
        button_line_pad = ( ( form_width - button_line_ln ) / 2 ) + 1;
    }
    //[ Buttons ]                                          rows                cols                     y                                     x
    ret[BUTTON_AUTODETECT] = ctune_UI_Form.field.create( &rsedit->form, BUTTON_AUTODETECT, (WindowProperty_t){ row_height, autodetect_button_width, 23, autodetect_button_col } );
    ret[BUTTON_CANCEL    ] = ctune_UI_Form.field.create( &rsedit->form, BUTTON_CANCEL,     (WindowProperty_t){ row_height, max_button_width,        28, button_line_pad } );
    ret[BUTTON_SAVE      ] = ctune_UI_Form.field.create( &rsedit->form, BUTTON_SAVE,       (WindowProperty_t){ row_height, max_button_width,        28, ( button_line_pad + max_button_width + button_separation ) } );

    for( int i = 0; i < FIELD_LAST; ++i ) {
        if( !ret[i] ) {
            return false; //EARLY RETURN
        }
    }

    ctune_UI_Form.field.setOptions( &rsedit->form, LABEL_UUID,           O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    ctune_UI_Form.field.setOptions( &rsedit->form, LABEL_NAME,           O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    ctune_UI_Form.field.setOptions( &rsedit->form, LABEL_URL,            O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    ctune_UI_Form.field.setOptions( &rsedit->form, LABEL_RESOLVED_URL,   O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    ctune_UI_Form.field.setOptions( &rsedit->form, LABEL_TAGS,           O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    ctune_UI_Form.field.setOptions( &rsedit->form, LABEL_HOMEPAGE,       O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    ctune_UI_Form.field.setOptions( &rsedit->form, LABEL_COUNTRY,        O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    ctune_UI_Form.field.setOptions( &rsedit->form, LABEL_COUNTRY_CODE,   O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    ctune_UI_Form.field.setOptions( &rsedit->form, LABEL_STATE,          O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    ctune_UI_Form.field.setOptions( &rsedit->form, LABEL_LANGUAGE,       O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    ctune_UI_Form.field.setOptions( &rsedit->form, LABEL_LANGUAGE_CODES, O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    ctune_UI_Form.field.setOptions( &rsedit->form, LABEL_CODEC,          O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    ctune_UI_Form.field.setOptions( &rsedit->form, LABEL_BITRATE,        O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    ctune_UI_Form.field.setOptions( &rsedit->form, LABEL_BITRATE_UNIT,   O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    ctune_UI_Form.field.setOptions( &rsedit->form, LABEL_COORDINATES,    O_VISIBLE | O_PUBLIC | O_AUTOSKIP );

    ctune_UI_Form.field.setOptions( &rsedit->form, FIELD_UUID,           O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    ctune_UI_Form.field.setOptions( &rsedit->form, INPUT_NAME,           O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE );
    ctune_UI_Form.field.setOptions( &rsedit->form, INPUT_URL,            O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE );
    ctune_UI_Form.field.setOptions( &rsedit->form, INPUT_RESOLVED_URL,   O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE );
    ctune_UI_Form.field.setOptions( &rsedit->form, INPUT_HOMEPAGE,       O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE );
    ctune_UI_Form.field.setOptions( &rsedit->form, INPUT_TAGS,           O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE );
    ctune_UI_Form.field.setOptions( &rsedit->form, INPUT_COUNTRY,        O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE );
    ctune_UI_Form.field.setOptions( &rsedit->form, INPUT_COUNTRY_CODE,   O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE | O_INPUT_LIMIT | O_NULLOK | O_STATIC );
    set_field_type( ctune_UI_Form.field.get( &rsedit->form, INPUT_COUNTRY_CODE ), TYPE_ALPHA, 2 );
    ctune_UI_Form.field.setOptions( &rsedit->form, INPUT_STATE,          O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE );
    ctune_UI_Form.field.setOptions( &rsedit->form, INPUT_LANGUAGE,       O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE );
    ctune_UI_Form.field.setOptions( &rsedit->form, INPUT_CODEC,          O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    ctune_UI_Form.field.setOptions( &rsedit->form, INPUT_BITRATE,        O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    set_field_type( ctune_UI_Form.field.get( &rsedit->form, INPUT_BITRATE ), TYPE_INTEGER, 0, 0, CTUNE_RADIOBROSWERFILTER_BITRATE_MAX_DFLT );
    set_field_type( ctune_UI_Form.field.get( &rsedit->form, INPUT_COORDINATE_LAT ), TYPE_NUMERIC, 6, 0, 0 );
    set_field_type( ctune_UI_Form.field.get( &rsedit->form, INPUT_COORDINATE_LONG ), TYPE_NUMERIC, 6, 0, 0 );

    ctune_UI_Form.field.setOptions( &rsedit->form, BUTTON_AUTODETECT,    O_VISIBLE | O_PUBLIC | O_ACTIVE | O_STATIC );
    ctune_UI_Form.field.setOptions( &rsedit->form, BUTTON_CANCEL,        O_VISIBLE | O_PUBLIC | O_ACTIVE | O_STATIC );
    ctune_UI_Form.field.setOptions( &rsedit->form, BUTTON_SAVE,          O_VISIBLE | O_PUBLIC | O_ACTIVE | O_STATIC );

    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_RSEdit_createFields( %p )] Fields created.", rsedit );
    return true;
}

/**
 * [PRIVATE] Initialises the Form (callback)
 * @return Success
 */
static bool ctune_UI_RSEdit_initForm( void * rsedit ) {
    ctune_UI_RSEdit_t * rse_ptr = rsedit;

    if( rse_ptr->cache.max_label_width == 0 ) {
        if( !ctune_UI_RSEdit_createFields( rsedit ) ) {
            CTUNE_LOG( CTUNE_LOG_FATAL,
                       "[ctune_UI_RSEdit_initForm( %p )] Failed to create fields for the form.",
                       rsedit
            );

            return false; //EARLY RETURN
        }
    }

    ctune_UI_RSEdit_initFields( rse_ptr );

    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_UI_RSEdit_initForm( %p )] Form desc field size = %i",
               rsedit, rse_ptr->cache.max_label_width
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
                                                 bool         (* validateURL)( const char * ) )
{
    return (ctune_UI_RSEdit_t) {
        .initialised = false,
        .form        = ctune_UI_Form.create( parent, getDisplayText( CTUNE_UI_TEXT_WIN_TITLE_RSEDIT ) ),
        .cache = {
            .max_label_width = 0,
        },
        .cb = {
            .getDisplayText = getDisplayText,
            .generateUUID   = generateUUID,
            .testStream     = testStream,
            .validateURL    = validateURL
        },
    };
}

/**
 * Initialises RSEdit (mostly checks base values are OK)
 * @param rsedit     Pointer to a ctune_UI_RSEdit_t object
 * @param mouse_ctrl Flag to turn init mouse controls
 * @return Success
 */
static bool ctune_UI_RSEdit_init( ctune_UI_RSEdit_t * rsedit, bool mouse_ctrl ) {
    if( ctune_UI_Form.init( &rsedit->form, mouse_ctrl, FIELD_COUNT ) ) {
        ctune_UI_Form.scrolling.setAutoScroll( &rsedit->form, 2, 22 );
        ctune_UI_Form.setFormInitCallback( &rsedit->form, rsedit, ctune_UI_RSEdit_initForm );

        bool error_state = false;

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
        }

        return ( rsedit->initialised = !error_state );
    }

    CTUNE_LOG( CTUNE_LOG_ERROR,
               "[ctune_UI_RSEdit_init( %p, %s )] Failed to initialise!",
               rsedit, ( mouse_ctrl ? "true" : "false" )
    );

    return false;
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
 * Switch mouse control UI on/off
 * @param rsedit          Pointer to ctune_UI_RSEdit_t object
 * @param mouse_ctrl_flag Flag to turn feature on/off
 */
static void ctune_UI_RSEdit_setMouseCtrl( ctune_UI_RSEdit_t * rsedit, bool mouse_ctrl_flag ) {
    ctune_UI_Form.mouse.setMouseCtrl( &rsedit->form, mouse_ctrl_flag );
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
    if( !ctune_UI_Form.display.show( &rsedit->form ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_RSEdit_show( %p )] Failed to show Form.", rsedit );
        return false; //EARLY RETURN
    }

    return true;
}

/**
 * [PRIVATE] Handle a mouse event
 * @param rsedit Pointer to ctune_UI_RSEdit_t object
 * @param event      Mouse event mask
 * @param exit_state Pointer to Form exit state variable
 * @return Exit request
 */
static bool ctune_UI_RSEdit_handleMouseEvent( ctune_UI_RSEdit_t * rsedit, MEVENT * event, ctune_FormExit_e * exit_state ) {
    const ctune_UI_WinCtrlMask_m win_ctrl = ctune_UI_Form.mouse.isWinCtrl( &rsedit->form, event->y, event->x );
    const ctune_UI_ScrollMask_m  scroll   = ctune_UI_WinCtrlMask.scrollMask( win_ctrl );

    if( win_ctrl ) {
        if( scroll ) {
            if( event->bstate & BUTTON1_CLICKED ) {
                ctune_UI_Form.scrolling.incrementalScroll( &rsedit->form, scroll );

            } else if( event->bstate & BUTTON1_DOUBLE_CLICKED ) {
                ctune_UI_Form.scrolling.incrementalScroll( &rsedit->form, ctune_UI_ScrollMask.setScrollFactor( scroll, 2 ) );

            } else if( event->bstate & BUTTON1_TRIPLE_CLICKED ) {
                ctune_UI_Form.scrolling.incrementalScroll( &rsedit->form, ctune_UI_ScrollMask.setScrollFactor( scroll, 3 ) );

            } else if( event->bstate & BUTTON3_CLICKED ) {
                ctune_UI_Form.scrolling.edgeScroll( &rsedit->form, scroll );
            }

        } else if( win_ctrl & CTUNE_UI_WINCTRLMASK_CLOSE ) {
            if( event->bstate & BUTTON1_CLICKED ) {
                return true; //EARLY RETURN
            }
        }

        return false; //EARLY RETURN
    }

    bool exit = false;
    int  pos  = 0;

    FIELD *   prev_selected_field = ctune_UI_Form.field.current( &rsedit->form );
    FIELD *   clicked_field       = ctune_UI_Form.mouse.click( &rsedit->form, LABEL_COUNT, FIELD_LAST, event->y, event->x, &pos );
    const int clicked_field_id    = ctune_UI_Form.field.currentIndex( &rsedit->form );

    //TODO sort out issue with url and resolved_url field only being underlined when clicked on the first time

    if( clicked_field ) {
        ctune_UI_RSEdit_highlightCurrField( rsedit );
        ctune_UI_Form.display.refreshView( &rsedit->form );

        if( ctune_UI_RSEdit_isButton( clicked_field_id ) ) {
            if( clicked_field_id == BUTTON_AUTODETECT ) {
                ctune_UI_RSEdit_autodetectStreamProperties( rsedit, &rsedit->cache.station );

            } else {
                exit = ctune_UI_RSEdit_isExitState( rsedit, clicked_field_id, exit_state );
            }

        } else if( prev_selected_field == clicked_field ) { //same editable field
            ctune_UI_Form.input.fwdToFormDriver( &rsedit->form, REQ_BEG_FIELD );

            for( int i = 0; i < pos; ++i ) {
                ctune_UI_Form.input.fwdToFormDriver( &rsedit->form, REQ_RIGHT_CHAR );
            }

        } else {
            ctune_UI_RSEdit_highlightCurrField( rsedit );
        }
    }

    return exit;
}

/**
 * Pass keyboard input to the form
 * @param rsedit Pointer to a ctune_UI_RSEdit_t object
 * @return Form exit state
 */
static ctune_FormExit_e ctune_UI_RSEdit_captureInput( ctune_UI_RSEdit_t * rsedit ) {
    bool             exit       = false;
    ctune_FormExit_e exit_state = CTUNE_UI_FORM_ESC;
    int              character;
    MEVENT           mouse_event;

    ctune_UI_Form.input.start( &rsedit->form );
    ctune_UI_Form.input.fwdToFormDriver( &rsedit->form, REQ_FIRST_FIELD );
    ctune_UI_RSEdit_highlightCurrField( rsedit );
    ctune_UI_Form.display.refreshView( &rsedit->form );

    while( !exit ) {
        character = ctune_UI_Form.input.getChar( &rsedit->form );

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
                ctune_UI_Form.input.fwdToFormDriver( &rsedit->form, REQ_BEG_FIELD );
            } break;

            case CTUNE_UI_ACTION_FIELD_END: {
                ctune_UI_Form.input.fwdToFormDriver( &rsedit->form, REQ_END_FIELD );
            } break;

            case CTUNE_UI_ACTION_FIELD_FIRST: {
                ctune_UI_Form.field.setCurrent( &rsedit->form, LABEL_COUNT );
                ctune_UI_RSEdit_highlightCurrField( rsedit );
                ctune_UI_Form.scrolling.autoscroll( &rsedit->form );
            } break;

            case CTUNE_UI_ACTION_FIELD_LAST: {
                ctune_UI_Form.field.setCurrent( &rsedit->form, ( FIELD_LAST - 1 ) );
                ctune_UI_RSEdit_highlightCurrField( rsedit );
                ctune_UI_Form.scrolling.autoscroll( &rsedit->form );
            } break;

            case CTUNE_UI_ACTION_FIELD_PREV: {
                ctune_UI_Form.input.fwdToFormDriver( &rsedit->form, REQ_PREV_FIELD );
                ctune_UI_RSEdit_highlightCurrField( rsedit );
                ctune_UI_Form.input.fwdToFormDriver( &rsedit->form, REQ_END_LINE );
                ctune_UI_Form.scrolling.autoscroll( &rsedit->form );
            } break;

            case CTUNE_UI_ACTION_FIELD_NEXT: {
                ctune_UI_Form.input.fwdToFormDriver( &rsedit->form, REQ_NEXT_FIELD );
                ctune_UI_RSEdit_highlightCurrField( rsedit );
                ctune_UI_Form.input.fwdToFormDriver( &rsedit->form, REQ_END_LINE );
                ctune_UI_Form.scrolling.autoscroll( &rsedit->form );
            } break;

            case CTUNE_UI_ACTION_GO_LEFT: {
                ctune_UI_Form.input.fwdToFormDriver( &rsedit->form, REQ_LEFT_CHAR );
            } break;

            case CTUNE_UI_ACTION_GO_RIGHT: {
                ctune_UI_Form.input.fwdToFormDriver( &rsedit->form, REQ_RIGHT_CHAR );
            } break;

            case CTUNE_UI_ACTION_TRIGGER: {
                if( ctune_UI_Form.field.current( &rsedit->form ) == ctune_UI_Form.field.get( &rsedit->form, BUTTON_AUTODETECT ) ) {
                    ctune_UI_RSEdit_autodetectStreamProperties( rsedit, &rsedit->cache.station );

                } else if( ( exit = ctune_UI_RSEdit_isExitState( rsedit, ctune_UI_Form.field.currentIndex( &rsedit->form ), &exit_state ) ) ) {
                    if( exit_state == CTUNE_UI_FORM_SUBMIT ) {
                        if( ctune_UI_RSEdit_packFieldValues( rsedit, &rsedit->cache.station ) ) {
                            ctune_UI_RSEdit_setChangeTimestamp( &rsedit->cache.station );
                        } else {
                            exit = false;
                        }
                    }

                } else {
                    ctune_UI_Form.input.fwdToFormDriver( &rsedit->form, REQ_NEXT_FIELD );
                    ctune_UI_RSEdit_highlightCurrField( rsedit );
                    ctune_UI_Form.input.fwdToFormDriver( &rsedit->form, REQ_END_LINE );
                }
            } break;

            case CTUNE_UI_ACTION_TOGGLE_ALT: { //'space'
                if( ctune_UI_Form.field.current( &rsedit->form ) == ctune_UI_Form.field.get( &rsedit->form, BUTTON_AUTODETECT ) ) {
                    ctune_UI_RSEdit_autodetectStreamProperties( rsedit, &rsedit->cache.station );

                } else if( ( exit = ctune_UI_RSEdit_isExitState( rsedit, ctune_UI_Form.field.currentIndex( &rsedit->form ), &exit_state ) ) ) {
                    if( exit_state == CTUNE_UI_FORM_SUBMIT ) {
                        if( ctune_UI_RSEdit_packFieldValues( rsedit, &rsedit->cache.station ) ) {
                            ctune_UI_RSEdit_setChangeTimestamp( &rsedit->cache.station );
                        } else {
                            exit = false;
                        }
                    }

                } else {
                    ctune_UI_Form.input.fwdToFormDriver( &rsedit->form, character );
                }
            } break;

            case CTUNE_UI_ACTION_CLEAR_ALL: {
                ctune_UI_Form.field.clearRange( &rsedit->form, LABEL_COUNT, FIELD_COUNT );
                ctune_UI_RSEdit_highlightCurrField( rsedit );
            } break;

            case CTUNE_UI_ACTION_CLEAR_SELECTED: {
                ctune_UI_Form.input.fwdToFormDriver( &rsedit->form, REQ_CLR_FIELD );
            } break;

            case CTUNE_UI_ACTION_DEL_PREV: {
                ctune_UI_Form.input.fwdToFormDriver( &rsedit->form, REQ_DEL_PREV );
            } break;

            case CTUNE_UI_ACTION_DEL_NEXT: {
                ctune_UI_Form.input.fwdToFormDriver( &rsedit->form, REQ_DEL_CHAR );
            } break;

            case CTUNE_UI_ACTION_MOUSE_EVENT: {
                if( getmouse( &mouse_event ) == OK ) {
                    if( ( exit = ctune_UI_RSEdit_handleMouseEvent( rsedit, &mouse_event, &exit_state ) ) && exit_state == CTUNE_UI_FORM_SUBMIT ) {
                        if( ctune_UI_RSEdit_packFieldValues( rsedit, &rsedit->cache.station ) ) {
                            ctune_UI_RSEdit_setChangeTimestamp( &rsedit->cache.station );
                        } else {
                            exit = false;
                        }
                    }
                }
            } break;

            default: {
                ctune_UI_Form.input.fwdToFormDriver( &rsedit->form, character );
            } break;
        }

        ctune_UI_Form.display.refreshView( &rsedit->form );
    }

    ctune_UI_Form.input.stop( &rsedit->form );

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
    ctune_UI_Form.freeContent( &rsedit->form );
    regfree( &rsedit->cache.url_regex );
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
    .setMouseCtrl  = &ctune_UI_RSEdit_setMouseCtrl,
    .newStation    = &ctune_UI_RSEdit_newStation,
    .copyStation   = &ctune_UI_RSEdit_copyStation,
    .loadStation   = &ctune_UI_RSEdit_loadStation,
    .show          = &ctune_UI_RSEdit_show,
    .captureInput  = &ctune_UI_RSEdit_captureInput,
    .getStation    = &ctune_UI_RSEdit_getStation,
    .free          = &ctune_UI_RSEdit_free,
};