#ifndef CTUNE_DATASTRUCTURE_VECTOR_H
#define CTUNE_DATASTRUCTURE_VECTOR_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#define VECTOR_INIT_CAPACITY 4
#define VECTOR_GROWTH_FACTOR 1.3

typedef struct Vector {
    bool    _init_state;
    void ** _items;
    size_t  _capacity;
    size_t  _length;
    size_t  _data_size;

    void (* free_fn)( void * el );
} Vector_t;

extern const struct ctune_Vector_Namespace {
    /**
     * Initialisation of a Vector
     * @param data_size Size of the element type (i.e.: `sizeof(..)`)
     * @param free      Function to use to free a single element type (for cases of nested pointers in elements of type `struct`)
     * @return Initialized Vector
     */
    struct Vector (* init)( size_t data_size, void(* free)( void * el ) );

    /**
     * Initialisation of a Vector pointer (assumed to be pre-allocated)
     * @param v         Vector pointer
     * @param data_size Size of the element type (i.e.: `sizeof(..)`)
     * @param free      Function to use to free a single element type (for cases of nested pointers in elements of type `struct`)
     */
    void (* init_ptr)( struct Vector * v, size_t data_size, void(* free)( void * el ) );

    /**
     * Re-initialises an initialised Vector
     * - free all element if any and keeps the data size + element free func. callback
     * @param v Vector instance
     * @return Success
     */
    bool (* reinit)( struct Vector * v );

    /**
     * Adds an element to the end of the collection
     * @param v  Vector instance
     * @param el Element to add
     * @return Success
     */
    bool (* add)( struct Vector * v, void * el );

    /**
     * Creates an un-initialised element in-place at the back of the data-structure
     * @param v Vector instance
     * @return Pointer newly created element or NULL if failed allocation
     */
    void * (* emplace_back)( struct Vector * v );

    /**
     * Initialises an element in-place at the back of the data-structure
     * @param v       Vector instance
     * @param init_fn Function to use to initialise the field(s) in the element
     * @return Pointer newly created element or NULL if failed allocation
     */
    void * (* init_back)( struct Vector * v, void (* init_fn)( void * el ) );

    /**
     * Access an element
     * @param v   Vector instance
     * @param pos Index position of element to access
     * @return Pointer to element or NULL if error
     */
    void * (* at)( struct Vector * v, size_t pos );

    /**
     * Removes an element in vector
     * @param v   Vector instance
     * @param pos Position of element
     * @return Removal success
     */
    bool (* remove)( struct Vector * v, size_t pos );

    /**
     * Gets the number of elements in the collection
     * @param v Vector instance
     * @return Number of elements
     */
    size_t (* size)( const struct Vector * v );

    /**
     * Gets the empty state in the collection
     * @param v Vector instance
     * @return Emptiness
     */
    bool (* empty)( const struct Vector * v );

    /**
     * Gets the current capacity of the collection
     * @param v Vector instance
     * @return Collection capacity
     */
    size_t (* capacity)( const struct Vector * v );

    /**
     * Clears and de-allocates all elements in the Vector
     * @param v Vector instance
     */
    void (* clear_vector)( struct Vector * v );

    /**
     * Quick-sorts the vector using a comparator
     * @param v             Vector instance
     * @param comparator_fn Comparator function for the elements to sort
     */
    void (* sort)( struct Vector * v, int (* comparator_fn)( const void *, const void * ) );

    /**
     * Prints all elements in vector
     * @param v           Vector instance
     * @param print_el_fn Function to print a single element
     */
    void (* print)( struct Vector * v, void (* print_el_fn( void * ) ) );

} Vector;

#endif //CTUNE_DATASTRUCTURE_VECTOR_H
