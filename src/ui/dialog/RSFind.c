#include "RSFind.h"

#include <sys/ioctl.h>
#include <errno.h>

#include "../../logger/Logger.h"
#include "../../ctune_err.h"
#include "../definitions/KeyBinding.h"
#include "../definitions/Theme.h"
#include "ContextHelp.h"
#include "../Resizer.h"

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
 * Private constant variables used across all instances
 */
static const struct {
    int row_height;
    int row_pos[FIELD_COUNT];

} private = {
    .row_height = 1,
    .row_pos    = {
        [LABEL_EXACT         ] =  0,
        [LABEL_NAME          ] =  2, [INPUT_NAME          ] =  2, [INPUT_NAME_EXACT    ] =  2,
        [LABEL_COUNTRY       ] =  4, [INPUT_COUNTRY       ] =  4, [INPUT_COUNTRY_EXACT ] =  4,
        [LABEL_COUNTRY_CODE  ] =  6, [INPUT_COUNTRY_CODE  ] =  6,
        [LABEL_STATE         ] =  8, [INPUT_STATE         ] =  8, [INPUT_STATE_EXACT   ] =  8,
        [LABEL_LANGUAGE      ] = 10, [INPUT_LANGUAGE      ] = 10, [INPUT_LANGUAGE_EXACT] = 10,
        [LABEL_TAGS          ] = 12, [INPUT_TAGS          ] = 12, [INPUT_TAGS_EXACT    ] = 12,
        [LABEL_CODEC         ] = 14, [INPUT_CODEC         ] = 14,
        [LABEL_BITRATE       ] = 16, [INPUT_BITRATE_MIN   ] = 16, [LABEL_BITRATE_TO    ] = 16, [INPUT_BITRATE_MAX   ] = 16, [LABEL_BITRATE_UNIT  ] = 16,
        [LABEL_ORDER_BY      ] = 18, [INPUT_ORDER_BY      ] = 18,
        [LABEL_REVERSE       ] = 20, [INPUT_REVERSE       ] = 20,
        [BUTTON_CANCEL       ] = 22, [BUTTON_SAVE         ] = 22,
        [FIELD_LAST          ] = 28,
    },
};

/**
 * [PRIVATE] Scroll form when given field is out-of-view
 * @param rsfind Pointer to a ctune_UI_RSFind_t object
 * @param field  Field to base scrolling on (currently selected?)
 */
static void ctune_UI_RSFind_autoScroll( ctune_UI_RSFind_t * rsfind, const FIELD * field ) {
    if( field == NULL )
        return; //EARLY RETURN

    int field_pos_y = 0;
    int field_pos_x = 0;

    if( field_info( field, NULL, NULL, &field_pos_y, &field_pos_x, NULL, NULL ) != E_OK ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_RSEdit_autoScroll( %p, %p )] Failed to get info for field." );
        return; //EARLY RETURN
    }

    ctune_UI_Dialog.autoScroll( &rsfind->dialog, field_pos_y, field_pos_x );
}

/**
 * [PRIVATE] Sets the buffer of a form's toggle field
 * @param rsfind Pointer to a ctune_UI_RSFind_t object
 * @param state  Toggle state
 * @param field  Toggle field to set the buffer of
 */
static void ctune_UI_RSFind_setToggleField( ctune_UI_RSFind_t * rsfind, bool state, FIELD * field ) {
    set_field_buffer( field, 0, ( state
                                  ? rsfind->cb.getDisplayText( CTUNE_UI_TEXT_TOGGLE_FIELD_ON )
                                  : rsfind->cb.getDisplayText( CTUNE_UI_TEXT_TOGGLE_FIELD_OFF ) )
    );
}

/**
 * [PRIVATE] Toggles a given field
 * @param rsfind Pointer to a ctune_UI_RSFind_t object
 * @param id     Toggle field ID of field
 * @param field  Form input field
 */
static void ctune_UI_RSFind_toggle( ctune_UI_RSFind_t * rsfind, RSFind_Input_e id, FIELD * field ) {
    if( field != rsfind->cache.fields[ id ] )
        return; //EARLY RETURN

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

    ctune_UI_RSFind_setToggleField( rsfind, state, field );
}

/**
 * [PRIVATE] Checks if field is a toggle input
 * @param rsfind Pointer to a ctune_UI_RSFind_t object
 * @param field  Form field
 * @return Toggle field ID (-1 if not valid)
 */
static int ctune_UI_RSFind_isToggle( const ctune_UI_RSFind_t * rsfind, const FIELD * field ) {
    if( field == NULL )
        return -1;
    if( field == rsfind->cache.fields[ INPUT_NAME_EXACT ] )
        return INPUT_NAME_EXACT;
    if( field == rsfind->cache.fields[ INPUT_COUNTRY_EXACT ] )
        return INPUT_COUNTRY_EXACT;
    if( field == rsfind->cache.fields[ INPUT_STATE_EXACT ] )
        return INPUT_STATE_EXACT;
    if( field == rsfind->cache.fields[ INPUT_LANGUAGE_EXACT ] )
        return INPUT_LANGUAGE_EXACT;
    if( field == rsfind->cache.fields[ INPUT_TAGS_EXACT ] )
        return INPUT_TAGS_EXACT;
    if( field == rsfind->cache.fields[ INPUT_REVERSE ] )
        return INPUT_REVERSE;
    return -1;
}

/**
 * [PRIVATE] Check if field is a button
 * @param rsfind Pointer to a ctune_UI_RSFind_t object
 * @param field  Form field
 * @return Button field status
 */
static bool ctune_UI_RSFind_isButton( const ctune_UI_RSFind_t * rsfind, const FIELD * field ) {
    if( field == NULL )
        return false;
    return ( field == rsfind->cache.fields[ BUTTON_CANCEL ]
          || field == rsfind->cache.fields[ BUTTON_SAVE   ] );
}

/**
 * [PRIVATE] Check if given field is a form exit
 * @param rsfind    Pointer to a ctune_UI_RSFind_t object
 * @param field     FIELD pointer
 * @param exit_type Variable pointer to set the exit type of the field
 * @return Exit state
 */
static bool ctune_UI_RSFind_isExitState( const ctune_UI_RSFind_t * rsfind, const FIELD * field, ctune_FormExit_e * exit_type ) {
    if( ctune_UI_RSFind_isButton( rsfind, field ) ) {
        if( field == rsfind->cache.fields[ BUTTON_CANCEL ] ) {
            *exit_type = CTUNE_UI_FORM_CANCEL;
            return true;
        }

        if( field == rsfind->cache.fields[ BUTTON_SAVE ] ) {
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
    if( selection >= STATION_ATTR_COUNT )
        return; //EARLY RETURN

    String_t out = String.init();

    if( selection == 0 )
        String.append_back( &out, "  " );
    else
        String.append_back( &out, "< " );

    size_t length     = strlen( rsfind->cache.order_items[ selection ] );
    size_t whitespace = ( length <= ( rsfind->cache.order_width - 4 ) ? ( ( rsfind->cache.order_width - 4 ) - length ) : 0 );

    String.append_back( &out, rsfind->cache.order_items[ selection ] );

    for( size_t i = 0; i < whitespace; ++ i )
        String.append_back( &out, " " );

    if( selection >= ( STATION_ATTR_COUNT - 1 ) )
        String.append_back( &out, "  " );
    else
        String.append_back( &out, " >" );

    set_field_buffer( rsfind->cache.fields[ INPUT_ORDER_BY ], 0, out._raw );

    String.free( &out );

    ctune_RadioBrowserFilter.set.ordering( &rsfind->cache.filter, selection );
}

/**
 * [PRIVATE] Highlights input field where the cursor is at
 * @param rsfind     Pointer to a ctune_UI_RSFind_t object
 * @param curr_field Current field
 */
static void ctune_UI_RSFind_highlightCurrField( ctune_UI_RSFind_t * rsfind, FIELD * curr_field ) {
    curs_set( ( ctune_UI_RSFind_isButton( rsfind, curr_field ) || ctune_UI_RSFind_isToggle( rsfind, curr_field ) >= 0 ) ? 0 : 1 );

    for( int i = 0; i < LABEL_COUNT; ++i ) {
        set_field_back( rsfind->cache.fields[ i ], A_NORMAL );
    }

    for( int i = LABEL_COUNT; i < FIELD_COUNT; ++i ) {
        if( rsfind->cache.fields[ i ] == curr_field ) {
            set_field_back( rsfind->cache.fields[ i ], A_REVERSE );
        } else {
            if( ctune_UI_RSFind_isButton( rsfind, rsfind->cache.fields[ i ] ) || ctune_UI_RSFind_isToggle( rsfind, rsfind->cache.fields[ i ] ) >= 0 )
                set_field_back( rsfind->cache.fields[ i ], A_NORMAL );
            else
                set_field_back( rsfind->cache.fields[ i ], A_UNDERLINE );
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
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_RSFind_packFieldValues( %p )] Packing field values...", rsfind );

    bool error_state = false;

    if( field_status( rsfind->cache.fields[ INPUT_NAME ] ) )
        ctune_RadioBrowserFilter.set.name( filter, ctune_trimspace( field_buffer( rsfind->cache.fields[ INPUT_NAME ], 0 ) ) );

    if( field_status( rsfind->cache.fields[ INPUT_COUNTRY ] ) )
        ctune_RadioBrowserFilter.set.country( filter, ctune_trimspace( field_buffer( rsfind->cache.fields[ INPUT_COUNTRY ], 0 ) ) );

    if( field_status( rsfind->cache.fields[ INPUT_COUNTRY_CODE ] ) ) {
        char * cc = ctune_trimspace( field_buffer( rsfind->cache.fields[ INPUT_COUNTRY_CODE ], 0 ) );

        ctune_RadioBrowserFilter.set.countryCode( filter, cc );

        if( cc )
            free( cc );
    }

    if( field_status( rsfind->cache.fields[ INPUT_STATE ] ) )
        ctune_RadioBrowserFilter.set.state( filter, ctune_trimspace( field_buffer( rsfind->cache.fields[ INPUT_STATE ], 0 ) ) );

    if( field_status( rsfind->cache.fields[ INPUT_LANGUAGE ] ) )
        ctune_RadioBrowserFilter.set.language( filter, ctune_trimspace( field_buffer( rsfind->cache.fields[ INPUT_LANGUAGE ], 0 ) ) );

    if( field_status( rsfind->cache.fields[ INPUT_CODEC ] ) )
        ctune_RadioBrowserFilter.set.codec( filter, ctune_trimspace( field_buffer( rsfind->cache.fields[ INPUT_CODEC ], 0 ) ) );

    { //Tag(s)
        StrList_t * list      = ctune_RadioBrowserFilter.get.tagList( filter );
        size_t      val_count = ctune_splitcss( field_buffer( rsfind->cache.fields[ INPUT_TAGS ], 0 ), list );

        CTUNE_LOG( CTUNE_LOG_DEBUG,
                   "[ctune_UI_RSFind_packFieldValues( %p )] Found %i values in field 'INPUT_TAGS': \"%s\"",
                   rsfind, val_count, field_buffer( rsfind->cache.fields[ INPUT_TAGS ], 0 )
        );

        if( val_count == 1 ) {
            ctune_RadioBrowserFilter.set.tag( filter, strdup( StrList.at( list, 0 )->data ) );

            if( ctune_RadioBrowserFilter.get.tag( filter ) != NULL ) {
                StrList.free_strlist( list );
            }
        }
    }

    { //Bitrate min/max
        ulong min = strtoul( field_buffer( rsfind->cache.fields[ INPUT_BITRATE_MIN ], 0 ), NULL, 10 );
        ulong max = strtoul( field_buffer( rsfind->cache.fields[ INPUT_BITRATE_MAX ], 0 ), NULL, 10 );



        if( max == 0 ) {
            max = CTUNE_RADIOBROSWERFILTER_BITRATE_MAX_DFLT;

            CTUNE_LOG( CTUNE_LOG_DEBUG,
                       "[ctune_UI_RSFind_packFieldValues( %p )] max bitrate = 0/ERR: auto-set to default maximum value (%lu).",
                       rsfind, max
            );

            set_field_buffer( rsfind->cache.fields[ INPUT_BITRATE_MAX ], 0, CTUNE_RADIOBROSWERFILTER_BITRATE_MAX_DFLT_STR );
        }

        if( max < min ) { //validation
            set_current_field( rsfind->form, rsfind->cache.fields[INPUT_BITRATE_MIN] );
            ctune_UI_RSFind_highlightCurrField( rsfind, current_field( rsfind->form ) );

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

    set_field_buffer( rsfind->cache.fields[ LABEL_NAME         ], 0, rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_STATION_NAME ) );
    set_field_buffer( rsfind->cache.fields[ LABEL_COUNTRY      ], 0, rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_COUNTRY ) );
    set_field_buffer( rsfind->cache.fields[ LABEL_COUNTRY_CODE ], 0, rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_COUNTRY_CODE ) );
    set_field_buffer( rsfind->cache.fields[ LABEL_STATE        ], 0, rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_STATE ) );
    set_field_buffer( rsfind->cache.fields[ LABEL_LANGUAGE     ], 0, rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_LANGUAGE ) );
    set_field_buffer( rsfind->cache.fields[ LABEL_TAGS         ], 0, rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_TAGS ) );
    set_field_buffer( rsfind->cache.fields[ LABEL_CODEC        ], 0, rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_CODEC ) );
    set_field_buffer( rsfind->cache.fields[ LABEL_BITRATE      ], 0, rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_BITRATE_RANGE ) );
    set_field_buffer( rsfind->cache.fields[ LABEL_BITRATE_TO   ], 0, rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_BITRATE_RANGE_SEPARATOR ) );
    set_field_buffer( rsfind->cache.fields[ LABEL_BITRATE_UNIT ], 0, rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_BITRATE_UNIT_LONG ) );
    set_field_buffer( rsfind->cache.fields[ LABEL_ORDER_BY     ], 0, rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_ORDER_BY ) );
    set_field_buffer( rsfind->cache.fields[ LABEL_REVERSE      ], 0, rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_REVERSE_ORDER ) );
    set_field_buffer( rsfind->cache.fields[ LABEL_EXACT        ], 0, rsfind->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_EXACT_MATCH ) );
    set_field_buffer( rsfind->cache.fields[ INPUT_NAME         ], 0, ctune_fallbackStr( ctune_RadioBrowserFilter.get.name( filter ), "" ) );
    set_field_buffer( rsfind->cache.fields[ INPUT_COUNTRY      ], 0, ctune_fallbackStr( ctune_RadioBrowserFilter.get.country( filter ), "" ) );
    set_field_buffer( rsfind->cache.fields[ INPUT_COUNTRY_CODE ], 0, ctune_fallbackStr( ctune_RadioBrowserFilter.get.countryCode( filter ), "" ) );
    set_field_buffer( rsfind->cache.fields[ INPUT_STATE        ], 0, ctune_fallbackStr( ctune_RadioBrowserFilter.get.state( filter ), "" ) );
    set_field_buffer( rsfind->cache.fields[ INPUT_LANGUAGE     ], 0, ctune_fallbackStr( ctune_RadioBrowserFilter.get.language( filter ), "" ) );
    set_field_buffer( rsfind->cache.fields[ INPUT_CODEC        ], 0, ctune_fallbackStr( ctune_RadioBrowserFilter.get.codec( filter ), "" ) );

    ctune_RadioBrowserFilter.set.ordering( filter, STATION_ATTR_NONE );
    ctune_UI_RSFind_printOrderBy( rsfind, ctune_RadioBrowserFilter.get.ordering( filter ) ); //INPUT_ORDER_BY

    { //Tag(s)
        const StrList_t * tag_list_ptr = ctune_RadioBrowserFilter.get.tagList( filter ); //shortcut pointer

        if( ctune_RadioBrowserFilter.get.tag( filter ) != NULL ) {
            set_field_buffer( rsfind->cache.fields[ INPUT_TAGS ], 0, ctune_RadioBrowserFilter.get.tag( filter ) );

        } else if( tag_list_ptr != NULL && !StrList.empty( tag_list_ptr ) ) {
            String_t list = String.init();

            if( StrList.stringify( tag_list_ptr, &list, ',' ) != StrList.size( tag_list_ptr ) ) {
                CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_RSFind_initFields()] Failed to stringify the filter's tagList." );
                String.free( &list );
                return false; //EARLY RETURN
            }

            set_field_buffer( rsfind->cache.fields[ INPUT_TAGS ], 0, list._raw );
            String.free( &list );
        } else {
            set_field_buffer( rsfind->cache.fields[ INPUT_TAGS ], 0, "" );
        }
    }

    ctune_UI_RSFind_setToggleField( rsfind, ctune_RadioBrowserFilter.get.reverseToggle( filter ), rsfind->cache.fields[ INPUT_REVERSE ] );
    ctune_UI_RSFind_setToggleField( rsfind, ctune_RadioBrowserFilter.get.exactNameToggle( filter ), rsfind->cache.fields[ INPUT_NAME_EXACT ] );
    ctune_UI_RSFind_setToggleField( rsfind, ctune_RadioBrowserFilter.get.exactCountryToggle( filter ), rsfind->cache.fields[ INPUT_COUNTRY_EXACT ] );
    ctune_UI_RSFind_setToggleField( rsfind, ctune_RadioBrowserFilter.get.exactStateToggle( filter ), rsfind->cache.fields[ INPUT_STATE_EXACT ] );
    ctune_UI_RSFind_setToggleField( rsfind, ctune_RadioBrowserFilter.get.exactLanguageToggle( filter ), rsfind->cache.fields[ INPUT_LANGUAGE_EXACT ] );
    ctune_UI_RSFind_setToggleField( rsfind, ctune_RadioBrowserFilter.get.exactTagToggle( filter ), rsfind->cache.fields[ INPUT_TAGS_EXACT ] );

    { //Bitrate fields (ulong) - loads the min/max defaults from the filter object as a guide
        String_t bitrate_min = String.init();
        String_t bitrate_max = String.init();
        ctune_utos( ctune_RadioBrowserFilter.get.bitrateMin( filter ), &bitrate_min );
        ctune_utos( ctune_RadioBrowserFilter.get.bitrateMax( filter ), &bitrate_max );
        set_field_buffer( rsfind->cache.fields[ INPUT_BITRATE_MIN ], 0, bitrate_min._raw );
        set_field_buffer( rsfind->cache.fields[ INPUT_BITRATE_MAX ], 0, bitrate_max._raw );
        String.free( &bitrate_min );
        String.free( &bitrate_max );
    }

    set_field_buffer( rsfind->cache.fields[ BUTTON_CANCEL ], 0, rsfind->cb.getDisplayText( CTUNE_UI_TEXT_BUTTON_CANCEL ) );
    set_field_buffer( rsfind->cache.fields[ BUTTON_SAVE   ], 0, rsfind->cb.getDisplayText( CTUNE_UI_TEXT_BUTTON_SUBMIT ) );

    set_field_back( rsfind->cache.fields[ BUTTON_CANCEL ], ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_DFLT ) );
    set_field_fore( rsfind->cache.fields[ BUTTON_CANCEL ], ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_DFLT ) );
    set_field_back( rsfind->cache.fields[ BUTTON_SAVE   ], ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_DFLT ) );
    set_field_fore( rsfind->cache.fields[ BUTTON_SAVE   ], ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_DFLT ) );

    return true;
}

/**
 * [PRIVATE] Clears all input fields on the form and move back to first field
 * @param rsfind Pointer to a ctune_UI_RSFind_t object
 */
static void ctune_UI_RSFind_clearAllFields( ctune_UI_RSFind_t * rsfind ) {
    for( int i = LABEL_COUNT; i < FIELD_COUNT; ++i ) {
        set_current_field( rsfind->form, rsfind->cache.fields[ i ] );
        form_driver( rsfind->form, REQ_CLR_FIELD );
    }

    ctune_UI_RSFind_initFields( rsfind );

    set_current_field( rsfind->form, rsfind->cache.fields[LABEL_COUNT] );
}

/**
 * [PRIVATE] Creates the fields (Called once and cached for the remainder of the runtime)
 * @param rsfind Pointer to a ctune_UI_RSFind_t object
 * @return Success
 */
static bool ctune_UI_RSFind_createFields( ctune_UI_RSFind_t * rsfind ) {
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_RSFind_createFields( %p )] Creating form fields...", rsfind );

    //Order-By "dropdown" window on field
    rsfind->cache.order_items[ STATION_ATTR_NONE           ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_NONE );
    rsfind->cache.order_items[ STATION_ATTR_NAME           ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_NAME );
    rsfind->cache.order_items[ STATION_ATTR_URL            ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_URL );
    rsfind->cache.order_items[ STATION_ATTR_HOMEPAGE       ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_HOMEPAGE );
    rsfind->cache.order_items[ STATION_ATTR_FAVICON        ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_FAVICON );
    rsfind->cache.order_items[ STATION_ATTR_TAGS           ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_TAGS );
    rsfind->cache.order_items[ STATION_ATTR_COUNTRY        ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_COUNTRY );
    rsfind->cache.order_items[ STATION_ATTR_STATE          ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_STATE );
    rsfind->cache.order_items[ STATION_ATTR_LANGUAGE       ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_LANGUAGE );
    rsfind->cache.order_items[ STATION_ATTR_VOTES          ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_VOTES );
    rsfind->cache.order_items[ STATION_ATTR_CODEC          ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_CODEC );
    rsfind->cache.order_items[ STATION_ATTR_BITRATE        ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_BITRATE );
    rsfind->cache.order_items[ STATION_ATTR_LASTCHECKOK    ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_LASTCHECKOK );
    rsfind->cache.order_items[ STATION_ATTR_LASTCHECKTIME  ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_LASTCHECKTIME );
    rsfind->cache.order_items[ STATION_ATTR_CLICKTIMESTAMP ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_CLICKTIMESTAMP );
    rsfind->cache.order_items[ STATION_ATTR_CLICKCOUNT     ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_CLICKCOUNT );
    rsfind->cache.order_items[ STATION_ATTR_CLICKTREND     ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_CLICKTREND );
    rsfind->cache.order_items[ STATION_ATTR_RANDOM         ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_RANDOM );
    rsfind->cache.order_items[ STATION_ATTR_STATIONCOUNT   ] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_STATIONCOUNT );
    rsfind->cache.order_items[ STATION_ATTR_CHANGETIMESTAMP] = rsfind->cb.getDisplayText( CTUNE_UI_TEXT_ORDERBY_CHANGETIMESTAMP );

    for( size_t i = 0; i < STATION_ATTR_COUNT; ++i )
        rsfind->cache.order_width = (int) ctune_max_ul( rsfind->cache.order_width, strlen( rsfind->cache.order_items[ i ] ) );
    rsfind->cache.order_width += 4; //"< " + string + " >"

//      <label_col>             <toggle_label_col>
//      |                       |
//      |     <field_col>       |   <toggle_field_col>
//      |     |                 |   |
//    .--------------[ FORM ]---------------.
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

    //[ Field labels ]                                        rows        cols                y                                        x
    rsfind->cache.fields[ LABEL_EXACT          ] = new_field( row_height, label_col_width,    private.row_pos[ LABEL_EXACT          ], toggle_label_col, 0, 0 );
    rsfind->cache.fields[ LABEL_NAME           ] = new_field( row_height, label_col_width,    private.row_pos[ LABEL_NAME           ], label_col, 0, 0 );
    rsfind->cache.fields[ LABEL_COUNTRY        ] = new_field( row_height, label_col_width,    private.row_pos[ LABEL_COUNTRY        ], label_col, 0, 0 );
    rsfind->cache.fields[ LABEL_COUNTRY_CODE   ] = new_field( row_height, label_col_width,    private.row_pos[ LABEL_COUNTRY_CODE   ], label_col, 0, 0 );
    rsfind->cache.fields[ LABEL_STATE          ] = new_field( row_height, label_col_width,    private.row_pos[ LABEL_STATE          ], label_col, 0, 0 );
    rsfind->cache.fields[ LABEL_LANGUAGE       ] = new_field( row_height, label_col_width,    private.row_pos[ LABEL_LANGUAGE       ], label_col, 0, 0 );
    rsfind->cache.fields[ LABEL_TAGS           ] = new_field( row_height, label_col_width,    private.row_pos[ LABEL_TAGS           ], label_col, 0, 0 );
    rsfind->cache.fields[ LABEL_CODEC          ] = new_field( row_height, label_col_width,    private.row_pos[ LABEL_CODEC          ], label_col, 0, 0 );
    rsfind->cache.fields[ LABEL_BITRATE        ] = new_field( row_height, label_col_width,    private.row_pos[ LABEL_BITRATE        ], label_col, 0, 0 );
    rsfind->cache.fields[ LABEL_BITRATE_TO     ] = new_field( row_height, bitrate_sep_len,    private.row_pos[ LABEL_BITRATE_TO     ], bitrate_sep_col, 0, 0 );
    rsfind->cache.fields[ LABEL_BITRATE_UNIT   ] = new_field( row_height, bitrate_unit_len,   private.row_pos[ LABEL_BITRATE_UNIT   ], bitrate_unit_col, 0, 0 );
    rsfind->cache.fields[ LABEL_ORDER_BY       ] = new_field( row_height, label_col_width,    private.row_pos[ LABEL_ORDER_BY       ], label_col, 0, 0 );
    rsfind->cache.fields[ LABEL_REVERSE        ] = new_field( row_height, label_col_width,    private.row_pos[ LABEL_REVERSE        ], label_col, 0, 0 );
    //[ Field inputs ]                                        rows        cols                y                                        x
    rsfind->cache.fields[ INPUT_NAME           ] = new_field( row_height, field_width,        private.row_pos[ INPUT_NAME           ], field_col, 0, 0 );
    rsfind->cache.fields[ INPUT_NAME_EXACT     ] = new_field( row_height, toggle_field_width, private.row_pos[ INPUT_NAME_EXACT     ], toggle_field_col, 0, 0 );
    rsfind->cache.fields[ INPUT_COUNTRY        ] = new_field( row_height, field_width,        private.row_pos[ INPUT_COUNTRY        ], field_col, 0, 0 );
    rsfind->cache.fields[ INPUT_COUNTRY_EXACT  ] = new_field( row_height, toggle_field_width, private.row_pos[ INPUT_COUNTRY_EXACT  ], toggle_field_col, 0, 0 );
    rsfind->cache.fields[ INPUT_COUNTRY_CODE   ] = new_field( row_height, cc_field_width,     private.row_pos[ INPUT_COUNTRY_CODE   ], field_col, 0, 0 );
    rsfind->cache.fields[ INPUT_STATE          ] = new_field( row_height, field_width,        private.row_pos[ INPUT_STATE          ], field_col, 0, 0 );
    rsfind->cache.fields[ INPUT_STATE_EXACT    ] = new_field( row_height, toggle_field_width, private.row_pos[ INPUT_STATE_EXACT    ], toggle_field_col, 0, 0 );
    rsfind->cache.fields[ INPUT_LANGUAGE       ] = new_field( row_height, field_width,        private.row_pos[ INPUT_LANGUAGE       ], field_col, 0, 0 );
    rsfind->cache.fields[ INPUT_LANGUAGE_EXACT ] = new_field( row_height, toggle_field_width, private.row_pos[ INPUT_LANGUAGE_EXACT ], toggle_field_col, 0, 0 );
    rsfind->cache.fields[ INPUT_TAGS           ] = new_field( row_height, field_width,        private.row_pos[ INPUT_TAGS           ], field_col, 0, 0 );
    rsfind->cache.fields[ INPUT_TAGS_EXACT     ] = new_field( row_height, toggle_field_width, private.row_pos[ INPUT_TAGS_EXACT     ], toggle_field_col, 0, 0 );
    rsfind->cache.fields[ INPUT_CODEC          ] = new_field( row_height, field_width,        private.row_pos[ INPUT_CODEC          ], field_col, 0, 0 );
    rsfind->cache.fields[ INPUT_BITRATE_MIN    ] = new_field( row_height, bitrate_input_len,  private.row_pos[ INPUT_BITRATE_MIN    ], field_col, 0, 0 );
    rsfind->cache.fields[ INPUT_BITRATE_MAX    ] = new_field( row_height, bitrate_input_len,  private.row_pos[ INPUT_BITRATE_MAX    ], bitrate_max_col, 0, 0 );
    rsfind->cache.fields[ INPUT_ORDER_BY       ] = new_field( row_height, order_field_width,  private.row_pos[ INPUT_ORDER_BY       ], field_col, 0, 0 );
    rsfind->cache.fields[ INPUT_REVERSE        ] = new_field( row_height, toggle_field_width, private.row_pos[ INPUT_REVERSE        ], field_col, 0, 0 );

    const int button_separation = 6;
    const int max_button_width  = (int) ctune_max_ul( strlen( rsfind->cb.getDisplayText( CTUNE_UI_TEXT_BUTTON_SUBMIT ) ),
                                                      strlen( rsfind->cb.getDisplayText( CTUNE_UI_TEXT_BUTTON_CANCEL ) ) );
    const int button_line_ln    = max_button_width + button_separation + max_button_width;
    int       button_line_pad   = 0;

    if( button_line_ln < form_width ) {
        button_line_pad = ( form_width - button_line_ln ) / 2;
    }
    //[ Buttons ]                                      rows        cols              y   x
    rsfind->cache.fields[ BUTTON_CANCEL ] = new_field( row_height, max_button_width, private.row_pos[ BUTTON_CANCEL ], button_line_pad, 0, 0 );
    rsfind->cache.fields[ BUTTON_SAVE   ] = new_field( row_height, max_button_width, private.row_pos[ BUTTON_SAVE   ], ( button_line_pad + max_button_width + button_separation ), 0, 0 );
    rsfind->cache.fields[ FIELD_LAST    ] = NULL;

    int err_state = false;

    for( int i = 0; i < FIELD_LAST; ++i ) {
        if( rsfind->cache.fields[i] == NULL ) {
            CTUNE_LOG( CTUNE_LOG_FATAL,
                       "[ctune_UI_RSFind_createFields()] Failed to create Field (#%i): %s",
                       i, strerror( errno )
            );
            err_state = true;
        }
    }

    if( err_state )
        return false;

    set_field_opts( rsfind->cache.fields[ LABEL_EXACT         ], O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    set_field_opts( rsfind->cache.fields[ LABEL_NAME          ], O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    set_field_opts( rsfind->cache.fields[ LABEL_COUNTRY       ], O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    set_field_opts( rsfind->cache.fields[ LABEL_COUNTRY_CODE  ], O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    set_field_opts( rsfind->cache.fields[ LABEL_STATE         ], O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    set_field_opts( rsfind->cache.fields[ LABEL_LANGUAGE      ], O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    set_field_opts( rsfind->cache.fields[ LABEL_TAGS          ], O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    set_field_opts( rsfind->cache.fields[ LABEL_CODEC         ], O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    set_field_opts( rsfind->cache.fields[ LABEL_BITRATE       ], O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    set_field_opts( rsfind->cache.fields[ LABEL_BITRATE_TO    ], O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    set_field_opts( rsfind->cache.fields[ LABEL_BITRATE_UNIT  ], O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    set_field_opts( rsfind->cache.fields[ LABEL_ORDER_BY      ], O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    set_field_opts( rsfind->cache.fields[ LABEL_REVERSE       ], O_VISIBLE | O_PUBLIC | O_AUTOSKIP );

    set_field_opts( rsfind->cache.fields[ INPUT_NAME           ], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE | O_NULLOK );
    set_field_opts( rsfind->cache.fields[ INPUT_COUNTRY        ], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE | O_NULLOK );
    set_field_type( rsfind->cache.fields[ INPUT_COUNTRY        ], TYPE_ALPHA, 4 ); //shortest country name length = 4 (Chad, Togo, ...)
    set_field_opts( rsfind->cache.fields[ INPUT_COUNTRY_CODE   ], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE | O_INPUT_LIMIT | O_NULLOK | O_STATIC );
    set_field_type( rsfind->cache.fields[ INPUT_COUNTRY_CODE   ], TYPE_ALPHA, 2 );
    set_field_opts( rsfind->cache.fields[ INPUT_STATE          ], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE | O_NULLOK );
    set_field_opts( rsfind->cache.fields[ INPUT_LANGUAGE       ], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE | O_NULLOK );
    set_field_opts( rsfind->cache.fields[ INPUT_TAGS           ], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE | O_NULLOK );
    set_field_opts( rsfind->cache.fields[ INPUT_CODEC          ], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE | O_NULLOK );
    set_field_opts( rsfind->cache.fields[ INPUT_BITRATE_MIN    ], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE );
    set_field_opts( rsfind->cache.fields[ INPUT_BITRATE_MAX    ], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE );
    set_field_opts( rsfind->cache.fields[ INPUT_ORDER_BY       ], O_VISIBLE | O_PUBLIC | O_ACTIVE | O_STATIC );
    set_field_opts( rsfind->cache.fields[ INPUT_REVERSE        ], O_VISIBLE | O_PUBLIC | O_ACTIVE | O_STATIC );
    set_field_opts( rsfind->cache.fields[ INPUT_NAME_EXACT     ], O_VISIBLE | O_PUBLIC | O_ACTIVE | O_STATIC );
    set_field_opts( rsfind->cache.fields[ INPUT_COUNTRY_EXACT  ], O_VISIBLE | O_PUBLIC | O_ACTIVE | O_STATIC );
    set_field_opts( rsfind->cache.fields[ INPUT_STATE_EXACT    ], O_VISIBLE | O_PUBLIC | O_ACTIVE | O_STATIC );
    set_field_opts( rsfind->cache.fields[ INPUT_LANGUAGE_EXACT ], O_VISIBLE | O_PUBLIC | O_ACTIVE | O_STATIC );
    set_field_opts( rsfind->cache.fields[ INPUT_TAGS_EXACT     ], O_VISIBLE | O_PUBLIC | O_ACTIVE | O_STATIC );

    set_field_opts( rsfind->cache.fields[ BUTTON_CANCEL        ], O_VISIBLE | O_PUBLIC | O_ACTIVE | O_STATIC );
    set_field_opts( rsfind->cache.fields[ BUTTON_SAVE        ], O_VISIBLE | O_PUBLIC | O_ACTIVE | O_STATIC );

    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_RSFind_createFields()] Fields created." );
    return true;
}

/**
 * [PRIVATE] Initialises the Form
 * @return Success
 */
static bool ctune_UI_RSFind_initForm( ctune_UI_RSFind_t * rsfind ) {
    if( rsfind->form != NULL ) {
        unpost_form( rsfind->form );
        free_form( rsfind->form );
        rsfind->form = NULL;
    }

    if( rsfind->cache.max_label_width == 0 ) {
        if( !ctune_UI_RSFind_createFields( rsfind ) ) {
            CTUNE_LOG( CTUNE_LOG_FATAL,
                       "[ctune_UI_RSFind_initForm()] Failed to create fields for the form."
            );

            return false; //EARLY RETURN
        }
    }

    if( !ctune_UI_RSFind_initFields( rsfind ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_UI_RSFind_initForm()] Failed to init fields for the form."
        );

        return false;
    }

    if( rsfind->form != NULL ) {
        free_form( rsfind->form );
        rsfind->form = NULL;
    }

    if( ( rsfind->form = new_form( rsfind->cache.fields ) ) == NULL ) {
        switch( errno ) {
            case E_BAD_ARGUMENT:
                CTUNE_LOG( CTUNE_LOG_FATAL,
                           "[ctune_UI_RSFind_initForm()] "
                           "Failed to create Form: Routine detected an incorrect or out-of-range argument."
                );
                break;

            case E_CONNECTED:
                CTUNE_LOG( CTUNE_LOG_FATAL,
                           "[ctune_UI_RSFind_initForm()] "
                           "Failed to create Form: The field is already connected to a form."
                );
                break;

            case E_SYSTEM_ERROR:
                CTUNE_LOG( CTUNE_LOG_FATAL,
                           "[ctune_UI_RSFind_initForm()] "
                           "Failed to create Form: System error occurred, e.g., malloc failure."
                );
                break;

            default:
                CTUNE_LOG( CTUNE_LOG_FATAL,
                           "[ctune_UI_RSFind_initForm()] "
                           "Failed to create Form: unknown error."
                );
        }

        return false; //EARLY RETURN
    }

    scale_form( rsfind->form, &rsfind->form_dimension.rows, &rsfind->form_dimension.cols );

    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_UI_RSFind_initForm()] Form size calculated as: rows = %i, cols = %i (desc field size = %i)",
               rsfind->form_dimension.rows, rsfind->form_dimension.cols, rsfind->cache.max_label_width
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
        .initialised    = false,
        .margins        = { 0, 1, 1, 1 },
        .screen_size    = parent,
        .form_dimension = { 0, 0, 0, 0 },
        .dialog         = ctune_UI_Dialog.init(),
        .form           = NULL,
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
 * @param rsfind Un-initialised ctune_UI_RSFind_t object
 * @return Success
 */
static bool ctune_UI_RSFind_init( ctune_UI_RSFind_t * rsfind ) {
    bool error_state = false;

    if( rsfind->initialised == true ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_RSFind_init( %p )] RSFind has already been initialised!",
                   rsfind
        );

        error_state = true;
        goto end;
    }

    if( CTUNE_UI_DIALOG_RSFIND_FIELD_COUNT != FIELD_COUNT ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_UI_RSFind_init( %p )] "
                   "Field count macro value does not match number of fields in enum. Check src code!",
                   rsfind
        );

        error_state = true;
        goto end;
    }

    if( rsfind->screen_size == NULL ) {
        CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_UI_RSFind_init( %p )] Pointer to screen size is NULL.", rsfind );
        error_state = true;
    }

    if( rsfind->cb.getDisplayText == NULL ) {
        CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_UI_RSFind_init( %p )] Pointer UI text callback method is NULL.", rsfind );
        error_state = true;
    }

    ctune_UI_Dialog.setAutoScrollOffset( &rsfind->dialog, 2, 15 );

    if( !error_state )
        rsfind->initialised = true;

    end:
        return !( error_state );
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
 * Create and show a populated window with the find form
 * @param rsfind Pointer to a ctune_UI_RSFind_t object
 * @return Success
 */
bool ctune_UI_RSFind_show( ctune_UI_RSFind_t * rsfind ) {
    if( !rsfind->initialised ) {
        ctune_err.set( CTUNE_ERR_UI );
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_RSFind_show( %p )] RSFind not initialised prior.", rsfind );
        return false; //EARLY RETURN
    }

    if( !ctune_UI_RSFind_initForm( rsfind ) ) {
        ctune_err.set( CTUNE_ERR_UI );
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_RSFind_show( %p )] Could not initialise form.", rsfind );
        return false; //EARLY RETURN
    }

    ctune_UI_Dialog.free( &rsfind->dialog );

    ctune_UI_Dialog.createScrollWin( &rsfind->dialog,
                                     rsfind->form_dimension.rows,
                                     rsfind->form_dimension.cols );

    ctune_UI_Dialog.createBorderWin( &rsfind->dialog,
                                     rsfind->screen_size,
                                     rsfind->cb.getDisplayText( CTUNE_UI_TEXT_WIN_TITLE_RSFIND ),
                                     &rsfind->margins );

    set_form_win( rsfind->form, rsfind->dialog.border_win.window );
    set_form_sub( rsfind->form, rsfind->dialog.canvas.pad );
    post_form( rsfind->form );

    ctune_UI_Dialog.show( &rsfind->dialog );
    ctune_UI_Dialog.refreshView( &rsfind->dialog );

    ctune_UI_Resizer.push( ctune_UI_RSFind.resize, rsfind );

    doupdate();
    return true;
}

/**
 * Redraws the dialog
 * @param rsfind Pointer to a ctune_UI_RSFind_t object
 */
static void ctune_UI_RSFind_resize( void * rsfind ) {
    CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_UI_RSFind_resize( %p )] Resize event called.", rsfind );

    if( rsfind == NULL ) {
        ctune_err.set( CTUNE_ERR_UI );
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_RSFind_resize( %p )] RSFind is NULL.", rsfind );
        return; //EARLY RETURN
    }

    ctune_UI_RSFind_t * rsf_dialog = rsfind;

    if( !rsf_dialog->initialised ) {
        ctune_err.set( CTUNE_ERR_UI );
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_RSFind_resize( %p )] RSFind is not initialised.", rsf_dialog );
        false; //EARLY RETURN
    }

    ctune_UI_Dialog.scrollHome( &rsf_dialog->dialog );

    ctune_UI_Dialog.createBorderWin( &rsf_dialog->dialog,
                                     rsf_dialog->screen_size,
                                     rsf_dialog->cb.getDisplayText( CTUNE_UI_TEXT_WIN_TITLE_RSFIND ),
                                     &rsf_dialog->margins );

    set_form_win( rsf_dialog->form, rsf_dialog->dialog.border_win.window );
    set_form_sub( rsf_dialog->form, rsf_dialog->dialog.canvas.pad );
    post_form( rsf_dialog->form );

    ctune_UI_Dialog.show( &rsf_dialog->dialog );
    ctune_UI_Dialog.refreshView( &rsf_dialog->dialog );
}

/**
 * Pass keyboard input to the form
 * @return Form exit state
 */
static ctune_FormExit_e ctune_UI_RSFind_captureInput( ctune_UI_RSFind_t * rsfind ) {
    keypad( rsfind->dialog.canvas.pad, TRUE );
    bool             exit       = false;
    ctune_FormExit_e exit_state = CTUNE_UI_FORM_ESC;
    int  character;

    form_driver( rsfind->form, REQ_FIRST_FIELD );
    ctune_UI_RSFind_highlightCurrField( rsfind, current_field( rsfind->form ) );
    ctune_UI_ScrollWin.refreshView( &rsfind->dialog.canvas );

    while( !exit ) {
        character = wgetch( rsfind->dialog.canvas.pad );

        switch( ctune_UI_KeyBinding.getAction( CTUNE_UI_CTX_RSFIND, character ) ) {
            case CTUNE_UI_ACTION_RESIZE: {
                ctune_UI_Resizer.resize();
            } break;

            case CTUNE_UI_ACTION_HELP: {
                ctune_UI_ContextHelp.show( CTUNE_UI_CTX_RSFIND );
                ctune_UI_ContextHelp.captureInput();
            } break;

            case CTUNE_UI_ACTION_ESC: {
                exit_state = CTUNE_UI_FORM_ESC;
                exit = true;
            } break;

            case CTUNE_UI_ACTION_FIELD_BEGIN: {
                set_current_field( rsfind->form, rsfind->cache.fields[LABEL_COUNT] );
                ctune_UI_RSFind_highlightCurrField( rsfind, current_field( rsfind->form ) );
            } break;

            case CTUNE_UI_ACTION_FIELD_END: {
                set_current_field( rsfind->form, rsfind->cache.fields[ ( FIELD_LAST - 1 ) ] );
                ctune_UI_RSFind_highlightCurrField( rsfind, current_field( rsfind->form ) );
                ctune_UI_RSFind_autoScroll( rsfind, current_field( rsfind->form ) );
            } break;

            case CTUNE_UI_ACTION_FIELD_FIRST: {
                form_driver( rsfind->form, REQ_FIRST_FIELD );
                ctune_UI_RSFind_highlightCurrField( rsfind, current_field( rsfind->form ) );
            } break;

            case CTUNE_UI_ACTION_FIELD_LAST: {
                form_driver( rsfind->form, REQ_LAST_FIELD );
                ctune_UI_RSFind_highlightCurrField( rsfind, current_field( rsfind->form ) );
            } break;

            case CTUNE_UI_ACTION_FIELD_PREV: {
                form_driver( rsfind->form, REQ_PREV_FIELD );
                ctune_UI_RSFind_highlightCurrField( rsfind, current_field( rsfind->form ) );
                form_driver( rsfind->form, REQ_END_LINE );
                ctune_UI_RSFind_autoScroll( rsfind, current_field( rsfind->form ) );
            } break;

            case CTUNE_UI_ACTION_FIELD_NEXT: {
                form_driver( rsfind->form, REQ_NEXT_FIELD );
                ctune_UI_RSFind_highlightCurrField( rsfind, current_field( rsfind->form ) );
                form_driver( rsfind->form, REQ_END_LINE );
                ctune_UI_RSFind_autoScroll( rsfind, current_field( rsfind->form ) );
            } break;

            case CTUNE_UI_ACTION_GO_LEFT: {
                if( current_field( rsfind->form ) == rsfind->cache.fields[ INPUT_ORDER_BY ] ) {
                    if( rsfind->cache.order_selection > 0 )
                        --rsfind->cache.order_selection;

                    ctune_UI_RSFind_printOrderBy( rsfind, rsfind->cache.order_selection );

                } else {
                    form_driver( rsfind->form, REQ_LEFT_CHAR );
                }
            } break;

            case CTUNE_UI_ACTION_GO_RIGHT: {
                if( current_field( rsfind->form ) == rsfind->cache.fields[ INPUT_ORDER_BY ] ) {
                    if( rsfind->cache.order_selection < ( STATION_ATTR_COUNT - 1 ) )
                        ++rsfind->cache.order_selection;

                    ctune_UI_RSFind_printOrderBy( rsfind, rsfind->cache.order_selection );

                } else {
                    form_driver( rsfind->form, REQ_RIGHT_CHAR );
                }
            } break;

            case CTUNE_UI_ACTION_TRIGGER: {
                if( ( exit = ctune_UI_RSFind_isExitState( rsfind, current_field( rsfind->form ), &exit_state ) ) ) {
                    if( exit_state == CTUNE_UI_FORM_SUBMIT && !ctune_UI_RSFind_packFieldValues( rsfind, &rsfind->cache.filter ) ) {
                        exit = false;
                    }

                } else {
                    form_driver( rsfind->form, REQ_NEXT_FIELD );
                    ctune_UI_RSFind_highlightCurrField( rsfind, current_field( rsfind->form ) );
                    form_driver( rsfind->form, REQ_END_LINE );
                    ctune_UI_RSFind_autoScroll( rsfind, current_field( rsfind->form ) );
                }
            } break;

            case CTUNE_UI_ACTION_TOGGLE: { //'x'
                int toggle_id = ctune_UI_RSFind_isToggle( rsfind, current_field( rsfind->form ) );
                if( toggle_id >= 0 ) {
                    ctune_UI_RSFind_toggle( rsfind, toggle_id, current_field( rsfind->form ) );
                } else {
                    form_driver( rsfind->form, character );
                }
            } break;

            case CTUNE_UI_ACTION_TOGGLE_ALT: { //'space'
                if( ( exit = ctune_UI_RSFind_isExitState( rsfind, current_field( rsfind->form ), &exit_state ) ) ) {
                    if( exit_state == CTUNE_UI_FORM_SUBMIT && !ctune_UI_RSFind_packFieldValues( rsfind, &rsfind->cache.filter ) ) {
                        exit = false;
                    }

                } else {
                    int toggle_id = ctune_UI_RSFind_isToggle( rsfind, current_field( rsfind->form ) );
                    if( toggle_id >= 0 ) {
                        ctune_UI_RSFind_toggle( rsfind, toggle_id, current_field( rsfind->form ) );
                    } else {
                        form_driver( rsfind->form, character );
                    }
                }
            } break;

            case CTUNE_UI_ACTION_CLEAR_ALL: {
                ctune_UI_RSFind_clearAllFields( rsfind );
                ctune_UI_RSFind_highlightCurrField( rsfind, current_field( rsfind->form ) );
            } break;

            case CTUNE_UI_ACTION_CLEAR_SELECTED: {
                form_driver( rsfind->form, REQ_CLR_FIELD );
            } break;

            case CTUNE_UI_ACTION_DEL_PREV: {
                form_driver( rsfind->form, REQ_DEL_PREV );
            } break;

            case CTUNE_UI_ACTION_DEL_NEXT: {
                form_driver( rsfind->form, REQ_DEL_CHAR );
            } break;

            default: {
                form_driver( rsfind->form, character );
            } break;
        }

        ctune_UI_ScrollWin.refreshView( &rsfind->dialog.canvas );
    }

    ctune_UI_Dialog.hide( &rsfind->dialog );
    ctune_UI_Resizer.pop();

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
    unpost_form( rsfind->form );
    free_form( rsfind->form );

    for( int i = 0; i < FIELD_COUNT; i++ )
        free_field( rsfind->cache.fields[i] );

    ctune_UI_Dialog.free( &rsfind->dialog );
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
    .show          = &ctune_UI_RSFind_show,
    .resize        = &ctune_UI_RSFind_resize,
    .captureInput  = &ctune_UI_RSFind_captureInput,
    .getFilter     = &ctune_UI_RSFind_getFilter,
    .free          = &ctune_UI_RSFind_free,
};