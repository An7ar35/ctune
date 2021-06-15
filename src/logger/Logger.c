#include "Logger.h"

#include <time.h>
#include <stdarg.h>
#include <syslog.h>
#include <wchar.h>

#include "LogQueue.h"
#include "LogWriter.h"
#include "project_version.h"
#include "../ctune_err.h"
#include "../utils/Timeout.h"
#include "../datastructure/String.h"

#define CTUNE_LOGGER_MSG_MAX_SIZE 512

static const char * log_level[] = {
    "|EMERGENCY| ", //0 (not implemented)
    "|--FATAL--| ", //1
    "|-CRITICAL| ", //2 (not implemented)
    "|--ERROR--| ", //3
    "|-WARNING-| ", //4
    "|-MESSAGE-| ", //5
    "|-NOTICE--| ", //6 (not implemented)
    "|--DEBUG--| ", //7
    "|--TRACE--| "  //8 (not in the syslog protocol - RFC.2454)
};

static struct Logger {
    bool                init_success;
    enum ctune_LogLevel level;
    int                 timeout_val;
    pthread_mutex_t     mutex;
    LogQueue_t          message_queue;

} logger = {
    .init_success  = false,
    .level         = CTUNE_LOG_DEBUG, //default level
    .timeout_val   = 5, //in seconds
    .mutex         = PTHREAD_MUTEX_INITIALIZER,
    .message_queue = {
        ._mutex = PTHREAD_MUTEX_INITIALIZER,
        ._front = NULL,
        ._back  = NULL,
    }
};

/**
 * [PRIVATE] Append date/time (UTC) to the end of a string
 * @param str String to append
 */
static void ctune_Logger_getTime( String_t * str ) {
    //Uses the POSIX function `clock_gettime(CLOCK_REALTIME, ts)`
    //https://pubs.opengroup.org/onlinepubs/9699919799/functions/clock_getres.html
    struct timespec ts;
    clock_gettime( CLOCK_REALTIME, &ts );

    char date_time1[128];
    strftime( date_time1, sizeof date_time1, "%F %T", gmtime( &ts.tv_sec ) );
    char date_time2[255];
    snprintf( date_time2, 255, "%s.%09ld ", date_time1, ts.tv_nsec );

    String.append_back( str, date_time2 );
}

/**
 * Initialises the logger
 * @param log_filepath Output file path
 * @param file_mode    File mode (see `fopen(..)` docs) for the output file
 * @param level        Minimal log level to process
 * @return Init success (if false all messages will be piped to syslog)
 */
static bool ctune_Logger_init( const char * log_filepath, const char * file_mode, enum ctune_LogLevel level ) {
    openlog( CTUNE_APPNAME, LOG_PID, LOG_USER ); //syslog

    bool error_state = false;

    pthread_mutex_lock( &logger.mutex );

    logger.level = level;

    ctune_LogQueue.setSendReadySignalCallback( &logger.message_queue, ctune_LogWriter.resume );

    if( !ctune_LogWriter.start( log_filepath, file_mode, &logger.message_queue ) ) {
        syslog( LOG_ERR, "[ctune_Logger_init( \"%s\", \"%s\", %i )] Failed to start the LogWriter.", log_filepath, file_mode, (int) level );
        error_state = true;
    }

    pthread_mutex_unlock( &logger.mutex );

    logger.init_success = !(error_state);
    return logger.init_success;
}

/**
 * Terminate logger and closes output file
 */
static void ctune_Logger_close() {
    pthread_mutex_lock( &logger.mutex );

    ctune_Timeout_t timer = ctune_Timeout.init( logger.timeout_val, CTUNE_ERR_LOG_FLUSH_TIMEOUT, ctune_err.set );

    while( !ctune_LogQueue.empty( &logger.message_queue ) && !ctune_Timeout.timedOut( &timer ) ); //wait for the queue to clear

    if( !ctune_LogQueue.empty( &logger.message_queue ) ) {
        syslog( LOG_ERR, "[ctune_Logger_close()] LogQueue flushing timed-out (%ld items left).", ctune_LogQueue.size( &logger.message_queue ) );
    }

    int ret = ctune_LogWriter.stop();

    if( ret != CTUNE_ERR_NONE ) {
        syslog( LOG_ERR, "[ctune_Logger_close()] LogWriter.stop() returned an error: (%i) %s.", ret, ctune_err.print( ret ) );
    }

    ctune_LogQueue.freeLogQueue( &logger.message_queue );

    pthread_mutex_unlock( &logger.mutex );

    closelog(); //syslog
}

/**
 * Sends a message to the log
 * @param lvl      Message log level
 * @param format   String format (similar to `printf`)
 * @param ...      Arguments (similar to `printf`)
 */
void ctune_Logger_log( enum ctune_LogLevel lvl, const char * format, ... ) {
    if( lvl > logger.level )
        return; //discard

    int      ret = 0;
    String_t str = String.init();

    ctune_Logger_getTime( &str );

    va_list args, args_cp;
    va_start( args, format );

    va_copy( args_cp, args );
    //reason for copy of args: https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/stdarg.h.html
    size_t req_size  = vsnprintf( NULL, 0, format, args_cp ) + 1; //OK with C99, not with older versions!
    size_t buff_size = ( CTUNE_LOGGER_MSG_MAX_SIZE < req_size ? req_size : CTUNE_LOGGER_MSG_MAX_SIZE );
    va_end( args_cp );

    char buffer[buff_size];

    if( ( ret = vsprintf( buffer, format, args ) ) < 0 ) {
        syslog( LOG_ERR,
                "[ctune_Logger_log( %i, \"%s\", ... )] "
                "Could not parse into string buffer (ret=%i).",
                (int) lvl, format, ret
        );

        goto end;
    }

    String.append_back( &str, log_level[(int) lvl] );
    String.append_back( &str, buffer );

    if( logger.init_success ) {
        ctune_LogQueue.enqueue( &logger.message_queue, str._raw );

    } else { //syslog
        if( lvl < CTUNE_LOG_TRACE ) //lvl>=8 not supported by syslog
           syslog( lvl, "%s", buffer );
    }

    end:
        va_end( args );
        String.free( &str );
}

/**
 * Sends a message to the log
 * @param lvl      Message log level
 * @param filename Source file name
 * @param line_num Line number in source file
 * @param format   String format (similar to `printf`)
 * @param ...      Arguments (similar to `printf`)
 */
void ctune_Logger_logDBG( enum ctune_LogLevel lvl, char * filename, int line_num, const char * format, ... ) {
    static const char * file_info_format = "(%s:%i) ";

    if( lvl > logger.level )
        return; //discard

    int      ret = 0;
    String_t str = String.init();

    ctune_Logger_getTime( &str );

    String.append_back( &str, log_level[(int) lvl] );

    if( filename != NULL ) {
        size_t req_size  = snprintf( NULL, 0, file_info_format, filename, line_num ); //OK with C99, not with older versions!
        size_t buff_size = ( CTUNE_LOGGER_MSG_MAX_SIZE < req_size ? req_size : CTUNE_LOGGER_MSG_MAX_SIZE ) + 1;

        char buffer[buff_size];
        size_t written = snprintf( buffer, ( buff_size - 1 ), file_info_format, filename, line_num );

        if( written > 0 && written < buff_size ) {
            String.append_back( &str, buffer );

        } else { //should not happen but just in case...
            syslog( LOG_ERR,
                    "[ctune_Logger_logDBG( %i, %s, %i, \"%s\", ... )] "
                    "Could not parse filename:line_no into string buffer (ret=%zu).",
                    (int) lvl, filename, line_num, format, written
            );
        }
    }

    va_list args, args_cp;
    va_start( args, format );

    va_copy( args_cp, args );
    //reason for copy of args: https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/stdarg.h.html
    size_t req_size  = vsnprintf( NULL, 0, format, args_cp ) + 1; //OK with C99, not with older versions!
    size_t buff_size = ( CTUNE_LOGGER_MSG_MAX_SIZE < req_size ? req_size : CTUNE_LOGGER_MSG_MAX_SIZE );
    va_end( args_cp );

    char buffer[buff_size];

    if( ( ret = vsprintf( buffer, format, args ) ) < 0 ) {
        syslog( LOG_ERR,
                "[ctune_Logger_logDBG( %i, %s, %i, \"%s\", ... )] "
                "Could not parse into string buffer (ret=%i).",
                (int) lvl, filename, line_num, format, ret
        );

        goto end;
    }

    String.append_back( &str, buffer );

    if( logger.init_success ) {
        ctune_LogQueue.enqueue( &logger.message_queue, str._raw );

    } else { //syslog
        if( lvl < CTUNE_LOG_TRACE ) //lvl>=8 not supported by syslog
            syslog( lvl, "%s", buffer );
    }

    end:
        va_end( args );
        String.free( &str );
}

/**
 * Namespace constructor
 */
const struct ctune_Logger_Singleton ctune_Logger = {
    .init   = &ctune_Logger_init,
    .close  = &ctune_Logger_close,
    .log    = &ctune_Logger_log,
    .logDBG = &ctune_Logger_logDBG
};