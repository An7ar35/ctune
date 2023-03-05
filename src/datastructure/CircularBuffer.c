#include "CircularBuffer.h"

#include <errno.h>
#include <string.h>
#include <limits.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "logger/src/Logger.h"

/**
 * [PRIVATE] Gets a string representation of the error enum val for pthread returns
 * @param i Error enum integer val
 * @return String
 */
static const char * CircularBuffer_getPThreadErrStr( int i ) {
    switch( i ) {
        case EINVAL : return "EINVAL";
        case EBUSY  : return "EBUSY";
        case EAGAIN : return "EAGAIN";
        case EDEADLK: return "EDEADLK";
        case EPERM  : return "EPERM";
        default     : return "UNKNOWN";
    }
}

/**
 * [PRIVATE] Gets a file descriptor for an anonymous file residing in memory (replica of https://man7.org/linux/man-pages/man2/memfd_create.2.html)
 * @param name  Name of file
 * @param flags Flags
 * @return File descriptor (-1 or error)
 */
static int CircularBuffer_memfd_create( const char * name, unsigned int flags ) {
    long fd   = syscall( __NR_memfd_create, name, flags );
    int  cast = 0;

    if( fd >= 0 ) {
        if( fd <= INT_MAX ) {
            cast = (int) fd;

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[CircularBuffer_memfd_create( \"%s\", %d )] Failed cast long->int (%ld)",
                       name, flags, fd
            );

            cast = -1;
        }
    }

    return cast;
}

/**
 * [PRIVATE] Frees the buffer
 * @param fd     Pointer to file descriptor variable to set
 * @param buffer Pointer to byte buffer pointer to set
 * @param size   Real size of page aligned byte buffer
 */
static void CircularBuffer_freeBuffer( int * fd, u_int8_t ** buffer, size_t size ) {
    if( (*buffer) != NULL && munmap( (*buffer) + size, size ) != 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[CircularBuffer_freeBuffer( %p, %p, %lu )] Failed unmap virtual buffer 2: %s",
                   fd, buffer, size, strerror( errno )
        );
    }

    if( (*buffer) != NULL && munmap( (*buffer), size ) != 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[CircularBuffer_freeBuffer( %p, %p, %lu )] Failed unmap virtual buffer 1: %s",
                   fd, buffer, size, strerror( errno )
        );
    }

    if( (*buffer) != NULL && close( (*fd) ) != 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[CircularBuffer_freeBuffer( %p, %p, %lu )] Failed close file descriptor: %s",
                   fd, buffer, size, strerror( errno )
        );
    }
}

/**
 * [PRIVATE] Creates the circular buffer in memory
 * @param size      Requested minimum buffer size
 * @param fd        Pointer to file descriptor variable to set
 * @param buffer    Pointer to byte buffer pointer to set
 * @param real_size Pointer to actual page-aligned buffer size variable to set
 * @return Success
 */
static bool CircularBuffer_createBuffer( size_t size, int * fd, u_int8_t ** buffer, size_t * real_size ) {
    bool error_state = false;

    { //calculate the actual min size based on the page size
        const size_t whole_pages = ( size / getpagesize() ) + ( size % getpagesize() > 0 ? 1 : 0 );

        (*real_size) = whole_pages * getpagesize();

        CTUNE_LOG( CTUNE_LOG_DEBUG,
                   "[CircularBuffer_createBuffer( %lu, %p, %p, %p )] "
                   "Calculated size: %lu bytes (detected page size: %lu bytes)",
                   size, fd, buffer, real_size, (*real_size), getpagesize()
        );
    }

    if( ( (*fd) = CircularBuffer_memfd_create( "circular_buffer", 0 ) ) < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[CircularBuffer_createBuffer( %lu, %p, %p, %p )] Failed to create raw buffer file descriptor: %s",
                   size, fd, buffer, real_size, strerror( errno )
        );

        error_state = true;
        goto end;
    }

    if( ftruncate( (*fd), (*real_size) ) < 0 ) { //truncate a file to a specified length
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[CircularBuffer_createBuffer( %lu, %p, %p, %p )] Failed to adjust raw buffer size (%lu): %s",
                   size, fd, buffer, real_size, size, strerror( errno )
        );

        error_state = true;
        goto end;
    }

    if( ( (*buffer) = mmap( NULL, 2 * (*real_size), PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0 ) ) == MAP_FAILED ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[CircularBuffer_createBuffer( %lu, %p, %p, %p )] Failed to map raw buffer: %s",
                   size, fd, buffer, real_size, strerror( errno )
        );

        error_state = true;
        goto end;
    }

    if( mmap( (*buffer), (*real_size), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, (*fd), 0 ) == MAP_FAILED ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[CircularBuffer_createBuffer( %lu, %p, %p, %p )] Failed to map virtual buffer section 1: %s",
                   size, fd, buffer, real_size, strerror( errno )
        );

        error_state = true;
        goto end;
    }

    if( mmap( ( (*buffer) + (*real_size) ), (*real_size), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, (*fd), 0 ) == MAP_FAILED ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[CircularBuffer_createBuffer( %lu, %p, %p, %p )] Failed to map virtual buffer section 2: %s",
                   size, fd, buffer, real_size, strerror( errno )
        );

        error_state = true;
        goto end;
    }

    end:
        return !( error_state );
}

/**
 * [PRIVATE] Advance the read position
 * @param buffer Pointer to CircularBuffer_t object
 * @param n      Number of bytes to advance position by
 */
static void CircularBuffer_advanceReadPos( CircularBuffer_t * buffer, size_t n ) {
    buffer->position.read = ( ( buffer->position.read + n ) % buffer->size );

    if( buffer->position.read == buffer->position.write )
        buffer->empty = true;
}

/**
 * [PRIVATE] Advance the write position
 * @param buffer Pointer to CircularBuffer_t object
 * @param n      Number of bytes to advance position by
 */
static void CircularBuffer_advanceWritePos( CircularBuffer_t * buffer, size_t n ) {
    buffer->position.write = ( ( buffer->position.write + n ) % buffer->size );

    if( n )
        buffer->empty = false;
}

/**
 * [PRIVATE] Gets the number of bytes currently occupied in the buffer
 * @param buffer Pointer to CircularBuffer_t object
 * @return Number of data bytes
 */
static size_t CircularBuffer_dataBytes( CircularBuffer_t * buffer ) {
    return ( ( buffer->position.write + buffer->size ) - buffer->position.read ) % buffer->size;
}

/**
 * [PRIVATE] Gets the number of bytes currently available in the buffer
 * @param buffer Pointer to CircularBuffer_t object
 * @return Number of free bytes
 */
static size_t CircularBuffer_freeBytes( CircularBuffer_t * buffer ) {
    if( buffer->empty ) {
        return buffer->size;
    } else {
        return ( ( buffer->position.read + buffer->size - buffer->position.write ) % buffer->size );
    }
}

/**
 * Initialises a circular buffer
 * @return Circular buffer object
 */
static CircularBuffer_t CircularBuffer_create( void ) {
    return (CircularBuffer_t) {
        .fd          = 0,
        .buffer      = NULL,
        .size        = 0,
        .mutex       = PTHREAD_MUTEX_INITIALIZER,
        .ready       = PTHREAD_COND_INITIALIZER,
        .empty       = true,
        .position    = { 0, 0 },
    };
}

/**
 * [THREAD-SAFE] Initialises the circular buffer
 * @param buffer    Pointer to CircularBuffer_t object
 * @param size      Required size for buffer
 * @param auto_grow Flag to automatically resize the buffer when it gets full
 * @return Success
 */
static bool CircularBuffer_init( CircularBuffer_t * buffer, size_t size, bool auto_grow ) {
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

    if( buffer == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[CircularBuffer_init( %p, %lu, %s )] CircularBuffer_t is NULL.",
                   buffer, size, ( auto_grow ? "true" : "false" )
        );

        error_state = true;
        goto end;
    }

    pthread_mutex_lock( &buffer->mutex );

    if( !CircularBuffer_createBuffer( size, &buffer->fd, &buffer->buffer, &real_size ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[CircularBuffer_init( %p, %lu, %s )] Failed to create a page-aligned buffer in memory",
                   buffer, size, ( auto_grow ? "true" : "false" )
        );

        CircularBuffer_freeBuffer( &buffer->fd, &buffer->buffer, real_size );
        error_state = true;
        goto end;
    }

    buffer->auto_grow      = auto_grow;
    buffer->size           = real_size;
    buffer->position.write = 0;
    buffer->position.read  = 0;

    end:
        pthread_mutex_unlock( &buffer->mutex );
        return !( error_state );
}

/**
 * Writes a chunk to the buffer (no arg checks)
 * @param buffer Pointer to CircularBuffer_t object
 * @param src    Source byte buffer
 * @param length Source length in bytes to copy
 * @return Number or bytes written
 */
static size_t CircularBuffer_writeChunk( CircularBuffer_t * buffer, const u_int8_t * src, size_t length ) {
    size_t bytes_writen = 0;

    pthread_mutex_lock( &buffer->mutex );

    size_t free_bytes = CircularBuffer_freeBytes( buffer );

    if( length > free_bytes ) {
        if( !buffer->auto_grow ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[CircularBuffer_writeChunk( %p, %p, %lu )] "
                       "Free space too small (%lu). Consider making the buffer larger (%lu).",
                       buffer, src, length,
                       free_bytes, buffer->size
            );

            return 0; //ErARLY RETURN
        }

        CTUNE_LOG( CTUNE_LOG_DEBUG,
                   "[CircularBuffer_writeChunk( %p, %p, %lu )] "
                   "Buffer needs to grow (available: %luB, required: %luB)",
                   buffer, src, length, free_bytes, length
        );

        int        new_fd        = -1;
        u_int8_t * new_buff      = NULL;
        size_t     new_size      = buffer->size + ( length - free_bytes );
        size_t     new_real_size = new_size;

        if( !CircularBuffer_createBuffer( new_size, &new_fd, &new_buff, &new_real_size ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[CircularBuffer_writeChunk( %p, %p, %lu )] "
                       "Failed to grow buffer (not enough free space (available: %luB)",
                       buffer, src, length, free_bytes
            );

            CircularBuffer_freeBuffer( &new_fd, &new_buff, new_real_size );
            return 0; //EARLY RETURN
        }

        const size_t bytes_to_copy = CircularBuffer_dataBytes( buffer );

        memcpy( &new_buff[0], &buffer->buffer[buffer->position.read], bytes_to_copy );

        CircularBuffer_freeBuffer( &buffer->fd, &buffer->buffer, buffer->size );

        buffer->fd             = new_fd;
        buffer->buffer         = new_buff;
        buffer->size           = new_real_size;
        buffer->position.read  = 0;
        buffer->position.write = bytes_to_copy;
    }

    memcpy( &buffer->buffer[buffer->position.write], src, length );
    CircularBuffer_advanceWritePos( buffer, length );
    bytes_writen = length;

    if( length > 0 ) {
        pthread_cond_signal( &buffer->ready );
    }

    pthread_mutex_unlock( &buffer->mutex );

    return bytes_writen;
}

/**
 * [THREAD-SAFE] Reads a chunk and copies to a buffer
 * @param buffer  Pointer to CircularBuffer_t object
 * @param target Target buffer
 * @param length Length to read and transfer to buffer
 * @return Actual length read
 */
static size_t CircularBuffer_readChunk( CircularBuffer_t * buffer, u_int8_t * target, size_t length ) {
    if( buffer == NULL || target == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[CircularBuffer_readChunk( %p, %p, %lu )] Pointer arg is NULL.",
                   buffer, target, length
        );

        return 0; //EARLY RETURN
    }

    int    ret        = 0;
    size_t bytes_read = 0;

    if( ( ret = pthread_mutex_lock( &buffer->mutex ) ) == 0 ) {
        while( buffer->empty ) {
            pthread_cond_wait( &buffer->ready, &buffer->mutex );
        }

        size_t bytes_available = CircularBuffer_dataBytes( buffer );

        bytes_read = ( bytes_available < length ? bytes_available : length );
        memcpy( target, &buffer->buffer[buffer->position.read], bytes_read );
        CircularBuffer_advanceReadPos( buffer, bytes_read );

        if( ( ret = pthread_mutex_unlock( &buffer->mutex ) ) != 0 ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[CircularBuffer_readChunk( %p, %p, %lu )] Failed to unlock mutex: %s (%d).",
                       buffer, target, length, CircularBuffer_getPThreadErrStr( ret ), ret
            );
        }

    } else {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[CircularBuffer_readChunk( %p, %p, %lu )] Failed to lock mutex: %s (%d).",
                   buffer, target, length, CircularBuffer_getPThreadErrStr( ret ), ret
        );
    }

    return bytes_read;
}

/**
 * [THREAD-SAFE] Checks if the buffer is empty
 * @param buffer Pointer to CircularBuffer_t object
 * @return Empty state
 */
static bool CircularBuffer_empty( CircularBuffer_t * buffer ) {
    bool empty = true;

    if( buffer != NULL ) {
        pthread_mutex_lock( &buffer->mutex );
        empty = buffer->empty;
        pthread_mutex_unlock( &buffer->mutex );
    }

    return empty;
}

/**
 * Gets the current buffer size
 * @param buffer Pointer to CircularBuffer_t object
 * @return Current buffer size in bytes
 */
static size_t CircularBuffer_size( CircularBuffer_t * buffer ) {
    size_t size = 0;

    if( buffer != NULL ) {
        size = buffer->size;

    } else {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[CircularBuffer_clear( %p )] CircularBuffer_t is NULL.",
                   buffer
        );
    }

    return size;
}

/**
 * Frees buffer content
 * @param buffer Pointer to CircularBuffer_t object
 */
static void CircularBuffer_free( CircularBuffer_t * buffer ) {
    if( buffer != NULL ) {
        CircularBuffer_freeBuffer( &buffer->fd, &buffer->buffer, buffer->size );
        pthread_mutex_destroy( &buffer->mutex );
        pthread_cond_destroy( &buffer->ready );
        buffer->fd             = 0;
        buffer->buffer         = NULL;
        buffer->position.read  = 0;
        buffer->position.write = 0;
    }
}

/**
 * Namespace constructor
 */
const struct CircularBuffer_Namespace CircularBuffer = {
    .create     = &CircularBuffer_create,
    .init       = &CircularBuffer_init,
    .writeChunk = &CircularBuffer_writeChunk,
    .readChunk  = &CircularBuffer_readChunk,
    .size       = &CircularBuffer_size,
    .empty      = &CircularBuffer_empty,
    .free       = &CircularBuffer_free,
};