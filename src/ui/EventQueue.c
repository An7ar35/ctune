#include "EventQueue.h"

#include "logger/src/Logger.h"
#include "../datastructure/CircularBuffer.h"

static CircularBuffer_t event_queue;
static processEventCb   event_processor_cb = NULL;

/**
 * Initialises the EventQueue
 * @param cb Event processing method callback
 * @return Success
 */
static bool ctune_UI_EventQueue_init( processEventCb cb ) {
    if( !CircularBuffer.init( &event_queue, 4096, true ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_UI_EventQueue_init( %p )] Failed to init the circular buffer for the event_queue!",
                   cb
        );

        return false;
    }

    if( !cb ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_UI_EventQueue_init( %p )] Callback is NULL.",
                   cb
        );

        return false;
    }

    event_processor_cb = cb;
    return true;
}

/**
 * Adds new event to queue
 * @param event Pointer to Event
 */
static void ctune_UI_EventQueue_add( ctune_UI_Event_t * event ) {
    const size_t ln = CircularBuffer.writeChunk( &event_queue, (const u_int8_t *) event, sizeof( ctune_UI_Event_t ) );

    if( ln != sizeof( ctune_UI_Event_t ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_EventQueue_add( %p )] Failed to write Event (written %d/%d bytes)",
                   event, ln, sizeof( ctune_UI_Event_t )
        );
    }
}

/**
 * Processes all data inside the event_queue
 */
static void ctune_UI_EventQueue_flush( void ) {
    CTUNE_LOG( CTUNE_LOG_MSG, "[ctune_UI_EventQueue_flush()] Flushing event queue..." );

    while( !ctune_UI_EventQueue.empty() ) {
        ctune_UI_Event_t event;

        const size_t ln = CircularBuffer.readChunk( &event_queue, (u_int8_t *) &event, sizeof( ctune_UI_Event_t ) );

        if( ln != sizeof( ctune_UI_Event_t ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_EventQueue_flush()] Failed to cast to Event (got %d, needed %d bytes).",
                       ln, sizeof( ctune_UI_Event_t )
            );

            continue;
        }

        event_processor_cb( &event );
    }
}

/**
 * Checks empty state of event_queue
 * @return Empty state
 */
static bool ctune_UI_EventQueue_empty( void ) {
    return CircularBuffer.empty( &event_queue );
}

/**
 * De-allocates internal variables
 */
static void ctune_UI_EventQueue_free( void ) {
    CircularBuffer.free( &event_queue );
}

/**
 * Namespace constructor
 */
const struct ctune_UI_EventQueue_Instance ctune_UI_EventQueue = {
    .init  = &ctune_UI_EventQueue_init,
    .add   = &ctune_UI_EventQueue_add,
    .flush = &ctune_UI_EventQueue_flush,
    .empty = &ctune_UI_EventQueue_empty,
    .free  = &ctune_UI_EventQueue_free,
} ;