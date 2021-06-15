#ifndef CTUNE_DATASTRUCTURE_SERVERLIST_H
#define CTUNE_DATASTRUCTURE_SERVERLIST_H

#include "netdb.h"

/**
 * Server object / list node
 * @param hostname     Hostname
 * @param ai_flags     Additional option flags
 * @param ai_family    Address family (IPv4/IPv6/..)
 * @param ai_socktype  Socket type (stream/datagram/..)
 * @param ai_protocol  Protocol
 * @param ai_addrlen   Address length
 * @param ai_addr      Address
 * @param ai_canonname Canonical name
 * @param prev         Pointer to previous server in list
 * @param next         Pointer to next server in list
 */
typedef struct ServerListNodeClass {
    char                       * hostname;
    int                          ai_flags;
    int                          ai_family;
    int                          ai_socktype;
    int                          ai_protocol;
    socklen_t                    ai_addrlen;
    struct sockaddr_storage    * ai_addr;
    char                       * ai_canonname;
    struct ServerListNodeClass * prev;
    struct ServerListNodeClass * next;

} ServerListNode;

/**
 * ServerList object
 */
typedef struct ctune_ServerList {
    ServerListNode * _front;
    ServerListNode * _back;
    size_t           _length;
} ctune_ServerList_t;

extern const struct ctune_ServerList_Namespace {
    /**
     * Initializer
     * @return An initialized ServerList instance
     */
    struct ctune_ServerList (* init)( void );

    /**
     * Allocator
     * @param p Pointer to ServerList pointer
     */
    void (* alloc)( struct ctune_ServerList ** p );

    /**
     * Creates a new ServerListNode
     * @param hostname  Hostname string
     * @param addr_info `addrinfo` struct to copy over to node
     * @param prev Pointer to the previous ServerListNode (NULL if none)
     * @param next Pointer to the next ServerListNode (NULL if none)
     * @return Created ServerListNode or NULL if creation fails
     */
    ServerListNode * (* create_node)( const char * hostname, const struct addrinfo * addr_info, ServerListNode * prev, ServerListNode * next );

    /**
     * Removes a ServerListNode in the ServerList
     * @param list ServerListClass instance
     * @param node Pointer to node to remove
     * @return Pointer to next ServerListNode in list or last if the removed node was the last
     */
    ServerListNode * (* remove)( struct ctune_ServerList * list, ServerListNode * node );

    /**
     * Insert a new ServerListNode in the StrList at given ServerListNode position
     * @param list      ServerListClass instance
     * @param node      Pointer to list node to insert new node at (pushes the old node forward)
     * @param hostname  Hostname string
     * @param addr_info `addrinfo` struct to copy over to node
     * @return Pointer to the newly inserted ServerListNode
     */
    ServerListNode * (* insert)( struct ctune_ServerList * list, ServerListNode * node, const char * hostname, const struct addrinfo * addr_info );

    /**
     * Inserts a new node at the front of the list
     * @param list      ServerListClass instance
     * @param hostname  Hostname string
     * @param addr_info `addrinfo` struct to copy over to node
     * @return Pointer to new node or NULL if failed
     */
    ServerListNode * (* insert_front)( struct ctune_ServerList * list, const char * hostname, const struct addrinfo * addr_info );

    /**
     *  Inserts a new node at the end of the list
     * @param list      ServerListClass instance
     * @param hostname  Hostname string
     * @param addr_info `addrinfo` struct to copy over to node
     * @return Pointer to new node
     */
    ServerListNode * (* insert_back)( struct ctune_ServerList * list, const char * hostname, const struct addrinfo * addr_info );

    /**
     * Emplace an orphan node into the front of a ServerList
     * @param list ServerListClass instance
     * @param node Node to emplace
     * @return Pointer to node
     */
    ServerListNode * (* emplace_front)( struct ctune_ServerList * list, ServerListNode * node );

    /**
     * Emplace an orphan node into the back of a ServerList
     * @param list ServerListClass instance
     * @param node Node to emplace
     * @return Pointer to node
     */
    ServerListNode * (* emplace_back)( struct ctune_ServerList * list, ServerListNode * node );

    /**
     * Remove a node from the list
     * @param list ServerListClass instance
     * @param node Node to extract from the list
     * @return Extracted node
     */
    ServerListNode * (* extract_node)( struct ctune_ServerList * list, ServerListNode * node );

    /**
     * Gets the node at a specified index
     * @param list ServerListClass instance
     * @param n    Index of node
     * @return Pointer of node located at given index (NULL if out-of-range)
     */
    ServerListNode * (* at)( struct ctune_ServerList * list, size_t n );

    /**
     * Gets the number of items in the list
     * @param list ServerListClass instance
     * @return Number of items
     */
    size_t (* size)( const struct ctune_ServerList * list );

    /**
     * Removes and de-allocates all node(s) in data-structure
     * @param list ServerList instance
     */
    void (* freeServerList)( struct ctune_ServerList * list );

    /**
     * De-allocates a ServerList
     * @param list Pointer to a ServerList pointer
     */
    void (* dealloc)( struct ctune_ServerList ** list );

} ctune_ServerList;

#endif //CTUNE_DATASTRUCTURE_SERVERLIST_H
