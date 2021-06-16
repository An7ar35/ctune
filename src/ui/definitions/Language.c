#include "Language.h"

#include "project_version.h"
#include "../../logger/Logger.h"

static const char * field_str[CTUNE_UI_TEXT_ENUM_COUNT] = {
    [CTUNE_UI_TEXT_WIN_TITLE_MAIN                   ] = CTUNE_APPNAME " " CTUNE_VERSION,
    [CTUNE_UI_TEXT_WIN_TITLE_FAV                    ] = "Favourites",
    [CTUNE_UI_TEXT_WIN_TITLE_SEARCH                 ] = "Search",
    [CTUNE_UI_TEXT_WIN_TITLE_BROWSE                 ] = "Browser",
    [CTUNE_UI_TEXT_WIN_TITLE_HELP                   ] = "[ Help ]",
    [CTUNE_UI_TEXT_WIN_TITLE_HELP_FAV               ] = "[ Help::Favourites ]",
    [CTUNE_UI_TEXT_WIN_TITLE_HELP_SEARCH            ] = "[ Help::Search ]",
    [CTUNE_UI_TEXT_WIN_TITLE_HELP_BROWSE            ] = "[ Help::Browse ]",
    [CTUNE_UI_TEXT_WIN_TITLE_HELP_RSINFO            ] = "[ Help::Radio Station Info ]",
    [CTUNE_UI_TEXT_WIN_TITLE_HELP_RSFIND            ] = "[ Help::Search for Station(s) ]",
    [CTUNE_UI_TEXT_WIN_TITLE_RSINFO_QUEUED          ] = "[ Queued Radio Station ]",
    [CTUNE_UI_TEXT_WIN_TITLE_RSINFO_SELECTED        ] = "[ Selected Radio Station ]",
    [CTUNE_UI_TEXT_WIN_TITLE_APPCFG                 ] = "[ Configuration ]",
    [CTUNE_UI_TEXT_WIN_TITLE_RSFIND                 ] = "[ Search for Station(s) ]",
    [CTUNE_UI_TEXT_WIN_TITLE_RSEDIT                 ] = "[ Add/Edit local station ]",
    [CTUNE_UI_TEXT_WIN_TITLE_OPTIONMENU             ] = "[ Options ]",
    [CTUNE_UI_TEXT_LABEL_STATION_NAME               ] = "Name",
    [CTUNE_UI_TEXT_LABEL_CHANGE_UUID                ] = "Change UUID",
    [CTUNE_UI_TEXT_LABEL_STATION_UUID               ] = "Station UUID",
    [CTUNE_UI_TEXT_LABEL_URL                        ] = "URL",
    [CTUNE_UI_TEXT_LABEL_URL_RESOLVED               ] = "Resolved URL",
    [CTUNE_UI_TEXT_LABEL_URL_HOMEPAGE               ] = "Homepage URL",
    [CTUNE_UI_TEXT_LABEL_URL_FAVICON                ] = "Favicon URL",
    [CTUNE_UI_TEXT_LABEL_TAGS                       ] = "Tag(s)",
    [CTUNE_UI_TEXT_LABEL_COUNTRY                    ] = "Country",
    [CTUNE_UI_TEXT_LABEL_STATE                      ] = "State",
    [CTUNE_UI_TEXT_LABEL_LANGUAGE                   ] = "Language",
    [CTUNE_UI_TEXT_LABEL_LANGUAGE_CODES             ] = "Language codes",
    [CTUNE_UI_TEXT_LABEL_VOTES                      ] = "Votes",
    [CTUNE_UI_TEXT_LABEL_LAST_CHANGED_TS            ] = "Last changed",
    [CTUNE_UI_TEXT_LABEL_LAST_CHANGED_TS_ISO8601    ] = "Last changed",
    [CTUNE_UI_TEXT_LABEL_CODEC                      ] = "Codec",
    [CTUNE_UI_TEXT_LABEL_BITRATE                    ] = "Bitrate",
    [CTUNE_UI_TEXT_LABEL_BITRATE_RANGE              ] = "Bitrate range",
    [CTUNE_UI_TEXT_LABEL_BITRATE_RANGE_SEPARATOR    ] = "to",
    [CTUNE_UI_TEXT_LABEL_BITRATE_UNIT_SHORT         ] = "k",
    [CTUNE_UI_TEXT_LABEL_BITRATE_UNIT_LONG          ] = "Kbps",
    [CTUNE_UI_TEXT_LABEL_HLS                        ] = "HTTP Live Streaming",
    [CTUNE_UI_TEXT_LABEL_LAST_CHECK_OK              ] = "Last check OK",
    [CTUNE_UI_TEXT_LABEL_LAST_CHECK_TS              ] = "Last checked",
    [CTUNE_UI_TEXT_LABEL_LAST_CHECK_TS_ISO8601      ] = "Last checked",
    [CTUNE_UI_TEXT_LABEL_LAST_CHECK_OK_TS           ] = "Last OK checked",
    [CTUNE_UI_TEXT_LABEL_LAST_CHECK_OK_TS_ISO8601   ] = "Last OK checked",
    [CTUNE_UI_TEXT_LABEL_LAST_CHECK_TS_LOCAL        ] = "Last checked (local)",
    [CTUNE_UI_TEXT_LABEL_LAST_CHECK_TS_LOCAL_ISO8601] = "Last checked (local)",
    [CTUNE_UI_TEXT_LABEL_CLICK_TS                   ] = "Click",
    [CTUNE_UI_TEXT_LABEL_CLICK_TS_ISO8601           ] = "Click",
    [CTUNE_UI_TEXT_LABEL_CLICKCOUNT                 ] = "Click count",
    [CTUNE_UI_TEXT_LABEL_CLICKTREND                 ] = "Click trend",
    [CTUNE_UI_TEXT_LABEL_COUNTRY_CODE               ] = "Country code",
    [CTUNE_UI_TEXT_LABEL_ORDER_BY                   ] = "Order by",
    [CTUNE_UI_TEXT_LABEL_REVERSE_ORDER              ] = "Reverse order",
    [CTUNE_UI_TEXT_LABEL_EXACT_MATCH                ] = "Exact match",
    [CTUNE_UI_TEXT_LABEL_GEO_COORDS                 ] = "Geo (x,y)",
    [CTUNE_UI_TEXT_LABEL_STATION_SOURCE             ] = "Source",
    [CTUNE_UI_TEXT_STATIONSOURCE_LOCAL              ] = "local",
    [CTUNE_UI_TEXT_STATIONSOURCE_RADIOBROWSER       ] = "www.radio-browser.info",
    [CTUNE_UI_TEXT_LABEL_SSL_ERROR                  ] = "SSL error",
    [CTUNE_UI_TEXT_LABEL_HIDE_BROKEN                ] = "Hide broken",
    [CTUNE_UI_TEXT_LABEL_LIMIT                      ] = "Result limit",
    [CTUNE_UI_TEXT_ORDERBY_NONE                     ] = "None",
    [CTUNE_UI_TEXT_ORDERBY_NAME                     ] = "Name",
    [CTUNE_UI_TEXT_ORDERBY_URL                      ] = "URL",
    [CTUNE_UI_TEXT_ORDERBY_HOMEPAGE                 ] = "Homepage",
    [CTUNE_UI_TEXT_ORDERBY_FAVICON                  ] = "Favicon",
    [CTUNE_UI_TEXT_ORDERBY_TAGS                     ] = "Tag(s)",
    [CTUNE_UI_TEXT_ORDERBY_COUNTRY                  ] = "Country",
    [CTUNE_UI_TEXT_ORDERBY_STATE                    ] = "State",
    [CTUNE_UI_TEXT_ORDERBY_LANGUAGE                 ] = "Language",
    [CTUNE_UI_TEXT_ORDERBY_VOTES                    ] = "Votes",
    [CTUNE_UI_TEXT_ORDERBY_CODEC                    ] = "Codec",
    [CTUNE_UI_TEXT_ORDERBY_BITRATE                  ] = "Bitrate",
    [CTUNE_UI_TEXT_ORDERBY_LASTCHECKOK              ] = "Last check OK",
    [CTUNE_UI_TEXT_ORDERBY_LASTCHECKTIME            ] = "Last check time",
    [CTUNE_UI_TEXT_ORDERBY_CLICKTIMESTAMP           ] = "Click timestamp",
    [CTUNE_UI_TEXT_ORDERBY_CLICKCOUNT               ] = "Click count",
    [CTUNE_UI_TEXT_ORDERBY_CLICKTREND               ] = "Click trend",
    [CTUNE_UI_TEXT_ORDERBY_RANDOM                   ] = "Random",
    [CTUNE_UI_TEXT_ORDERBY_STATIONCOUNT             ] = "Station count",
    [CTUNE_UI_TEXT_CAT_COUNTRIES                    ] = "Countries",
    [CTUNE_UI_TEXT_CAT_COUNTRY_CODES                ] = "Country codes",
    [CTUNE_UI_TEXT_CAT_CODECS                       ] = "Codecs",
    [CTUNE_UI_TEXT_CAT_STATES                       ] = "States",
    [CTUNE_UI_TEXT_CAT_LANGUAGES                    ] = "Languages",
    [CTUNE_UI_TEXT_CAT_TAGS                         ] = "Tags",
    [CTUNE_UI_TEXT_CAT_CLICKS                       ] = "Most clicked",
    [CTUNE_UI_TEXT_CAT_VOTES                        ] = "Votes",
    [CTUNE_UI_TEXT_CAT_RECENT_CLICKS                ] = "Recently clicked",
    [CTUNE_UI_TEXT_CAT_RECENT_ADD_OR_MOD            ] = "Recently added/modified",
    [CTUNE_UI_TEXT_MENU_SORT_STATIONS               ] = "Sort stations",
    [CTUNE_UI_TEXT_MENU_SORT_STATIONS_NAME          ] = "Name",
    [CTUNE_UI_TEXT_MENU_SORT_STATIONS_NAME_R        ] = "Name (desc)",
    [CTUNE_UI_TEXT_MENU_SORT_STATIONS_TAGS          ] = "Tags",
    [CTUNE_UI_TEXT_MENU_SORT_STATIONS_TAGS_R        ] = "Tags (desc)",
    [CTUNE_UI_TEXT_MENU_SORT_STATIONS_COUNTRY       ] = "Country",
    [CTUNE_UI_TEXT_MENU_SORT_STATIONS_COUNTRY_R     ] = "Country (desc)",
    [CTUNE_UI_TEXT_MENU_SORT_STATIONS_CC            ] = "Country Code",
    [CTUNE_UI_TEXT_MENU_SORT_STATIONS_CC_R          ] = "Country Code (desc)",
    [CTUNE_UI_TEXT_MENU_SORT_STATIONS_STATE         ] = "State",
    [CTUNE_UI_TEXT_MENU_SORT_STATIONS_STATE_R       ] = "State (desc)",
    [CTUNE_UI_TEXT_MENU_SORT_STATIONS_LANGUAGE      ] = "Language",
    [CTUNE_UI_TEXT_MENU_SORT_STATIONS_LANGUAGE_R    ] = "Language (desc)",
    [CTUNE_UI_TEXT_MENU_SORT_STATIONS_CODEC         ] = "Codec",
    [CTUNE_UI_TEXT_MENU_SORT_STATIONS_CODEC_R       ] = "Codec (desc)",
    [CTUNE_UI_TEXT_MENU_SORT_STATIONS_BITRATE       ] = "Bitrate",
    [CTUNE_UI_TEXT_MENU_SORT_STATIONS_BITRATE_R     ] = "Bitrate (desc)",
    [CTUNE_UI_TEXT_MENU_SORT_STATIONS_SOURCE        ] = "Source",
    [CTUNE_UI_TEXT_MENU_SORT_STATIONS_SOURCE_R      ] = "Source (desc)",
    [CTUNE_UI_TEXT_MENU_NEW_STATION                 ] = "New station",
    [CTUNE_UI_TEXT_MENU_EDIT_STATION                ] = "Edit selected",
    [CTUNE_UI_TEXT_MENU_TOGGLE_FAV                  ] = "Remove selected",
    [CTUNE_UI_TEXT_MENU_SYNC_UPSTREAM_BULK          ] = "Sync all",
    [CTUNE_UI_TEXT_MENU_SYNC_UPSTREAM               ] = "Sync selected",
    [CTUNE_UI_TEXT_TRUE                             ] = "Yes",
    [CTUNE_UI_TEXT_FALSE                            ] = "No",
    [CTUNE_UI_TEXT_OK                               ] = "OK",
    [CTUNE_UI_TEXT_BUTTON_CANCEL                    ] = "[ Cancel ]",
    [CTUNE_UI_TEXT_BUTTON_SUBMIT                    ] = "[ Submit ]",
    [CTUNE_UI_TEXT_BUTTON_SAVE                      ] = "[  Save  ]",
    [CTUNE_UI_TEXT_BUTTON_AUTODETECT_STREAM         ] = "[ Auto-detect/Test ]",
    [CTUNE_UI_TEXT_ERROR                            ] = "ERROR",
    [CTUNE_UI_TEXT_ELLIPSIS_LINE                    ] = "....",
    [CTUNE_UI_TEXT_STOP_LINE                        ] = "----",
    [CTUNE_UI_TEXT_TOGGLE_FIELD_ON                  ] = "[x]",
    [CTUNE_UI_TEXT_TOGGLE_FIELD_OFF                 ] = "[ ]",
    [CTUNE_UI_TEXT_FETCH_MORE_RESULTS               ] = "Get more results",
    [CTUNE_UI_TEXT_NULL_RSI_RESULTS                 ] = "No more results",
    [CTUNE_UI_TEXT_PLAYBACK_START                   ] = "Play",
    [CTUNE_UI_TEXT_PLAYBACK_START_AND_FAV           ] = "Fav & Play",
    [CTUNE_UI_TEXT_STATION_NOT_EDITABLE             ] = "Station not editable",
    [CTUNE_UI_TEXT_SYNC_SUCCESS                     ] = "Station synchronised with remote.",
    [CTUNE_UI_TEXT_SYNC_FAIL_FETCH                  ] = "Failed station synchronisation: see log",
    [CTUNE_UI_TEXT_SYNC_FAIL_FETCH_REMOTE_NOT_FOUND ] = "Failed station synchronisation: remote station does not exist (deleted?)",
    [CTUNE_UI_TEXT_SYNC_FAIL_LOCAL_STATION          ] = "Failed station synchronisation: local stations are not sync-able",
    [CTUNE_UI_TEXT_HELP                             ] = "Help",
    [CTUNE_UI_TEXT_HELP_KEY                         ] = "Show/Hide contextual help",
    [CTUNE_UI_TEXT_HELP_ESC                         ] = "Go back",
    [CTUNE_UI_TEXT_HELP_CMD_ESC                     ] = "Escape command line environment",
    [CTUNE_UI_TEXT_HELP_FIRST_ENTRY                 ] = "Select first item in list",
    [CTUNE_UI_TEXT_HELP_LAST_ENTRY                  ] = "Select last item in list",
    [CTUNE_UI_TEXT_HELP_NEXT_ENTRY                  ] = "Select next item",
    [CTUNE_UI_TEXT_HELP_PREV_ENTRY                  ] = "Select previous item",
    [CTUNE_UI_TEXT_HELP_NEXT_ENTRY_PAGE             ] = "Select next item 1 page away",
    [CTUNE_UI_TEXT_HELP_PREV_ENTRY_PAGE             ] = "Select previous item 1 page away",
    [CTUNE_UI_TEXT_HELP_SCROLL_HOME                 ] = "Scroll back to the top left position",
    [CTUNE_UI_TEXT_HELP_SCROLL_LEFT                 ] = "Scroll left",
    [CTUNE_UI_TEXT_HELP_SCROLL_RIGHT                ] = "Scroll right",
    [CTUNE_UI_TEXT_HELP_SCROLL_UP                   ] = "Scroll up",
    [CTUNE_UI_TEXT_HELP_SCROLL_DOWN                 ] = "Scroll down",
    [CTUNE_UI_TEXT_HELP_FOCUS_RIGHT                 ] = "Shift focus right",
    [CTUNE_UI_TEXT_HELP_FOCUS_LEFT                  ] = "Shift focus left",
    [CTUNE_UI_TEXT_HELP_FORM_ESC                    ] = "Close dialog",
    [CTUNE_UI_TEXT_HELP_FIRST_FIELD                 ] = "Go to first field in form",
    [CTUNE_UI_TEXT_HELP_LAST_FIELD                  ] = "Go to last field in form",
    [CTUNE_UI_TEXT_HELP_PREV_FIELD                  ] = "Go to the previous field in form",
    [CTUNE_UI_TEXT_HELP_NEXT_FIELD                  ] = "Go to the next field in form",
    [CTUNE_UI_TEXT_HELP_FIELD_RETURN                ] = "Next field/Press button",
    [CTUNE_UI_TEXT_HELP_FIELD_BEGIN                 ] = "Move cursor to the beginning of the field",
    [CTUNE_UI_TEXT_HELP_FIELD_END                   ] = "Move cursor to the end of the field",
    [CTUNE_UI_TEXT_HELP_FIELD_TOGGLE                ] = "Toggle field",
    [CTUNE_UI_TEXT_HELP_FIELD_TOGGLE_ALT            ] = "Toggle field/Press button",
    [CTUNE_UI_TEXT_HELP_CLEAR_CURR_FIELD            ] = "Clear current field",
    [CTUNE_UI_TEXT_HELP_CLEAR_ALL_FIELDS            ] = "Clear all fields",
    [CTUNE_UI_TEXT_HELP_CLOSE_FORM                  ] = "Close form",
    [CTUNE_UI_TEXT_HELP_SUBMIT_FORM                 ] = "Submit form",
    [CTUNE_UI_TEXT_HELP_CHANGE_TAB_1                ] = "Navigate to the 'Favourites' tab",
    [CTUNE_UI_TEXT_HELP_CHANGE_TAB_2                ] = "Navigate to the 'Search' tab",
    [CTUNE_UI_TEXT_HELP_CHANGE_TAB_3                ] = "Navigate to the 'Browser' tab",
    [CTUNE_UI_TEXT_HELP_OPEN_RSFIND_FORM            ] = "Open up the radio station search form",
    [CTUNE_UI_TEXT_HELP_OPEN_RSEDIT_FORM_NEW        ] = "Create a new locally stored station",
    [CTUNE_UI_TEXT_HELP_OPEN_RSEDIT_FORM_EDIT       ] = "Edit a locally stored station",
    [CTUNE_UI_TEXT_HELP_OPEN_RSINFO_QUEUED          ] = "Show detailed information on the queued radio station",
    [CTUNE_UI_TEXT_HELP_OPEN_RSINFO_SELECTED        ] = "Show detailed information for selected radio station",
    [CTUNE_UI_TEXT_HELP_OPEN_OPTIONS                ] = "Opens the option menu",
    [CTUNE_UI_TEXT_HELP_PLAYBACK_START              ] = "Start playback of selected station",
    [CTUNE_UI_TEXT_HELP_PLAYBACK_RESUME             ] = "Resume playback of queued station",
    [CTUNE_UI_TEXT_HELP_PLAYBACK_STOP               ] = "Stop playback",
    [CTUNE_UI_TEXT_HELP_VOL_UP                      ] = "Increases volume by +5",
    [CTUNE_UI_TEXT_HELP_VOL_DOWN                    ] = "Decreases volume by -5",
    [CTUNE_UI_TEXT_HELP_TOGGLE_FAV                  ] = "Toggle \"favourite\" state",
    [CTUNE_UI_TEXT_HELP_CMDLINE                     ] = "Access ctune command line (press ESC to leave)",
    [CTUNE_UI_TEXT_HELP_QUIT                        ] = "Quit",
    [CTUNE_UI_TEXT_MSG_CONFIRM_UNFAV                ] = "Remove station from favourites? [y/N]",
    [CTUNE_UI_TEXT_MSG_CONFIRM_QUIT                 ] = "Quit ctune? [q/y/N]",
    [CTUNE_UI_TEXT_MSG_EMPTY_QUEUE                  ] = "No station queued, please select a station first.",
    [CTUNE_UI_TEXT_BLANK                            ] = "",
};

/**
 * Gets the string for a piece of display text
 * @param text_id Text ID enum
 * @return String associated with given enum
 */
static const char * ctune_UI_Language_text( ctune_UI_TextID_e text_id ) {
    if( ( sizeof( field_str ) / sizeof( field_str[0] ) ) <= text_id ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_Language_text( %i )] ID not implemented.", text_id );
        return "## NO ASSOCIATED TEXT ##";
    }

    return field_str[text_id];
}


const struct ctune_UI_Language_Instance ctune_UI_Language = {
    .text = &ctune_UI_Language_text,
};