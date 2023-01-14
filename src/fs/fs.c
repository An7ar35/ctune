#include "fs.h"

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#include "../logger/Logger.h"

/**
 * Get file state
 * @param filename File path/name
 * @param size Pointer to container to set the found file size (only on CTUNE_FILE_FOUND)
 * @return CTUNE_FILE_*
 */
static ctune_FileState_e ctune_fs_getFileState( const char * filename, ssize_t * size ) {
    struct stat file_stat;
    const int err    = stat( filename, &file_stat );
    const int err_no = errno;

    if( err == 0 ) {
        if( size != NULL ) {
            *size = file_stat.st_size;
        }

        return CTUNE_FILE_FOUND;

    } else if( err == -1 && err_no == ENOENT ) {
        return CTUNE_FILE_NOTFOUND;

    } else {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_fs_getFileState( \"%s\" )] Failed to get stats: %s",
                   filename,
                   strerror( err_no )
        );

        return CTUNE_FILE_ERR;
    }
}

/**
 * [PRIVATE] Checks and creates a directory path if absent
 * @param dir_path Directory path to create
 * @return Success/Path exists
 */
static bool ctune_fs_createDirectory( String_t * dir_path ) {
    struct stat st = { 0 };

    if( stat( dir_path->_raw, &st ) == -1 ) {
        CTUNE_LOG( CTUNE_LOG_MSG,
                   "[ctune_fs_createDirectory( \"%s\" )] "
                   "Directory does not exist - creating it.",
                   dir_path->_raw
        );

        if( mkdir( dir_path->_raw, 0700 ) != 0 ) { //read/write/execute for owner
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_fs_createDirectory( \"%s\" )] "
                       "Could not create directory via stat call: %s",
                       dir_path->_raw, strerror( errno )
            );

            String_t cmd = String.init();
            String.set( &cmd, "mkdir -m 0700 -p " );
            String.append_back( &cmd, dir_path->_raw );

            int sys_ret = system( cmd._raw );

            if( sys_ret != 0 ) {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_fs_createDirectory( \"%s\" )] "
                           "Could not create directory via system call: %s",
                           cmd._raw, strerror( errno )
                );
            }

            String.free( &cmd );

            return ( sys_ret == 0 );
        }
    }

    return true;
}

/**
 * Create a copy of a file's content
 * @param src Source filepath
 * @param target Target filepath
 * @param buff_size Size of buffer to use
 * @return Success
 */
static bool ctune_fs_duplicateFile( const char * src, const char * target, size_t buff_size ) {
    bool   error_state      = false;
    FILE * from_fd          = NULL;
    FILE * to_fd            = NULL;
    size_t source_size      = 0;
    size_t read_from_src    = 0;
    size_t writen_to_target = 0;
    char   buffer[buff_size];

    if( ctune_fs.getFileState( src, &source_size ) != CTUNE_FILE_FOUND ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_fs_duplicateFile( \"%s\", \"%s\" )] Failed to get stats for source: %s", src, target, strerror( errno ) );
        goto end;
    }

    if( ( from_fd = fopen( src, "r" ) ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_fs_duplicateFile( \"%s\", \"%s\" )] Failed to open source: %s", src, target, strerror( errno ) );
        error_state = true;
        goto end;
    }

    if( ( to_fd = fopen( target, "w" ) ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_fs_duplicateFile( \"%s\", \"%s\" )] Failed to open target: %s", src, target, strerror( errno ) );
        error_state = true;
        goto end;
    }

    size_t read = 0;

    while( ( read = fread( buffer, 1, sizeof( buffer ), from_fd ) ) > 0 ) {
        read_from_src    += read;
        writen_to_target += fwrite( buffer, 1, read,to_fd );
    }

    if( source_size != writen_to_target ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_fs_duplicateFile( \"%s\", \"%s\" )] Read/Write mismatch: %lu/%lu", src, target, writen_to_target, source_size );
        error_state = true;
    } else {
        CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_fs_duplicateFile( \"%s\", \"%s\" )] Copied %lu/%lu successfully.", src, target, writen_to_target, source_size );
    }

    end:
        if( from_fd ) {
            fclose( from_fd );
        }

        if( to_fd ) {
            fclose( to_fd );
        }

        return !( error_state );
}

/**
 * Reads a file into a String_t object
 * @param file_path File path
 * @param content   String_t object to dump file content into (assumed to be pre-initialised)
 * @return Success
 */
static bool ctune_fs_readFile( const char * file_path, String_t * content ) {
    FILE * file        = fopen( file_path , "r" );
    bool   error_state = false;

    if( file == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_fs_readFile( \"%s\", %p )] Error opening file.", file_path, content );
        return false; //EARLY RETURN
    }

    fseek( file, 0, SEEK_END );
    size_t length = ftell( file );
    size_t read   = 0;
    fseek( file, 0, SEEK_SET );

    char buffer[ (length + 1) ];
    read = fread( &buffer[0], 1, length, file ); //note: ctune input files should be < 4GB so we're fine with this.

    if( read == 0 ) {
        CTUNE_LOG( CTUNE_LOG_WARNING, "[ctune_fs_readFile( \"%s\", %p )] Nothing to read in file.", file_path, content );
        error_state = true;

    } else if( read != length ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_fs_readFile( \"%s\", %p )] Error reading file content.", file_path, content );
        error_state = true;

    } else {
        buffer[ length ] = '\0';
    }

    fclose( file );

    if( !String.set( content, &buffer[0] ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_fs_readFile( \"%s\", %p )] Failed to parse buffer to String_t.", file_path, content );
        error_state = true;
    };

    return !( error_state );
}

/**
 * Namespace definitions
 */
const struct ctune_fs_Namespace ctune_fs = {
    .getFileState    = &ctune_fs_getFileState,
    .createDirectory = &ctune_fs_createDirectory,
    .duplicateFile   = &ctune_fs_duplicateFile,
    .readFile        = &ctune_fs_readFile,
};