#ifndef CTUNE_DATASTRUCTURE_DEQUE_H
#define CTUNE_DATASTRUCTURE_DEQUE_H

#include <stddef.h>
#include <stdbool.h>

/*           back          front
 *            |              |
 *   next <--[ ]--[ ]--[ ]--[ ]--> prev
 *
 */

/**
 * Deque Node
 * @param data Anonymous item pointer
 * @param prev Previous node
 * @param next Next node
 */
typedef struct DequeNode {
    void             * data;
    struct DequeNode * prev;
    struct DequeNode * next;
} ListNode_t;

/**
 * Deque
 * @param front First node
 * @param back  Last node
 * @param size  Number of nodes
 */
typedef struct Deque {
    ListNode_t * front;
    ListNode_t * back;
    size_t       length;
} Deque_t;


extern const struct ctune_Deque_Namespace {
    /**
     * Initialise a DEQUE
     * @return Initialised Deque object
     */
    struct Deque (* init)( void );

    /**
     * Insert item at front of Deque
     * @param deque Deque_t object
     * @param item  Item pointer
     */
    void (* pushFront)( Deque_t * deque, void * item );

    /**
     * Insert item at back of Deque
     * @param deque Deque_t object
     * @param item  Item pointer
     */
    void (* pushBack)( Deque_t * deque, void * item );

    /**
     * Removes the item in front of the Deque
     * @param deque Deque_t object
     * @return Data pointer in removed item
     */
    void * (* popFront)( Deque_t * deque );

    /**
     * Removes the item at the back of the Deque
     * @param deque Deque_t object
     * @return Data pointer in removed item
     */
    void * (* popBack)( Deque_t * deque );

    /**
     * Gets the item at the front of the queue
     * @param deque Deque_t object
     * @return Pointer at the front of queue
     */
    void * (* front)( Deque_t * deque );

    /**
     * Gets the item at the back of the queue
     * @param deque Deque_t object
     * @return Pointer at the back of queue
     */
    void * (* back)( Deque_t * deque );

    /**
     * Finds the first data packet matching the provided argument
     * @param deque Deque_t object
     * @param arg   Pointer to the argument to use when checking the equivalent state with the provided function
     * @param equal Comparator function taking in the argument to find and the data pointer stored on the nodes
     * @return Anonymous data pointer matching or NULL if none are found
     */
    void * (* find)( Deque_t * deque, const void * arg, bool (*equal)( const void * arg, const void * data ) );

    /**
     * Gets the number of items in the Deque
     * @param deque Deque_t object
     * @return Size
     */
    size_t (* size)( Deque_t * deque );

    /**
     * Checks the empty state of the Deque
     * @param deque Deque_t object
     * @return Empty state
     */
    bool (* empty)( Deque_t * deque );

    /**
     * Clears queue
     * @param deque   Deque_t object
     * @param free_fn De-allocation method for data object (optional)
     */
    void (* free)( Deque_t * deque, void(* free_fn)( void * ) );

} Deque;

#endif //CTUNE_DATASTRUCTURE_DEQUE_H
