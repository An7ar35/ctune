#include "ServerList.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * Creates a new ServerListNode
 * @param hostname  Hostname string
 * @param addr_info `addrinfo` struct to copy over to node
 * @param prev Pointer to the previous ServerListNode (NULL if none)
 * @param next Pointer to the next ServerListNode (NULL if none)
 * @return Created ServerListNode or NULL if creation fails
 */
static ServerListNode * ctune_ServerList_create_node( const char * hostname, const struct addrinfo * addr_info, ServerListNode * prev, ServerListNode * next ) {
    ServerListNode * node;

    if( ( node = (ServerListNode *) malloc( sizeof( ServerListNode ) ) ) == NULL ) {
        fprintf( stderr, "[ctune_ServerList_create_node( \"%s\", %p, %p, %p )] Failed `ServerListNode` malloc.\n",
                 hostname, addr_info, prev, next
        );

        return NULL; //EARLY RETURN
    }

    //Copy hostname
    if( ( node->hostname = malloc( strlen( hostname ) + 1 ) ) == NULL ) {
        free( node );

        fprintf( stderr, "[ctune_ServerList_create_node( \"%s\", %p, %p, %p )] Failed `ServerListNode->hostname` malloc.\n",
                 hostname, addr_info, prev, next
        );

        return NULL; //EARLY RETURN
    }

    strcpy( node->hostname, hostname );

    //Copy addrinfo
    node->ai_flags     = addr_info->ai_flags;
    node->ai_family    = addr_info->ai_family;
    node->ai_socktype  = addr_info->ai_socktype;
    node->ai_protocol  = addr_info->ai_protocol;
    node->ai_addrlen   = addr_info->ai_addrlen;
    node->ai_canonname = NULL;

    //Copy cannonname
    if( addr_info->ai_canonname ) {
        if( ( node->ai_canonname = malloc( strlen( addr_info->ai_canonname ) + 1 ) ) == NULL ) {
            free( node->hostname );
            free( node );

            fprintf( stderr, "[ctune_ServerList_create_node( \"%s\", %p, %p, %p )] Failed `ServerListNode->ai_canonname` malloc.\n",
                     hostname, addr_info, prev, next
            );

            return NULL; //EARLY RETURN
        }

        strcpy( node->ai_canonname, addr_info->ai_canonname );
    }

    //Copy `sockaddr` struct
    if( ( node->ai_addr = malloc( sizeof( struct sockaddr_storage ) ) ) == NULL ) {
        free( node->hostname );
        free( node->ai_canonname );
        free( node );

        fprintf( stderr, "[ctune_ServerList_create_node( \"%s\", %p, %p, %p )] Failed `ServerListNode->ai_addr` malloc.\n",
                 hostname, addr_info, prev, next
        );

        return NULL; //EARLY RETURN
    }

    memcpy( node->ai_addr, addr_info->ai_addr, addr_info->ai_addrlen );

    //set the neighbouring nodes
    node->prev = prev;
    node->next = next;

    return node;
}

/**
 * Removes a ServerListNode in the ServerList
 * @param list ServerListClass instance
 * @param node Pointer to node to remove
 * @return Pointer to next ServerListNode in list or last if the removed node was the last
 */
static ServerListNode * ctune_ServerList_remove( struct ctune_ServerList * list, ServerListNode * node ) {
    ServerListNode * last = NULL;

    if( list && node ) {
        ServerListNode * tmp  = node;

        last = node->next;

        if( list->_front == list->_back ) {
            if( node != list->_front ) {
                fprintf( stderr, "[ctune_ServerList_remove( %p, %p )] Node does not belong to the list - nothing done.\n", list, node );
                return NULL; //EARLY RETURN
            }

            list->_front = NULL;
            list->_back  = NULL;

        } else if( node == list->_front ) {
            list->_front = node->next;

            if( list->_front )
                list->_front->prev = NULL;

        } else if( node == list->_back ) {
            list->_back = node->prev;

            if( list->_back )
                list->_back->next = NULL;

            last = list->_back;

        } else {
            node->prev->next = node->next;
            node->next->prev = node->prev;
        }

        free( tmp->hostname );
        free( tmp->ai_canonname );
        free( tmp->ai_addr );
        free( tmp );

        list->_length -= 1;

    } else { //ERROR
        fprintf( stderr, "[ctune_ServerList_remove( %p, %p )] NULL ptr passed as argument.\n", list, node );
    }

    return last;
}

/**
 * Insert a new ServerListNode in the StrList at given ServerListNode position
 * @param list      ServerListClass instance
 * @param node      Pointer to list node to insert new node at (pushes the old node forward)
 * @param hostname  Hostname string
 * @param addr_info `addrinfo` struct to copy over to node
 * @return Pointer to the newly inserted ServerListNode
 */
static ServerListNode * ctune_ServerList_insert( struct ctune_ServerList * list, ServerListNode * node, const char * hostname, const struct addrinfo * addr_info ) {
    if( list && node && addr_info ) {
        ServerListNode * prev = node->prev;
        ServerListNode * next = node;
        ServerListNode * tmp  = ctune_ServerList_create_node( hostname, &(*addr_info), prev, next );

        if( tmp == NULL ) { //ERROR CONTROL
            fprintf( stderr, "[ctune_ServerList_insert( %p, %p, \"%s\", %p )] ServerListNode creation failed.\n",
                     list, node, hostname, addr_info
            );

            return NULL; //EARLY RETURN
        }

        if( node == list->_front ) {
            list->_front = tmp;
            next->prev   = list->_front;

        } else {
            prev->next = tmp;
            next->prev = prev->next;
        }

        list->_length += 1;
        return prev->next;
    }

    fprintf( stderr, "[ctune_ServerList_insert( %p, %p, \"%s\", %p )] NULL ptr passed as argument.\n",
             list, node, hostname, addr_info
    );

    return NULL;
}

/**
 * Inserts a new node at the front of the list
 * @param list      ServerListClass instance
 * @param hostname  Hostname string
 * @param addr_info `addrinfo` struct to copy over to node
 * @return Pointer to new node or NULL if failed
 */
static ServerListNode * ctune_ServerList_insert_front( struct ctune_ServerList * list, const char * hostname, const struct addrinfo * addr_info ) {
    if( list == NULL ) {
        fprintf( stderr, "[ctune_ServerList_insert_front( %p, %s, %p )] Pointer is NULL.\n", list, hostname, addr_info );
        return NULL;
    }

    ServerListNode * tmp = NULL;

    if( list->_front == NULL ) { //i.e.: empty list
        if( ( tmp = ctune_ServerList_create_node( hostname, addr_info, NULL, NULL ) ) == NULL ) { //ERROR CONTROL
            fprintf( stderr, "[ctune_ServerList_insert_front( %p, \"%s\", %p )] ServerListNode malloc failed.\n",
                     list, hostname, addr_info
            );
            return NULL; //EARLY RETURN
        }

        list->_front = tmp;
        list->_back  = list->_front;

    } else {
        if( ( tmp = ctune_ServerList_create_node( hostname, addr_info, NULL, list->_front ) ) == NULL ) { //ERROR CONTROL
            fprintf( stderr, "[ctune_ServerList_insert_front( %p, \"%s\", %p )] ServerListNode malloc failed.\n",
                     list, hostname, addr_info
            );
            return NULL; //EARLY RETURN
        }

        list->_front->prev = tmp;
        list->_front       = list->_front->prev;
    }

    list->_length += 1;
    return list->_front;
}

/**
 *  Inserts a new node at the end of the list
 * @param list      ServerListClass instance
 * @param hostname  Hostname string
 * @param addr_info `addrinfo` struct to copy over to node
 * @return Pointer to new node
 */
static ServerListNode * ctune_ServerList_insert_back( struct ctune_ServerList * list, const char * hostname, const struct addrinfo * addr_info ) {
    if( list == NULL ) {
        fprintf( stderr, "[ctune_ServerList_insert_back( %p, %s, %p )] Pointer is NULL.\n", list, hostname, addr_info );
        return NULL;
    }

    ServerListNode * tmp = NULL;

    if( list->_back == NULL ) { //i.e.: empty list
        if( ( tmp = ctune_ServerList_create_node( hostname, addr_info, NULL, NULL ) ) == NULL ) { //ERROR CONTROL
            fprintf( stderr, "[ctune_ServerList_insert_back( %p, \"%s\", %p )] ServerListNode malloc failed.\n",
                     list, hostname, addr_info
            );
            return NULL; //EARLY RETURN
        }

        list->_back  = tmp;
        list->_front = list->_back;

    } else {
        if( ( tmp = ctune_ServerList_create_node( hostname, addr_info, list->_back, NULL ) ) == NULL ) { //ERROR CONTROL
            fprintf( stderr, "[ctune_ServerList_insert_back( %p, \"%s\", %p )] ServerListNode malloc failed.\n",
                     list, hostname, addr_info
            );
            return NULL; //EARLY RETURN
        }

        list->_back->next = tmp;
        list->_back       = list->_back->next;
    }

    list->_length += 1;
    return list->_back;
}

/**
 * Emplace an orphan node into the front of a ServerList
 * @param list ServerListClass instance
 * @param node Node to emplace
 * @return Pointer to node
 */
static ServerListNode * ctune_ServerList_emplace_front( struct ctune_ServerList * list, ServerListNode * node ) {
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
 * Emplace an orphan node into the back of a ServerList
 * @param list ServerListClass instance
 * @param node Node to emplace
 * @return Pointer to node
 */
static ServerListNode * ctune_ServerList_emplace_back( struct ctune_ServerList * list, ServerListNode * node ) {
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
 * Remove a node from the list
 * @param list ServerListClass instance
 * @param node Node to extract from the list
 * @return Extracted node
 */
static ServerListNode * ctune_ServerList_extract_node( struct ctune_ServerList * list, ServerListNode * node ) {
    if( list && node ) {

        if( node == list->_front ) {
            list->_front = node->next;

            if( list->_front ) {
                list->_front->prev = NULL;
            } else { //no more nodes
                list->_back = NULL;
            }

        } else if( node == list->_back ) {
            list->_back = node->prev;

            if( list->_back ) {
                list->_back->next = NULL;
            } else { //no more nodes
                list->_front = NULL;
            }

        } else {
            ServerListNode * prev = node->prev;
            ServerListNode * next = node->next;

            if( prev )
                prev->next = next;

            if( next )
                next->prev = prev;
        }

        list->_length -= 1;

        node->next = NULL;
        node->prev = NULL;
    }

    return node;
}

/**
 * Gets the node at a specified index
 * @param list ServerListClass instance
 * @param n    Index of node
 * @return Pointer of node located at given index (NULL if out-of-range)
 */
static ServerListNode * ctune_ServerList_at( struct ctune_ServerList * list, size_t n ) {
    if( list == NULL || n < 0 || n >= list->_length ) {
        fprintf( stderr, "[ServerList.at( %p, %lu )] Out of range.", list, n );
        return NULL;
    }

    int              i    = 0;
    ServerListNode * curr = list->_front;

    while( i < n ) {
        if( curr->next == NULL ) {
            fprintf( stderr, "[ServerList.at( %p, %lu )] Pointer to next node is NULL (i=%i).", list, n, i );
            return NULL;

        } else {
            curr = curr->next;
            ++i;
        }
    }

    return curr;
}

/**
 * Gets the number of items in the list
 * @param list ServerListClass instance
 * @return Number of items
 */
static size_t ctune_ServerList_size( const struct ctune_ServerList * list ) {
    if( list == NULL ) {
        fprintf( stderr, "[ctune_ServerList_size( %p )] Pointer is NULL.\n", list );
        return 0;
    }

    return list->_length;
}

/**
 * Removes and de-allocates all node(s) in data-structure
 * @param list ServerList instance
 */
static void ctune_ServerList_freeServerList( struct ctune_ServerList * list ) {
    if( list == NULL ) {
        fprintf( stderr, "[ctune_ServerList_freeServerList( %p )] Pointer is NULL.\n", list );
        return;
    }

    if( list->_front ) {
        ServerListNode * next = list->_front;

        while( next ) {
            ServerListNode * tmp = next->next;

            free( next->hostname );
            free( next->ai_canonname );
            free( next->ai_addr );
            free( next );

            next = tmp;
        }

        list->_front  = NULL;
        list->_back   = NULL;
        list->_length = 0;
    }
}

/**
 * De-allocates a ServerList
 * @param list Pointer to a ServerList pointer
 */
static void ctune_ServerList_dealloc( struct ctune_ServerList ** list ) {
    if( list == NULL ) {
        fprintf( stderr, "[ctune_ServerList_dealloc( %p )] Pointer is NULL.\n", list );
        return;
    }

    ctune_ServerList_freeServerList( (*list) );
    free( *list );
}

/**
 * Initializer
 * @return An initialized ServerList instance
 */
static struct ctune_ServerList ctune_ServerList_init() {
    return (struct ctune_ServerList){
        ._front         = NULL,
        ._back          = NULL,
        ._length        = 0,
    };
}

/**
 * Allocator
 * @param p Pointer to ServerList pointer
 */
static void ctune_ServerList_alloc( struct ctune_ServerList ** p ) {
    (*p) = malloc( sizeof( struct ctune_ServerList ) );
    (*p)->_back   = NULL;
    (*p)->_front  = NULL;
    (*p)->_length = 0;
}

/**
 * Constructor
 */
const struct ctune_ServerList_Namespace ctune_ServerList = {
    .init           = &ctune_ServerList_init,
    .alloc          = &ctune_ServerList_alloc,
    .create_node    = &ctune_ServerList_create_node,
    .insert         = &ctune_ServerList_insert,
    .remove         = &ctune_ServerList_remove,
    .insert_front   = &ctune_ServerList_insert_front,
    .insert_back    = &ctune_ServerList_insert_back,
    .emplace_front  = &ctune_ServerList_emplace_front,
    .emplace_back   = &ctune_ServerList_emplace_back,
    .extract_node   = &ctune_ServerList_extract_node,
    .at             = &ctune_ServerList_at,
    .size           = &ctune_ServerList_size,
    .freeServerList = &ctune_ServerList_freeServerList,
    .dealloc        = &ctune_ServerList_dealloc
};