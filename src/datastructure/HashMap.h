#ifndef CTUNE_DATASTRUCTURE_HASHMAP_H
#define CTUNE_DATASTRUCTURE_HASHMAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "Vector.h"

enum NodeColour {
    BLACK = 0,
    RED   = 1
};

/**
 * Bucket Item (linked-list nodes for collision purposes)
 * @param value Pointer to stored value
 * @param next  Pointer to next item in linked list
 */
typedef struct BucketItem {
    void              * value;
    struct BucketItem * next;

} BucketItem_t;

/**
 * Tree Node/Bucket
 * @param hashkey     Hashed item key
 * @param node_type   Node colour
 * @param parent      Node's parent
 * @param child_left  Left child node
 * @param child_right Right child node
 * @param items       Front pointer to linked list of bucket items where values matching the hashed key are stored
 * @param count       Item count of linked list
 */
typedef struct Bucket {
    uint64_t            hashkey;
    enum NodeColour     colour;
    struct Bucket     * parent;
    struct Bucket     * child_left;
    struct Bucket     * child_right;
    struct BucketItem * items;
    size_t              count;

} Bucket_t;

/**
 * HashMap object
 * @param _root       Root of the Red-Black tree data-structure used for the HashMap's buckets
 * @param _item_count Keeps track of the number of values stored in the data-structure
 * @param _node_count Keeps track of the number of buckets stored in the data-structure
 * @param free_fn     Callback method to use for de-allocating value from map
 * @param copy_fn     Callback method to use for copying value to map
 * @param hash_fn     Callback method to use for hashing a value's key
 * @param equal_fn    Callback method to use for checking equivalence in a value and its key
 */
typedef struct HashMap {
    Bucket_t * _root;
    size_t     _item_count;
    size_t     _node_count;

    void     (* free_fn)( void * el );
    void *   (* copy_fn)( const void * el );
    uint64_t (* hash_fn)( const void * key );
    bool     (* equal_fn)( const void * key, const void * el );

} HashMap_t;


extern const struct ctune_HashMap_Namespace {
    /**
     * Initialises a HashMap
     * @param free_cb  Callback method to use for de-allocating value from map
     * @param copy_cb  Callback method to use for duplicating value into map
     * @param hash_cb  Callback method to use for hashing a value's key
     * @param equal_cb Callback method to use for checking equivalence in a value and its key
     * @return Initialised HashMap_t object
     */
    HashMap_t (* init)( void     (* free_cb)( void * ),
                        void *   (* copy_cb)( const void * ),
                        uint64_t (* hash_cb)( const void * ),
                        bool     (* equal_cb)( const void *, const void * ) );

    /**
     * Gets item that matches key
     * @param map HashMap_t object
     * @param key Pointer to key
     * @return Matching value for the key or NULL for no match found
     */
    void * (* at)( HashMap_t * map, const void * key );

    /**
     * Adds a key/value pair into the HashMap
     * @param map HashMap_t object
     * @param key Pointer to key to use for hashing
     * @param val Pointer to value
     * @return Success
     */
    bool (* add)( HashMap_t * map, const void * key, const void * val );

    /**
     * Remove an item
     * @param map HashMap_t object
     * @param key Key for the value to remove
     * @return Success (false if error or not found)
     */
    bool (* remove)( HashMap_t * map, const void * key );

    /**
     * Clears the HashMap of everything
     * @param map HashMap_t object
     */
    void (* clear)( HashMap_t * map );

    /**
     * Gets the number of values stored in the map
     * @param map HashMap_t object
     * @return Item count
     */
    size_t (* size)( const HashMap_t * map );

    /**
     * Gets the empty state of the map
     * @param map HashMap_t object
     * @return Empty state
     */
    bool (* empty)( const HashMap_t * map );

    /**
     * Exports the values of the HashMap into a Vector_t
     * @param map     Source HashMap_t object
     * @param vector  Target Vector_t object
     * @param init_fn Initialisation method for elements
     * @param cp_fn   Copying method for elements
     * @return Number of exported elements
     */
    size_t (* export)( const HashMap_t * map, Vector_t * vector, void (* init_fn)( void * ), void (* cp_fn)( const void *, void * ) );

} HashMap;

#endif //CTUNE_DATASTRUCTURE_HASHMAP_H