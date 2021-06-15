#include "PlaybackLog.h"

#include <stdio.h>
#include <wchar.h>

#include "../logger/Logger.h"

static FILE * playback_log = NULL;

/**
 * Opens the log file for writing
 * @param file_path      File path
 * @param overwrite_flag Flag for overwriting the file (true=overwrite, false=append)
 * @return Success
 */
static bool ctune_PlaybackLog_open( const char * file_path, bool overwrite_flag ) {
    if( playback_log != NULL) {
        CTUNE_LOG( CTUNE_LOG_WARNING, "[ctune_PlaybackLog_open( \"%s\", %i )] Playback log file is already opened.", file_path, overwrite_flag );
        return false;
    }

    return ( playback_log = fopen( file_path , ( overwrite_flag ? "w" :  "a" ) ) ) != NULL;
}

/**
 * Gets the underlining file descriptor for the log
 * @return Internal FILE pointer
 */
static FILE * ctune_PlaybackLog_output() {
    return playback_log;
}

/**
 * Closes log file descriptor
 */
static void ctune_PlaybackLog_close() {
    fflush( playback_log );

    if( playback_log != NULL ) {
        fclose( playback_log );
        playback_log = NULL;
    }
}

/**
 * Writes string to log
 * @param str String to write
 */
static void ctune_PlaybackLog_write( const char * str ) {
    if( playback_log == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_PlaybackLog_write( %p )] Playback log file is not opened (null).", str );
        return;
    }

    fprintf( playback_log, "%s", str );
    fflush( playback_log );
}

/**
 * Write string and append newline after it
 * @param str String to write
 */
static void ctune_PlaybackLog_writeln( const char * str ) {
    if( playback_log == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_PlaybackLog_writeln( %p )] Playback log file is not opened (null).", str );
        return;
    }

    fprintf( playback_log, "%s\n", str );
    fflush( playback_log );
}

/**
 * Write String_t to log
 * @param string String_t to write
 */
static void ctune_PlaybackLog_writeString( const String_t * string ) {
    if( playback_log == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_PlaybackLog_writeString( %p )] Playback log file is not opened (null).", string );
        return;
    }

    if( string == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_PlaybackLog_writeString( %p )] String is NULL.", string );
        return;
    }

    fprintf( playback_log, "%s", string->_raw );
    fflush( playback_log );
}

/**
 * Instance constructor
 */
const struct ctune_PlaybackLog_Instance ctune_PlaybackLog = {
    .open        = &ctune_PlaybackLog_open,
    .output      = &ctune_PlaybackLog_output,
    .close       = &ctune_PlaybackLog_close,
    .write       = &ctune_PlaybackLog_write,
    .writeln     = &ctune_PlaybackLog_writeln,
    .writeString = &ctune_PlaybackLog_writeString,
};