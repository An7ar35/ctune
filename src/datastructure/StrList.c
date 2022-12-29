#include "StrList.h"

#include <stdio.h>
#include <stdlib.h>

/**
 * Initializer
 * @return An initialized StrList
 */
static struct StrList ctune_StrList_init() {
    return (struct StrList){
        ._front        = NULL,
        ._back         = NULL,
        ._length       = 0
    };
}

/**
 * Creates a new ListNode
 * @param str  String
 * @param prev Pointer to the previous ListNode (NULL if none)
 * @param next Pointer to the next ListNode (NULL if none)
 * @return Created ListNode
 */
static ListNode * ctune_StrList_create_node( const char * str, ListNode * prev, ListNode * next ) {
    ListNode * node;

    if( ( node = (ListNode *) malloc( sizeof( ListNode ) ) ) == NULL ) {
        fprintf( stderr, "[ctune_StrList_create_node( \"%s\", %p, %p )] Failed `ListNode` malloc.\n", str, prev, next );
        return NULL;
    }

    if( ( node->data = malloc( strlen( str ) + 1 ) ) == NULL ) {
        free( node );
        fprintf( stderr, "[ctune_StrList_create_node( \"%s\", %p, %p )] Failed `ListNode->data` malloc.\n", str, prev, next );
        return NULL;
    }
    
    strcpy( node->data, str );

    node->prev = prev;
    node->next = next;

    return node;
}

/**
 * Removes a ListNode in the StrList
 * @param list StrList instance
 * @param node Pointer to node to remove
 * @return Pointer to next ListNode in list or last if the removed node was the last (NULL if list is empty)
 */
static ListNode * ctune_StrList_remove_node( struct StrList * list, ListNode * node ) {
    if( list && node ) {
        ListNode * tmp  = node;
        ListNode * next = NULL;

        if( list->_front == list->_back ) {
            if( node != list->_front ) {
                fprintf( stderr, "[ctune_StrList_remove_node( %p, %p )] Node does not belong to the list - nothing done.\n", list, node );
                return NULL; //EARLY RETURN
            }

            list->_front = NULL;
            list->_back  = NULL;

        } else if( node == list->_front ) {
            next               = node->next;
            list->_front       = node->next;
            list->_front->prev = NULL;

        } else if( node == list->_back ) {
            list->_back       = node->prev;
            list->_back->next = NULL;

        } else {
            next             = node->next;
            node->prev->next = node->next;
            node->next->prev = node->prev;
        }

        free( tmp->data );
        free( tmp );

        list->_length -= 1;
        return next;
    }

    //ERROR
    fprintf( stderr, "[ctune_StrList_remove_node( %p, %p )] NULL ptr passed as argument.\n", list, node );
    return NULL;
}

/**
 * Insert a new ListNode in the StrList at given ListNode position
 * @param list StrList instance
 * @param node Pointer to list node to insert new node at (pushes the old node forward)
 * @param str  String for new node
 * @return Pointer to the newly inserted ListNode (or NULL if failed)
 */
static ListNode * ctune_StrList_insert( struct StrList * list, ListNode * node, const char * str ) {
    if( list && node ) {
        ListNode * prev = node->prev;
        ListNode * next = node;
        ListNode * tmp  = StrList.create_node( str, prev, next );

        if( tmp == NULL ) { //ERROR CONTROL
            fprintf( stderr, "[ctune_StrList_insert( %p, %p, \"%s\" )] ListNode creation failed.\n", list, node, str );
            return NULL; //EARLY RETURN
        }

        if( node == list->_front ) {
            list->_front = tmp;
            next->prev   = list->_front;

        } else {
            prev->next   = tmp;
            next->prev   = prev->next;
        }

        list->_length += 1;
        return prev->next;
    }

    //ERROR
    fprintf( stderr, "[ctune_StrList_insert( %p, %p, \"%s\" )] NULL ptr passed as argument.\n", list, node, str );
    return NULL;
}

/**
 * Inserts a new node at the front of the list
 * @param list StrList instance
 * @param str string for new node
 * @return Pointer to new node or NULL if failed
 */
static ListNode * ctune_StrList_insert_front( struct StrList * list, const char * str ) {
    ListNode * tmp = NULL;

    if( list->_front == NULL ) { //i.e.: empty list
        if( ( tmp = StrList.create_node( str, NULL, NULL ) ) == NULL ) { //ERROR CONTROL
            fprintf( stderr, "[ctune_StrList_insert_front( %p, \"%s\" )] ListNode malloc failed.\n", list, str );
            return NULL; //EARLY RETURN
        }

        list->_front = tmp;
        list->_back  = list->_front;

    } else {
        if( ( tmp = StrList.create_node( str, NULL, list->_front ) ) == NULL ) { //ERROR CONTROL
            fprintf( stderr, "[ctune_StrList_insert_front( %p, \"%s\" )] ListNode malloc failed.\n", list, str );
            return NULL; //EARLY RETURN
        }

        list->_front->prev = tmp;
        list->_front       = list->_front->prev;
    }

    list->_length += 1;
    return list->_front;
}

/**
 * Inserts a new node at the end of the list
 * @param list StrList instance
 * @param str string for new node
 * @return Pointer to new node
 */
static ListNode * ctune_StrList_insert_back( struct StrList * list, const char * str ) {
    ListNode * tmp = NULL;

    if( list->_back == NULL ) { //i.e.: empty list
        if( ( tmp = StrList.create_node( str, NULL, NULL ) ) == NULL ) { //ERROR CONTROL
            fprintf( stderr, "[ctune_StrList_insert_back( %p, \"%s\" )] ListNode malloc failed.\n", list, str );
            return NULL; //EARLY RETURN
        }

        list->_back  = tmp;
        list->_front = list->_back;

    } else {
        if( ( tmp = StrList.create_node( str, list->_back, NULL ) ) == NULL ) { //ERROR CONTROL
            fprintf( stderr, "[ctune_StrList_insert_back( %p, \"%s\" )] ListNode malloc failed.\n", list, str );
            return NULL; //EARLY RETURN
        }

        list->_back->next = tmp;
        list->_back       = list->_back->next;
    }

    list->_length += 1;
    return list->_back;
}

/**
 * Emplace an orphan node into the front of a StrList
 * @param list StrList instance
 * @param node Node to emplace
 * @return Pointer to node
 */
static ListNode * ctune_StrList_emplace_front( struct StrList * list, ListNode * node ) {
    if( list && node ) {
        if( node == list->_front )
            return node;

        node->prev = NULL;
        node->next = NULL;

        if( list->_front == NULL ) { //i.e.: empty list
            list->_front = node;
            list->_back  = list->_front;

        } else {
            list->_front->prev = node;
            node->next         = list->_front;
            list->_front       = node;
        }

        list->_length += 1;
        return list->_front;
    }

    return node;
}

/**
 * Emplace an orphan node into the back of a StrList
 * @param list StrList instance
 * @param node Node to emplace
 * @return Pointer to node
 */
static ListNode * ctune_StrList_emplace_back( struct StrList * list, ListNode * node ) {
    if( list && node ) {
        if( node == list->_back )
            return node;

        node->prev = NULL;
        node->next = NULL;

        if( list->_front == NULL ) { //i.e.: empty list
            list->_front = node;
            list->_back  = list->_front;

        } else {
            list->_back->next = node;
            node->prev        = list->_back;
            list->_back       = node;
        }

        list->_length += 1;
        return list->_back;
    }

    return node;
}

/**
 * Remove a ListNode from a StrList
 * @param list StrList instance
 * @param node ListNode to extract from the StrList
 * @return Extracted ListNode
 */
static ListNode * ctune_StrList_extract_node( struct StrList * list, ListNode * node ) {
    if( list && node ) {

        if( node == list->_front ) {
            list->_front = node->next;
            list->_front->prev = NULL;

        } else if( node == list->_back ) {
            list->_back = node->prev;
            list->_back->next = NULL;

        } else {
            ListNode * prev = node->prev;
            ListNode * next = node->next;
            prev->next = next;
            next->prev = prev;
        }

        list->_length -= 1;

        node->next = NULL;
        node->prev = NULL;
    }

    return node;
}

/**
 * Gets the ListNode at a specified index
 * @param list StrList instance
 * @param n    Index of node
 * @return Pointer of ListNode located at given index (NULL if out-of-range)
 */
static const ListNode * ctune_StrList_at( const struct StrList * list, size_t n ) {
    if( list == NULL || n >= list->_length ) {
        fprintf( stderr, "[StrList.at( %p, %lu )] Out of range.\n", list, n );
        return NULL;
    }

    size_t     i    = 0;
    ListNode * curr = list->_front;

    while( i < n ) {
        if( curr->next == NULL ) {
            fprintf( stderr, "[StrList.at( %p, %lu )] Pointer to next node is NULL (i=%zu).\n", list, n, i );
            return NULL;

        } else {
            curr = curr->next;
            ++i;
        }
    }

    return curr;
}

/**
 * Gets the number of items in StrList
 * @param list StrList instance
 * @return number of items
 */
static size_t ctune_StrList_size( const struct StrList * list ) {
    return list->_length;
}

/**
 * Gets the empty state of the StrList
 * @param list StrList instance
 * @return Empty state
 */
static bool ctune_StrList_empty( const struct StrList * list ) {
    return ( list->_length == 0 );
}

/**
 * Removes and de-allocates all ListNode(s) in StrList
 * @param list StrList instance
 */
static void ctune_StrList_clear( struct StrList * list ) {
    if( list->_front ) {
        ListNode * next = list->_front;

        while( next ) {
            ListNode * tmp = next->next;

            free( next->data );
            free( next );

            next = tmp;
        }

        list->_front  = NULL;
        list->_back   = NULL;
        list->_length = 0;
    }
}

/**
 * Stringify the StrList into a String_t
 * @param list StrList object
 * @param str  String_t object
 * @param sep  List item separation character (e.g. ',')
 * @return Number of items parsed (can be compared the StrList.size(..) to check for any failures)
 */
size_t ctune_StrList_stringify( const struct StrList * list, String_t * str, const char sep ) {
    if( str == NULL )
        return 0; //EARLY RETURN

    size_t       count    = 0;
    ListNode   * next     = list->_front;
    const char   delim[2] = { [0] = sep, [1] = '\0' };

    while( next ) {
        bool ok = false;

        if( next->data ) {
            if( ( ok = String.append_back( str, next->data ) ) )
                ++count;
        }

        next = next->next;

        if( next && ok ) {
            String.append_back( str, &delim[0] );
        }
    }

    return count;
}

/**
 * Namespace constructor
 */
const struct StrListClass StrList = {
    .init          = &ctune_StrList_init,
    .create_node   = &ctune_StrList_create_node,
    .insert        = &ctune_StrList_insert,
    .remove        = &ctune_StrList_remove_node,
    .insert_front  = &ctune_StrList_insert_front,
    .insert_back   = &ctune_StrList_insert_back,
    .emplace_front = &ctune_StrList_emplace_front,
    .emplace_back  = &ctune_StrList_emplace_back,
    .extract_node  = &ctune_StrList_extract_node,
    .at            = &ctune_StrList_at,
    .size          = &ctune_StrList_size,
    .empty         = &ctune_StrList_empty,
    .free_strlist  = &ctune_StrList_clear,
    .stringify     = &ctune_StrList_stringify,
};