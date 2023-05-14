#ifndef CTUNE_UI_EVENTQUEUE_H
#define CTUNE_UI_EVENTQUEUE_H

#include <stdbool.h>

#include "../enum/PlaybackCtrl.h"
#include "enum/EventType.h"

#pragma pack(push, 1)
typedef struct {
    ctune_UI_EventType_e type;

    union Data {
        void               * pointer;
        int                  integer;
        ctune_PlaybackCtrl_e playback_ctrl;
    } data;
} ctune_UI_Event_t;
#pragma pack(pop)


typedef void (* processEventCb)( ctune_UI_Event_t * );

/**
 * Event Queue
 */
extern const struct ctune_UI_EventQueue_Instance {
    /**
     * Initialises the EventQueue
     * @param cb Event processing method callback
     * @return Success
     */
    bool (* init)( processEventCb cb );

    /**
     * Adds new event to queue
     * @param event Pointer to Event
     */
    void (* add)( ctune_UI_Event_t * event );

    /**
     * Processes all data inside the event_queue
     */
    void (* flush)( void );

    /**
     * Checks empty state of event_queue
     * @return Empty state
     */
    bool (* empty)( void );

    /**
     * De-allocates internal variables and resets everything back to an initialised state
     */
    void (* free)( void );

} ctune_UI_EventQueue;

#endif //CTUNE_UI_EVENTQUEUE_H