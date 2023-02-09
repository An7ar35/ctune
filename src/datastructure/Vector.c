#include "Vector.h"

#include <assert.h>

#include "logger/src/Logger.h"

/**
 * [PRIVATE] Grows the vector data-structure
 * @param v       Vector instance
 * @param new_cap Proposed target capacity
 * @return Success
 */
static bool Vector_resize( struct Vector * v, size_t new_cap ) {
    void ** items = realloc( v->_items, v->_data_size * new_cap );

    if( items == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[Vector_resize( %p )] Error resizing (%lu->%lu): failed memory realloc.", v, v->_capacity, new_cap );
        return false;
    }

    v->_items    = items;
    v->_capacity = new_cap;
    return true;
}

/**
 * [PRIVATE] Allocates memory for a default base collection in the case where `_items` was not malloc-ed
 * @param v Vector instance
 * @return Memory allocation success or already allocated state
 */
static bool Vector_malloc( struct Vector * v ) {
    if( v == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[Vector_malloc( %p )] Error: pointer to vector is NULL.", v );
        return false;
    }

    if( v->_items == NULL ) {
        v->_capacity = VECTOR_INIT_CAPACITY;
        v->_length   = 0;
        v->_items    = malloc( sizeof( void * ) * VECTOR_INIT_CAPACITY );

        return !( v->_items == NULL );
    }

    return true;
}

/**
 * [PRIVATE] Swaps pointers
 * @param lhs Pointer A
 * @param rhs Pointer B
 */
void Vector_swap( void ** lhs, void ** rhs ) {
    void * temp = (*lhs);
    (*lhs) = (*rhs);
    (*rhs) = temp;
}

/**
 * [PRIVATE] Quicksort partition method
 * @param v             Vector instance
 * @param comparator_fn Comparator function
 * @param low            Left side index
 * @param high            Right side index
 * @return New pivot index
 */
size_t Vector_partition( struct Vector * v, int (* comparator_fn)( const void *, const void * ), size_t low, size_t high ) {
    size_t mid = low + ( high - low ) / 2;
    size_t i   = low + 1;
    size_t j   = high;

    // {[ ][ ]...[ ]...[ ]}
    //   |  |     |      |
    //   lo i     mid    hi/j

    const void * pivot = Vector.at( v, mid );
    Vector_swap( &(v->_items[mid]), &(v->_items[low]) );

    while( i <= j ) {
        while( i <= j && comparator_fn( Vector.at( v, i ), pivot ) <= 0 ) {
            ++i;
        }

        while( i <= j && comparator_fn( Vector.at( v, j ), pivot ) > 0 ) {
            --j;
        }

        if( i < j ) {
            Vector_swap( &(v->_items[i]), &(v->_items[j]) );
        }
    }

    Vector_swap( &(v->_items[(i - 1)]), &(v->_items[low]) );

    return (i - 1);
}

/**
 * [PRIVATE] Quicksort
 * @param v             Vector instance
 * @param comparator_fn Comparator method
 * @param low            Left side index
 * @param hi            Right side index
 */
void Vector_quicksort( struct Vector * v, int (* comparator_fn)( const void *, const void * ), size_t low, size_t high ) {
    if( low < high ) {
        size_t p = Vector_partition( v, comparator_fn, low, high );
        Vector_quicksort( v, comparator_fn, low, ( p > 0 ? (p - 1) : 0 ) ); //note: avoids underflow on 'high'
        Vector_quicksort( v, comparator_fn, (p + 1), high );
    }
}

/**
 * Adds an element to the end of the collection
 * @param v  Vector instance
 * @param el Element to add
 * @return Success
 */
static bool Vector_add( struct Vector * v, void * el ) {
    if( !Vector_malloc( v ) )
        return false;

    bool grown_ok = true;

    if( v->_length == v->_capacity ) //need more space
        grown_ok = Vector_resize( v, ( (double) v->_capacity * VECTOR_GROWTH_FACTOR ) );

    if( grown_ok ) {
        v->_items[ v->_length ] = el;
        v->_length             += 1;
        return true;
    }

    return false;
}

/**
 * Creates an un-initialised element in-place at the back of the data-structure
 * @param v Vector instance
 * @return Pointer newly created element or NULL if failed allocation
 */
static void * Vector_emplace_back( struct Vector * v ) {
    if( !Vector_malloc( v ) )
        return NULL;

    bool grown_ok = true;

    if( v->_length == v->_capacity ) //need more space
        grown_ok = Vector_resize( v, ( (double) v->_capacity * VECTOR_GROWTH_FACTOR ) );

    if( grown_ok ) {
        v->_items[ v->_length ] = malloc( v->_data_size );

        if( v->_items[ v->_length ] == NULL ) {
            CTUNE_LOG( CTUNE_LOG_ERROR, "[Vector_emplace_back( %p )] Failed malloc of Vector element.", v );
            return NULL;
        }

        return v->_items[ ( v->_length++ ) ];
    }

    return NULL;
}

/**
 * Initialises an element in-place at the back of the data-structure
 * @param v       Vector instance
 * @param init_fn Function to use to initialise the field(s) in the element
 * @return Pointer newly created element or NULL if failed allocation
 */
void * Vector_init_back( struct Vector * v, void(* init_fn)( void * ) ) {
    void * el = Vector_emplace_back( v );
    init_fn( el );
    return el;
}

/**
 * Access an element
 * @param v   Vector instance
 * @param pos Index position of element to access
 * @return Pointer to element or NULL if error
 */
static void * Vector_at( struct Vector * v, size_t pos ) {
    if( pos >= v->_length ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[Vector_at( %p, %lu )] Error: position is out of bounds (length = %lu).", v, pos, v->_length );
        return NULL;
    }

    if( v->_items[pos] == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[Vector_at( %p, %lu )] Error: item at position is NULL.", v, pos );
    }

    return v->_items[pos];
}

/**
 * Removes an element in vector
 * @param v   Vector instance
 * @param pos Position of element
 * @return Removal success
 */
static bool Vector_remove( struct Vector * v, size_t pos ) {
    if( v == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[Vector_remove( %p, %lu )] Error: vector pointer is NULL.", v, pos );
        return false;
    }

    if( pos >= v->_length ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[Vector_remove( %p, %lu )] Error: position is out of bounds (length = %lu).", v, pos, v->_length );
        return false;
    }

    v->free_fn( v->_items[pos] );
    free( v->_items[pos] );
    v->_items[pos] = NULL;

    for( size_t i = pos; i < v->_length - 1; ++i ) {
        v->_items[ i ] = v->_items[ i + 1 ];
        v->_items[ i + 1 ] = NULL;
    }

    v->_length -= 1;

    size_t proposed_capacity = (float) v->_capacity / VECTOR_GROWTH_FACTOR;

    if( v->_length > VECTOR_INIT_CAPACITY && v->_length < proposed_capacity ) {
        if( !Vector_resize( v, proposed_capacity ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR, "[Vector_remove( %p, %lu )] Error: resizing failed (length = %lu).", v, pos, v->_length );
        }
    }

    return true;
}

/**
 * Gets the number of elements in the collection
 * @param v Vector instance
 * @return Number of elements
 */
static size_t Vector_size( const struct Vector * v ) {
    if( v == NULL )
        return 0;

    return v->_length;
}

/**
 * Gets the empty state in the collection
 * @param v Vector instance
 * @return Emptiness
 */
static bool Vector_empty( const struct Vector * v ) {
    if( v == NULL )
        return 0;

    return ( v->_length == 0 );
}

/**
 * Gets the current capacity of the collection
 * @param v Vector instance
 * @return Collection capacity
 */
static size_t Vector_capacity( const struct Vector * v ) {
    if( v == NULL )
        return 0;

    return v->_capacity;
}

/**
 * Clears and de-allocates all elements in the Vector
 * @param v Vector instance
 */
static void Vector_clear( struct Vector * v ) {
    if( v == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[Vector_clear( %p )] Error: vector pointer is NULL.", v );
        return; //EARLY RETURN
    }
    if( !v->_init_state ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[Vector_clear( %p )] Error: vector is not initialised.", v );
        return; //EARLY RETURN
    }

    for( size_t i = 0; i < v->_length; ++i ) {
        if( v->free_fn )
            v->free_fn( v->_items[i] );
        free( v->_items[i] );
    }

    free( v->_items );
    v->_init_state = false;
}

/**
 * Initialisation of a Vector
 * @param data_size Size of the element type (i.e.: `sizeof(..)`)
 * @param free_el   Function to use to free a single element type (for cases of nested pointers in elements of type `struct`)
 * @return Initialized Vector
 */
static struct Vector Vector_init( size_t data_size, void(* free_el)( void * el ) ) {
    assert( data_size > 0 );

    return (struct Vector) {
        ._init_state = true,
        ._data_size  = data_size,
        ._capacity   = VECTOR_INIT_CAPACITY,
        ._length     = 0,
        ._items      = malloc( sizeof( void * ) * VECTOR_INIT_CAPACITY ),
        .free_fn     = free_el
    };
}

/**
 * Initialisation of a Vector pointer (assumed to be pre-allocated)
 * @param v         Vector pointer
 * @param data_size Size of the element type (i.e.: `sizeof(..)`)
 * @param free_el   Function to use to free a single element type (for cases of nested pointers in elements of type `struct`)
 */
void Vector_init_ptr( struct Vector * v, size_t data_size, void(* free_el)( void * el ) ) {
    if( v == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[Vector_init_ptr( %p, %lu, %p )] Error: pointer to vector is NULL", v, data_size, free );
        return; //EARLY RETURN
    }

    assert( data_size > 0 );

    v->_init_state = true;
    v->_data_size = data_size;
    v->_capacity   = VECTOR_INIT_CAPACITY;
    v->_length     = 0;
    v->_items      = malloc( sizeof( void * ) * VECTOR_INIT_CAPACITY );
    v->free_fn     = free_el;
}

/**
 * Re-initialises an initialised Vector
 * - free all element if any and keeps the data size + element free func. callback
 * @param v Vector instance
 * @return Success
 */
static bool Vector_reinit( struct Vector * v ) {
    if( v->_init_state ) {
        Vector.clear_vector( v );
        v->_init_state = true;
        v->_capacity   = VECTOR_INIT_CAPACITY;
        v->_length     = 0;
        v->_items      = malloc( sizeof( void * ) * VECTOR_INIT_CAPACITY );
        return true;
    }
    return false;
}

/**
 * Quick-sorts the vector using a comparator
 * @param v             Vector instance
 * @param comparator_fn Comparator function for the elements to sort
 */
void Vector_sort( struct Vector * v, int (* comparator_fn)( const void *, const void * ) ) {
    if( v == NULL || comparator_fn == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[Vector_sort( %p, %p )] One or more args is NULL.", v, comparator_fn );
        return; //EARLY RETURN
    }

    if( Vector.empty( v ) )
        return; //EARLY RETURN

    Vector_quicksort( v, comparator_fn, 0, ( Vector.size( v ) - 1 ) );
}

/**
 * Prints all elements in vector
 * @param v           Vector instance
 * @param print_el_fn Function to print a single element
 */
void Vector_print( struct Vector * v, void (* print_el_fn( void * ) ) ) {
    printf( "[ " );
    for( size_t i = 0; i < Vector.size( v ); ++i ) {
        if( Vector_at( v, i ) != NULL )
            print_el_fn( Vector_at( v, i ) );
        else
            printf( "%s", "NULL" );

        if( i < ( Vector.size( v ) - 1 ) )
            printf( ", " );
    }
    printf( " ]");
}

/**
 * Constructor
 */
const struct ctune_Vector_Namespace Vector = {
    .init          = &Vector_init,
    .init_ptr      = &Vector_init_ptr,
    .reinit        = &Vector_reinit,
    .add           = &Vector_add,
    .emplace_back  = &Vector_emplace_back,
    .init_back     = &Vector_init_back,
    .at            = &Vector_at,
    .remove        = &Vector_remove,
    .size          = &Vector_size,
    .empty         = &Vector_empty,
    .capacity      = &Vector_capacity,
    .clear_vector  = &Vector_clear,
    .sort          = &Vector_sort,
    .print         = &Vector_print,
};