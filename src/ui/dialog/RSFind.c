#include "RSFind.h"

#include "../../logger/Logger.h"
#include "../definitions/KeyBinding.h"
#include "../definitions/Theme.h"
#include "ContextHelp.h"
//#include "../Resizer.h"

typedef enum {
    LABEL_NAME = 0,
    LABEL_COUNTRY,
    LABEL_COUNTRY_CODE,
    LABEL_STATE,
    LABEL_LANGUAGE,
    LABEL_TAGS,
    LABEL_CODEC,
    LABEL_BITRATE,
    LABEL_ORDER_BY,
    LABEL_REVERSE,
    LABEL_EXACT, //#11  (boolean)
    //separator label
    LABEL_BITRATE_TO, //#12
    LABEL_BITRATE_UNIT, //#13

    LABEL_COUNT,
} RSFind_Label_e;

typedef enum {
    INPUT_NAME = LABEL_COUNT,
    INPUT_NAME_EXACT,
    INPUT_COUNTRY,
    INPUT_COUNTRY_EXACT,
    INPUT_COUNTRY_CODE,
    INPUT_STATE,
    INPUT_STATE_EXACT,
    INPUT_LANGUAGE,
    INPUT_LANGUAGE_EXACT,
    INPUT_TAGS,
    INPUT_TAGS_EXACT,
    INPUT_CODEC,
    INPUT_BITRATE_MIN,
    INPUT_BITRATE_MAX,
    INPUT_ORDER_BY,
    INPUT_REVERSE,

    BUTTON_CANCEL,
    BUTTON_SAVE,

    FIELD_LAST,

    FIELD_COUNT,
} RSFind_Input_e;


/**
 * [PRIVATE] Sets the buffer of the form's toggle field
 * @param rsfind Pointer to a ctune_UI_RSFind_t object
 * @param field  Pointer to the FIELD
 * @param state  Toggle state
 */
static void ctune_UI_RSFind_setToggleField( ctune_UI_RSFind_t * rsfind, size_t field_id, bool state ) {
    set_field_buffer(
        ctune_UI_Form.field.get( &rsfind->form, field_id ),
        0,
        ( state ? rsfind->cb.getDisplayText( CTUNE_UI_TEXT_TOGGLE_FIELD_ON )
                : rsfind->cb.getDisplayText( CTUNE_UI_TEXT_TOGGLE_FIELD_OFF ) )
    );
}

/**
 * [PRIVATE] Toggles a given field
 * @param rsfind         Pointer to a ctune_UI_RSFind_t object
 * @param id             Toggle field ID of field
 * @param field_property Form input field
 */
static void ctune_UI_RSFind_toggle( ctune_UI_RSFind_t * rsfind, RSFind_Input_e id, FIELD * field ) {
    if( field != ctune_UI_Form.field.get( &rsfind->form, id ) ) { //sanity check
        return; //EARLY RETURN
    }

    bool state = false;

    ctune_RadioBrowserFilter_t * filter = &rsfind->cache.filter; //shortcut pointer

    switch( id ) {
        case INPUT_NAME_EXACT: {
            ctune_RadioBrowserFilter.set.exactNameToggle( filter, !ctune_RadioBrowserFilter.get.exactNameToggle( filter ) );
            state = ctune_RadioBrowserFilter.get.exactNameToggle( filter );
        } break;

        case INPUT_COUNTRY_EXACT: {
            ctune_RadioBrowserFilter.set.exactCountryToggle( filter, !ctune_RadioBrowserFilter.get.exactCountryToggle( filter ) );
            state = ctune_RadioBrowserFilter.get.exactCountryToggle( filter );
        } break;

        case INPUT_STATE_EXACT: {
            ctune_RadioBrowserFilter.set.exactStateToggle( filter, !ctune_RadioBrowserFilter.get.exactStateToggle( filter ) );
            state = ctune_RadioBrowserFilter.get.exactStateToggle( filter );
        } break;

        case INPUT_LANGUAGE_EXACT: {
            ctune_RadioBrowserFilter.set.exactLanguageToggle( filter, !ctune_RadioBrowserFilter.get.exactLanguageToggle( filter ) );
            state = ctune_RadioBrowserFilter.get.exactLanguageToggle( filter );
        } break;

        case INPUT_TAGS_EXACT: {
            ctune_RadioBrowserFilter.set.exactTagToggle( filter, !ctune_RadioBrowserFilter.get.exactTagToggle( filter ) );
            state = ctune_RadioBrowserFilter.get.exactTagToggle( filter );
        } break;

        case INPUT_REVERSE: {
            ctune_RadioBrowserFilter.set.reverseToggle( filter, !ctune_RadioBrowserFilter.get.reverseToggle( filter ) );
            state = ctune_RadioBrowserFilter.get.reverseToggle( filter );
        } break;

        default:
            return;
    }

    ctune_UI_RSFind_setToggleField( rsfind, id, state );
}

/**
 * [PRIVATE] Checks if field is a toggle input
 * @param rsfind Pointer to a ctune_UI_RSFind_t object
 * @param field  Form field
 * @return Toggle field ID (-1 if not valid)
 */
static int ctune_UI_RSFind_isToggle( ctune_UI_RSFind_t * rsfind, const FIELD * field ) {
    if( field != NULL ) {
        if( field == ctune_UI_Form.field.get( &rsfind->form, INPUT_NAME_EXACT ) ) {
            return INPUT_NAME_EXACT;
        } else if( field == ctune_UI_Form.field.get( &rsfind->form, INPUT_COUNTRY_EXACT ) ) {
            return INPUT_COUNTRY_EXACT;
        } else if( field == ctune_UI_Form.field.get( &rsfind->form, INPUT_STATE_EXACT ) ) {
            return INPUT_STATE_EXACT;
        } else if( field == ctune_UI_Form.field.get( &rsfind->form, INPUT_LANGUAGE_EXACT ) ) {
            return INPUT_LANGUAGE_EXACT;
        } else if( field == ctune_UI_Form.field.get( &rsfind->form, INPUT_TAGS_EXACT ) ) {
            return INPUT_TAGS_EXACT;
        } else if( field == ctune_UI_Form.field.get( &rsfind->form, INPUT_REVERSE ) ) {
            return INPUT_REVERSE;
        }
    }

    return -1;
}

/**
 * [PRIVATE] Check if field is a button
 * @param rsfind Pointer to a ctune_UI_RSFind_t object
 * @param field  Form field
 * @return Button field status
 */
static bool ctune_UI_RSFind_isButton( ctune_UI_RSFind_t * rsfind, const FIELD * field ) {
    return field == ctune_UI_Form.field.get( &rsfind->form, BUTTON_CANCEL )
        || field == ctune_UI_Form.field.get( &rsfind->form, BUTTON_SAVE   );
}

/**
 * [PRIVATE] Check if given field is a form exit
 * @param rsfind    Pointer to a ctune_UI_RSFind_t object
 * @param field     FIELD pointer
 * @param exit_type Variable pointer to set the exit type of the field
 * @return Exit state
 */
static bool ctune_UI_RSFind_isExitState( ctune_UI_RSFind_t * rsfind, const FIELD * field, ctune_FormExit_e * exit_type ) {
    if( ctune_UI_RSFind_isButton( rsfind, field ) ) {
        if( field == ctune_UI_Form.field.get( &rsfind->form, BUTTON_CANCEL ) ) {
            *exit_type = CTUNE_UI_FORM_CANCEL;
            return true;
        }

        if( field == ctune_UI_Form.field.get( &rsfind->form, BUTTON_SAVE ) ) {
            *exit_type = CTUNE_UI_FORM_SUBMIT;
            return true;
        }
    }

    return false;
}

/**
 * [PRIVATE] Prints the order-by selection to the field buffer
 * @param rsfind    Pointer to a ctune_UI_RSFind_t object
 * @param selection Selection item (ctune_StationAttr_e)
 */
static void ctune_UI_RSFind_printOrderBy( ctune_UI_RSFind_t * rsfind, size_t selection ) {
    if( selection >= STATION_ATTR_COUNT ) {
        return; //EARLY RETURN
    }

    String_t out = String.init();

    if( selection == 0 ) {
        String.append_back( &out, "  " );
    } else {
        String.append_back( &out, "< " );
    }

    size_t length     = strlen( rsfind->cache.order_items[ selection ] );
    size_t whitespace = ( length <= ( rsfind->cache.order_width - 4 ) ? ( ( rsfind->cache.order_width - 4 ) - length ) : 0 );

    String.append_back( &out, rsfind->cache.order_items[ selection ] );

    for( size_t i = 0; i < whitespace; ++ i ) {
        String.append_back( &out, " " );
    }

    if( selection >= ( STATION_ATTR_COUNT - 1 ) ) {
        String.append_back( &out, "  " );
    } else {
        String.append_back( &out, " >" );
    }

    ctune_UI_Form.field.setBuffer( &rsfind->form, INPUT_ORDER_BY, out._raw );

    String.free( &out );

    ctune_RadioBrowserFilter.set.ordering( &rsfind->cache.filter, selection );
}

/**
 * [PRIVATE] Highlights input field where the cursor is at
 * @param rsfind Pointer to a ctune_UI_RSFind_t object
 */
static void ctune_UI_RSFind_highlightCurrField( ctune_UI_RSFind_t * rsfind ) {
    const FIELD * curr_field    = ctune_UI_Form.field.current( &rsfind->form );
    const int     curr_field_id = ctune_UI_Form.field.currentIndex( &rsfind->form );

    const bool curr_is_button   = ctune_UI_RSFind_isButton( rsfind, curr_field );
    const bool curr_is_toggle   = ( ctune_UI_RSFind_isToggle( rsfind, curr_field ) >= 0 );
    const bool curr_is_editable = !( curr_is_button || curr_is_toggle || curr_field_id == INPUT_ORDER_BY );

    curs_set( ( curr_is_editable ? 1 : 0 ) );

    for( int i = 0; i < LABEL_COUNT; ++i ) {
        ctune_UI_Form.field.setBackground( &rsfind->form, i, A_NORMAL );
    }

    for( int i = LABEL_COUNT; i < FIELD_COUNT; ++i ) {
        FIELD * field = ctune_UI_Form.field.get( &rsfind->form, i );

        if( field == curr_field ) {
            ctune_UI_Form.field.setBackground( &rsfind->form, i, A_REVERSE );

        } else {
            if( ctune_UI_RSFind_isButton( rsfind, field ) || ctune_UI_RSFind_isToggle( rsfind, field ) >= 0 ) {
                ctune_UI_Form.field.setBackground( &rsfind->form, i, A_NORMAL );
            } else {
                ctune_UI_Form.field.setBackground( &rsfind->form, i, A_UNDERLINE );
            }
        }
    }
}

/**
 * [PRIVATE] Packs all the non-toggle field values into the cached filter
 * @param rsfind Pointer to a ctune_UI_RSFind_t object
 * @return Error-less state
 */
static bool ctune_UI_RSFind_packFieldValues( ctune_UI_RSFind_t * rsfind, ctune_RadioBrowserFilter_t * filter ) {
    //Note: toggle fields and order-by choice are set 'live' - see `ctune_UI_RSFind_toggle(..)` and `ctune_UI_RSFind_printOrderBy(..)`
    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_UI_RSFind_packFieldValues( %p, &p )] Packing field values...",
               rsfind, filter
    );

    bool error_state = false;

    if( ctune_UI_Form.field.status( &rsfind->form, INPUT_NAME ) ) {
        ctune_RadioBrowserFilter.set.name( filter, ctune_trimspace( ctune_UI_Form.field.buffer( &rsfind->form, INPUT_NAME ) ) );
    }

    if( ctune_UI_Form.field.status( &rsfind->form, INPUT_COUNTRY ) ) {
        ctune_RadioBrowserFilter.set.country( filter, ctune_trimspace( ctune_UI_Form.field.buffer( &rsfind->form, INPUT_COUNTRY ) ) );
    }

    if( ctune_UI_Form.field.status( &rsfind->form, INPUT_COUNTRY_CODE ) ) {
        char * cc = ctune_trimspace( ctune_UI_Form.field.buffer( &rsfind->form, INPUT_COUNTRY_CODE ) );
        ctune_RadioBrowserFilter.set.countryCode( filter, cc );
        free( cc );
    }

    if( ctune_UI_Form.field.status( &rsfind->form, INPUT_STATE ) ) {
        ctune_RadioBrowserFilter.set.state( filter, ctune_trimspace( ctune_UI_Form.field.buffer( &rsfind->form, INPUT_STATE ) ) );
    }

    if( ctune_UI_Form.field.status( &rsfind->form, INPUT_LANGUAGE ) ) {
        ctune_RadioBrowserFilter.set.language( filter, ctune_trimspace( ctune_UI_Form.field.buffer( &rsfind->form, INPUT_LANGUAGE ) ) );
    }

    if( ctune_UI_Form.field.status( &rsfind->form, INPUT_CODEC ) ) {
        ctune_RadioBrowserFilter.set.codec( filter, ctune_trimspace( ctune_UI_Form.field.buffer( &rsfind->form, INPUT_CODEC ) ) );
    }

    { //Tag(s)
        StrList_t * list      = ctune_RadioBrowserFilter.get.tagList( filter );
        size_t      val_count = ctune_splitcss( ctune_UI_Form.field.buffer( &rsfind->form, INPUT_TAGS ), list );

        CTUNE_LOG( CTUNE_LOG_DEBUG,
                   "[ctune_UI_RSFind_packFieldValues( %p, %p )] Found %i values in field 'INPUT_TAGS': \"%s\"",
                   rsfind, filter, val_count, ctune_UI_Form.field.buffer( &rsfind->form, INPUT_TAGS )
        );

        if( val_count == 1 ) {
            ctune_RadioBrowserFilter.set.tag( filter, strdup( StrList.at( list, 0 )->data ) );

            if( ctune_RadioBrowserFilter.get.tag( filter ) != NULL ) {
                StrList.free_strlist( list );
            }
        }
    }

    { //Bitrate min/max
        const ulong min = strtoul( ctune_UI_Form.field.buffer( &rsfind->form, INPUT_BITRATE_MIN ), NULL, 10 );
        ulong       max = strtoul( ctune_UI_Form.field.buffer( &rsfind->form, INPUT_BITRATE_MAX ), NULL, 10 );

        if( max == 0 ) {
            max = CTUNE_RADIOBROSWERFILTER_BITRATE_MAX_DFLT;

            CTUNE_LOG( CTUNE_LOG_DEBUG,
                       "[ctune_UI_RSFind_packFieldValues( %p, %p )] max bitrate = 0/ERR: auto-set to default maximum value (%lu).",
                       rsfind, filter, max
            );

            ctune_UI_Form.field.setBuffer( &rsfind->form, INPUT_BITRATE_MAX, CTUNE_RADIOBROSWERFILTER_BITRATE_MAX_DFLT_STR );
        }

        if( max < min ) { //validation
            ctune_UI_Form.field.setCurrent( &rsfind->form, INPUT_BITRATE_MIN );
            ctune_UI_RSFind_highlightCurrField( rsfind );

            error_state = true;
        }

        ctune_RadioBrowserFilter.set.bitrate( filter, min, max );
    }

    ctune_RadioBrowserFilter.set.stationSource( filter, CTUNE_STATIONSRC_RADIOBROWSER );

    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_RSFind_packFieldValues( %p )] Field values packed.", rsfind );
    return !( error_state );
}

/**
 * [PRIVATE] Initialises cached filter and fields
 * @param rsfind Pointer to a ctune_UI_RSFind_t object
 * @return Success
 */
static bool ctune_UI_RSFind_initFields( ctune_UI_RSFind_t * rsfind ) {
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_RSFind_initFields()] Initialising form fields..." );

    ctune_RadioBrowserFilter.freeContent( &rsfind->cache.filter );
    rsfind->cache.filter = ctune_RadioBrowserFilter.init();

    ctune_RadioBrowserFilter_t * filter = &rsfind->cache.filter; //shortcut pointer

    ctune_UI_Form.field.setBuffer( &rsfind->form, LABEL_NAME,         rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_STATION_NAME ) );
    ctune_UI_Form.field.setBuffer( &rsfind->form, LABEL_NAME,         rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_STATION_NAME ) );
    ctune_UI_Form.field.setBuffer( &rsfind->form, LABEL_COUNTRY,      rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_COUNTRY ) );
    ctune_UI_Form.field.setBuffer( &rsfind->form, LABEL_COUNTRY_CODE, rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_COUNTRY_CODE ) );
    ctune_UI_Form.field.setBuffer( &rsfind->form, LABEL_STATE,        rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_STATE ) );
    ctune_UI_Form.field.setBuffer( &rsfind->form, LABEL_LANGUAGE,     rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_LANGUAGE ) );
    ctune_UI_Form.field.setBuffer( &rsfind->form, LABEL_TAGS,         rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_TAGS ) );
    ctune_UI_Form.field.setBuffer( &rsfind->form, LABEL_CODEC,        rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_CODEC ) );
    ctune_UI_Form.field.setBuffer( &rsfind->form, LABEL_BITRATE,      rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_BITRATE_RANGE ) );
    ctune_UI_Form.field.setBuffer( &rsfind->form, LABEL_BITRATE_TO,   rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_BITRATE_RANGE_SEPARATOR ) );
    ctune_UI_Form.field.setBuffer( &rsfind->form, LABEL_BITRATE_UNIT, rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_BITRATE_UNIT_LONG ) );
    ctune_UI_Form.field.setBuffer( &rsfind->form, LABEL_ORDER_BY,     rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_ORDER_BY ) );
    ctune_UI_Form.field.setBuffer( &rsfind->form, LABEL_REVERSE,      rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_REVERSE_ORDER ) );
    ctune_UI_Form.field.setBuffer( &rsfind->form, LABEL_EXACT,        rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_EXACT_MATCH ) );
    ctune_UI_Form.field.setBuffer( &rsfind->form, INPUT_NAME,         ctune_fallbackStr( ctune_RadioBrowserFilter.get.name( filter ), "" ) );
    ctune_UI_Form.field.setBuffer( &rsfind->form, INPUT_COUNTRY,      ctune_fallbackStr( ctune_RadioBrowserFilter.get.country( filter ), "" ) );
    ctune_UI_Form.field.setBuffer( &rsfind->form, INPUT_COUNTRY_CODE, ctune_fallbackStr( ctune_RadioBrowserFilter.get.countryCode( filter ), "" ) );
    ctune_UI_Form.field.setBuffer( &rsfind->form, INPUT_STATE,        ctune_fallbackStr( ctune_RadioBrowserFilter.get.state( filter ), "" ) );
    ctune_UI_Form.field.setBuffer( &rsfind->form, INPUT_LANGUAGE,     ctune_fallbackStr( ctune_RadioBrowserFilter.get.language( filter ), "" ) );
    ctune_UI_Form.field.setBuffer( &rsfind->form, INPUT_CODEC,        ctune_fallbackStr( ctune_RadioBrowserFilter.get.codec( filter ), "" ) );

    ctune_RadioBrowserFilter.set.ordering( filter, STATION_ATTR_NONE );
    ctune_UI_RSFind_printOrderBy( rsfind, ctune_RadioBrowserFilter.get.ordering( filter ) ); //INPUT_ORDER_BY

    { //Tag(s)
        const StrList_t * tag_list_ptr = ctune_RadioBrowserFilter.get.tagList( filter ); //shortcut pointer

        if( ctune_RadioBrowserFilter.get.tag( filter ) != NULL ) {
           ctune_UI_Form.field.setBuffer( &rsfind->form,  INPUT_TAGS, ctune_RadioBrowserFilter.get.tag( filter ) );

        } else if( tag_list_ptr != NULL && !StrList.empty( tag_list_ptr ) ) {
            String_t list = String.init();

            if( StrList.stringify( tag_list_ptr, &list, ',' ) != StrList.size( tag_list_ptr ) ) {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_UI_RSFind_initFields( %p )] Failed to stringify the filter's tagList.",
                           rsfind
                );

                String.free( &list );
                return false; //EARLY RETURN
            }

           ctune_UI_Form.field.setBuffer( &rsfind->form,  INPUT_TAGS, list._raw );
            String.free( &list );
        } else {
           ctune_UI_Form.field.setBuffer( &rsfind->form,  INPUT_TAGS, "" );
        }
    }

    ctune_UI_RSFind_setToggleField( rsfind, INPUT_REVERSE,        ctune_RadioBrowserFilter.get.reverseToggle( filter ) );
    ctune_UI_RSFind_setToggleField( rsfind, INPUT_NAME_EXACT,     ctune_RadioBrowserFilter.get.exactNameToggle( filter ) );
    ctune_UI_RSFind_setToggleField( rsfind, INPUT_COUNTRY_EXACT,  ctune_RadioBrowserFilter.get.exactCountryToggle( filter ) );
    ctune_UI_RSFind_setToggleField( rsfind, INPUT_STATE_EXACT,    ctune_RadioBrowserFilter.get.exactStateToggle( filter ) );
    ctune_UI_RSFind_setToggleField( rsfind, INPUT_LANGUAGE_EXACT, ctune_RadioBrowserFilter.get.exactLanguageToggle( filter ) );
    ctune_UI_RSFind_setToggleField( rsfind, INPUT_TAGS_EXACT,     ctune_RadioBrowserFilter.get.exactTagToggle( filter ) );

    { //Bitrate fields (ulong) - loads the min/max defaults from the filter object as a guide
        String_t bitrate_min = String.init();
        String_t bitrate_max = String.init();
        ctune_utos( ctune_RadioBrowserFilter.get.bitrateMin( filter ), &bitrate_min );
        ctune_utos( ctune_RadioBrowserFilter.get.bitrateMax( filter ), &bitrate_max );
        ctune_UI_Form.field.setBuffer( &rsfind->form, INPUT_BITRATE_MIN, bitrate_min._raw );
        ctune_UI_Form.field.setBuffer( &rsfind->form, INPUT_BITRATE_MAX, bitrate_max._raw );
        String.free( &bitrate_min );
        String.free( &bitrate_max );
    }

    ctune_UI_Form.field.setBuffer( &rsfind->form, BUTTON_CANCEL, rsfind->cb.getDisplayText( CTUNE_UI_TEXT_BUTTON_CANCEL ) );
    ctune_UI_Form.field.setBuffer( &rsfind->form, BUTTON_SAVE, rsfind->cb.getDisplayText( CTUNE_UI_TEXT_BUTTON_SUBMIT ) );

    ctune_UI_Form.field.setBackground( &rsfind->form, BUTTON_CANCEL, ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_DFLT ) );
    ctune_UI_Form.field.setForeground( &rsfind->form, BUTTON_CANCEL, ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_DFLT ) );
    ctune_UI_Form.field.setBackground( &rsfind->form, BUTTON_SAVE,   ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_DFLT ) );
    ctune_UI_Form.field.setForeground( &rsfind->form, BUTTON_SAVE,   ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_DFLT ) );

    return true;
}

/**
 * [PRIVATE] Creates the fields (Called once and cached for the remainder of the runtime)
 * @param rsfind Pointer to a ctune_UI_RSFind_t object
 * @return Success
 */
static bool ctune_UI_RSFind_createFields( ctune_UI_RSFind_t * rsfind ) {
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_RSFind_createFields( %p )] Creating form fields...", rsfind );

    //Order-By "dropdown" window on field
    rsfind->cache.order_items[STATION_ATTR_NONE           ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_NONE );
    rsfind->cache.order_items[STATION_ATTR_NAME           ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_NAME );
    rsfind->cache.order_items[STATION_ATTR_URL            ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_URL );
    rsfind->cache.order_items[STATION_ATTR_HOMEPAGE       ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_HOMEPAGE );
    rsfind->cache.order_items[STATION_ATTR_FAVICON        ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_FAVICON );
    rsfind->cache.order_items[STATION_ATTR_TAGS           ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_TAGS );
    rsfind->cache.order_items[STATION_ATTR_COUNTRY        ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_COUNTRY );
    rsfind->cache.order_items[STATION_ATTR_STATE          ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_STATE );
    rsfind->cache.order_items[STATION_ATTR_LANGUAGE       ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_LANGUAGE );
    rsfind->cache.order_items[STATION_ATTR_VOTES          ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_VOTES );
    rsfind->cache.order_items[STATION_ATTR_CODEC          ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_CODEC );
    rsfind->cache.order_items[STATION_ATTR_BITRATE        ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_BITRATE );
    rsfind->cache.order_items[STATION_ATTR_LASTCHECKOK    ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_LASTCHECKOK );
    rsfind->cache.order_items[STATION_ATTR_LASTCHECKTIME  ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_LASTCHECKTIME );
    rsfind->cache.order_items[STATION_ATTR_CLICKTIMESTAMP ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_CLICKTIMESTAMP );
    rsfind->cache.order_items[STATION_ATTR_CLICKCOUNT     ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_CLICKCOUNT );
    rsfind->cache.order_items[STATION_ATTR_CLICKTREND     ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_CLICKTREND );
    rsfind->cache.order_items[STATION_ATTR_RANDOM         ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_RANDOM );
    rsfind->cache.order_items[STATION_ATTR_STATIONCOUNT   ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_STATIONCOUNT );
    rsfind->cache.order_items[STATION_ATTR_CHANGETIMESTAMP] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_CHANGETIMESTAMP );

    for( size_t i = 0; i < STATION_ATTR_COUNT; ++i ) {
        rsfind->cache.order_width = (int) ctune_max_ul( rsfind->cache.order_width,
                                                        strlen( rsfind->cache.order_items[i] ) );
    }
    rsfind->cache.order_width += 4; //"< " + string + " >"

//      <label_col>             <toggle_label_col>
//      |                       |
//      |     <field_col>       |   <toggle_field_col>
//      |     |                 |   |
//    .--------------[FORM]---------------.
//    |                         exact match |
//    |                                     |
//    | Name  _________________     [x]     |
//    .                                     .
//    .                                     .

    if( rsfind->cache.max_label_width == 0 ) {
        //iteration includes the bitrate separator/units but since they
        //should all be smaller in length than the other labels... meh
        rsfind->cache.max_label_width = ctune_max_ul( rsfind->cache.max_label_width, strlen( rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_STATION_NAME ) ) );
        rsfind->cache.max_label_width = ctune_max_ul( rsfind->cache.max_label_width, strlen( rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_COUNTRY ) ) );
        rsfind->cache.max_label_width = ctune_max_ul( rsfind->cache.max_label_width, strlen( rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_COUNTRY_CODE ) ) );
        rsfind->cache.max_label_width = ctune_max_ul( rsfind->cache.max_label_width, strlen( rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_STATE ) ) );
        rsfind->cache.max_label_width = ctune_max_ul( rsfind->cache.max_label_width, strlen( rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_LANGUAGE ) ) );
        rsfind->cache.max_label_width = ctune_max_ul( rsfind->cache.max_label_width, strlen( rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_TAGS ) ) );
        rsfind->cache.max_label_width = ctune_max_ul( rsfind->cache.max_label_width, strlen( rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_CODEC ) ) );
        rsfind->cache.max_label_width = ctune_max_ul( rsfind->cache.max_label_width, strlen( rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_BITRATE_RANGE ) ) );
        rsfind->cache.max_label_width = ctune_max_ul( rsfind->cache.max_label_width, strlen( rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_BITRATE_RANGE_SEPARATOR ) ) );
        rsfind->cache.max_label_width = ctune_max_ul( rsfind->cache.max_label_width, strlen( rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_BITRATE_UNIT_LONG ) ) );
        rsfind->cache.max_label_width = ctune_max_ul( rsfind->cache.max_label_width, strlen( rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_ORDER_BY ) ) );
        rsfind->cache.max_label_width = ctune_max_ul( rsfind->cache.max_label_width, strlen( rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_REVERSE_ORDER ) ) );
    }

    int label_col_width   = 0;
    int order_field_width = 0;

    if( !ctune_utoi( rsfind->cache.max_label_width, &label_col_width ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_RSFind_createFields( %p )] Failed size_t->int cast of `label_col_width` (%lu).",
                   rsfind, rsfind->cache.max_label_width
        );

        return false; //EARLY RETURN
    }

    if( !ctune_utoi( rsfind->cache.order_width, &order_field_width ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_RSFind_createFields( %p )] Failed size_t->int cast of `order_field_width` (%lu).",
                   rsfind, rsfind->cache.order_width
        );

        return false;
    }

    const int row_height         = 1;
    const int label_col          = 0;
    const int field_col          = ( label_col_width + 2 );
    const int field_width        = ( order_field_width > 40 ? order_field_width : 40 );
    const int cc_field_width     = 2; //(country code)

    const int toggle_label_col    = field_col + field_width;
    const int toggle_label_size   = (int) strlen( rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_EXACT_MATCH ) );
    const int toggle_field_width  = 3; //"[x]"
    const int toggle_field_offset = ( toggle_label_size > toggle_field_width ? ( toggle_label_size / 2 - toggle_field_width / 2 ) : 0 );
    const int toggle_field_col    = ( toggle_label_col + toggle_field_offset );

    const int form_width          = toggle_label_col + toggle_label_size;

    const int bitrate_sep_len   = (int) strlen( rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_BITRATE_RANGE_SEPARATOR ) );
    const int bitrate_unit_len  = (int) strlen( rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_BITRATE_UNIT_LONG ) );
    const int bitrate_input_len = 8;
    const int bitrate_sep_col   = ( field_col + bitrate_input_len + 1 );
    const int bitrate_max_col   = ( bitrate_sep_col + bitrate_sep_len + 1 );
    const int bitrate_unit_col  = ( bitrate_max_col + bitrate_input_len + 1 );

    bool ret[FIELD_LAST]; //errors
    //Field labels                                                                                                   rows        cols                y   x
    ret[LABEL_EXACT         ] = ctune_UI_Form.field.create( &rsfind->form, LABEL_EXACT,          (WindowProperty_t){ row_height, label_col_width,     0, toggle_label_col } );
    ret[LABEL_COUNTRY       ] = ctune_UI_Form.field.create( &rsfind->form, LABEL_NAME,           (WindowProperty_t){ row_height, label_col_width,     2, label_col } );
    ret[LABEL_COUNTRY       ] = ctune_UI_Form.field.create( &rsfind->form, LABEL_COUNTRY,        (WindowProperty_t){ row_height, label_col_width,     4, label_col } );
    ret[LABEL_COUNTRY_CODE  ] = ctune_UI_Form.field.create( &rsfind->form, LABEL_COUNTRY_CODE,   (WindowProperty_t){ row_height, label_col_width,     6, label_col } );
    ret[LABEL_STATE         ] = ctune_UI_Form.field.create( &rsfind->form, LABEL_STATE,          (WindowProperty_t){ row_height, label_col_width,     8, label_col } );
    ret[LABEL_LANGUAGE      ] = ctune_UI_Form.field.create( &rsfind->form, LABEL_LANGUAGE,       (WindowProperty_t){ row_height, label_col_width,    10, label_col } );
    ret[LABEL_TAGS          ] = ctune_UI_Form.field.create( &rsfind->form, LABEL_TAGS,           (WindowProperty_t){ row_height, label_col_width,    12, label_col } );
    ret[LABEL_CODEC         ] = ctune_UI_Form.field.create( &rsfind->form, LABEL_CODEC,          (WindowProperty_t){ row_height, label_col_width,    14, label_col } );
    ret[LABEL_BITRATE       ] = ctune_UI_Form.field.create( &rsfind->form, LABEL_BITRATE,        (WindowProperty_t){ row_height, label_col_width,    16, label_col } );
    ret[LABEL_BITRATE_TO    ] = ctune_UI_Form.field.create( &rsfind->form, LABEL_BITRATE_TO,     (WindowProperty_t){ row_height, bitrate_sep_len,    16, bitrate_sep_col } );
    ret[LABEL_BITRATE_UNIT  ] = ctune_UI_Form.field.create( &rsfind->form, LABEL_BITRATE_UNIT,   (WindowProperty_t){ row_height, bitrate_unit_len,   16, bitrate_unit_col } );
    ret[LABEL_ORDER_BY      ] = ctune_UI_Form.field.create( &rsfind->form, LABEL_ORDER_BY,       (WindowProperty_t){ row_height, label_col_width,    18, label_col } );
    ret[LABEL_REVERSE       ] = ctune_UI_Form.field.create( &rsfind->form, LABEL_REVERSE,        (WindowProperty_t){ row_height, label_col_width,    20, label_col } );
    //Field inputs                                                                                                  rows        cols                y    x
    ret[INPUT_NAME          ] = ctune_UI_Form.field.create( &rsfind->form, INPUT_NAME,           (WindowProperty_t){ row_height, field_width,         2, field_col } );
    ret[INPUT_NAME_EXACT    ] = ctune_UI_Form.field.create( &rsfind->form, INPUT_NAME_EXACT,     (WindowProperty_t){ row_height, toggle_field_width,  2, toggle_field_col } );
    ret[INPUT_COUNTRY       ] = ctune_UI_Form.field.create( &rsfind->form, INPUT_COUNTRY,        (WindowProperty_t){ row_height, field_width,         4, field_col } );
    ret[INPUT_COUNTRY_EXACT ] = ctune_UI_Form.field.create( &rsfind->form, INPUT_COUNTRY_EXACT,  (WindowProperty_t){ row_height, toggle_field_width,  4, toggle_field_col } );
    ret[INPUT_COUNTRY_CODE  ] = ctune_UI_Form.field.create( &rsfind->form, INPUT_COUNTRY_CODE,   (WindowProperty_t){ row_height, cc_field_width,      6, field_col } );
    ret[INPUT_STATE         ] = ctune_UI_Form.field.create( &rsfind->form, INPUT_STATE,          (WindowProperty_t){ row_height, field_width,         8, field_col } );
    ret[INPUT_STATE_EXACT   ] = ctune_UI_Form.field.create( &rsfind->form, INPUT_STATE_EXACT,    (WindowProperty_t){ row_height, toggle_field_width,  8, toggle_field_col } );
    ret[INPUT_LANGUAGE      ] = ctune_UI_Form.field.create( &rsfind->form, INPUT_LANGUAGE,       (WindowProperty_t){ row_height, field_width,        10, field_col } );
    ret[INPUT_LANGUAGE_EXACT] = ctune_UI_Form.field.create( &rsfind->form, INPUT_LANGUAGE_EXACT, (WindowProperty_t){ row_height, toggle_field_width, 10, toggle_field_col } );
    ret[INPUT_TAGS          ] = ctune_UI_Form.field.create( &rsfind->form, INPUT_TAGS,           (WindowProperty_t){ row_height, field_width,        12, field_col } );
    ret[INPUT_TAGS_EXACT    ] = ctune_UI_Form.field.create( &rsfind->form, INPUT_TAGS_EXACT,     (WindowProperty_t){ row_height, toggle_field_width, 12, toggle_field_col } );
    ret[INPUT_CODEC         ] = ctune_UI_Form.field.create( &rsfind->form, INPUT_CODEC,          (WindowProperty_t){ row_height, field_width,        14, field_col } );
    ret[INPUT_BITRATE_MIN   ] = ctune_UI_Form.field.create( &rsfind->form, INPUT_BITRATE_MIN,    (WindowProperty_t){ row_height, bitrate_input_len,  16, field_col } );
    ret[INPUT_BITRATE_MAX   ] = ctune_UI_Form.field.create( &rsfind->form, INPUT_BITRATE_MAX,    (WindowProperty_t){ row_height, bitrate_input_len,  16, bitrate_max_col } );
    ret[INPUT_ORDER_BY      ] = ctune_UI_Form.field.create( &rsfind->form, INPUT_ORDER_BY,       (WindowProperty_t){ row_height, order_field_width,  18, field_col } );
    ret[INPUT_REVERSE       ] = ctune_UI_Form.field.create( &rsfind->form, INPUT_REVERSE,        (WindowProperty_t){ row_height, toggle_field_width, 20, field_col } );

    const int button_separation = 6;
    const int max_button_width  = (int) ctune_max_ul( strlen( rsfind->cb.getDisplayText( CTUNE_UI_TEXT_BUTTON_SUBMIT ) ),
                                                      strlen( rsfind->cb.getDisplayText( CTUNE_UI_TEXT_BUTTON_CANCEL ) ) );
    const int button_line_ln    = max_button_width + button_separation + max_button_width;
    int       button_line_pad   = 0;

    if( button_line_ln < form_width ) {
        button_line_pad = ( form_width - button_line_ln ) / 2;
    }
    //Buttons                                                                                         rows        cols              y   x
    ret[BUTTON_CANCEL] = ctune_UI_Form.field.create( &rsfind->form, BUTTON_CANCEL, (WindowProperty_t){ row_height, max_button_width, 22, button_line_pad } );
    ret[BUTTON_SAVE  ] = ctune_UI_Form.field.create( &rsfind->form, BUTTON_SAVE,   (WindowProperty_t){ row_height, max_button_width, 22, ( button_line_pad + max_button_width + button_separation ) } );

    for( int i = 0; i < FIELD_LAST; ++i ) {
        if( !ret[i] ) {
            return false; //EARLY RETURN
        }
    }

    ctune_UI_Form.field.setOptions( &rsfind->form, LABEL_EXACT, O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    ctune_UI_Form.field.setOptions( &rsfind->form, LABEL_NAME, O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    ctune_UI_Form.field.setOptions( &rsfind->form, LABEL_COUNTRY, O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    ctune_UI_Form.field.setOptions( &rsfind->form, LABEL_COUNTRY_CODE, O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    ctune_UI_Form.field.setOptions( &rsfind->form, LABEL_STATE, O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    ctune_UI_Form.field.setOptions( &rsfind->form, LABEL_LANGUAGE, O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    ctune_UI_Form.field.setOptions( &rsfind->form, LABEL_TAGS, O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    ctune_UI_Form.field.setOptions( &rsfind->form, LABEL_CODEC, O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    ctune_UI_Form.field.setOptions( &rsfind->form, LABEL_BITRATE, O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    ctune_UI_Form.field.setOptions( &rsfind->form, LABEL_BITRATE_TO, O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    ctune_UI_Form.field.setOptions( &rsfind->form, LABEL_BITRATE_UNIT, O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    ctune_UI_Form.field.setOptions( &rsfind->form, LABEL_ORDER_BY, O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    ctune_UI_Form.field.setOptions( &rsfind->form, LABEL_REVERSE, O_VISIBLE | O_PUBLIC | O_AUTOSKIP );

    ctune_UI_Form.field.setOptions( &rsfind->form, INPUT_NAME, O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE | O_NULLOK );
    ctune_UI_Form.field.setOptions( &rsfind->form, INPUT_COUNTRY, O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE | O_NULLOK );
    set_field_type( ctune_UI_Form.field.get( &rsfind->form, INPUT_COUNTRY ), TYPE_ALPHA, 4 ); //shortest country name length = 4 (Chad, Togo, ...)
    ctune_UI_Form.field.setOptions( &rsfind->form, INPUT_COUNTRY_CODE, O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE | O_INPUT_LIMIT | O_NULLOK | O_STATIC );
    set_field_type( ctune_UI_Form.field.get( &rsfind->form, INPUT_COUNTRY_CODE ), TYPE_ALPHA, 2 );
    ctune_UI_Form.field.setOptions( &rsfind->form, INPUT_STATE, O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE | O_NULLOK );
    ctune_UI_Form.field.setOptions( &rsfind->form, INPUT_LANGUAGE, O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE | O_NULLOK );
    ctune_UI_Form.field.setOptions( &rsfind->form, INPUT_TAGS, O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE | O_NULLOK );
    ctune_UI_Form.field.setOptions( &rsfind->form, INPUT_CODEC, O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE | O_NULLOK );
    ctune_UI_Form.field.setOptions( &rsfind->form, INPUT_BITRATE_MIN, O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE );
    ctune_UI_Form.field.setOptions( &rsfind->form, INPUT_BITRATE_MAX, O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE );
    ctune_UI_Form.field.setOptions( &rsfind->form, INPUT_ORDER_BY, O_VISIBLE | O_PUBLIC | O_ACTIVE | O_STATIC );
    ctune_UI_Form.field.setOptions( &rsfind->form, INPUT_REVERSE, O_VISIBLE | O_PUBLIC | O_ACTIVE | O_STATIC );
    ctune_UI_Form.field.setOptions( &rsfind->form, INPUT_NAME_EXACT, O_VISIBLE | O_PUBLIC | O_ACTIVE | O_STATIC );
    ctune_UI_Form.field.setOptions( &rsfind->form, INPUT_COUNTRY_EXACT, O_VISIBLE | O_PUBLIC | O_ACTIVE | O_STATIC );
    ctune_UI_Form.field.setOptions( &rsfind->form, INPUT_STATE_EXACT, O_VISIBLE | O_PUBLIC | O_ACTIVE | O_STATIC );
    ctune_UI_Form.field.setOptions( &rsfind->form, INPUT_LANGUAGE_EXACT, O_VISIBLE | O_PUBLIC | O_ACTIVE | O_STATIC );
    ctune_UI_Form.field.setOptions( &rsfind->form, INPUT_TAGS_EXACT, O_VISIBLE | O_PUBLIC | O_ACTIVE | O_STATIC );

    ctune_UI_Form.field.setOptions( &rsfind->form, BUTTON_CANCEL, O_VISIBLE | O_PUBLIC | O_ACTIVE | O_STATIC );
    ctune_UI_Form.field.setOptions( &rsfind->form, BUTTON_SAVE, O_VISIBLE | O_PUBLIC | O_ACTIVE | O_STATIC );

    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_RSFind_createFields()] Fields created." );
    return true;
}

/**
 * [PRIVATE] Initialises the Form (callback)
 * @return Success
 */
static bool ctune_UI_RSFind_initForm( void * rsfind ) {
    ctune_UI_RSFind_t * rsf_ptr = rsfind;

    if( rsf_ptr->cache.max_label_width == 0 ) {
        if( !ctune_UI_RSFind_createFields( rsf_ptr ) ) {
            CTUNE_LOG( CTUNE_LOG_FATAL,
                       "[ctune_UI_RSFind_initForm( %p )] Failed to create fields for the form.",
                       rsfind
            );

            return false; //EARLY RETURN
        }
    }

    if( !ctune_UI_RSFind_initFields( rsf_ptr ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_UI_RSFind_initForm( %p )] Failed to init fields for the form.",
                   rsfind
        );

        return false;
    }

    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_UI_RSFind_initForm( %p )] Form desc field size = %i",
               rsfind, rsf_ptr->cache.max_label_width
    );

    return true;
}

/**
 * Creates a base ctune_UI_RSFind_t object
 * @param parent         Pointer to size property of the parent window
 * @param getDisplayText Callback method to get text strings for the display
 * @return Basic un-initialised ctune_UI_RSFind_t object
 */
static ctune_UI_RSFind_t ctune_UI_RSFind_create( const WindowProperty_t * parent, const char * (* getDisplayText)( ctune_UI_TextID_e ) ) {
    return (ctune_UI_RSFind_t) {
        .initialised = false,
        .form        = ctune_UI_Form.create( parent, getDisplayText( CTUNE_UI_TEXT_WIN_TITLE_RSFIND ) ),
        .cache = {
            .filter          = ctune_RadioBrowserFilter.init(),
            .max_label_width = 0,
            .order_width     = 0,
            .order_selection = STATION_ATTR_NONE,
        },
        .cb.getDisplayText = getDisplayText,
    };
}

/**
 * Initialises RSFind (mostly checks base values are OK)
 * @param rsfind     Un-initialised ctune_UI_RSFind_t object
 * @param mouse_ctrl Flag to turn init mouse controls
 * @return Success
 */
static bool ctune_UI_RSFind_init( ctune_UI_RSFind_t * rsfind, bool mouse_ctrl ) {
    if( ctune_UI_Form.init( &rsfind->form, mouse_ctrl, FIELD_COUNT ) ) {
        ctune_UI_Form.scrolling.setAutoScroll( &rsfind->form, 2, 15 );
        ctune_UI_Form.setFormInitCallback( &rsfind->form, rsfind, ctune_UI_RSFind_initForm );
        return ( rsfind->initialised = true );
    }

    CTUNE_LOG( CTUNE_LOG_ERROR,
               "[ctune_UI_RSFind_init( %p, %s )] Failed to initialise!",
               rsfind, ( mouse_ctrl ? "true" : "false" )
    );

    return false;
}

/**
 * Get the initialised state of the instance
 * @param rsfind Pointer to a ctune_UI_RSFind_t object
 * @return Initialised state
 */
static bool ctune_UI_RSFind_isInitialised( ctune_UI_RSFind_t * rsfind ) {
    return rsfind->initialised;
}

/**
 * Switch mouse control UI on/off
 * @param rsfind          Pointer to ctune_UI_RSFind_t object
 * @param mouse_ctrl_flag Flag to turn feature on/off
 */
static void ctune_UI_RSFind_setMouseCtrl( ctune_UI_RSFind_t * rsfind, bool mouse_ctrl_flag ) {
    ctune_UI_Form.mouse.setMouseCtrl( &rsfind->form, mouse_ctrl_flag );
}

/**
 * Create and show a populated window with the find form
 * @param rsfind Pointer to a ctune_UI_RSFind_t object
 * @return Success
 */
bool ctune_UI_RSFind_show( ctune_UI_RSFind_t * rsfind ) {
    if( !ctune_UI_Form.display.show( &rsfind->form ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_RSFind_show( %p )] Failed to show Form.", rsfind );
        return false; //EARLY RETURN
    }

    return true;
}

/**
 * [PRIVATE] Handle a mouse event
 * @param rsfind     Pointer to ctune_UI_RSFind_t object
 * @param event      Mouse event mask
 * @param exit_state Pointer to Form exit state variable
 * @return Exit request
 */
static bool ctune_UI_RSFind_handleMouseEvent( ctune_UI_RSFind_t * rsfind, MEVENT * event, ctune_FormExit_e * exit_state ) {
    const ctune_UI_WinCtrlMask_m win_ctrl = ctune_UI_Form.mouse.isWinCtrl( &rsfind->form, event->y, event->x );
    const ctune_UI_ScrollMask_m  scroll   = ctune_UI_WinCtrlMask.scrollMask( win_ctrl );

    if( win_ctrl ) {
        if( scroll ) {
            if( event->bstate & BUTTON1_CLICKED ) {
                ctune_UI_Form.scrolling.incrementalScroll( &rsfind->form, scroll );

            } else if( event->bstate & BUTTON1_DOUBLE_CLICKED ) {
                ctune_UI_Form.scrolling.incrementalScroll( &rsfind->form, ctune_UI_ScrollMask.setScrollFactor( scroll, 2 ) );

            } else if( event->bstate & BUTTON1_TRIPLE_CLICKED ) {
                ctune_UI_Form.scrolling.incrementalScroll( &rsfind->form, ctune_UI_ScrollMask.setScrollFactor( scroll, 3 ) );

            } else if( event->bstate & BUTTON3_CLICKED ) {
                ctune_UI_Form.scrolling.edgeScroll( &rsfind->form, scroll );
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

    FIELD *   prev_selected_field = ctune_UI_Form.field.current( &rsfind->form );
    FIELD *   clicked_field       = ctune_UI_Form.mouse.click( &rsfind->form, LABEL_COUNT, FIELD_LAST, event->y, event->x, &pos );
    const int clicked_field_id    = ctune_UI_Form.field.currentIndex( &rsfind->form );

    if( clicked_field ) {
        const int toggle_field_id = ctune_UI_RSFind_isToggle( rsfind, clicked_field );

        if( clicked_field_id == INPUT_ORDER_BY ) { //order-by selector field
            size_t middle = ( rsfind->cache.order_width / 2 );

            if( pos < middle ) { //previous option ('<')
                if( rsfind->cache.order_selection > 0 ) {
                    --rsfind->cache.order_selection;
                }

            } else { //next option ('>')
                if( rsfind->cache.order_selection < ( STATION_ATTR_COUNT - 1 ) ) {
                    ++rsfind->cache.order_selection;
                }
            }

            ctune_UI_RSFind_printOrderBy( rsfind, rsfind->cache.order_selection );
            ctune_UI_RSFind_highlightCurrField( rsfind );

        } else if( toggle_field_id >= 0 ) { //toggle field
            ctune_UI_RSFind_highlightCurrField( rsfind );
            ctune_UI_RSFind_toggle( rsfind, ctune_UI_Form.field.currentIndex( &rsfind->form ), clicked_field );

        } else if( prev_selected_field == clicked_field ) { //same editable field
            ctune_UI_Form.input.fwdToFormDriver( &rsfind->form, REQ_BEG_FIELD );

            for( int i = 0; i < pos; ++i ) {
                ctune_UI_Form.input.fwdToFormDriver( &rsfind->form, REQ_RIGHT_CHAR );
            }

        } else if( ctune_UI_RSFind_isButton( rsfind, clicked_field ) ) {
            ctune_UI_RSFind_highlightCurrField( rsfind );
            exit = ctune_UI_RSFind_isExitState( rsfind, clicked_field, exit_state );

        } else {
            ctune_UI_RSFind_highlightCurrField( rsfind );
        }
    }

    return exit;
}

/**
 * Pass keyboard input to the form
 * @return Form exit state
 */
static ctune_FormExit_e ctune_UI_RSFind_captureInput( ctune_UI_RSFind_t * rsfind ) {
    bool             exit       = false;
    ctune_FormExit_e exit_state = CTUNE_UI_FORM_ESC;
    int              character;
    MEVENT           mouse_event;

    ctune_UI_Form.input.start( &rsfind->form );
    ctune_UI_Form.input.fwdToFormDriver( &rsfind->form, REQ_FIRST_FIELD );
    ctune_UI_RSFind_highlightCurrField( rsfind );
    ctune_UI_Form.display.refreshView( &rsfind->form );

    while( !exit ) {
        character = ctune_UI_Form.input.getChar( &rsfind->form );

        switch( ctune_UI_KeyBinding.getAction( CTUNE_UI_CTX_RSFIND, character ) ) {
            case CTUNE_UI_ACTION_ERR   : //fallthrough
            case CTUNE_UI_ACTION_RESIZE: break;

            case CTUNE_UI_ACTION_HELP: {
                ctune_UI_ContextHelp.show( CTUNE_UI_CTX_RSFIND );
                ctune_UI_ContextHelp.captureInput();
            } break;

            case CTUNE_UI_ACTION_ESC: {
                exit_state = CTUNE_UI_FORM_ESC;
                exit = true;
            } break;

            case CTUNE_UI_ACTION_FIELD_BEGIN: {
                ctune_UI_Form.input.fwdToFormDriver( &rsfind->form, REQ_BEG_FIELD );
            } break;

            case CTUNE_UI_ACTION_FIELD_END: {
                ctune_UI_Form.input.fwdToFormDriver( &rsfind->form, REQ_END_FIELD );
            } break;

            case CTUNE_UI_ACTION_FIELD_FIRST: {
                ctune_UI_Form.input.fwdToFormDriver( &rsfind->form, REQ_FIRST_FIELD );
                ctune_UI_RSFind_highlightCurrField( rsfind );
                ctune_UI_Form.scrolling.autoscroll( &rsfind->form );
            } break;

            case CTUNE_UI_ACTION_FIELD_LAST: {
                ctune_UI_Form.input.fwdToFormDriver( &rsfind->form, REQ_LAST_FIELD );
                ctune_UI_RSFind_highlightCurrField( rsfind );
                ctune_UI_Form.scrolling.autoscroll( &rsfind->form );
            } break;

            case CTUNE_UI_ACTION_FIELD_PREV: {
                ctune_UI_Form.input.fwdToFormDriver( &rsfind->form, REQ_PREV_FIELD );
                ctune_UI_RSFind_highlightCurrField( rsfind );
                ctune_UI_Form.input.fwdToFormDriver( &rsfind->form, REQ_END_LINE );
                ctune_UI_Form.scrolling.autoscroll( &rsfind->form );
            } break;

            case CTUNE_UI_ACTION_FIELD_NEXT: {
                ctune_UI_Form.input.fwdToFormDriver( &rsfind->form, REQ_NEXT_FIELD );
                ctune_UI_RSFind_highlightCurrField( rsfind );
                ctune_UI_Form.input.fwdToFormDriver( &rsfind->form, REQ_END_LINE );
                ctune_UI_Form.scrolling.autoscroll( &rsfind->form );
            } break;

            case CTUNE_UI_ACTION_GO_LEFT: {
                if( ctune_UI_Form.field.isCurrent( &rsfind->form, INPUT_ORDER_BY ) ) {
                    if( rsfind->cache.order_selection > 0 ) {
                        --rsfind->cache.order_selection;
                    }

                    ctune_UI_RSFind_printOrderBy( rsfind, rsfind->cache.order_selection );

                } else {
                    ctune_UI_Form.input.fwdToFormDriver( &rsfind->form, REQ_LEFT_CHAR );
                }
            } break;

            case CTUNE_UI_ACTION_GO_RIGHT: {
                if( ctune_UI_Form.field.isCurrent( &rsfind->form, INPUT_ORDER_BY ) ) {
                    if( rsfind->cache.order_selection < ( STATION_ATTR_COUNT - 1 ) ) {
                        ++rsfind->cache.order_selection;
                    }

                    ctune_UI_RSFind_printOrderBy( rsfind, rsfind->cache.order_selection );

                } else {
                    ctune_UI_Form.input.fwdToFormDriver( &rsfind->form, REQ_RIGHT_CHAR );
                }
            } break;

            case CTUNE_UI_ACTION_TRIGGER: {
                if( ( exit = ctune_UI_RSFind_isExitState( rsfind, ctune_UI_Form.field.current( &rsfind->form ), &exit_state ) ) ) {
                    if( exit_state == CTUNE_UI_FORM_SUBMIT && !ctune_UI_RSFind_packFieldValues( rsfind, &rsfind->cache.filter ) ) {
                        exit = false;
                    }

                } else {
                    ctune_UI_Form.input.fwdToFormDriver( &rsfind->form, REQ_NEXT_FIELD );
                    ctune_UI_RSFind_highlightCurrField( rsfind );
                    ctune_UI_Form.input.fwdToFormDriver( &rsfind->form, REQ_END_LINE );
                    ctune_UI_Form.scrolling.autoscroll( &rsfind->form );
                }
            } break;

            case CTUNE_UI_ACTION_TOGGLE: { //'x'
                int toggle_id = ctune_UI_RSFind_isToggle( rsfind, ctune_UI_Form.field.current( &rsfind->form ) );

                if( toggle_id >= 0 ) {
                    ctune_UI_RSFind_toggle( rsfind, toggle_id, ctune_UI_Form.field.current( &rsfind->form ) );
                } else {
                    ctune_UI_Form.input.fwdToFormDriver( &rsfind->form, character );
                }
            } break;

            case CTUNE_UI_ACTION_TOGGLE_ALT: { //'space'
                if( ( exit = ctune_UI_RSFind_isExitState( rsfind, ctune_UI_Form.field.current( &rsfind->form ), &exit_state ) ) ) {
                    if( exit_state == CTUNE_UI_FORM_SUBMIT && !ctune_UI_RSFind_packFieldValues( rsfind, &rsfind->cache.filter ) ) {
                        exit = false;
                    }

                } else {
                    int toggle_id = ctune_UI_RSFind_isToggle( rsfind, ctune_UI_Form.field.current( &rsfind->form ) );

                    if( toggle_id >= 0 ) {
                        ctune_UI_RSFind_toggle( rsfind, toggle_id, ctune_UI_Form.field.current( &rsfind->form ) );
                    } else {
                        ctune_UI_Form.input.fwdToFormDriver( &rsfind->form, character );
                    }
                }
            } break;

            case CTUNE_UI_ACTION_CLEAR_ALL: {
                ctune_UI_Form.field.clearRange( &rsfind->form, LABEL_COUNT, FIELD_COUNT );
                ctune_UI_RSFind_initFields( rsfind );
                ctune_UI_Form.field.setCurrent( &rsfind->form, LABEL_COUNT );
                ctune_UI_RSFind_highlightCurrField( rsfind );
            } break;

            case CTUNE_UI_ACTION_CLEAR_SELECTED: {
                ctune_UI_Form.input.fwdToFormDriver( &rsfind->form, REQ_CLR_FIELD );
            } break;

            case CTUNE_UI_ACTION_DEL_PREV: {
                ctune_UI_Form.input.fwdToFormDriver( &rsfind->form, REQ_DEL_PREV );
            } break;

            case CTUNE_UI_ACTION_DEL_NEXT: {
                ctune_UI_Form.input.fwdToFormDriver( &rsfind->form, REQ_DEL_CHAR );
            } break;

            case CTUNE_UI_ACTION_MOUSE_EVENT: {
                if( getmouse( &mouse_event ) == OK ) {
                    if( ( exit = ctune_UI_RSFind_handleMouseEvent( rsfind, &mouse_event, &exit_state ) ) ) {
                        if( exit_state == CTUNE_UI_FORM_SUBMIT && !ctune_UI_RSFind_packFieldValues( rsfind, &rsfind->cache.filter ) ) {
                            exit = false;
                        }
                    }
                }
            } break;

            default: {
                ctune_UI_Form.input.fwdToFormDriver( &rsfind->form, character );
            } break;
        }

        ctune_UI_Form.display.refreshView( &rsfind->form );
    }

    ctune_UI_Form.input.stop( &rsfind->form );

    return ( exit_state );
}

/**
 * Gets the internal filter object
 * @param rsfind Pointer to a ctune_UI_RSFind_t object
 * @return Pointer to internal RadioStationFilter_t DTO
 */
static ctune_RadioBrowserFilter_t * ctune_UI_RSFind_getFilter( ctune_UI_RSFind_t * rsfind ) {
    return &rsfind->cache.filter;
}

/**
 * De-allocates the form and its fields
 * @param rsfind Pointer to a ctune_UI_RSFind_t object
 */
static void ctune_UI_RSFind_free( ctune_UI_RSFind_t * rsfind ) {
    ctune_UI_Form.freeContent( &rsfind->form );
    ctune_RadioBrowserFilter.freeContent( &rsfind->cache.filter );
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_RSFind_free( %p )] RSFind freed.", rsfind );
}

/**
 * Namespace constructor
 */
const struct ctune_UI_RSFind_Namespace ctune_UI_RSFind = {
    .create        = &ctune_UI_RSFind_create,
    .init          = &ctune_UI_RSFind_init,
    .isInitialised = &ctune_UI_RSFind_isInitialised,
    .setMouseCtrl  = &ctune_UI_RSFind_setMouseCtrl,
    .show          = &ctune_UI_RSFind_show,
    .captureInput  = &ctune_UI_RSFind_captureInput,
    .getFilter     = &ctune_UI_RSFind_getFilter,
    .free          = &ctune_UI_RSFind_free,
};