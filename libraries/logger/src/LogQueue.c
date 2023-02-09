#include "LogQueue.h"

#include <stdlib.h>

/**
 * Creates a new LogQueueNode
 * @param str  String
 * @param next Pointer to the next node in the Queue (NULL if none)
 * @return Created LogQueueNode
 */
LogQueueNode_t * ctune_LogQueue_create_node( const char * str, LogQueueNode_t * next ) {
    LogQueueNode_t * node;

    if( ( node = (LogQueueNode_t *) malloc( sizeof( LogQueueNode_t ) ) ) == NULL ) {
        return NULL;
    }

    if( ( node->data = malloc( strlen( str ) + 1 ) ) == NULL ) {
        free( node );
        return NULL;
    }

    strcpy( node->data, str );
    node->next = next;

    return node;
}

/**
 * [THREAD SAFE] Add a node containing string to the queue
 * @param queue LogQueue instance
 * @param str   String to copy to the queued node
 * @return Success
 */
bool ctune_LogQueue_enqueue( struct ctune_LogQueue * queue, const char * str ) {
    if( queue == NULL )
        return false; //EARLY RETURN

    LogQueueNode_t * node    = ctune_LogQueue.create_node( str, NULL );
    bool             success = false;

    pthread_mutex_lock( &queue->_mutex );

    if( node != NULL ) {

        if( queue->_back == NULL ) { //i.e.: empty queue
            queue->_back  = node;
            queue->_front = queue->_back;

        } else {
            queue->_back->next = node;
            queue->_back       = queue->_back->next;
        }

        queue->_length += 1;
        success = true;
    }

    pthread_mutex_unlock( &queue->_mutex );

    queue->queued_items_signal_cb();
    return success;
}

/**
 * [THREAD SAFE] Dequeues a node
 * @param queue LogQueue instance
 * @return Pointer to dequeued node
 */
LogQueueNode_t * ctune_LogQueue_dequeue( struct ctune_LogQueue * queue ) {
    LogQueueNode_t * node = NULL;

    pthread_mutex_lock( &queue->_mutex );

    if( queue != NULL && queue->_front ) {
        node = queue->_front;

        if( queue->_front == queue->_back ) { //i.e.: was last node in queue
            queue->_front = NULL;
            queue->_back  = NULL;

        } else {
            queue->_front = queue->_front->next;
        }

        queue->_length -= 1;
        node->next = NULL;
    }

    pthread_mutex_unlock( &queue->_mutex );

    return node;
}

/**
 * Gets the number of items in the LogQueue
 * @param self LogQueue instance
 * @return number of items
 */
static size_t ctune_LogQueue_size( struct ctune_LogQueue * queue ) {
    size_t l = 0;

    pthread_mutex_lock( &queue->_mutex );
    l = queue->_length;
    pthread_mutex_unlock( &queue->_mutex );

    return l;
}

/**
 * [THREAD SAFE] Gets the empty state of the LogQueue
 * @param queue LogQueue instance
 * @return Empty state
 */
bool ctune_LogQueue_empty( struct ctune_LogQueue * queue ) {
    bool empty_state = false;

    pthread_mutex_lock( &queue->_mutex );
    empty_state = ( queue->_front == NULL );
    pthread_mutex_unlock( &queue->_mutex );

    return empty_state;
}

/**
 * Removes and de-allocates all LogQueueNode(s) in LogQueue
 * @param queue LogQueue instance
 */
void ctune_LogQueue_freeLogQueue( struct ctune_LogQueue * queue ) {
    pthread_mutex_lock( &queue->_mutex );

    if( queue->_front ) {
        LogQueueNode_t * next = queue->_front;

        while( next ) {
            LogQueueNode_t * tmp = next->next;

            free( next->data );
            free( next );

            next = tmp;
        }

        queue->_front  = NULL;
        queue->_back   = NULL;
        queue->_length = 0;
    }

    pthread_mutex_unlock( &queue->_mutex );
}

/**
 * De-allocates a _dequeued_ LogQueueNode
 * @param node Dequeued node to free
 */
void ctune_LogQueue_freeLogQueueNode( LogQueueNode_t * node ) {
    if( node ) {
        if( node->data )
            free( node->data );

        free( node );
    }
}

/**
 * Sets a callback to signal items are in the queue
 * @param queue LogQueue instance
 * @param cb    Callback method to use
 */
void ctune_LogQueue_setSendReadySignalCallback( LogQueue_t * queue, void(* cb)( void ) ) {
    queue->queued_items_signal_cb = cb;
}

/**
 * Initializer
 * @return An initialized LogQueue
 */
static struct ctune_LogQueue ctune_LogQueue_init() {
    return (struct ctune_LogQueue) {
        ._mutex        = PTHREAD_MUTEX_INITIALIZER,
        ._front        = NULL,
        ._back         = NULL,
    };
}

/**
 * Namespace constructor
 */
const struct LogQueueClass ctune_LogQueue={
    .init                       = &ctune_LogQueue_init,
    .create_node                = &ctune_LogQueue_create_node,
    .enqueue                    = &ctune_LogQueue_enqueue,
    .dequeue                    = &ctune_LogQueue_dequeue,
    .size                       = &ctune_LogQueue_size,
    .empty                      = &ctune_LogQueue_empty,
    .freeLogQueue               = &ctune_LogQueue_freeLogQueue,
    .freeLogQueueNode           = &ctune_LogQueue_freeLogQueueNode,
    .setSendReadySignalCallback = &ctune_LogQueue_setSendReadySignalCallback,
};