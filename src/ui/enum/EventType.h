#ifndef CTUNE_UI_ENUM_EVENTTYPE_H
#define CTUNE_UI_ENUM_EVENTTYPE_H

typedef enum {
    EVENT_SONG_CHANGE,
    EVENT_VOLUME_CHANGE,
    EVENT_PLAYBACK_STATE_CHANGE,
    EVENT_SEARCH_STATE_CHANGE,
    EVENT_ERROR_MSG,
    EVENT_STATUS_MSG,
    EVENT_STATION_CHANGE
} ctune_UI_EventType_e;

extern const struct ctune_UI_EventType_Namespace {
    /**
     * Gets corresponding string for an EventType
     * @param id EventType enum type
     * @return String
     */
    const char * (* str)( ctune_UI_EventType_e event_type );

} ctune_UI_EventType;


#endif //CTUNE_UI_ENUM_EVENTTYPE_H
