#include "Deque.h"

#include <stdlib.h>

#include "logger/src/Logger.h"

/**
 * [PRIVATE] Helper method to create a Deque node
 * @param item Pointer to assign to the node's data pointer
 * @param prev Pointer to previous node
 * @param next Pointer to next node
 * @return Created Deque node
 */
static struct DequeNode * Deque_createNode( void * item, struct DequeNode * prev, struct DequeNode * next ) {
    struct DequeNode * node = NULL;

    if( ( node = (struct DequeNode *) malloc( sizeof( struct DequeNode ) ) ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[Deque_createNode( %p, %p, %p )] Failed `DequeNode` malloc.", item, prev, next );
        return NULL;
    }

    node->data = item;
    node->prev = prev;
    node->next = next;

    return node;
}

/**
 * [PRIVATE] Helper method to remove a Deque node
 * @param deque Deque_t object
 * @param node  Node to remove
 * @return Pointer to removed node's data
 */
static void * Deque_removeNode( Deque_t * deque, struct DequeNode * node ) {
    if( node == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[Deque_removeNode( %p, %p )] NULL node passed as argument.", deque, node );
        return NULL; //EARLY RETURN
    }

    struct DequeNode * tmp  = node;
    void             * data = node->data;

    if( deque->front == deque->back ) {
        if( node != deque->front ) {
            CTUNE_LOG( CTUNE_LOG_WARNING, "[Deque_removeNode( %p, %p )] Node does not belong to the list - nothing done.", deque, node );
            return NULL; //EARLY RETURN
        }

        deque->front = NULL;
        deque->back  = NULL;

    } else if( node == deque->front ) {
        deque->front       = node->next;
        deque->front->prev = NULL;

    } else if( node == deque->back ) {
        deque->back       = node->prev;
        deque->back->next = NULL;

    } else {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    deque->length -= 1;

    free( tmp );
    return data;
}

/**
 * Initialise a DEQUE
 * @return Initialised Deque object
 */
static struct Deque Deque_init() {
    return (struct Deque) {
        .front  = NULL,
        .back   = NULL,
        .length = 0,
    };
}

/**
 * Insert item at front of Deque
 * @param deque Deque_t object
 * @param item  Item pointer
 */
static void Deque_pushFront( Deque_t * deque, void * item ) {
    if( deque == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[Deque_pushFront( %p, %p )] Pointer to Deque_t is NULL.", deque, item );
        return; //EARLY RETURN
    }

    struct DequeNode * tmp = NULL;

    if( deque->front == NULL ) { //i.e.: empty list
        if( ( tmp = Deque_createNode( item, NULL, NULL ) ) == NULL ) { //ERROR CONTROL
            CTUNE_LOG( CTUNE_LOG_ERROR, "[Deque_pushFront( %p, %p )] DequeNode malloc failed.", deque, item );
            return; //EARLY RETURN
        }

        deque->front = tmp;
        deque->back  = deque->front;

    } else {
        if( ( tmp = Deque_createNode( item, NULL, deque->front ) ) == NULL ) { //ERROR CONTROL
            CTUNE_LOG( CTUNE_LOG_ERROR, "[Deque_pushFront( %p, %p )] DequeNode malloc failed.", deque, item );
            return; //EARLY RETURN
        }

        deque->front->prev = tmp;
        deque->front       = deque->front->prev;
    }

    deque->length += 1;
}

/**
 * Insert item at back of Deque
 * @param deque Deque_t object
 * @param item  Item pointer
 */
static void Deque_pushBack( Deque_t * deque, void * item ) {
    if( deque == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[Deque_pushBack( %p, %p )] Pointer to Deque_t is NULL.", deque, item );
        return; //EARLY RETURN
    }

    struct DequeNode * tmp = NULL;

    if( deque->back == NULL ) { //i.e.: empty list
        if( ( tmp = Deque_createNode( item, NULL, NULL ) ) == NULL ) { //ERROR CONTROL
            CTUNE_LOG( CTUNE_LOG_ERROR, "[Deque_pushBack( %p, %p )] DequeNode malloc failed.", deque, item );
            return; //EARLY RETURN
        }

        deque->back  = tmp;
        deque->front = deque->back;

    } else {
        if( ( tmp = Deque_createNode( item, deque->back, NULL ) ) == NULL ) { //ERROR CONTROL
            CTUNE_LOG( CTUNE_LOG_ERROR, "[Deque_pushBack( %p, %p )] DequeNode malloc failed.", deque, item );
            return; //EARLY RETURN
        }

        deque->back->next = tmp;
        deque->back       = deque->back->next;
    }

    deque->length += 1;
}

/**
 * Removes the item in front of the Deque
 * @param deque Deque_t object
 * @return Data pointer in removed item
 */
static void * Deque_popFront( Deque_t * deque ) {
    if( deque == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[Deque_popFront( %p )] Pointer to Deque_t is NULL.", deque );
        return NULL; //EARLY RETURN
    }

    return Deque_removeNode( deque, deque->front );
}

/**
 * Removes the item at the back of the Deque
 * @param deque Deque_t object
 * @return Data pointer in removed item
 */
static void * Deque_popBack( Deque_t * deque ) {
    if( deque == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[Deque_popBack( %p )] Pointer to Deque_t is NULL.", deque );
        return NULL; //EARLY RETURN
    }

    return Deque_removeNode( deque, deque->back );
}

/**
 * Gets the item at the front of the queue
 * @param deque Deque_t object
 * @return Pointer at the front of queue
 */
static void * Deque_front( Deque_t * deque ) {
    if( deque == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[Deque_front( %p )] Pointer to Deque_t is NULL.", deque );
        return NULL; //EARLY RETURN
    }

    return deque->front->data;
}

/**
 * Gets the item at the back of the queue
 * @param deque Deque_t object
 * @return Pointer at the back of queue
 */
static void * Deque_back( Deque_t * deque ) {
    if( deque == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[Deque_back( %p )] Pointer to Deque_t is NULL.", deque );
        return NULL; //EARLY RETURN
    }

    return deque->back->data;
}

/**
 * Finds the first data packet matching the provided argument from front to back
 * @param deque Deque_t object
 * @param arg   Pointer to the argument to use when checking the equivalent state with the provided function
 * @param equal Comparator function taking in the argument to find and the data pointer stored on the nodes
 * @return Anonymous data pointer matching or NULL if none are found
 */
static void * Deque_find( Deque_t * deque, const void * arg, bool (* equal)( const void *, const void * ) ) {
    if( deque == NULL || arg == NULL || equal == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[Deque_find( %p, %p, %p )] NULL argument(s).", deque, arg, equal );
        return NULL; //EARLY RETURN
    }

    if( deque->front == NULL ) //i.e.: empty
        return NULL; //EARLY RETURN

    struct DequeNode * node = deque->front;

    while( node ) {
        if( node->data != NULL ) {
            if( equal( arg, node->data ) )
                return node->data; //EARLY RETURN
        }

        node = node->next;
    }

    return NULL;
}

/**
 * Gets the number of items in the Deque
 * @param deque Deque_t object
 * @return Size
 */
static size_t Deque_size( Deque_t * deque ) {
    if( deque == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[Deque_size( %p )] Pointer to Deque_t is NULL.", deque );
        return 0; //EARLY RETURN
    }

    return deque->length;
}

/**
 * Checks the empty state of the Deque
 * @param deque Deque_t object
 * @return Empty state
 */
static bool Deque_empty( Deque_t * deque ) {
    if( deque == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[Deque_empty( %p )] Pointer to Deque_t is NULL.", deque );
        return true; //EARLY RETURN
    }

    return ( deque->length == 0 );
}

/**
 * Clears queue
 * @param deque   Deque_t object
 * @param free_fn De-allocation method for data object (optional)
 */
void Deque_free( Deque_t * deque, void(* free_fn)( void * ) ) {
    if( deque != NULL ) {
        struct DequeNode * node = deque->back;

        while( node != NULL ) {

            if( free_fn )
                free_fn( node->data );

            Deque_removeNode( deque, node );

            node = deque->back;
        }
    }
}


/**
 * Namespace initializer
 */
const struct ctune_Deque_Namespace Deque = {
    .init      = &Deque_init,
    .pushFront = &Deque_pushFront,
    .pushBack  = &Deque_pushBack,
    .popFront  = &Deque_popFront,
    .popBack   = &Deque_popBack,
    .front     = &Deque_front,
    .back      = &Deque_back,
    .find      = &Deque_find,
    .size      = &Deque_size,
    .empty     = &Deque_empty,
    .free      = &Deque_free,
};