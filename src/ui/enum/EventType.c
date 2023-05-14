#include "EventType.h"

/**
 * Gets corresponding string for an EventType
 * @param id EventType enum type
 * @return String
 */
static const char * ctune_UI_EventType_str( ctune_UI_EventType_e event_type ) {
    static const char * str[7] = {
        [EVENT_SONG_CHANGE          ] = "song change",
        [EVENT_VOLUME_CHANGE        ] = "volume change",
        [EVENT_PLAYBACK_STATE_CHANGE] = "playback state change",
        [EVENT_SEARCH_STATE_CHANGE  ] = "search state change",
        [EVENT_ERROR_MSG            ] = "error message",
        [EVENT_STATUS_MSG           ] = "status message",
        [EVENT_STATION_CHANGE       ] = "station change",
    };

    return str[event_type];
}

/**
 * Namespace constructor
 */
const struct ctune_UI_EventType_Namespace ctune_UI_EventType = {
    .str = &ctune_UI_EventType_str,
};