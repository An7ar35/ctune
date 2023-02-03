#ifndef CTUNE_UI_ENUM_FIELDID_H
#define CTUNE_UI_ENUM_FIELDID_H

typedef enum ctune_UI_TextID {
    //Application name on the top bar
    CTUNE_UI_TEXT_WIN_TITLE_MAIN = 0,
    //Tab titles
    CTUNE_UI_TEXT_WIN_TITLE_FAV,
    CTUNE_UI_TEXT_WIN_TITLE_SEARCH,
    CTUNE_UI_TEXT_WIN_TITLE_BROWSE,
    //Popup titles
    CTUNE_UI_TEXT_WIN_TITLE_HELP,
    CTUNE_UI_TEXT_WIN_TITLE_HELP_FAV,
    CTUNE_UI_TEXT_WIN_TITLE_HELP_SEARCH,
    CTUNE_UI_TEXT_WIN_TITLE_HELP_BROWSE,
    CTUNE_UI_TEXT_WIN_TITLE_HELP_RSINFO,
    CTUNE_UI_TEXT_WIN_TITLE_HELP_RSFIND,
    CTUNE_UI_TEXT_WIN_TITLE_RSINFO_QUEUED,
    CTUNE_UI_TEXT_WIN_TITLE_RSINFO_SELECTED,
    CTUNE_UI_TEXT_WIN_TITLE_RSFIND,
    CTUNE_UI_TEXT_WIN_TITLE_RSEDIT,
    CTUNE_UI_TEXT_WIN_TITLE_OPTIONMENU,
    //Radio Station information
    CTUNE_UI_TEXT_LABEL_STATION_NAME,
    CTUNE_UI_TEXT_LABEL_CHANGE_UUID,
    CTUNE_UI_TEXT_LABEL_STATION_UUID,
    CTUNE_UI_TEXT_LABEL_URL,
    CTUNE_UI_TEXT_LABEL_URL_RESOLVED,
    CTUNE_UI_TEXT_LABEL_URL_HOMEPAGE,
    CTUNE_UI_TEXT_LABEL_URL_FAVICON,
    CTUNE_UI_TEXT_LABEL_TAGS,
    CTUNE_UI_TEXT_LABEL_COUNTRY,
    CTUNE_UI_TEXT_LABEL_STATE,
    CTUNE_UI_TEXT_LABEL_LANGUAGE,
    CTUNE_UI_TEXT_LABEL_LANGUAGE_CODES,
    CTUNE_UI_TEXT_LABEL_VOTES,
    CTUNE_UI_TEXT_LABEL_LAST_CHANGED_TS,
    CTUNE_UI_TEXT_LABEL_LAST_CHANGED_TS_ISO8601,
    CTUNE_UI_TEXT_LABEL_CODEC,
    CTUNE_UI_TEXT_LABEL_BITRATE,
    CTUNE_UI_TEXT_LABEL_BITRATE_RANGE,
    CTUNE_UI_TEXT_LABEL_BITRATE_RANGE_SEPARATOR,
    CTUNE_UI_TEXT_LABEL_BITRATE_UNIT_SHORT,
    CTUNE_UI_TEXT_LABEL_BITRATE_UNIT_LONG,
    CTUNE_UI_TEXT_LABEL_HLS,
    CTUNE_UI_TEXT_LABEL_LAST_CHECK_OK,
    CTUNE_UI_TEXT_LABEL_LAST_CHECK_TS,
    CTUNE_UI_TEXT_LABEL_LAST_CHECK_TS_ISO8601,
    CTUNE_UI_TEXT_LABEL_LAST_CHECK_OK_TS,
    CTUNE_UI_TEXT_LABEL_LAST_CHECK_OK_TS_ISO8601,
    CTUNE_UI_TEXT_LABEL_LAST_CHECK_TS_LOCAL,
    CTUNE_UI_TEXT_LABEL_LAST_CHECK_TS_LOCAL_ISO8601,
    CTUNE_UI_TEXT_LABEL_CLICK_TS,
    CTUNE_UI_TEXT_LABEL_CLICK_TS_ISO8601,
    CTUNE_UI_TEXT_LABEL_CLICKCOUNT,
    CTUNE_UI_TEXT_LABEL_CLICKTREND,
    CTUNE_UI_TEXT_LABEL_SSL_ERROR,
    CTUNE_UI_TEXT_LABEL_GEO_COORDS,
    CTUNE_UI_TEXT_LABEL_STATION_SOURCE,
    CTUNE_UI_TEXT_STATIONSOURCE_LOCAL,
    CTUNE_UI_TEXT_STATIONSOURCE_RADIOBROWSER,
    CTUNE_UI_TEXT_LABEL_HIDE_BROKEN,
    CTUNE_UI_TEXT_LABEL_LIMIT,
    CTUNE_UI_TEXT_ORDERBY_NONE,
    CTUNE_UI_TEXT_ORDERBY_NAME,
    CTUNE_UI_TEXT_ORDERBY_URL,
    CTUNE_UI_TEXT_ORDERBY_HOMEPAGE,
    CTUNE_UI_TEXT_ORDERBY_FAVICON,
    CTUNE_UI_TEXT_ORDERBY_TAGS,
    CTUNE_UI_TEXT_ORDERBY_COUNTRY,
    CTUNE_UI_TEXT_ORDERBY_STATE,
    CTUNE_UI_TEXT_ORDERBY_LANGUAGE,
    CTUNE_UI_TEXT_ORDERBY_VOTES,
    CTUNE_UI_TEXT_ORDERBY_CODEC,
    CTUNE_UI_TEXT_ORDERBY_BITRATE,
    CTUNE_UI_TEXT_ORDERBY_LASTCHECKOK,
    CTUNE_UI_TEXT_ORDERBY_LASTCHECKTIME,
    CTUNE_UI_TEXT_ORDERBY_CLICKTIMESTAMP,
    CTUNE_UI_TEXT_ORDERBY_CLICKCOUNT,
    CTUNE_UI_TEXT_ORDERBY_CLICKTREND,
    CTUNE_UI_TEXT_ORDERBY_RANDOM,
    CTUNE_UI_TEXT_ORDERBY_STATIONCOUNT,
    CTUNE_UI_TEXT_ORDERBY_CHANGETIMESTAMP,
    //Find radio station
    CTUNE_UI_TEXT_LABEL_COUNTRY_CODE,
    CTUNE_UI_TEXT_LABEL_ORDER_BY,
    CTUNE_UI_TEXT_LABEL_REVERSE_ORDER,
    CTUNE_UI_TEXT_LABEL_EXACT_MATCH,
    //Configuration
    CTUNE_UI_TEXT_ROWSIZE_1X,
    CTUNE_UI_TEXT_ROWSIZE_2X,
    CTUNE_UI_TEXT_MOUSE_DISABLE,
    CTUNE_UI_TEXT_MOUSE_ENABLE,
    CTUNE_UI_TEXT_ICONS_ASCII,
    CTUNE_UI_TEXT_ICONS_UNICODE,
    CTUNE_UI_TEXT_FAV_THEMING_OFF,
    CTUNE_UI_TEXT_FAV_THEMING_ON,
    CTUNE_UI_TEXT_FAV_USE_CUSTOM_COLOURS,
    CTUNE_UI_TEXT_FAV_USE_PRESET_COLOURS,
    //Browser categories
    CTUNE_UI_TEXT_CAT_COUNTRIES,
    CTUNE_UI_TEXT_CAT_COUNTRY_CODES,
    CTUNE_UI_TEXT_CAT_CODECS,
    CTUNE_UI_TEXT_CAT_STATES,
    CTUNE_UI_TEXT_CAT_LANGUAGES,
    CTUNE_UI_TEXT_CAT_TAGS,
    CTUNE_UI_TEXT_CAT_CLICKS,
    CTUNE_UI_TEXT_CAT_VOTES,
    CTUNE_UI_TEXT_CAT_RECENT_CLICKS,
    CTUNE_UI_TEXT_CAT_RECENT_ADD_OR_MOD,
    //Options menu
    CTUNE_UI_TEXT_MENU_CONFIGURATION,
    CTUNE_UI_TEXT_MENU_SORT_STATIONS,
    CTUNE_UI_TEXT_MENU_SORT_STATIONS_NAME,
    CTUNE_UI_TEXT_MENU_SORT_STATIONS_NAME_R,
    CTUNE_UI_TEXT_MENU_SORT_STATIONS_TAGS,
    CTUNE_UI_TEXT_MENU_SORT_STATIONS_TAGS_R,
    CTUNE_UI_TEXT_MENU_SORT_STATIONS_COUNTRY,
    CTUNE_UI_TEXT_MENU_SORT_STATIONS_COUNTRY_R,
    CTUNE_UI_TEXT_MENU_SORT_STATIONS_CC,
    CTUNE_UI_TEXT_MENU_SORT_STATIONS_CC_R,
    CTUNE_UI_TEXT_MENU_SORT_STATIONS_STATE,
    CTUNE_UI_TEXT_MENU_SORT_STATIONS_STATE_R,
    CTUNE_UI_TEXT_MENU_SORT_STATIONS_LANGUAGE,
    CTUNE_UI_TEXT_MENU_SORT_STATIONS_LANGUAGE_R,
    CTUNE_UI_TEXT_MENU_SORT_STATIONS_CODEC,
    CTUNE_UI_TEXT_MENU_SORT_STATIONS_CODEC_R,
    CTUNE_UI_TEXT_MENU_SORT_STATIONS_BITRATE,
    CTUNE_UI_TEXT_MENU_SORT_STATIONS_BITRATE_R,
    CTUNE_UI_TEXT_MENU_SORT_STATIONS_SOURCE,
    CTUNE_UI_TEXT_MENU_SORT_STATIONS_SOURCE_R,
    CTUNE_UI_TEXT_MENU_NEW_STATION,
    CTUNE_UI_TEXT_MENU_EDIT_STATION,
    CTUNE_UI_TEXT_MENU_TOGGLE_FAV,
    CTUNE_UI_TEXT_MENU_SYNC_UPSTREAM_BULK,
    CTUNE_UI_TEXT_MENU_SYNC_UPSTREAM,
    CTUNE_UI_TEXT_MENU_OPTIONS,
    CTUNE_UI_TEXT_MENU_UI_THEME,
    CTUNE_UI_TEXT_MENU_STREAM_TIMEOUT,
    CTUNE_UI_TEXT_MENU_MOUSE,
    CTUNE_UI_TEXT_MENU_MOUSE_CLICK_INTERVAL,
    //General
    CTUNE_UI_TEXT_TRUE,
    CTUNE_UI_TEXT_FALSE,
    CTUNE_UI_TEXT_OK,
    CTUNE_UI_TEXT_DEFAULT,
    CTUNE_UI_TEXT_CUSTOM,
    CTUNE_UI_TEXT_BUTTON_CANCEL,
    CTUNE_UI_TEXT_BUTTON_SUBMIT,
    CTUNE_UI_TEXT_BUTTON_SAVE,
    CTUNE_UI_TEXT_BUTTON_AUTODETECT_STREAM,
    CTUNE_UI_TEXT_ERROR,
    CTUNE_UI_TEXT_ELLIPSIS_LINE,
    CTUNE_UI_TEXT_STOP_LINE,
    CTUNE_UI_TEXT_TOGGLE_FIELD_ON,
    CTUNE_UI_TEXT_TOGGLE_FIELD_OFF,
    //Action
    CTUNE_UI_TEXT_FETCH_MORE_RESULTS,
    CTUNE_UI_TEXT_NULL_RSI_RESULTS,
    CTUNE_UI_TEXT_PLAYBACK_START,
    CTUNE_UI_TEXT_PLAYBACK_START_AND_FAV,
    //Feedback message
    CTUNE_UI_TEXT_STATION_NOT_EDITABLE,
    CTUNE_UI_TEXT_SYNC_SUCCESS,
    CTUNE_UI_TEXT_SYNC_FAIL_LOCAL_STATION,
    CTUNE_UI_TEXT_SYNC_FAIL_FETCH_REMOTE_NOT_FOUND,
    CTUNE_UI_TEXT_SYNC_FAIL_FETCH,
    //Help text
    CTUNE_UI_TEXT_HELP,
    CTUNE_UI_TEXT_HELP_KEY,
    CTUNE_UI_TEXT_HELP_ESC,
    CTUNE_UI_TEXT_HELP_CMD_ESC,
    CTUNE_UI_TEXT_HELP_FIRST_ENTRY,
    CTUNE_UI_TEXT_HELP_LAST_ENTRY,
    CTUNE_UI_TEXT_HELP_NEXT_ENTRY,
    CTUNE_UI_TEXT_HELP_PREV_ENTRY,
    CTUNE_UI_TEXT_HELP_NEXT_ENTRY_PAGE,
    CTUNE_UI_TEXT_HELP_PREV_ENTRY_PAGE,
    CTUNE_UI_TEXT_HELP_SCROLL_HOME,
    CTUNE_UI_TEXT_HELP_SCROLL_LEFT,
    CTUNE_UI_TEXT_HELP_SCROLL_RIGHT,
    CTUNE_UI_TEXT_HELP_SCROLL_UP,
    CTUNE_UI_TEXT_HELP_SCROLL_DOWN,
    CTUNE_UI_TEXT_HELP_FOCUS_RIGHT,
    CTUNE_UI_TEXT_HELP_FOCUS_LEFT,
    CTUNE_UI_TEXT_HELP_FORM_ESC,
    CTUNE_UI_TEXT_HELP_FIRST_FIELD,
    CTUNE_UI_TEXT_HELP_LAST_FIELD,
    CTUNE_UI_TEXT_HELP_PREV_FIELD,
    CTUNE_UI_TEXT_HELP_NEXT_FIELD,
    CTUNE_UI_TEXT_HELP_FIELD_RETURN,
    CTUNE_UI_TEXT_HELP_FIELD_BEGIN,
    CTUNE_UI_TEXT_HELP_FIELD_END,
    CTUNE_UI_TEXT_HELP_FIELD_TOGGLE,
    CTUNE_UI_TEXT_HELP_FIELD_TOGGLE_ALT,
    CTUNE_UI_TEXT_HELP_CLEAR_CURR_FIELD,
    CTUNE_UI_TEXT_HELP_CLEAR_ALL_FIELDS,
    CTUNE_UI_TEXT_HELP_CLOSE_FORM,
    CTUNE_UI_TEXT_HELP_SUBMIT_FORM,
    CTUNE_UI_TEXT_HELP_CHANGE_TAB_1,
    CTUNE_UI_TEXT_HELP_CHANGE_TAB_2,
    CTUNE_UI_TEXT_HELP_CHANGE_TAB_3,
    CTUNE_UI_TEXT_HELP_OPEN_RSFIND_FORM,
    CTUNE_UI_TEXT_HELP_OPEN_RSEDIT_FORM_NEW,
    CTUNE_UI_TEXT_HELP_OPEN_RSEDIT_FORM_EDIT,
    CTUNE_UI_TEXT_HELP_OPEN_RSINFO_QUEUED,
    CTUNE_UI_TEXT_HELP_OPEN_RSINFO_SELECTED,
    CTUNE_UI_TEXT_HELP_OPEN_OPTIONS,
    CTUNE_UI_TEXT_HELP_PLAYBACK_START,
    CTUNE_UI_TEXT_HELP_PLAYBACK_RESUME,
    CTUNE_UI_TEXT_HELP_PLAYBACK_STOP,
    CTUNE_UI_TEXT_HELP_VOL_UP,
    CTUNE_UI_TEXT_HELP_VOL_DOWN,
    CTUNE_UI_TEXT_HELP_TOGGLE_FAV,
    CTUNE_UI_TEXT_HELP_CMDLINE,
    CTUNE_UI_TEXT_HELP_QUIT,
    //Message
    CTUNE_UI_TEXT_MSG_CONFIRM_UNFAV,
    CTUNE_UI_TEXT_MSG_CONFIRM_QUIT,
    CTUNE_UI_TEXT_MSG_EMPTY_QUEUE,
    //formatting
    CTUNE_UI_TEXT_BLANK,

    CTUNE_UI_TEXT_ENUM_COUNT
} ctune_UI_TextID_e;

#endif //CTUNE_UI_ENUM_FIELDID_H
