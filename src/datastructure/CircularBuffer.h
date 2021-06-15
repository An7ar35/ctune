#ifndef CTUNE_DATASTRUCTURE_CIRCULARBUFFER_H
#define CTUNE_DATASTRUCTURE_CIRCULARBUFFER_H

#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

/**
 * CircularBuffer object
 * @param mutex      Mutex for read/write locks
 * @param read_cond  Read access condition
 * @param write_cond Write access condition
 * @param buffer     Raw buffer
 * @param fd         File descriptor for the virtual buffer
 * @param size       Total size of the buffer
 * @param read_pos   Index of the read head
 * @param write_pos  Index of the write head
 */
typedef struct ctune_CircularBuffer {
    pthread_mutex_t mutex;
    pthread_cond_t  read_ready;
    int             fd;
    u_int8_t      * buffer;
    size_t          size;
    size_t          read_pos;
    size_t          write_pos;

} CircularBuffer_t;

/**
 * CircularBuffer namespace
 */
extern const struct ctune_CircularBuffer_Namespace {
    /**
     * Initialises a circular buffer
     * @return Circular buffer object
     */
    CircularBuffer_t (* create)( void );

    /**
     * [THREAD-SAFE] Initialises the circular buffer
     * @param cbuff Pointer to CircularBuffer_t object
     * @param size  Required size for buffer
     * @return Success
     */
    bool (* init)( CircularBuffer_t * cbuff, size_t size );

    /**
     * [THREAD-SAFE] Writes a chunk to the buffer
     * @param cbuff  Pointer to CircularBuffer_t object
     * @param src    Source byte buffer
     * @param length Source length in bytes to copy
     * @return Number or bytes written
     */
    size_t (* writeChunk)( CircularBuffer_t * cbuff, const u_int8_t * src, size_t length );

    /**
     * [THREAD-SAFE] Write a byte to the buffer
     * @param cbuff Pointer to CircularBuffer_t object
     * @param byte  Byte to write
     * @return Number of bytes written (0, 1)
     */
    size_t (* writeByte)( CircularBuffer_t * cbuff, const u_int8_t byte );

    /**
     * [THREAD-SAFE] Copy a chunk of bytes to a target buffer
     * @param cbuff  Pointer to CircularBuffer_t object
     * @param target Pointer to target buffer
     * @param size   Number of bytes to copy (<= to target buffer's size)
     * @return Number of bytes copied
     */
    size_t (* copyChunk)( CircularBuffer_t * cbuff, u_int8_t * target, size_t size );

    /**
     * Raw read access API
     * -
     * Remember to lock prior and unlock post use
     */
    struct {
        /**
         * [THREAD-SAFE] Locks buffer for read access
         * @param cbuff Pointer to CircularBuffer_t object
         * @return Success
         */
        bool (* readLock)( CircularBuffer_t * cbuff );

        /**
         * Gets the number of bytes available to read
         * @param cbuff Pointer to CircularBuffer_t object
         * @return Bytes available
         */
        size_t (* readAvailable)( CircularBuffer_t * cbuff );

        /**
         * Get a pointer to the raw buffer at the current read position
         * @param cbuff Pointer to CircularBuffer_t object
         * @return Pointer to circular buffer
         */
        u_int8_t * (* getReadBuffer)( CircularBuffer_t * cbuff );

        /**
         * Moves the read position index
         * @param cbuff Pointer to CircularBuffer_t object
         * @param bytes Bytes to advance the read position by
         */
        void (* advanceReadPosition)( CircularBuffer_t * cbuff, size_t bytes );

        /**
         * [THREAD-SAFE] Unlocks buffer read access
         * @param cbuff Pointer to CircularBuffer_t object
         * @return Success
         */
        bool (* readUnlock)( CircularBuffer_t * cbuff );
    } raw;

    /**
     * [THREAD-SAFE] Checks if the buffer is empty
     * @param cbuff Pointer to CircularBuffer_t object
     * @return Empty state
     */
    bool (* empty)( CircularBuffer_t * cbuff );

    /**
     * [THREAD-SAFE] Gets the current buffer size
     * @param cbuff Pointer to CircularBuffer_t object
     * @return Current buffer size in bytes
     */
    size_t (* size)( CircularBuffer_t * cbuff );

    /**
     * [THREAD-SAFE] Clears the buffer and resets positions
     * @param cbuff Pointer to CircularBuffer_t object
     */
    void (* clear)( CircularBuffer_t * cbuff );

    /**
     * Frees buffer content
     * @param cbuff Pointer to CircularBuffer_t object
     */
    void (* free)( CircularBuffer_t * cbuff );

} CircularBuffer;

#endif //CTUNE_DATASTRUCTURE_CIRCULARBUFFER_H