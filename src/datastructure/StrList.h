#ifndef CTUNE_DATASTRUCTURE_STRLIST_H
#define CTUNE_DATASTRUCTURE_STRLIST_H

#include <string.h>
#include <stdbool.h>

#include "String.h"

typedef struct StrListNode {
    char               * data;
    struct StrListNode * prev;
    struct StrListNode * next;

} ListNode;

typedef struct StrList {
    ListNode * _front;
    ListNode * _back;
    size_t     _length;
} StrList_t;

extern const struct StrListClass {
    /**
     * Initializer
     * @return An initialized StrList
     */
    struct StrList (* init)( void );

    /**
     * Creates a new ListNode
     * @param str  String
     * @param prev Pointer to the previous ListNode (NULL if none)
     * @param next Pointer to the next ListNode (NULL if none)
     * @return Created ListNode
     */
    ListNode * (* create_node)( const char * str, ListNode * prev, ListNode * next );

    /**
     * Removes a ListNode in the StrList
     * @param list StrList instance
     * @param node Pointer to node to remove
     * @return Pointer to next ListNode in list or last if the removed node was the last (NULL if list is empty)
     */
    ListNode * (* remove)( struct StrList * list, ListNode * node );

    /**
     * Insert a new ListNode in the StrList at given ListNode position
     * @param list StrList instance
     * @param node Pointer to list node to insert new node at (pushes the old node forward)
     * @param str  String for new node
     * @return Pointer to the newly inserted ListNode (or NULL if failed)
     */
    ListNode * (* insert)( struct StrList * list, ListNode * node, const char * str );

    /**
     * Inserts a new node at the front of the list
     * @param list StrList instance
     * @param str string for new node
     * @return Pointer to new node or NULL if failed
     */
    ListNode * (* insert_front)( struct StrList * list, const char * str );

    /**
     * Inserts a new node at the end of the list
     * @param list StrList instance
     * @param str string for new node
     * @return Pointer to new node
     */
    ListNode * (* insert_back)( struct StrList * list, const char * str );

    /**
     * Emplace an orphan node into the front of a StrList
     * @param list StrList instance
     * @param node Node to emplace
     * @return Pointer to node
     */
    ListNode * (* emplace_front)( struct StrList * list, ListNode * node );

    /**
     * Emplace an orphan node into the back of a StrList
     * @param list StrList instance
     * @param node Node to emplace
     * @return Pointer to node
     */
    ListNode * (* emplace_back)( struct StrList * list, ListNode * node );

    /**
     * Remove a ListNode from a StrList
     * @param list StrList instance
     * @param node ListNode to extract from the StrList
     * @return Extracted ListNode
     */
    ListNode * (* extract_node)( struct StrList * list, ListNode * node );

    /**
     * Copies ListNodes to the end of another list
     * @param from   Origin StrList
     * @param to     Destination StrList
     * @param offset Offset from which to copy nodes at origin
     * @param n      Number of nodes to copy
     * @return Number of successfully copied nodes
     */
    size_t (* copy)( const struct StrList * from, struct StrList * to, size_t offset, size_t n );

    /**
     * Moves ListNodes to the end of another list
     * @param from   Origin StrList
     * @param to     Destination StrList
     * @param offset Offset from which to move nodes at origin
     * @param n      Number of nodes to move
     * @return Number of successfully moved nodes
     */
    size_t (* move)( struct StrList * from, struct StrList * to, size_t offset, size_t n );

    /**
     * Gets the ListNode at a specified index
     * @param list StrList instance
     * @param n    Index of node
     * @return Pointer of ListNode located at given index (NULL if out-of-range)
     */
    const ListNode * (* at)( const struct StrList * list, size_t n );

    /**
     * Gets the number of items in StrList
     * @param list StrList instance
     * @return number of items
     */
    size_t (* size)( const struct StrList * list );

    /**
     * Gets the empty state of the StrList
     * @param list StrList instance
     * @return Empty state
     */
    bool (* empty)( const struct StrList * list );

    /**
     * Removes and de-allocates all ListNode(s) in StrList
     * @param list StrList instance
     */
    void (* free_strlist)( struct StrList * list );

    /**
     * Stringify the StrList into a String_t
     * @param list StrList object
     * @param str  String_t object
     * @param sep  List item separation character (e.g. ',')
     * @return Number of items parsed (can be compared the StrList.size(..) to check for any failures)
     */
    size_t (* stringify)( const struct StrList * list, String_t * str, const char sep );

} StrList;

#endif //CTUNE_DATASTRUCTURE_STRLIST_H
