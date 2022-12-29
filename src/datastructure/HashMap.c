#include "HashMap.h"

#include <string.h>
#include <stdlib.h>

#include "Deque.h"
#include "../logger/Logger.h"

/**
 * [PRIVATE] Allocates and initialises a new bucket
 * @param hash   Bucket hash
 * @param colour Bucket node colour
 * @param parent Pointer to parent node
 * @return Pointer to new bucket
 */
static Bucket_t * HashMap_newBucket( uint64_t hash, enum NodeColour colour, Bucket_t * parent ) {
    Bucket_t * bucket = malloc( sizeof( Bucket_t ) );

    if( bucket != NULL ) {
        bucket->hashkey     = hash;
        bucket->colour   = colour;
        bucket->count       = 0;
        bucket->items       = NULL;
        bucket->parent      = parent;
        bucket->child_left  = NULL;
        bucket->child_right = NULL;

        CTUNE_LOG( CTUNE_LOG_DEBUG, "[HashMap_newBucket( %lu, %s, %p )] New Bucket created.",
                   bucket->hashkey, ( bucket->colour == RED ? "RED" : "BLACK" ), bucket->parent );
    }

    return bucket;
}

/**
 * [PRIVATE] Compare hash keys
 * @param lhs Hash
 * @param rhs Hash to compareItem to
 * @return -1: lhs < rhs, 0: lhs == rhs, +1: lhs > rhs
 */
static int HashMap_compareHash( uint64_t lhs, uint64_t rhs ) {
    if( lhs > rhs )
        return +1;
    else if( lhs < rhs )
        return -1;
    return 0;
}

/**
 * [PRIVATE] Swaps bucket content (hash and items)
 * @param lhs Pointer to bucket
 * @param rhs Pointer to bucket
 */
static void HashMap_swapBucketContents( Bucket_t * lhs, Bucket_t * rhs ) {
    { //Bucket items linked list
        BucketItem_t * tmp = lhs->items;
        lhs->items         = rhs->items;
        rhs->items         = tmp;
    }

    { //Bucket items linked list counter
        size_t tmp = lhs->count;
        lhs->count = rhs->count;
        rhs->count = tmp;
    }

    { //Bucket hash
        int64_t tmp  = lhs->hashkey;
        lhs->hashkey = rhs->hashkey;
        rhs->hashkey = tmp;
    }
}

/**
 * [PRIVATE] Gets the parent of a node
 * @param node Bucket_t node
 * @return Pointer to parent
 */
static Bucket_t * HashMap_getParent( const Bucket_t * node ) {
    return node->parent;
}

/**
 * [PRIVATE] Gets a pointer to the parent's node pointer
 * @param map  HashMap_t object
 * @param node Bucket_t node
 * @return Pointer to parent's pointer or NULL if node is orphaned
 */
static Bucket_t ** HashMap_getParentPtr( HashMap_t * map, const Bucket_t * node ) {
    if( node->parent == NULL ) {
        if( node == map->_root )
            return &map->_root;
        else
            return NULL;

    } else if( node->parent->child_right == node ) {
        /*      parent
         *       /  \ <-(ptr)
         *      ?  (node)
         */
        return &node->parent->child_right;

    } else {
        /*         parent
         *   (ptr)->/  \
         *       (node) ?
         *
         */
        return &node->parent->child_left;
    }
}

/**
 * [PRIVATE] Gets the grand parent of a node
 * @param node Bucket_t node
 * @return Pointer to grand parent
 */
static Bucket_t  * HashMap_getGrandParent( const Bucket_t * node ) {
    return ( node->parent != NULL ? node->parent->parent : NULL );
}

/**
 * [PRIVATE] Gets the sibling of a node
 * @param node Bucket_t node
 * @return Pointer to sibling node
 */
static Bucket_t * HashMap_getSibling( const Bucket_t * node ) {
    if( node->parent == NULL )
        return NULL; //EARLY RETURN

    return ( node->parent->child_left == node ? node->parent->child_right : node->parent->child_left );
}

/**
 * [PRIVATE] Gets the uncle of a node
 * @param node Bucket_t node
 * @return Pointer to uncle node
 */
static Bucket_t * HashMap_getUncle( const Bucket_t * node ) {
    Bucket_t * parent = HashMap_getParent( node );

    if( parent == NULL )
        return NULL; //EARLY RETURN

    return HashMap_getSibling( parent );
}

/**
 * [PRIVATE] Left Rotate
 * @param map HashMap_t object
 * @param ptr Pointer to node
 * @return Success
 */
static bool HashMap_rotateLeft( HashMap_t * map, Bucket_t * node ) {
    /*          parent          parent
     *            |<-(*root_ptr)->|
     *   (node)->(x)             (y)
     *           / \        =>   / \
     *          a  (y)         (x)  c
     *             / \         / \
     *            b   c       a   b
     *
     */
    if( map == NULL || node == NULL || node->child_right == NULL )
        return false; //EARLY RETURN

    Bucket_t ** root_ptr = HashMap_getParentPtr( map, node );
    Bucket_t  * parent   = HashMap_getParent( node );

    if( root_ptr == NULL )
        root_ptr = &map->_root;

    Bucket_t * x = node;
    Bucket_t * y = x->child_right;
    Bucket_t * b = y->child_left;

    y->child_left  = x;
    y->parent      = parent;
    x->child_right = b;
    x->parent      = y;
    (*root_ptr)    = y;

    if( b != NULL )
        b->parent = x;

    return true;
}

/**
 * [PRIVATE] Right rotate
 * @param map HashMap_t object
 * @param ptr Pointer to node
 * @return Success
 */
static bool HashMap_rotateRight( HashMap_t * map , Bucket_t * node ) {
    /*         parent           parent
     *            |<-(*root_ptr)->|
     *   (node)->(x)             (y)
     *           / \     =>      / \
     *          (y)  c          a  (x)
     *          / \                / \
     *         a   b              b   c
     *
     */
    if( map == NULL || node == NULL || node->child_left == NULL )
        return false; //EARLY RETURN

    Bucket_t ** root_ptr = HashMap_getParentPtr( map, node );
    Bucket_t  * parent   = HashMap_getParent( node );

    if( root_ptr == NULL )
        root_ptr = &map->_root;

    Bucket_t * x = node;
    Bucket_t * y = x->child_left;
    Bucket_t * b = y->child_right;

    y->child_right = x;
    y->parent      = parent;
    x->child_left  = b;
    x->parent      = y;
    (*root_ptr)    = y;

    if( b != NULL )
        b->parent = x;

    return true;
}

/**
 * [PRIVATE] Plain BST style insertion of a Bucket node in the RB Tree
 * @param map    HashMap_t object
 * @param parent Pointer to current root's parent
 * @param node   Pointer to current root node pointer
 * @param hash   Bucket hash
 * @return Pointer to inserted/matching bucket
 */
static Bucket_t * HashMap_insertBST( HashMap_t * map, Bucket_t * parent, Bucket_t ** node, uint64_t hash ) {
    if( (*node) == NULL ) {
        (*node) = HashMap_newBucket( hash, RED, parent );

        if( (*node) == NULL ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[HashMap_insertBST( %p, %p, Bucket_t **, %lu )] alloc error for new bucket.",
                       map, parent, hash
            );
        }

        return (*node); //EARLY RETURN
    }

    int res = HashMap_compareHash( hash, (*node)->hashkey );

    if( res == 0 ) {
        CTUNE_LOG( CTUNE_LOG_TRACE,
                   "[HashMap_insertBST( %p, %p, Bucket_t **, %lu )] Bucket with same hash exists already (hash collision).",
                   map, parent, hash
        );

        return (*node); //no insertion

    } else if( res > 0 ) {
        return HashMap_insertBST( map, (*node), &(*node)->child_right, hash );
    } else { //comp < 0
        return HashMap_insertBST( map, (*node), &(*node)->child_left, hash );
    }
}

/**
 * [PRIVATE] Search tree for a bucket
 * @param map  HashMap_t object
 * @param root Pointer to current root node
 * @param hash Hash key to find
 * @return Pointer to Bucket_t whose key matches the hash or NULL if no match found
 */
static Bucket_t * HashMap_searchBST( const HashMap_t * map, Bucket_t * root, uint64_t hash ) {
    if( root == NULL )
        return NULL;
    else if( hash > root->hashkey )
        return HashMap_searchBST( map, root->child_right, hash );
    else if( hash < root->hashkey )
        return HashMap_searchBST( map, root->child_left, hash );
    return root; // i.e.: ( hash == root->hashkey )
}

/**
 * [PRIVATE] Gets the min bucket in a tree
 * @param root Root node of tree
 * @return Pointer to min bucket
 */
static Bucket_t * HashMap_minBucket( Bucket_t * root ) {
    if( root == NULL )
        return NULL;
    else if( root->child_left == NULL )
        return root;
    else
        return HashMap_minBucket( root->child_left );
}

/**
 * [PRIVATE] Searches the tree for a bucket
 * @param map  HashMap_t object
 * @param hash Hash key to find
 * @return Pointer to Bucket_t whose key matches the hash or NULL if no match found
 */
static Bucket_t * HashMap_searchBucket( HashMap_t * map, int64_t hash ) {
    return HashMap_searchBST( map, map->_root, hash );
}

/**
 * [PRIVATE] Search item within a Bucket for absolute key match
 * @param map    HashMap_t object
 * @param bucket Bucket to search
 * @param key    Key to find exact match
 * @return Pointer to pointer of bucket item or NULL if no match found
 */
static BucketItem_t ** HashMap_searchBucketItem( HashMap_t * map, Bucket_t * bucket, const void * key ) {
    if( bucket->count > 1 ) { //i.e.: hash collision
        BucketItem_t ** curr = &bucket->items;

        while( curr != NULL ) {
            if( map->equal_fn( key, (*curr)->value ) )
                return curr;

            curr = &( (*curr)->next );
        }

        return NULL;

    } else {
        return &bucket->items;
    }
}

/**
 * [PRIVATE] Repair the tree if it fails the BR tree rules
 * @param map    HashMap_t object
 * @param bucket Pointer to current node
 */
static void HashMap_insertRepair( HashMap_t * map, Bucket_t * bucket ) { //Case 1: root node
    if( HashMap_getParent( bucket ) == NULL ) {
        //    (B)

        bucket->colour = BLACK;

    } else if( HashMap_getParent( bucket )->colour == BLACK ) { //Case 2: tree depth = 2
        /*      B
         *       \
         *       (B)
         *
         */
        return; //EARLY RETURN

    } else if( HashMap_getUncle( bucket ) != NULL && HashMap_getUncle( bucket )->colour == RED ) { //Case 3
        /*          B               R
         *         / \             / \
         *        R   R     =>    B   B
         *       / \ / \         / \ / \
         *     (R)             (R)
         *     / \             / \
         *
         */
        HashMap_getParent( bucket )->colour      = BLACK;
        HashMap_getUncle( bucket )->colour       = BLACK;
        HashMap_getGrandParent( bucket )->colour = RED;

        HashMap_insertRepair( map, HashMap_getGrandParent( bucket ) );

    } else { //Case 4
        Bucket_t * curr_bucket = bucket; //(n)

        { //step 1
            /*         g/B                g/B
             *        /   \              /   \
             *      p/R   u/B     =>   n/R   u/B
             *     / \     / \         / \   / \
             *    a (n/R) d   e    (p/R)  c d   e
             *       / \            / \
             *      b   c          a   b
             *
             */
            Bucket_t * parent       = HashMap_getParent( curr_bucket );      //p
            Bucket_t * grand_parent = HashMap_getGrandParent( curr_bucket ); //g

            if( curr_bucket == parent->child_right && parent == grand_parent->child_left ) {
                if( !HashMap_rotateLeft( map, parent ) )
                    CTUNE_LOG( CTUNE_LOG_TRACE, "[HashMap_insertRepair( %p )] Failed rotate left (%p).", bucket, parent );
                curr_bucket = bucket->child_left;

            } else if( curr_bucket == parent->child_left && parent == grand_parent->child_right ) {
                if( !HashMap_rotateRight( map, parent ) )
                    CTUNE_LOG( CTUNE_LOG_TRACE, "[HashMap_insertRepair( %p )] Failed rotate right (%p).", bucket, parent );
                curr_bucket = curr_bucket->child_right;
            }
        }

        { //step 2

            /*          g/B             ->p/B
             *         /   \             /   \
             *     ->p/R   u/B     => (n/R)  g/R
             *       / \   / \         / \   / \
             *   (n/R)  c d   e       a   b c  u/B
             *    / \                          / \
             *   a   b                        d   e
             *
             */
            Bucket_t * parent       = HashMap_getParent( curr_bucket );      //p
            Bucket_t * grand_parent = HashMap_getGrandParent( curr_bucket ); //g

            if( curr_bucket == parent->child_left ) {
                if( !HashMap_rotateRight( map, grand_parent ) )
                    CTUNE_LOG( CTUNE_LOG_TRACE, "[HashMap_insertRepair( %p )] Failed rotate left (%p).", bucket, grand_parent );
            } else {
                if( !HashMap_rotateLeft( map, grand_parent ) )
                    CTUNE_LOG( CTUNE_LOG_TRACE, "[HashMap_insertRepair( %p )] Failed rotate right (%p).", bucket, grand_parent );
            }

            parent->colour       = BLACK;
            grand_parent->colour = RED;
        }
    }
}

/**
 * [PRIVATE] Allocates a new bucket item at the front of a bucket's linked list
 * @param bucket Bucket pointer
 * @return Allocated bucket item
 */
static BucketItem_t * HashMap_newBucketItem( Bucket_t * bucket ) {
    BucketItem_t * item = malloc( sizeof( BucketItem_t ) );

    if( item != NULL ) { //insert front of item list
        item->next     = bucket->items;
        bucket->items  = item;
        bucket->count += 1;
    }

    return item;
}

/**
 * [PRIVATE] Adds a new bucket item
 * @param map HashMap_t object
 * @param key Key of value
 * @param val Value to add
 * @return Success
 */
static bool HashMap_addBucketItem( HashMap_t * map, const void * key, const void * val ) {
    if( map == NULL || val == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[HashMap_addBucketItem( %p, %p )] NULL map/val.", map, val );
        return false;
    }

    uint64_t   hash = map->hash_fn( key );
    Bucket_t * node = HashMap_insertBST( map, NULL, &map->_root, hash );

    if( node == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[HashMap_addBucketItem( %p, %p )] Failed node allocation.", map, val );
        return false; //EARLY RETURN
    }

    if( node->count > 0 )
        CTUNE_LOG( CTUNE_LOG_TRACE, "[HashMap_addBucketItem( %p, %p )] Hash collision detected ('%lu' x %lu)", map, val, hash, node->count );

    BucketItem_t * item = HashMap_newBucketItem( node );

    if( item == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[HashMap_addBucketItem( %p, %p )] Failed item allocation", map, val );
        return false; //EARLY RETURN
    }

    if( ( item->value = map->copy_fn( val ) ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[HashMap_addBucketItem( %p, %p )] Failed item value allocation", map, val );
        return false; //EARLY RETURN
    }

    HashMap_insertRepair( map, node );
    ++map->_node_count;
    ++map->_item_count;

    return true;
}

/**
 * [PRIVATE] Removes a bucket item
 * @param map      HashMap_t pointer
 * @param bucket   Pointer to Bucket_t object
 * @param item_ptr Pointer to Bucket item pointer to remove
 * @return Success
 */
static bool HashMap_removeBucketItem( HashMap_t * map, Bucket_t * bucket, BucketItem_t ** item_ptr ) {
    if( item_ptr != NULL && *item_ptr != NULL ) {
        CTUNE_LOG( CTUNE_LOG_TRACE, "[HashMap_removeBucketItem( %p, %p, %p )] Removing element.", map, bucket, item_ptr );

        BucketItem_t * tmp = (*item_ptr);
        *item_ptr = (*item_ptr)->next;
        map->free_fn( tmp->value );
        free( tmp );

        bucket->count    -= 1;
        map->_item_count -= 1;

        return true;

    } else {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[HashMap_removeBucketItem( %p, %p )] "
                   "BucketItem_t * is NULL or pointer to BucketItem_t is NULL",
                   map, item_ptr
        );

        return false;
    }
}

/**
 * [PRIVATE] De-allocates a bucket and content if any
 * @param map    HashMap_t object
 * @param bucket Bucket to be freed
 */
static void HashMap_freeBucket( HashMap_t * map, Bucket_t * bucket ) {
    if( bucket->parent != NULL ) {
        if( bucket == bucket->parent->child_left )
            bucket->parent->child_left = NULL;

        if( bucket == bucket->parent->child_right )
            bucket->parent->child_right = NULL;
    }

    if( bucket->items != NULL ) {
        CTUNE_LOG( CTUNE_LOG_WARNING,
                   "[HashMap_freeBucket( %p )] Bucket has %lu items to be de-allocated inside of it.", bucket,
                   bucket->count );

        while( bucket->items != NULL ) {
            if( !HashMap_removeBucketItem( map, bucket, &bucket->items ) ) {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[HashMap_freeBucket( %p, %p )] Bucket item removal failed.",
                           map, bucket
                );
            }
        }
    }

    if( bucket->child_left || bucket->child_right )
        CTUNE_LOG( CTUNE_LOG_WARNING, "[HashMap_freeBucket( %p )] Freeing bucket orphans its children!", bucket );

    free( bucket );
    map->_node_count -= 1;
}

/**
 * [PRIVATE] Gets the replacement node for a node targeted for deletion
 * @param node Node to get a replacement for
 * @return Pointer to replacement node (or NULL for none)
 */
static Bucket_t * HashMap_getReplacementNode( Bucket_t * node ) {
    if( node == NULL )
        return NULL; //EARLY RETURN

    if( node->child_left == NULL ) {
        return node->child_right; //EARLY RETURN

    } else if( node->child_right == NULL ) {
        return node->child_left; //EARLY RETURN

    } else {
        return HashMap_minBucket( node->child_right ); //EARLY RETURN
    }
}

/**
 * [PRIVATE] Applies R/B double black fix recursively on a tree
 * @param map  HashMap_t object
 * @param node Root of RB tree to start fixing from
 */
void HashMap_fixDoubleBlack( HashMap_t * map, Bucket_t * node ) {
    if( node == map->_root )
        return; //EARLY RETURN

    Bucket_t * sibling = HashMap_getSibling( node );
    Bucket_t * parent  = HashMap_getParent( node );

    if( sibling == NULL ) {
        HashMap_fixDoubleBlack( map, parent );

    } else {
        if( sibling->colour == RED ) {
            parent->colour  = RED;
            sibling->colour = BLACK;

            if( sibling == sibling->parent->child_left )
                HashMap_rotateRight( map, parent );
            else
                HashMap_rotateLeft( map, parent );

            HashMap_fixDoubleBlack( map, node );

        } else { //sibling->colour == BLACK
            if( ( sibling->child_left  != NULL && sibling->child_left->colour  == RED ) ||
                ( sibling->child_right != NULL && sibling->child_right->colour == RED ) )
            { //i.e. sibling has 1+ red child
                if( sibling->child_left != NULL && sibling->child_left->colour == RED ) {
                    if( sibling == sibling->parent->child_left ) {
                        sibling->child_left->colour = sibling->colour;
                        sibling->colour             = parent->colour;
                        HashMap_rotateRight( map, parent );

                    } else {
                        sibling->child_left->colour = parent->colour;
                        HashMap_rotateRight( map, sibling );
                        HashMap_rotateLeft( map, parent );
                    }

                } else {
                    if( sibling == sibling->parent->child_left ) {
                        sibling->child_right->colour = parent->colour;
                        HashMap_rotateLeft( map, sibling );
                        HashMap_rotateRight( map, parent );

                    } else {
                        sibling->child_right->colour = sibling->colour;
                        sibling->colour              = parent->colour;
                        HashMap_rotateLeft( map, parent );
                    }
                }

                parent->colour = BLACK;

            } else { //i.e.: 2x BLACK children
                sibling->colour = RED;

                if( parent->colour == BLACK)
                    HashMap_fixDoubleBlack( map, parent );
                else
                    parent->colour = BLACK;
            }
        }
    }
}

/**
 * [PRIVATE] Removes a bucket node in the RB tree
 * @param map HashMap_t object
 * @param v   Pointer to node targeted for removal
 */
static void HashMap_deleteBucketNode( HashMap_t * map, Bucket_t * v ) {
    CTUNE_LOG( CTUNE_LOG_TRACE,
               "[HashMap_deleteBucketNode( %p, %p )] Removing bucket (hash='%lu').",
               map, v, v->hashkey
    );

    Bucket_t * u            = HashMap_getReplacementNode( v );
    bool       double_black = ( ( u == NULL || u->colour == BLACK ) && ( v->colour == BLACK ) );
    Bucket_t * parent       = v->parent;

    if( u == NULL ) {
        if( v == map->_root ) {
            map->_root = NULL;

        } else {
            if( double_black ) {
                HashMap_fixDoubleBlack( map, v );

            } else {
                Bucket_t * sibling = HashMap_getSibling( v );

                if( sibling != NULL)
                    sibling->colour = RED;
            }

            if ( v == v->parent->child_left )
                parent->child_left  = NULL;
            else
                parent->child_right = NULL;
        }

        HashMap_freeBucket( map, v );
        return; //EARLY RETURN
    }

    if( v->child_left == NULL || v->child_right == NULL ) {
        if( v == map->_root ) {
            HashMap_swapBucketContents( v, u );
            HashMap_freeBucket( map, u );

        } else {
            if( v == v->parent->child_left )
                parent->child_left  = u;
            else
                parent->child_right = u;

            free( v );

            u->parent = parent;

            if( double_black )
                HashMap_fixDoubleBlack( map, u );
            else
                u->colour = BLACK;
        }

        return; //EARLY RETURN;
    }

    HashMap_swapBucketContents( u, v );
    HashMap_deleteBucketNode( map, u );
}

/**
 * Initialises a HashMap
 * @param free_cb  Callback method to use for de-allocating value from map
 * @param copy_cb  Callback method to use for duplicating value into map
 * @param hash_cb  Callback method to use for hashing a value's key
 * @param equal_cb Callback method to use for checking equivalence in a value and its key
 * @return Initialised HashMap_t object
 */
static HashMap_t HashMap_init(
    void (* free_cb)( void * ),
    void * (* copy_cb)( const void * ),
    uint64_t (* hash_cb)( const void * ),
    bool (* equal_cb)( const void *, const void * ) )
{
    return (struct HashMap) {
        ._root       = NULL,
        ._node_count = 0,
        ._item_count = 0,
        .free_fn     = free_cb,
        .copy_fn     = copy_cb,
        .hash_fn     = hash_cb,
        .equal_fn    = equal_cb,
    };
}

/**
 * Gets item that matches key
 * @param map HashMap_t object
 * @param key Pointer to key
 * @return Matching value for the key or NULL for no match found
 */
static void * HashMap_at( HashMap_t * map, const void * key ) {
    int64_t    hash   = map->hash_fn( key );
    Bucket_t * bucket = HashMap_searchBucket( map, hash );

    if( bucket == NULL || bucket->count == 0 )
        return NULL; //EARLY RETURN

    BucketItem_t ** item_ptr = HashMap_searchBucketItem( map, bucket, key );

    return ( ( item_ptr == NULL || (*item_ptr) == NULL )
             ? NULL
             : (*item_ptr)->value );
}

/**
 * Adds a key/value pair into the HashMap
 * @param map HashMap_t object
 * @param key Pointer to key to use for hashing
 * @param val Pointer to value
 * @return Success
 */
static bool HashMap_add( HashMap_t * map, const void * key, const void * val ) {
    uint64_t hash = map->hash_fn( key );

    if( HashMap_at( map, key ) != NULL ) {
        CTUNE_LOG( CTUNE_LOG_TRACE,
                   "[HashMap_add( %p, %p, %p )] K/V pair already exists in HashMap (hash='%lu').",
                   map, key, val, hash
        );

        return false; //EARLY RETURN
    }

    if( !HashMap_addBucketItem( map, key, val ) ) {
        CTUNE_LOG( CTUNE_LOG_TRACE,
                   "[HashMap_add( %p, %p, %p )] Failed to add K/V pair into HashMap (hash='%lu').",
                   map, key, val, hash
        );

        return false; //EARLY RETURN
    }

    return true;
}

/**
 * Remove an item
 * @param map HashMap_t object
 * @param key Key for the value to remove
 * @return Success (false if error or not found)
 */
static bool HashMap_remove( HashMap_t * map, const void * key ) {
    if( map == NULL || key == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[HashMap_remove( %p, %p )] One or more args is NULL.", map, key );
        return false; //EARLY RETURN
    }

    int64_t    hash   = map->hash_fn( key );
    Bucket_t * bucket = HashMap_searchBucket( map, hash );

    if( bucket == NULL ) {
        CTUNE_LOG( CTUNE_LOG_DEBUG, "[HashMap_remove( %p, %p )] No bucket found matching key hash ('%lu').", map, key, hash );
        return false; //EARLY RETURN
    }

    BucketItem_t ** item_ptr = HashMap_searchBucketItem( map, bucket, key );

    if( item_ptr == NULL || (*item_ptr) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_DEBUG, "[HashMap_remove( %p, %p )] No bucket item found matching key (hash='%lu').", map, key, hash );
        return false; //EARLY RETURN
    }

    bool success = HashMap_removeBucketItem( map, bucket, item_ptr );

    if( !success ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[HashMap_remove( %p, %p )] Error removing bucket item.", map, key );
        return false; //EARLY RETURN
    }

    if( bucket->items == NULL )
        HashMap_deleteBucketNode( map, bucket );

    return true;
}

/**
 * Clears the HashMap of everything
 * @param map HashMap_t object
 */
static void HashMap_clear( HashMap_t * map ) {
    if( map == NULL || map->_root == NULL )
        return; //EARLY RETURN

    Deque_t deque   = Deque.init();
    size_t  deleted = 0;

    Deque.pushBack( &deque, map->_root );

    while( !Deque.empty( &deque ) ) {
        Bucket_t * curr = Deque.popFront( &deque );

        if( curr->child_left != NULL )
            Deque.pushBack( &deque, curr->child_left );

        if( curr->child_right != NULL )
            Deque.pushBack( &deque, curr->child_right );

        while( curr->items != NULL )
            HashMap_removeBucketItem( map, curr, &curr->items );

        free( curr );
        map->_node_count -= 1;

        ++deleted;
    }

    if( map->_item_count != 0 || map->_node_count != 0 ) { //assertion - should not happen but just in case...
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[HashMap_clear( %p )] Clearing incomplete/corrupt: items = %lu, nodes = %lu",
                   map, map->_item_count, map->_node_count
        );

    } else {
        CTUNE_LOG( CTUNE_LOG_DEBUG,
                   "[HashMap_clear( %p )] %lu items cleared.",
                   map, deleted
        );

        map->_root = NULL;
    }
}

/**
 * Gets the number of values stored in the map
 * @param map HashMap_t object
 * @return Item count
 */
static size_t HashMap_size( const HashMap_t * map ) {
    return map->_item_count;
}

/**
 * Gets the empty state of the map
 * @param map HashMap_t object
 * @return Empty state
 */
static bool HashMap_empty( const HashMap_t * map ) {
    return ( map->_item_count == 0 );
}

/**
 * Exports the values of the HashMap into a Vector_t
 * @param map        Source HashMap_t object
 * @param vector     Target Vector_t object
 * @param init_fn    Initialisation method for elements
 * @param cp_fn      Copying method for elements
 * @return Number of exported elements
 */
static size_t HashMap_export( const HashMap_t * map, Vector_t * vector, void (* init_fn)( void * ), void (* cp_fn)( const void *, void * ) ) {
    if( map == NULL || vector == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[HashMap_export( %p, %p, %p %p )] map/vector arg NULL.", map, vector, init_fn, cp_fn );
        return 0;
    }

    if( HashMap.empty( map ) ) {
        CTUNE_LOG( CTUNE_LOG_DEBUG, "[HashMap_export( %p, %p, %p %p )] HashMap is empty.", map, vector, init_fn, cp_fn );
        return 0;
    }

    size_t pre_count = Vector.size( vector );

    if( ( UINT64_MAX - HashMap.size( map ) ) < pre_count  ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[HashMap_export( %p, %p, %p, %p )] "
                                    "Appending the content of the HashMap to the non-empty vector would create an integer overflow. "
                                    "Consider clearing the target vector first.",
                                    map, vector, init_fn, cp_fn );

        return 0;
    }

    Deque_t deque = Deque.init();

    Deque.pushBack( &deque, map->_root );

    while( !Deque.empty( &deque ) ) {
        Bucket_t * curr = Deque.popFront( &deque );

        if( curr->child_left != NULL )
            Deque.pushBack( &deque, curr->child_left );

        if( curr->child_right != NULL )
            Deque.pushBack( &deque, curr->child_right );

        BucketItem_t * item = curr->items;

        while( item != NULL ) {
            if( cp_fn == NULL ) {
                Vector.add( vector, item->value );
            } else {
                void * el = Vector.init_back( vector, init_fn );
                cp_fn( item->value, el );
            }

            item = item->next;
        }
    }

    if( ( map->_item_count + pre_count ) != Vector.size( vector ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[HashMap_export( %p, %p, %p )] Incomplete/inconsistent map item export (%lu + %lu -> %lu)",
                   map, vector, cp_fn, pre_count, HashMap.size( map ), Vector.size( vector )
        );
    }

    return Vector.size( vector );
}

/**
 * Namespace constructor
 */
const struct ctune_HashMap_Namespace HashMap = {
    .init   = &HashMap_init,
    .at     = &HashMap_at,
    .add    = &HashMap_add,
    .remove = &HashMap_remove,
    .clear  = &HashMap_clear,
    .size   = &HashMap_size,
    .empty  = &HashMap_empty,
    .export = &HashMap_export,
};
