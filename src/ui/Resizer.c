#include "Resizer.h"

#include <stdlib.h>

#include "logger/src/Logger.h"
#include "../datastructure/Deque.h"

/**
 * Container structure
 * @param resize_cb Pointer to a resize method callback
 * @param data      Pointer to pass to callback
 */
typedef struct {
    void (* resize_cb)( void * );
    void * data;
} WinCallback_t;

/**
 * Private variables
 * @param initialised Init flag
 * @param queue       List of resize callbacks
 */
struct {
    bool    initialised;
    Deque_t queue;

} resizer = {
    .initialised = false,
};

/**
 * Initialises the Resizer
 */
static void ctune_UI_Resizer_init( void ) {
    if( !resizer.initialised ) {
        resizer.queue       = Deque.init();
        resizer.initialised = true;
    }
}

/**
 * Adds a new resize callback method to the list
 * @param resize_fn Callback for resizing
 * @param data      Pointer to pass to callback
 * @return Success
 */
static bool ctune_UI_Resizer_push( void(* resize_fn)( void * ), void * data ) {
    if( !resizer.initialised ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_Resizer_push( %p, %p )] Resizer not initialised.", resize_fn, data );
        return false; //EARLY RETURN
    }

    if( resize_fn == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_Resizer_push( %p, %p )] Callback method is NULL.", resize_fn, data );
        return false; //EARLY RETURN
    }

    WinCallback_t * container = malloc( sizeof( WinCallback_t ) );

    if( container == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_Resizer_push( %p, %p )] Failed to allocate container memory.", resize_fn, data );
        return false; //EARLY RETURN
    }

    container->resize_cb = resize_fn;
    container->data      = data;

    Deque.pushBack( &resizer.queue, container );

    CTUNE_LOG( CTUNE_LOG_TRACE,
               "[ctune_UI_Resizer_push( %p, %p )] Pushed new UI item in resizer deque (count = %lu).",
               resize_fn, data, Deque.size( &resizer.queue )
    );

    return true;
}

/**
 * Removes the last resize callback from the list
 */
static void ctune_UI_Resizer_pop( void ) {
    if( !resizer.initialised ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_Resizer_pop()] Resizer not initialised." );
        return; //EARLY RETURN
    }

    WinCallback_t * wc = Deque.popBack( &resizer.queue );
    free( wc );
}

/**
 * Calls all callbacks stored in the list in FIFO order
 */
static void ctune_UI_Resizer_resize( void ) {
    if( !resizer.initialised ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_Resizer_resize()] Resizer not initialised." );
        return; //EARLY RETURN
    }

    const size_t total_cb = Deque.size( &resizer.queue );

    if( !Deque.empty( &resizer.queue ) ) {
        size_t             count = 0;
        struct DequeNode * node  = resizer.queue.front;

        while( node != NULL ) {
            CTUNE_LOG( CTUNE_LOG_MSG,
                       "[ctune_UI_Resizer_resize()] Resize event called: %lu/%lu callbacks.",
                       ( count + 1 ), total_cb
            );

            WinCallback_t * wcb = node->data;
            wcb->resize_cb( wcb->data );
            node = node->next;
            ++count;
        }
    }
}

/**
 * De-allocates internal variables and resets everything back to an initialised state
 */
static void ctune_UI_Resizer_free( void ) {
    if( resizer.initialised ) {
        Deque.free( &resizer.queue, free );
        resizer.initialised     = false;

        CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_Resizer_free()] Resizer's vars freed." );
    }
}

/**
 * Namespace constructor
 */
const struct ctune_UI_Resizer_Instance ctune_UI_Resizer = {
    .init   = &ctune_UI_Resizer_init,
    .push   = &ctune_UI_Resizer_push,
    .pop    = &ctune_UI_Resizer_pop,
    .resize = &ctune_UI_Resizer_resize,
    .free   = &ctune_UI_Resizer_free,
} ;