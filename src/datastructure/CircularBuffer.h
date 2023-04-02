#ifndef CTUNE_DATASTRUCTURE_CIRCULARBUFFER_H
#define CTUNE_DATASTRUCTURE_CIRCULARBUFFER_H

//TODO circ. buffer with variable width lined up on mem page

#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdatomic.h>

/**
 * CircularBuffer object
 * @param auto_grow  Flag for auto-grow the buffer when it gets full
 * @param mutex      Mutex for read/write locks
 * @param ready      Read access condition
 * @param empty      Empty state of the buffer
 * @param position   Read/Write positions
 * @param fd         File descriptor for the virtual buffer
 * @param buffer     Raw buffer
 * @param size       Total size of the buffer
 */
typedef struct CircularBuffer {
    bool            auto_grow;
    pthread_mutex_t mutex;
    pthread_cond_t  ready;
    atomic_bool     empty;

    struct {
        size_t read;
        size_t write;
    } position;

    int             fd;
    u_int8_t      * buffer;
    size_t          size;

} CircularBuffer_t;

/**
 * CircularBuffer namespace
 */
extern const struct CircularBuffer_Namespace {
    /**
     * Initialises a circular buffer
     * @return Circular buffer object
     */
    CircularBuffer_t (* create)( void );

    /**
     * Initialises the circular buffer
     * @param buffer    Pointer to CircularBuffer_t object
     * @param size      Required size for buffer
     * @param auto_grow Flag to automatically resize the buffer when it gets full
     * @return Success
     */
    bool (* init)( CircularBuffer_t * buffer, size_t size, bool auto_grow );

    /**
     * [THREAD-SAFE] Writes a chunk to the buffer
     * @param buffer  Pointer to CircularBuffer_t object
     * @param src     Source byte buffer
     * @param length  Source length in bytes to copy
     * @return Number or bytes written
     */
    size_t (* writeChunk)( CircularBuffer_t * buffer, const u_int8_t * src, size_t length );

    /**
     * [THREAD-SAFE] Reads a chunk and copies to a buffer
     * @param buffer  Pointer to CircularBuffer_t object
     * @param target Target buffer
     * @param length Length to read and transfer to buffer
     * @return Actual length read
     */
    size_t (* readChunk)( CircularBuffer_t * buffer, u_int8_t * target, size_t length );

    /**
     * [THREAD-SAFE] Gets the current buffer size
     * @param buffer Pointer to CircularBuffer_t object
     * @return Current buffer size in bytes
     */
    size_t (* size)( CircularBuffer_t * buffer );

    /**
     * [TREAD-SAFE] Gets the empty state of the buffer
     * @param buffer Pointer to CircularBuffer_t object
     * @return Empty state
     */
    bool (* empty)( CircularBuffer_t * buffer );

    /**
     * Frees buffer content
     * @param buffer Pointer to CircularBuffer_t object
     */
    void (* free)( CircularBuffer_t * buffer );

} CircularBuffer;

#endif //CTUNE_DATASTRUCTURE_CIRCULARBUFFER_H
