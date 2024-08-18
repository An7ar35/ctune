#ifndef CTUNE_LOGGER_LOGQUEUE_H
#define CTUNE_LOGGER_LOGQUEUE_H

#include <string.h>
#include <stdbool.h>
#include <pthread.h>

typedef struct ctune_LogQueueNode {
    char                      * data;
    struct ctune_LogQueueNode * next;

} LogQueueNode_t;

struct ctune_LogQueue {
    pthread_mutex_t  _mutex;
    LogQueueNode_t * _front;
    LogQueueNode_t * _back;
    size_t           _length;

    void(* queued_items_signal_cb)( void );
};

typedef struct ctune_LogQueue LogQueue_t;

extern const struct LogQueueClass {
    /**
     * Initializer
     * @return An initialized LogQueue
     */
    struct ctune_LogQueue (* init)( void );

    /**
     * Creates a new LogQueueNode
     * @param str  String
     * @param next Pointer to the next node in the Queue (NULL if none)
     * @return Created LogQueueNode
     */
    LogQueueNode_t * (* create_node)( const char * str, LogQueueNode_t * next );

    /**
     * [THREAD SAFE] Add a node containing string to the queue
     * @param queue LogQueue instance
     * @param str   String to copy to the queued node
     * @return Success
     */
    bool (* enqueue)( LogQueue_t * queue, const char * str );

    /**
     * [THREAD SAFE] Dequeues a node
     * @param queue LogQueue instance
     * @return Pointer to dequeued node
     */
    LogQueueNode_t * (* dequeue)( LogQueue_t * queue );

    /**
     * Gets the number of items in the LogQueue
     * @param self LogQueue instance
     * @return number of items
     */
    size_t (* size)( LogQueue_t * queue );

    /**
     * [THREAD SAFE] Gets the empty state of the LogQueue
     * @param queue LogQueue instance
     * @return Empty state
     */
    bool (* empty)( LogQueue_t * queue );

    /**
     * Removes and de-allocates all LogQueueNode(s) in LogQueue
     * @param queue LogQueue instance
     */
    void (* freeLogQueue)( LogQueue_t * queue );

    /**
     * De-allocates a _dequeued_ LogQueueNode
     * @param node Dequeued node to free
     */
    void (* freeLogQueueNode)( LogQueueNode_t * node );

    /**
     * Sets a callback to signal items are in the queue
     * @param queue LogQueue instance
     * @param cb    Callback method to use
     */
    void (* setSendReadySignalCallback)( LogQueue_t * queue, void(* cb)( void ) );

} ctune_LogQueue;

#endif //CTUNE_LOGGER_LOGQUEUE_H
