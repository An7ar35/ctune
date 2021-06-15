#include "CircularBuffer.h"

#define _GNU_SOURCE

#include <errno.h>
#include <string.h>
#include <limits.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "../logger/Logger.h"

/**
 * Gets a string representation of the error enum val for pthread returns
 * @param i Error enum integer val
 * @return String
 */
static const char * ctune_CircularBuffer_getPThreadErrStr( int i ) {
    switch( i ) {
        case EINVAL:
            return "EINVAL";
        case EBUSY:
            return "EBUSY";
        case EAGAIN:
            return "EAGAIN";
        case EDEADLK:
            return "EDEADLK";
        case EPERM:
            return "EPERM";
        default:
            return "UNKNOWN";
    }
}

/**
 * [PRIVATE] Gets a file descriptor for an anonymous file residing in memory (replica of https://man7.org/linux/man-pages/man2/memfd_create.2.html)
 * @param name  Name of file
 * @param flags Flags
 * @return File descriptor (-1 or error)
 */
static int ctune_CircularBuffer_memfd_create( const char * name, unsigned int flags ) {
    long fd   = syscall( __NR_memfd_create, name, flags );
    int  cast = 0;

    if( fd >= 0 ) {
        if( fd <= INT_MAX ) {
            cast = (int) fd;

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_CircularBuffer_memfd_create( \"%s\", %d )] Failed cast long->int (%l)",
                       name, flags, fd
            );

            cast = -1;
        }
    }

    return cast;
}

/**
 * [PRIVATE] Advance the read position
 * @param cbuff Pointer to CircularBuffer_t object
 * @param n     Number of bytes to advance position by
 */
static void ctune_CircularBuffer_advanceReadPos( CircularBuffer_t * cbuff, size_t n ) {
    cbuff->read_pos = ( ( cbuff->read_pos + n ) % cbuff->size );
}

/**
 * [PRIVATE] Advance the write position
 * @param cbuff Pointer to CircularBuffer_t object
 * @param n     Number of bytes to advance position by
 */
static void ctune_CircularBuffer_advanceWritePos( CircularBuffer_t * cbuff, size_t n ) {
    cbuff->write_pos = ( ( cbuff->write_pos + n ) % cbuff->size );
}

/**
 * Initialises a circular buffer
 * @return Circular buffer object
 */
static CircularBuffer_t ctune_CircularBuffer_create( void ) {
    return (CircularBuffer_t) {
        .mutex     = PTHREAD_MUTEX_INITIALIZER,
        .fd        = 0,
        .buffer    = NULL,
        .size      = 0,
        .read_pos  = 0,
        .write_pos = 0,
    };
}

/**
 * [THREAD-SAFE] Initialises the circular buffer
 * @param cbuff Pointer to CircularBuffer_t object
 * @param size  Required size for buffer
 * @return Success
 */
static bool ctune_CircularBuffer_init( CircularBuffer_t * cbuff, size_t size ) {
    /*
     * raw buffer (fd): [##########]
     *                   |        |
     *                   |<------>| (n * page size)
     *                   |        |
     *  virtual buffer: [##########|##########]
     *                   ^        ^ ^        ^
     *                   0        n 0        n
     *                   |          |
     *                   section 1  section 2
     */
    bool   error_state = false;
    size_t real_size   = size;

    if( cbuff == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_CircularBuffer_init( %p, %lu )] CircularBuffer_t is NULL.",
                   cbuff, size
        );

        error_state = true;
        goto end;
    }

    pthread_mutex_lock( &cbuff->mutex );

    if( size < 1 || size > LONG_MAX ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_CircularBuffer_init( %p, %lu )] Bad size (0 > size =< %lu).",
                   cbuff, size, size, LONG_MAX
        );

        error_state = true;
        goto end;
    }

    { //calculate the actual min size based on the page size
        const size_t whole_pages = ( size / getpagesize() ) + ( size % getpagesize() > 0 ? 1 : 0 );

        real_size = whole_pages * getpagesize();

        CTUNE_LOG( CTUNE_LOG_DEBUG,
                   "[ctune_CircularBuffer_init( %p, %lu )] Calculated size: %lu bytes (detected page size: %lu bytes)",
                   cbuff, size, real_size, getpagesize()
        );
    }

    if( ( cbuff->fd = ctune_CircularBuffer_memfd_create( "circular_buffer", 0 ) ) < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_CircularBuffer_init( %p, %lu )] Failed to create raw buffer file descriptor: %s",
                   cbuff, size, strerror( errno )
        );

        error_state = true;
        goto end;
    }

    if( ftruncate( cbuff->fd, real_size ) < 0 ) { //truncate a file to a specified length
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_CircularBuffer_init( %p, %lu )] Failed to adjust raw buffer size (%lu): %s",
                   cbuff, size, size, strerror( errno )
        );

        error_state = true;
        goto end;
    }

    if( ( cbuff->buffer = mmap( NULL, 2 * real_size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0 ) ) == MAP_FAILED ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_CircularBuffer_init( %p, %lu )] Failed to map raw buffer: %s",
                   cbuff, size, strerror( errno )
        );

        error_state = true;
        goto end;
    }

    if( mmap( cbuff->buffer, real_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, cbuff->fd, 0 ) == MAP_FAILED ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_CircularBuffer_init( %p, %lu )] Failed to map virtual buffer section 1: %s",
                   cbuff, size, strerror( errno )
        );

        error_state = true;
        goto end;
    }

    if( mmap( ( cbuff->buffer + real_size ), real_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, cbuff->fd, 0 ) == MAP_FAILED ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_CircularBuffer_init( %p, %lu )] Failed to map virtual buffer section 2: %s",
                   cbuff, size, strerror( errno )
        );

        error_state = true;
        goto end;
    }

    pthread_cond_init( &cbuff->read_ready, NULL );

    cbuff->size      = real_size;
    cbuff->write_pos = 0;
    cbuff->read_pos  = 0;

    end:
        pthread_mutex_unlock( &cbuff->mutex );
        return !( error_state );
}

/**
 * Writes a chunk to the buffer (no arg checks)
 * @param buffer Pointer to CircularBuffer_t object
 * @param src    Source byte buffer
 * @param length Source length in bytes to copy
 * @return Number or bytes written
 */
static size_t ctune_CircularBuffer_writeChunk( CircularBuffer_t * cbuff, const u_int8_t * src, size_t length ) {
    size_t writen = 0;

    pthread_mutex_lock( &cbuff->mutex );

    if( length <= cbuff->size - ( cbuff->write_pos - cbuff->read_pos ) ) {
        memcpy( &cbuff->buffer[cbuff->write_pos], src, length );
        ctune_CircularBuffer_advanceWritePos( cbuff, length );
        writen += length;

        pthread_cond_signal( &cbuff->read_ready );

    } else {
        CTUNE_LOG( CTUNE_LOG_WARNING,
                   "[ctune_CircularBuffer_writeChunk( %p, %p, %lu )] "
                   "Free space too small (%lu). Consider making the buffer larger (%lu).",
                   cbuff, src, length,
                   ( cbuff->size - ( cbuff->write_pos - cbuff->read_pos ) ), cbuff->size
        );
    }

    pthread_mutex_unlock( &cbuff->mutex );

    return writen;
}

/**
 * Write a byte to the buffer
 * @param cbuff Pointer to CircularBuffer_t object
 * @param byte   Byte to write
 * @return Number of bytes written (0, 1)
 */
static size_t ctune_CircularBuffer_writeByte( CircularBuffer_t * cbuff, const u_int8_t byte ) {
    if( cbuff == NULL || cbuff->write_pos == cbuff->read_pos )
        return 0;

    pthread_mutex_lock( &cbuff->mutex );

    cbuff->buffer[cbuff->write_pos] = byte;
    ctune_CircularBuffer_advanceWritePos( cbuff, 1 );

    pthread_cond_signal( &cbuff->read_ready );
    pthread_mutex_unlock( &cbuff->mutex );

    return 1;
}

/**
 * [THREAD-SAFE] Copy a chunk of bytes to a target buffer
 * @param cbuff  Pointer to CircularBuffer_t object
 * @param target Pointer to target buffer
 * @param size   Number of bytes to copy (<= to target buffer's size)
 * @return Number of bytes copied
 */
static size_t ctune_CircularBuffer_copyChunk( CircularBuffer_t * cbuff, u_int8_t * target, size_t size ) {
    if( cbuff == NULL || target == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_CircularBuffer_copyChunk( %p, %p, %lu )] Arg(s) NULL.", cbuff, target, size );
        return 0; //EARLY RETURN
    }

    size_t bytes_copied = 0;

    pthread_mutex_lock( &cbuff->mutex );

    {
        while( cbuff->read_pos == cbuff->write_pos ) {
            pthread_cond_wait( &cbuff->read_ready, &cbuff->mutex );
        }

        size_t available = ( ( cbuff->read_pos + cbuff->size ) - cbuff->write_pos );
        size_t copy_ln   = ( available < size ? available : size );

        memcpy( &cbuff->buffer[ cbuff->read_pos ], target, copy_ln );

        ctune_CircularBuffer_advanceReadPos( cbuff, copy_ln );
        bytes_copied = copy_ln;
    }

    pthread_mutex_unlock( &cbuff->mutex );

    return bytes_copied;
}

/**
 * [THREAD-SAFE] Locks buffer for read access
 * @param cbuff Pointer to CircularBuffer_t object
 * @return Success
 */
static bool ctune_CircularBuffer_raw_readLock( CircularBuffer_t * cbuff ) {
    if( cbuff == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_CircularBuffer_raw_readLock( %p )] Pointer is NULL.", cbuff );
        return false; //EARLY RETURN
    }

    int ret = pthread_mutex_lock( &cbuff->mutex );

    if( ret != 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_CircularBuffer_raw_readLock( %p )] Failed to lock mutex: %s (%d).",
                   cbuff, ctune_CircularBuffer_getPThreadErrStr( ret ), ret
        );

        return false;
    }

    return true;
}

/**
 * Gets the number of bytes available to read
 * @param cbuff Pointer to CircularBuffer_t object
 * @return Bytes available
 */
static size_t ctune_CircularBuffer_raw_readAvailable( CircularBuffer_t * cbuff ) {
    return ( ( cbuff->read_pos + cbuff->size ) - cbuff->write_pos );
}

/**
 * Get a pointer to the raw buffer at the current read position
 * @param cbuff Pointer to CircularBuffer_t object
 * @return Pointer to circular buffer
 */
static u_int8_t * ctune_CircularBuffer_raw_getReadBuffer( CircularBuffer_t * cbuff ) {
    return &cbuff->buffer[cbuff->read_pos];
}

/**
 * Moves the read position index
 * @param cbuff Pointer to CircularBuffer_t object
 * @param bytes Bytes to advance the read position by
 */
static void ctune_CircularBuffer_raw_advanceReadPosition( CircularBuffer_t * cbuff, size_t bytes ) {
    ctune_CircularBuffer_advanceReadPos( cbuff, bytes );
}

/**
 * [THREAD-SAFE] Unlocks buffer read access
 * @param cbuff Pointer to CircularBuffer_t object
 * @return Success
 */
static bool ctune_CircularBuffer_raw_readUnlock( CircularBuffer_t * cbuff ) {
    if( cbuff == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_CircularBuffer_raw_readLock( %p )] Pointer is NULL." );
        return 0; //EARLY RETURN
    }

    int ret = pthread_mutex_unlock( &cbuff->mutex );

    if( ret != 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_CircularBuffer_raw_readUnlock( %p )] Failed to unlock mutex: %s (%d).",
                   cbuff, ctune_CircularBuffer_getPThreadErrStr( ret ), ret
        );

        return false;
    }

    return true;
}

/**
 * [THREAD-SAFE] Checks if the buffer is empty
 * @param cbuff Pointer to CircularBuffer_t object
 * @return Empty state
 */
static bool ctune_CircularBuffer_empty( CircularBuffer_t * cbuff ) {
    bool empty = true;

    if( cbuff != NULL ) {
        pthread_mutex_lock( &cbuff->mutex );
        empty = ( cbuff->read_pos == cbuff->write_pos );
        pthread_mutex_unlock( &cbuff->mutex );
    }

    return empty;
}

/**
 * [THREAD-SAFE] Gets the current buffer size
 * @param cbuff Pointer to CircularBuffer_t object
 * @return Current buffer size in bytes
 */
static size_t ctune_CircularBuffer_size( CircularBuffer_t * cbuff ) {
    size_t size = 0;

    if( cbuff != NULL ) {
        pthread_mutex_lock( &cbuff->mutex );
        size = cbuff->size;
        pthread_mutex_unlock( &cbuff->mutex );

    } else {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_CircularBuffer_clear( %p )] CircularBuffer_t is NULL.", cbuff );
    }

    return size;
}


/**
 * [THREAD-SAFE] Clears the buffer and resets positions
 * @param cbuff Pointer to CircularBuffer_t object
 */
static void ctune_CircularBuffer_clear( CircularBuffer_t * cbuff ) {
    if( cbuff == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_CircularBuffer_clear( %p )] CircularBuffer_t is NULL.", cbuff );
        return; //EARLY RETURN
    }

    pthread_mutex_lock( &cbuff->mutex );

    cbuff->read_pos  = 0;
    cbuff->write_pos = 0;

    pthread_mutex_unlock( &cbuff->mutex );
}

/**
 * Frees buffer content
 * @param cbuff Pointer to CircularBuffer_t object
 */
static void ctune_CircularBuffer_free( CircularBuffer_t * cbuff ) {
    if( cbuff != NULL ) {
        if( cbuff->buffer != NULL && munmap( cbuff->buffer + cbuff->size, cbuff->size ) != 0 ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_CircularBuffer_free( %p )] Failed unmap virtual buffer 2: %s",
                       cbuff, strerror( errno )
            );
        }

        if( cbuff->buffer != NULL && munmap( cbuff->buffer, cbuff->size ) != 0 ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_CircularBuffer_free( %p )] Failed unmap virtual buffer 1: %s",
                       cbuff, strerror( errno )
            );
        }

        if( cbuff->buffer != NULL && close( cbuff->fd ) != 0 ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_CircularBuffer_free( %p )] Failed close file descriptor: %s",
                       cbuff, strerror( errno )
            );
        }

        pthread_cond_destroy( &cbuff->read_ready );
        pthread_mutex_destroy( &cbuff->mutex );
        cbuff->fd        = 0;
        cbuff->buffer    = NULL;
        cbuff->read_pos  = 0;
        cbuff->write_pos = 0;
    }
}

/**
 * Namespace constructor
 */
const struct ctune_CircularBuffer_Namespace CircularBuffer = {
    .create         = &ctune_CircularBuffer_create,
    .init           = &ctune_CircularBuffer_init,
    .writeChunk     = &ctune_CircularBuffer_writeChunk,
    .writeByte      = &ctune_CircularBuffer_writeByte,
    .copyChunk      = &ctune_CircularBuffer_copyChunk,

    .raw = {
        .readLock            = &ctune_CircularBuffer_raw_readLock,
        .readAvailable       = &ctune_CircularBuffer_raw_readAvailable,
        .getReadBuffer       = &ctune_CircularBuffer_raw_getReadBuffer,
        .advanceReadPosition = &ctune_CircularBuffer_raw_advanceReadPosition,
        .readUnlock          = &ctune_CircularBuffer_raw_readUnlock,
    },

    .empty          = &ctune_CircularBuffer_empty,
    .size           = &ctune_CircularBuffer_size,
    .clear          = &ctune_CircularBuffer_clear,
    .free           = &ctune_CircularBuffer_free,
};