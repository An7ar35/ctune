#include "RSInfo.h"

#include <string.h>
#include <panel.h>

#include "../../logger/Logger.h"
#include "../../ctune_err.h"
#include "../../utils/utilities.h"
#include "../definitions/Theme.h"
#include "../definitions/KeyBinding.h"
#include "ContextHelp.h"
#include "../Resizer.h"

#define KEY_ESC 27

typedef enum { //Note: don't forget to change the value of `CTUNE_UI_DIALOG_RSINFO_FIELD_COUNT` if modifying the enums
    RSI_LABEL_STATION_NAME = 0,
    RSI_LABEL_CHANGE_UUID,
    RSI_LABEL_STATION_UUID,
    RSI_LABEL_URL,
    RSI_LABEL_URL_RESOLVED,
    RSI_LABEL_URL_HOMEPAGE,
    RSI_LABEL_URL_FAVICON,
    RSI_LABEL_TAGS,
    RSI_LABEL_COUNTRY,
    RSI_LABEL_COUNTRY_CODE,
    RSI_LABEL_STATE,
    RSI_LABEL_LANGUAGE,
    RSI_LABEL_LANGUAGE_CODES,
    RSI_LABEL_VOTES,
    RSI_LABEL_LAST_CHANGED_TS,
    RSI_LABEL_LAST_CHANGED_TS_ISO,
    RSI_LABEL_CODEC,
    RSI_LABEL_BITRATE,
    RSI_LABEL_HLS,
    RSI_LABEL_LAST_CHECK_OK,
    RSI_LABEL_LAST_CHECK_TS,
    RSI_LABEL_LAST_CHECK_TS_ISO,
    RSI_LABEL_LAST_CHECK_OK_TS,
    RSI_LABEL_LAST_CHECK_OK_TS_ISO,
    RSI_LABEL_LAST_CHECK_TS_LOCAL,
    RSI_LABEL_LAST_CHECK_TS_LOCAL_ISO,
    RSI_LABEL_CLICK_TS,
    RSI_LABEL_CLICK_TS_ISO,
    RSI_LABEL_CLICKCOUNT,
    RSI_LABEL_CLICKTREND,
    RSI_LABEL_SSL_ERROR,
    RSI_LABEL_GEO_COORDS,

    RSI_LABEL_STATION_SOURCE,
} RSI_Field_e;

/**
 * [PRIVATE] Container for storing canvas and column properties
 * @param cols         Total number of char spaces (win cols) required to display content
 * @param rows         Total number of lines required to display content
 * @param label_size   Label column max size
 * @param field_size   Field column max size
 * @param padding_size Size of padding between the label and field columns
 */
typedef struct {
    int cols;
    int rows;
    int label_size;
    int field_size;
    int padding_size;
} RSInfoPrintProperties_t;

/**
 * [PRIVATE] Gets the language string for a given station source
 * @param getTextCb Method to get UI display text
 * @param e         ctune_StationSrc_e value
 * @return Language string
 */
static const char * ctune_UI_RSInfo_getStationSrcUIText( const char * (* getTextCb)( ctune_UI_TextID_e ), ctune_StationSrc_e e ) {
    switch( e ) {
        case CTUNE_STATIONSRC_LOCAL:
            return getTextCb( CTUNE_UI_TEXT_STATIONSOURCE_LOCAL );
        case CTUNE_STATIONSRC_RADIOBROWSER:
            return getTextCb( CTUNE_UI_TEXT_STATIONSOURCE_RADIOBROWSER );
        default:
            return getTextCb( CTUNE_UI_TEXT_ERROR );
    }
}

/**
 * [PRIVATE] Gets the maximum string length of teh RSI values
 * @param rsinfo RSInfo_t instance
 * @param rsi    Pointer to RadioStationInfo_t object
 * @return Printing size properties
 */
static RSInfoPrintProperties_t ctune_UI_RSInfo_getMaxDimensions( ctune_UI_RSInfo_t * rsinfo, const ctune_RadioStationInfo_t * rsi ) {
    size_t                  field_cols = 0;
    size_t                  label_cols = 0;
    int                     rows       = 0;

    field_cols = ctune_max_ul( strlen( rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_TRUE  ) ),
                               strlen( rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_FALSE ) ) );

    //COMPULSORY FIELDS
    label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_STATION_NAME] ) );
    field_cols = ctune_max_ul( field_cols, ( ctune_RadioStationInfo.get.stationName( rsi ) != NULL ? strlen( ctune_RadioStationInfo.get.stationName( rsi ) ) : 0 ) );
    label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_STATION_UUID] ) );
    field_cols = ctune_max_ul( field_cols, ( ctune_RadioStationInfo.get.stationUUID( rsi ) != NULL ? strlen( ctune_RadioStationInfo.get.stationUUID( rsi ) ) : 0 ) );
    label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_URL] ) );
    field_cols = ctune_max_ul( field_cols, ( ctune_RadioStationInfo.get.stationURL( rsi ) != NULL ? strlen( ctune_RadioStationInfo.get.stationURL( rsi ) ) : 0 ) );
    label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_CODEC] ) );
    field_cols = ctune_max_ul( field_cols, ( ctune_RadioStationInfo.get.codec( rsi ) != NULL ? strlen( ctune_RadioStationInfo.get.codec( rsi ) ) : 0 ) );
    label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_GEO_COORDS] ) );
    field_cols = ctune_max_ul( field_cols, 20 ); //geo coordinates length: "(0.000000, 0.000000)"
    label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_STATION_SOURCE] ) );
    field_cols = ctune_max_ul( field_cols, strlen( ctune_UI_RSInfo_getStationSrcUIText( rsinfo->cb.getDisplayText, ctune_RadioStationInfo.get.stationSource( rsi ) ) ) );

    { //bitrate
        label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_BITRATE] ) );

        String_t tmp = String.init();
        ctune_utos( ctune_RadioStationInfo.get.bitrate( rsi ), &tmp );
        field_cols = ctune_max_ul( field_cols, String.length( &tmp ) );
        String.free( &tmp );
    }

    rows += 7;


    //OPTIONAL FIELDS
    if( ctune_RadioStationInfo.get.changeUUID( rsi ) != NULL ) {
        label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_CHANGE_UUID] ) );
        field_cols = ctune_max_ul( field_cols, strlen( ctune_RadioStationInfo.get.changeUUID( rsi ) ) );
        rows += 1;
    }

    if( ctune_RadioStationInfo.get.resolvedURL( rsi ) != NULL ) {
        label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_URL_RESOLVED] ) );
        field_cols = ctune_max_ul( field_cols, strlen( ctune_RadioStationInfo.get.resolvedURL( rsi ) ) );
        rows += 1;
    }

    if( ctune_RadioStationInfo.get.homepage( rsi ) != NULL ) {
        label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_URL_HOMEPAGE] ) );
        field_cols = ctune_max_ul( field_cols, strlen( ctune_RadioStationInfo.get.homepage( rsi ) ) );
        rows += 1;
    }

    if( ctune_RadioStationInfo.get.faviconURL( rsi ) != NULL ) {
        label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_URL_FAVICON] ) );
        field_cols = ctune_max_ul( field_cols, strlen( ctune_RadioStationInfo.get.faviconURL( rsi ) ) );
        rows += 1;
    }

    if( ctune_RadioStationInfo.get.tags( rsi ) != NULL ) {
        label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_TAGS] ) );
        field_cols = ctune_max_ul( field_cols, strlen( ctune_RadioStationInfo.get.tags( rsi ) ) );
        rows += 1;
    }

    if( ctune_RadioStationInfo.get.country( rsi ) != NULL ) {
        label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_COUNTRY] ) );
        field_cols = ctune_max_ul( field_cols, strlen( ctune_RadioStationInfo.get.country( rsi ) ) );
        rows += 1;
    }

    if( ctune_RadioStationInfo.get.countryCode( rsi ) != NULL ) {
        label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_COUNTRY_CODE] ) );
        field_cols = ctune_max_ul( field_cols, strlen( ctune_RadioStationInfo.get.countryCode( rsi ) ) );
        rows += 1;
    }

    if( ctune_RadioStationInfo.get.state( rsi ) != NULL ) {
        label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_STATE] ) );
        field_cols = ctune_max_ul( field_cols, strlen( ctune_RadioStationInfo.get.state( rsi ) ) );
        rows += 1;
    }

    if( ctune_RadioStationInfo.get.language( rsi ) != NULL ) {
        label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_LANGUAGE] ) );
        field_cols = ctune_max_ul( field_cols, strlen( ctune_RadioStationInfo.get.language( rsi ) ) );
        rows += 1;
    }

    if( ctune_RadioStationInfo.get.languageCodes( rsi ) != NULL ) {
        label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_LANGUAGE_CODES] ) );
        field_cols = ctune_max_ul( field_cols, strlen( ctune_RadioStationInfo.get.languageCodes( rsi ) ) );
        rows += 1;
    }

    if( ctune_RadioStationInfo.get.lastChangeTS( rsi ) != NULL ) {
        label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_LAST_CHANGED_TS_ISO] ) );
        field_cols = ctune_max_ul( field_cols, strlen( ctune_RadioStationInfo.get.lastChangeTS( rsi ) ) );
        rows += 1;
    } else if ( rsi->last_change_time != NULL ) { //FIXME DEPRECIATED timestamps
        label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_LAST_CHANGED_TS] ) );
        field_cols = ctune_max_ul( field_cols, strlen( rsi->last_change_time ) );
        rows += 1;
    }

    if( ctune_RadioStationInfo.get.lastCheckTS( rsi ) != NULL ) {
        label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_LAST_CHECK_OK] ) );
        label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_LAST_CHECK_TS_ISO] ) );
        label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_LAST_CHECK_OK_TS_ISO] ) );
        label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_LAST_CHECK_TS_LOCAL_ISO] ) );

        field_cols = ctune_max_ul( field_cols, strlen( ctune_RadioStationInfo.get.lastCheckTS( rsi ) ) );
        field_cols = ctune_max_ul( field_cols, ( ctune_RadioStationInfo.get.lastCheckOkTS( rsi ) != NULL ? strlen( ctune_RadioStationInfo.get.lastCheckOkTS( rsi ) ) : 0 ) );
        field_cols = ctune_max_ul( field_cols, ( ctune_RadioStationInfo.get.lastLocalCheckTS( rsi ) != NULL ? strlen( ctune_RadioStationInfo.get.lastLocalCheckTS( rsi ) ) : 0 ) );

        rows += 4; //(inc. ctune_RadioStationInfo.get.lastCheckOK( rsi ))

    } else if( rsi->last_check_time ) { //FIXME DEPRECIATED timestamps
        label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_LAST_CHECK_OK] ) );
        label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_LAST_CHECK_TS] ) );
        label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_LAST_CHECK_OK_TS] ) );
        label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_LAST_CHECK_TS_LOCAL] ) );

        field_cols = ctune_max_ul( field_cols, strlen( rsi->last_check_time ) );
        field_cols = ctune_max_ul( field_cols, ( rsi->last_check_ok_time != NULL ? strlen( rsi->last_check_ok_time ) : 0 ) );
        field_cols = ctune_max_ul( field_cols, ( rsi->last_local_check_time != NULL ? strlen( rsi->last_local_check_time ) : 0 ) );

        rows += 4; //(inc. ctune_RadioStationInfo.get.lastCheckOK( rsi ))
    }

    if( ctune_RadioStationInfo.get.stationSource( rsi ) != CTUNE_STATIONSRC_LOCAL ) {
        label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_HLS] ) );
        rows += 1; //for "HTTP live streaming" flag

        //click timestamp
        if( ctune_RadioStationInfo.get.clickTS( rsi ) != NULL ) {
            label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_CLICK_TS_ISO] ) );
            field_cols = ctune_max_ul( field_cols, strlen( ctune_RadioStationInfo.get.clickTS( rsi ) ) );
            rows += 1;
        } else if( rsi->click_timestamp != NULL ) { //FIXME DEPRECIATED timestamps
            label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_CLICK_TS] ) );
            field_cols = ctune_max_ul( field_cols, strlen( rsi->click_timestamp ) );
            rows += 1;
        }

        { //votes
            label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_VOTES] ) );

            String_t tmp = String.init();
            ctune_utos( ctune_RadioStationInfo.get.votes( rsi ), &tmp );
            field_cols = ctune_max_ul( field_cols, String.length( &tmp ) );
            String.free( &tmp );
            rows += 1;
        }

        {//click count
            label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_CLICKCOUNT] ) );

            String_t tmp = String.init();
            ctune_utos( ctune_RadioStationInfo.get.clickCount( rsi ), &tmp );
            field_cols = ctune_max_ul( field_cols, String.length( &tmp ) );
            String.free( &tmp );
            rows += 1;
        }

        { //click trend
            label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_CLICKTREND] ) );

            String_t tmp = String.init();
            ctune_ltos( ctune_RadioStationInfo.get.clickTrend( rsi ), &tmp );
            field_cols = ctune_max_ul( field_cols, String.length( &tmp ) );
            String.free( &tmp );
            rows += 1;
        }

        { //ssl error code
            label_cols = ctune_max_ul( label_cols, strlen( rsinfo->label_txt[RSI_LABEL_SSL_ERROR] ) );

            String_t tmp = String.init();
            ctune_utos( ctune_RadioStationInfo.get.sslErrCode( rsi ), &tmp );
            field_cols = ctune_max_ul( field_cols, String.length( &tmp ) );
            String.free( &tmp );
            rows += 1;
        }
    }

    RSInfoPrintProperties_t properties = { 0, 0, 0, 0, 0 };

    {
        if( !ctune_utoi( label_cols, &properties.label_size ) ) {
            ctune_err.set( CTUNE_ERR_BAD_CAST );
            properties.label_size = 30; //fallback
            CTUNE_LOG( CTUNE_LOG_FATAL,
                       "[ctune_UI_RSInfo_getMaxFieldWidth( %p )] Integer overflow on RSI label text size: %lu",
                       rsi, field_cols
            );
        }

        if( !ctune_utoi( field_cols, &properties.field_size ) ) {
            ctune_err.set( CTUNE_ERR_BAD_CAST );
            properties.field_size = 100; //fallback
            CTUNE_LOG( CTUNE_LOG_FATAL,
                       "[ctune_UI_RSInfo_getMaxFieldWidth( %p )] Integer overflow on RSI value text size: %lu",
                       rsi, field_cols
            );
        }

        properties.padding_size = (int) strlen( rsinfo->col_separator_str );
        properties.rows         = rows;
        properties.cols         = ( properties.label_size + properties.padding_size + properties.field_size );
    }

    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_UI_RSInfo_getMaxFieldWidth( %p )] Max size: rows=%d, cols=%d",
               rsi, properties.rows, properties.cols
    );

    return properties;
}

/**
 * [PRIVATE] Prints the labels and values to the canvas
 * @param rsinfo         Pointer to ctune_UI_RSInfo_t object
 * @param rsi            Pointer to RadioStationInfo_t object
 * @param max_label_size Max length of the labels
 * @param padding_size   Size of the padding between the labels and their respective fields
 */
static void ctune_UI_RSInfo_printFields( ctune_UI_RSInfo_t * rsinfo, const ctune_RadioStationInfo_t * rsi, int max_label_size, int padding_size ) {
    //starting position on the canvas
    int       x              = 0;
    int       y              = 0;
    const int col_offset     = x + max_label_size + padding_size; //description field length and value field offset
    //shortcut pointers
    const char * true_str  = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_TRUE );
    const char * false_str = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_FALSE );

    //Station name
    wattron( rsinfo->dialog.canvas.pad, A_BOLD );
    mvwprintw( rsinfo->dialog.canvas.pad, y, x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_STATION_NAME], rsinfo->col_separator_str );
    wattroff( rsinfo->dialog.canvas.pad, A_BOLD );
    mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "%s", ctune_fallbackStr( ctune_RadioStationInfo.get.stationName( rsi ), "" ) );

    //Change UUID
    if( ctune_RadioStationInfo.get.changeUUID( rsi ) != NULL ) {
        wattron( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, y, x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_CHANGE_UUID], rsinfo->col_separator_str );
        wattroff( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "%s", ctune_RadioStationInfo.get.changeUUID( rsi ) );
    }

    //Station UUID
    wattron( rsinfo->dialog.canvas.pad, A_BOLD );
    mvwprintw( rsinfo->dialog.canvas.pad, y, x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_STATION_UUID], rsinfo->col_separator_str );
    wattroff( rsinfo->dialog.canvas.pad, A_BOLD );
    mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "%s", ctune_fallbackStr( ctune_RadioStationInfo.get.stationUUID( rsi ), "" ) );

    //URL
    wattron( rsinfo->dialog.canvas.pad, A_BOLD );
    mvwprintw( rsinfo->dialog.canvas.pad, y, x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_URL], rsinfo->col_separator_str );
    wattroff( rsinfo->dialog.canvas.pad, A_BOLD );
    mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "%s", ctune_fallbackStr( ctune_RadioStationInfo.get.stationURL( rsi ), "" )  );

    //URL RESOLVED
    if( ctune_RadioStationInfo.get.resolvedURL( rsi ) != NULL ) {
        wattron( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, y, x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_URL_RESOLVED], rsinfo->col_separator_str );
        wattroff( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "%s", ctune_RadioStationInfo.get.resolvedURL( rsi ) );
    }

    //HOMEPAGE URL
    if( ctune_RadioStationInfo.get.homepage( rsi ) != NULL ) {
        wattron( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, y, x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_URL_HOMEPAGE], rsinfo->col_separator_str );
        wattroff( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "%s", ctune_RadioStationInfo.get.homepage( rsi ) );
    }

    //FAVICON URL
    if( ctune_RadioStationInfo.get.faviconURL( rsi ) != NULL ) {
        wattron( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, y, x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_URL_FAVICON], rsinfo->col_separator_str );
        wattroff( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "%s", ctune_RadioStationInfo.get.faviconURL( rsi ) );
    }

    //TAGS
    if( ctune_RadioStationInfo.get.tags( rsi ) != NULL ) {
        wattron( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, y, x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_TAGS], rsinfo->col_separator_str );
        wattroff( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "%s", ctune_RadioStationInfo.get.tags( rsi ) );
    }

    //COUNTRY
    if( ctune_RadioStationInfo.get.country( rsi ) != NULL ) {
        wattron( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, y, x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_COUNTRY], rsinfo->col_separator_str );
        wattroff( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "%s", ctune_RadioStationInfo.get.country( rsi ) );
    }

    //COUNTRY CODE
    if( ctune_RadioStationInfo.get.countryCode( rsi ) != NULL ) {
        wattron( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, y, x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_COUNTRY_CODE], rsinfo->col_separator_str );
        wattroff( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "%.*s", 2, ctune_RadioStationInfo.get.countryCode( rsi ) );
    }

    //STATE
    if( ctune_RadioStationInfo.get.state( rsi ) != NULL ) {
        wattron( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, y, x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_STATE], rsinfo->col_separator_str );
        wattroff( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "%s", ctune_RadioStationInfo.get.state( rsi ) );
    }

    //LANGUAGE
    if( ctune_RadioStationInfo.get.language( rsi ) != NULL ) {
        wattron( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, y, x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_LANGUAGE], rsinfo->col_separator_str );
        wattroff( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "%s", ctune_RadioStationInfo.get.language( rsi ) );
    }

    //LANGUAGE CODE
    if( ctune_RadioStationInfo.get.languageCodes( rsi ) != NULL ) {
        wattron( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, y, x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_LANGUAGE_CODES], rsinfo->col_separator_str );
        wattroff( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "%s", ctune_RadioStationInfo.get.languageCodes( rsi ) );
    }

    //VOTES
    if( ctune_RadioStationInfo.get.stationSource( rsi ) != CTUNE_STATIONSRC_LOCAL ) {
        wattron( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, y, x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_VOTES], rsinfo->col_separator_str );
        wattroff( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "%lu", ctune_RadioStationInfo.get.votes( rsi ) );
    }

    //LAST CHANGE TIMESTAMP
    if( ctune_RadioStationInfo.get.lastChangeTS( rsi ) != NULL ) {
        wattron( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, y, x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_LAST_CHANGED_TS_ISO], rsinfo->col_separator_str );
        wattroff( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "%s", ctune_RadioStationInfo.get.lastChangeTS( rsi ) );

    } else if ( rsi->last_change_time != NULL ) { //FIXME DEPRECIATED timestamps
        wattron( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, y, x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_LAST_CHANGED_TS], rsinfo->col_separator_str );
        wattroff( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "%s", rsi->last_change_time );
    }

    //CODEC
    wattron( rsinfo->dialog.canvas.pad, A_BOLD );
    mvwprintw( rsinfo->dialog.canvas.pad, y, x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_CODEC], rsinfo->col_separator_str );
    wattroff( rsinfo->dialog.canvas.pad, A_BOLD );
    mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "%s",
               ctune_fallbackStr( ctune_RadioStationInfo.get.codec( rsi ), "" ) );

    //BITRATE
    wattron( rsinfo->dialog.canvas.pad, A_BOLD );
    mvwprintw( rsinfo->dialog.canvas.pad, y, x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_BITRATE], rsinfo->col_separator_str );
    wattroff( rsinfo->dialog.canvas.pad, A_BOLD );
    mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "%lu Kbps", ctune_RadioStationInfo.get.bitrate( rsi ) );

    //HTTP LIVE STREAMING FLAG
    if( ctune_RadioStationInfo.get.stationSource( rsi ) != CTUNE_STATIONSRC_LOCAL ) {
        wattron( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, y, x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_HLS], rsinfo->col_separator_str );
        wattroff( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "%s", ( ctune_RadioStationInfo.get.hls( rsi ) ? true_str : false_str ) );
    }

    //LAST CHECK TIMESTAMPS (OK FLAG, LAST CHECK, LAST OK CHECK, LAST CHECK LOCAL)
    if( ctune_RadioStationInfo.get.lastCheckTS( rsi ) != NULL ) {
        int y_start = y;
        wattron( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, ( y_start++ ), x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_LAST_CHECK_OK], rsinfo->col_separator_str );
        mvwprintw( rsinfo->dialog.canvas.pad, ( y_start++ ), x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_LAST_CHECK_TS_ISO], rsinfo->col_separator_str );
        mvwprintw( rsinfo->dialog.canvas.pad, ( y_start++ ), x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_LAST_CHECK_OK_TS_ISO], rsinfo->col_separator_str );
        mvwprintw( rsinfo->dialog.canvas.pad,   y_start,     x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_LAST_CHECK_TS_LOCAL_ISO], rsinfo->col_separator_str );
        wattroff( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "%s", ( ctune_RadioStationInfo.get.lastCheckOK( rsi ) ? true_str : false_str ) );
        mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "%s", ctune_fallbackStr( ctune_RadioStationInfo.get.lastCheckTS( rsi ), "" ) );
        mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "%s", ctune_fallbackStr( ctune_RadioStationInfo.get.lastCheckOkTS( rsi ), "" ) );
        mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "%s", ctune_fallbackStr( ctune_RadioStationInfo.get.lastLocalCheckTS( rsi ), "" ) );

    } else if( rsi->last_check_time != NULL ) { //FIXME DEPRECIATED timestamps
        int y_start = y;
        wattron( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, ( y_start++ ), x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_LAST_CHECK_OK], rsinfo->col_separator_str );
        mvwprintw( rsinfo->dialog.canvas.pad, ( y_start++ ), x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_LAST_CHECK_TS], rsinfo->col_separator_str );
        mvwprintw( rsinfo->dialog.canvas.pad, ( y_start++ ), x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_LAST_CHECK_OK_TS], rsinfo->col_separator_str );
        mvwprintw( rsinfo->dialog.canvas.pad,   y_start,     x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_LAST_CHECK_TS_LOCAL], rsinfo->col_separator_str );
        wattroff( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "%s", ( ctune_RadioStationInfo.get.lastCheckOK( rsi ) ? true_str : false_str ) );
        mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "%s", ctune_fallbackStr( rsi->last_check_time, "" ) );
        mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "%s", ctune_fallbackStr( rsi->last_check_ok_time, "" ) );
        mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "%s", ctune_fallbackStr( rsi->last_local_check_time, "" ) );
    }

    if( ctune_RadioStationInfo.get.stationSource( rsi ) != CTUNE_STATIONSRC_LOCAL ) {
        //CLICK TIMESTAMP
        if( ctune_RadioStationInfo.get.clickTS( rsi ) != NULL ) {
            wattron( rsinfo->dialog.canvas.pad, A_BOLD );
            mvwprintw( rsinfo->dialog.canvas.pad, y, x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_CLICK_TS_ISO], rsinfo->col_separator_str );
            wattroff( rsinfo->dialog.canvas.pad, A_BOLD );
            mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "%s", ctune_RadioStationInfo.get.clickTS( rsi ) );

        } else if( rsi->click_timestamp != NULL ) { //FIXME DEPRECIATED timestamps
            wattron( rsinfo->dialog.canvas.pad, A_BOLD );
            mvwprintw( rsinfo->dialog.canvas.pad, y, x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_CLICK_TS], rsinfo->col_separator_str );
            wattroff( rsinfo->dialog.canvas.pad, A_BOLD );
            mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "%s", rsi->click_timestamp );
        }

        //CLICK COUNT
        wattron( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, y, x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_CLICKCOUNT], rsinfo->col_separator_str );
        wattroff( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "%lu", ctune_RadioStationInfo.get.clickCount( rsi ) );

        //CLICK TREND
        wattron( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, y, x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_CLICKTREND], rsinfo->col_separator_str );
        wattroff( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "%ld", ctune_RadioStationInfo.get.clickTrend( rsi ) );

        //SSL ERROR CODE
        wattron( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, y, x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_SSL_ERROR], rsinfo->col_separator_str );
        wattroff( rsinfo->dialog.canvas.pad, A_BOLD );
        mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "%ld", ctune_RadioStationInfo.get.sslErrCode( rsi ) );
    }

    //GEO COORDINATES
    wattron( rsinfo->dialog.canvas.pad, A_BOLD );
    mvwprintw( rsinfo->dialog.canvas.pad, y, x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_GEO_COORDS], rsinfo->col_separator_str );
    wattroff( rsinfo->dialog.canvas.pad, A_BOLD );
    mvwprintw( rsinfo->dialog.canvas.pad, ( y++ ), col_offset, "(%f, %f)", ctune_RadioStationInfo.get.geoLatitude( rsi ), ctune_RadioStationInfo.get.geoLongitude( rsi ) );

    //STATION SOURCE
    wattron( rsinfo->dialog.canvas.pad, A_BOLD );
    mvwprintw( rsinfo->dialog.canvas.pad, y, x, "%*s%s", max_label_size, rsinfo->label_txt[RSI_LABEL_STATION_SOURCE], rsinfo->col_separator_str );
    wattroff( rsinfo->dialog.canvas.pad, A_BOLD );
    mvwprintw( rsinfo->dialog.canvas.pad, y, col_offset, "%s", ctune_UI_RSInfo_getStationSrcUIText( rsinfo->cb.getDisplayText, ctune_RadioStationInfo.get.stationSource( rsi ) ) );
}


/**
 * [PRIVATE] Print to a scrolling window
 * @param rsinfo Pointer to ctune_UI_RSInfo_t object
 * @param rsi    Pointer to RadioStationInfo_t object
 */
static void ctune_UI_RSInfo_printScroll( ctune_UI_RSInfo_t * rsinfo, const ctune_RadioStationInfo_t * rsi ) {
    const RSInfoPrintProperties_t properties = ctune_UI_RSInfo_getMaxDimensions( rsinfo, rsi );

    CTUNE_LOG( CTUNE_LOG_TRACE,
               "[ctune_UI_RSInfo_printScroll( %p, %p )] Printing ScrollWin: rows = %i, cols = %i",
               rsinfo, rsi, properties.rows, properties.cols
    );

    ctune_UI_Dialog.createScrollWin( &rsinfo->dialog, properties.rows, properties.cols );
    ctune_UI_RSInfo_printFields( rsinfo, rsi, properties.label_size, properties.padding_size );
}

/**
 * Creates a base ctune_UI_RSInfo_t object
 * @param parent         Pointer to size property of the parent window
 * @param getDisplayText Callback method to get text strings for the display
 * @param col_sep        Text to use as separation between the label and field column
 * @return Basic un-initialised ctune_UI_RSInfo_t object
 */
static ctune_UI_RSInfo_t ctune_UI_RSInfo_create( const WindowProperty_t * parent, const char * (* getDisplayText)( ctune_UI_TextID_e ), const char * col_sep ) {
    return (ctune_UI_RSInfo_t) {
        .initialised       = false,
        .screen_size       = parent,
        .margins           = { 0, 1, 1, 1 },
        .dialog            = ctune_UI_Dialog.init(),
        .col_separator_str = col_sep,
        .cache             = { .win_title = String.init(), .max_label_width = 0 },
        .cb                = { .getDisplayText = getDisplayText },
    };
}

/**
 * Initialises a ctune_UI_RSInfo_t object
 * @param rsinfo Un-initialised ctune_UI_RSInfo_t object
 * @return Success
 */
static bool ctune_UI_RSInfo_init( ctune_UI_RSInfo_t * rsinfo ) {
    if( rsinfo->initialised ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_RSInfo_init( %p )] RSInfo has already been initialised!", rsinfo );
        return false; //EARLY RETURN
    }

    if( rsinfo->cb.getDisplayText == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_RSInfo_init( %p )] Field description text method is NULL.", rsinfo );
        return false; //EARLY RETURN
    }

    rsinfo->cache.max_label_width  = 0;

    rsinfo->label_txt[ RSI_LABEL_STATION_NAME           ] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_STATION_NAME );
    rsinfo->label_txt[ RSI_LABEL_CHANGE_UUID            ] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_CHANGE_UUID );
    rsinfo->label_txt[ RSI_LABEL_STATION_UUID           ] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_STATION_UUID );
    rsinfo->label_txt[ RSI_LABEL_URL                    ] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_URL );
    rsinfo->label_txt[ RSI_LABEL_URL_RESOLVED           ] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_URL_RESOLVED );
    rsinfo->label_txt[ RSI_LABEL_URL_HOMEPAGE           ] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_URL_HOMEPAGE );
    rsinfo->label_txt[ RSI_LABEL_URL_FAVICON            ] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_URL_FAVICON );
    rsinfo->label_txt[ RSI_LABEL_TAGS                   ] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_TAGS );
    rsinfo->label_txt[ RSI_LABEL_COUNTRY                ] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_COUNTRY );
    rsinfo->label_txt[ RSI_LABEL_COUNTRY_CODE           ] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_COUNTRY_CODE );
    rsinfo->label_txt[ RSI_LABEL_STATE                  ] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_STATE );
    rsinfo->label_txt[ RSI_LABEL_LANGUAGE               ] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_LANGUAGE );
    rsinfo->label_txt[ RSI_LABEL_LANGUAGE_CODES         ] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_LANGUAGE_CODES );
    rsinfo->label_txt[ RSI_LABEL_VOTES                  ] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_VOTES );
    rsinfo->label_txt[ RSI_LABEL_LAST_CHANGED_TS        ] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_LAST_CHANGED_TS );
    rsinfo->label_txt[ RSI_LABEL_LAST_CHANGED_TS_ISO    ] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_LAST_CHANGED_TS_ISO8601 );
    rsinfo->label_txt[ RSI_LABEL_CODEC                  ] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_CODEC );
    rsinfo->label_txt[ RSI_LABEL_BITRATE                ] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_BITRATE );
    rsinfo->label_txt[ RSI_LABEL_HLS                    ] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_HLS );
    rsinfo->label_txt[ RSI_LABEL_LAST_CHECK_OK          ] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_LAST_CHECK_OK );
    rsinfo->label_txt[ RSI_LABEL_LAST_CHECK_TS          ] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_LAST_CHECK_TS );
    rsinfo->label_txt[ RSI_LABEL_LAST_CHECK_TS_ISO      ] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_LAST_CHECK_TS_ISO8601 );
    rsinfo->label_txt[ RSI_LABEL_LAST_CHECK_OK_TS       ] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_LAST_CHECK_OK_TS );
    rsinfo->label_txt[ RSI_LABEL_LAST_CHECK_OK_TS_ISO   ] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_LAST_CHECK_OK_TS_ISO8601 );
    rsinfo->label_txt[ RSI_LABEL_LAST_CHECK_TS_LOCAL    ] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_LAST_CHECK_TS_LOCAL );
    rsinfo->label_txt[ RSI_LABEL_LAST_CHECK_TS_LOCAL_ISO] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_LAST_CHECK_TS_LOCAL_ISO8601 );
    rsinfo->label_txt[ RSI_LABEL_CLICK_TS               ] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_CLICK_TS );
    rsinfo->label_txt[ RSI_LABEL_CLICK_TS_ISO           ] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_CLICK_TS_ISO8601 );
    rsinfo->label_txt[ RSI_LABEL_CLICKCOUNT             ] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_CLICKCOUNT );
    rsinfo->label_txt[ RSI_LABEL_CLICKTREND             ] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_CLICKTREND );
    rsinfo->label_txt[ RSI_LABEL_SSL_ERROR              ] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_SSL_ERROR );
    rsinfo->label_txt[ RSI_LABEL_GEO_COORDS             ] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_GEO_COORDS );
    rsinfo->label_txt[ RSI_LABEL_STATION_SOURCE         ] = rsinfo->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_STATION_SOURCE );

    if( rsinfo->cache.max_label_width <= 0 ) {
        for( int i = 0; i < CTUNE_UI_DIALOG_RSINFO_FIELD_COUNT; ++i ) {
            size_t l = strlen( rsinfo->label_txt[ i ] );
            if( rsinfo->cache.max_label_width < l )
                rsinfo->cache.max_label_width = l;
        }
    }

    return ( rsinfo->initialised = true );
}

/**
 * Get the initialised state of the instance
 * @param rsinfo Pointer to ctune_UI_RSInfo_t object
 * @return Initialised state
 */
static bool ctune_UI_RSInfo_isInitialised( const ctune_UI_RSInfo_t * rsinfo ) {
    return rsinfo->initialised;
}

/**
 * Create and show a populated window with the radio station's info
 * @param rsinfo Pointer to ctune_UI_RSInfo_t object
 * @param title  Title for the RSInfo dialog
 * @param rsi    RadioStationInfo_t object
 * @return Success
 */
static bool ctune_UI_RSInfo_show( ctune_UI_RSInfo_t * rsinfo, const char * title, const ctune_RadioStationInfo_t * rsi ) {
    if( !rsinfo->initialised ) {
        ctune_err.set( CTUNE_ERR_UI );
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_UI_RSInfo_show( %p, \"%s\", %p )] RSInfo_t was not initiated prior!",
                   rsinfo, title, rsi
        );

        return false; //EARLY RETURN - no init done
    }

    if( rsi == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_RSInfo_show( %p, \"%s\", %p )] RadioStationInfo DTO is NULL.",
                   rsinfo, title, rsi
        );

        return false; //EARLY RETURN - nothing to show here
    }

    ctune_UI_Dialog.free( &rsinfo->dialog );

    ctune_UI_RSInfo_printScroll( rsinfo, rsi );

    String.set( &rsinfo->cache.win_title, title ); //cache title in case of a repaint event

    ctune_UI_Dialog.createBorderWin( &rsinfo->dialog, rsinfo->screen_size, title, &rsinfo->margins );
    ctune_UI_Dialog.show( &rsinfo->dialog );

    ctune_UI_Resizer.push( ctune_UI_RSInfo.resize, rsinfo );

    return true;
}

/**
 * Resize the dialog
 * @param rsinfo Pointer to ctune_UI_RSInfo_t object
 */
static void ctune_UI_RSInfo_resize( void * rsinfo ) {
    CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_UI_RSInfo_resize( %p )] Resize event called.", rsinfo );

    if( rsinfo == NULL ) {
        ctune_err.set( CTUNE_ERR_UI );
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_RSInfo_resize( %p )] RSInfo_t is NULL.", rsinfo );
        return; //EARLY RETURN
    }

    ctune_UI_RSInfo_t * rsi_dialog = rsinfo;

    if( !rsi_dialog->initialised ) {
        ctune_err.set( CTUNE_ERR_UI );
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_RSInfo_resize( %p )] RSInfo_t is not initialised.", rsi_dialog );
        return; //EARLY RETURN - no init done
    }

    ctune_UI_Dialog.scrollHome( &rsi_dialog->dialog );
    ctune_UI_Dialog.createBorderWin( &rsi_dialog->dialog, rsi_dialog->screen_size, rsi_dialog->cache.win_title._raw, &rsi_dialog->margins );
    ctune_UI_Dialog.show( &rsi_dialog->dialog );
}

/**
 * Pass keyboard input to the form
 * @param rsinfo Pointer to ctune_UI_RSInfo_t object
 */
static void ctune_UI_RSInfo_captureInput( ctune_UI_RSInfo_t * rsinfo ) {
    keypad( rsinfo->dialog.canvas.pad, TRUE );
    curs_set( 0 );
    bool exit = false;
    int  ch;

    while( !exit ) {
        ch = wgetch( rsinfo->dialog.canvas.pad );

        switch( ctune_UI_KeyBinding.getAction( CTUNE_UI_CTX_RSINFO, ch ) ) {
            case CTUNE_UI_ACTION_RESIZE: {
                ctune_UI_Resizer.resize();
            } break;

            case CTUNE_UI_ACTION_ESC: {
                exit = true;
            } break;

            case CTUNE_UI_ACTION_HELP: {
                ctune_UI_ContextHelp.show( CTUNE_UI_CTX_RSINFO );
                ctune_UI_ContextHelp.captureInput();
            } break;

            case CTUNE_UI_ACTION_SCROLL_UP: {
                if( ctune_UI_Dialog.isScrollableY( &rsinfo->dialog ) )
                    ctune_UI_Dialog.scrollUp( &rsinfo->dialog );
                else
                    exit = true;
            } break;

            case CTUNE_UI_ACTION_SCROLL_DOWN: {
                if( ctune_UI_Dialog.isScrollableY( &rsinfo->dialog ) )
                    ctune_UI_Dialog.scrollDown( &rsinfo->dialog );
                else
                    exit = true;
            } break;

            case CTUNE_UI_ACTION_SCROLL_LEFT: {
                if( ctune_UI_Dialog.isScrollableX( &rsinfo->dialog ) )
                    ctune_UI_Dialog.scrollLeft( &rsinfo->dialog );
                else
                    exit = true;
            } break;

            case CTUNE_UI_ACTION_SCROLL_RIGHT: {
                if( ctune_UI_Dialog.isScrollableX( &rsinfo->dialog ) )
                    ctune_UI_Dialog.scrollRight( &rsinfo->dialog );
                else
                    exit = true;
            } break;

            case CTUNE_UI_ACTION_SCROLL_HOME: {
                if( ctune_UI_Dialog.isScrollableY( &rsinfo->dialog ) || ctune_UI_Dialog.isScrollableX( &rsinfo->dialog ) )
                    ctune_UI_Dialog.scrollHome( &rsinfo->dialog );
                else
                    exit = true;
            } break;

            default:
                exit = true;
                break;
        }

        ctune_UI_Dialog.refreshView( &rsinfo->dialog );
    }

    ctune_UI_Dialog.hide( &rsinfo->dialog );
    ctune_UI_Resizer.pop();

    doupdate();

    keypad( rsinfo->dialog.canvas.pad, FALSE );
}

/**
 * De-allocates resources
 * @param rsinfo Pointer to ctune_UI_RSInfo_t object
 */
static void ctune_UI_RSInfo_free( ctune_UI_RSInfo_t * rsinfo ) {
    if( rsinfo->initialised )
        ctune_UI_Dialog.free( &rsinfo->dialog );
    String.free( &rsinfo->cache.win_title );
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_RSInfo_free( %p )] RSInfo_t freed.", rsinfo );
}


/**
 * Namespace constructor
 */
const struct ctune_UI_Dialog_RSInfo_Namespace ctune_UI_RSInfo = {
    .create        = &ctune_UI_RSInfo_create,
    .init          = &ctune_UI_RSInfo_init,
    .isInitialised = &ctune_UI_RSInfo_isInitialised,
    .show          = &ctune_UI_RSInfo_show,
    .resize        = &ctune_UI_RSInfo_resize,
    .captureInput  = &ctune_UI_RSInfo_captureInput,
    .free          = &ctune_UI_RSInfo_free,
};