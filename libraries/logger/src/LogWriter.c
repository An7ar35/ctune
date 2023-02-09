#include "LogWriter.h"

#include <stdio.h>
#include <syslog.h>
#include <errno.h>
#include <wchar.h>
#include <time.h>

#include "Logger.h"
#include "../src/ctune_err.h"
#include "../src/utils/Timeout.h"

#define CTUNE_LOGGER_LOGWRITER_STOP_TIMEOUT 5 //in seconds

static struct LogWriterCfg {
    pthread_mutex_t sleep_mutex;
    volatile bool   sleeping; //sleep signal
    pthread_cond_t  wakeup_cond;

    pthread_mutex_t interrupt_mutex;
    volatile bool   interrupt; //main loop interrupt

    FILE          * out;
    LogQueue_t    * msg_queue;

    struct Timer {
        const    u_int64_t timeout;
        volatile u_int64_t start;

    } timer; //microsecond resolution

} log_writer = {
    .sleep_mutex     = PTHREAD_MUTEX_INITIALIZER,
    .sleeping        = false,
    .wakeup_cond     = PTHREAD_COND_INITIALIZER,
    .interrupt_mutex = PTHREAD_MUTEX_INITIALIZER,
    .interrupt       = false,
    .out             = NULL,
    .msg_queue       = NULL,
    .timer           = { .timeout = 1500000 /* 1.5s */ },
};

static pthread_t log_writer_thread;

/**
 * [PRIVATE] Get time now in microseconds (1us = 0.000001s)
 * @return Time in microseconds
 */
static u_int64_t ctune_LogWriter_getTimeNow() {
    struct timespec ts;
    if ( clock_gettime( CLOCK_MONOTONIC, &ts ) == 0 )
        return (u_int64_t) ( ts.tv_sec * 1000000 + ts.tv_nsec / 1000 );
    else
        return 0;
}

/**
 * [PRIVATE/THREAD SAFE] check the running state of the main loop
 * @return Running state
 */
static bool ctune_LogWriter_checkInterruptState() {
    bool state = false;
    pthread_mutex_lock( &log_writer.interrupt_mutex );
    state = log_writer.interrupt;
    pthread_mutex_unlock( &log_writer.interrupt_mutex );
    return state;
}

/**
 * [PRIVATE/THREAD SAFE] Puts the log writer to sleep if the timer timed-out
 * @param start       Start value of the timer
 * @param timeout_val Timeout value for the timer
 * @param split       Timer value when method is called (i.e. now)
 */
void ctune_LogWriter_checkTired( u_int64_t start, u_int64_t timeout_val, u_int64_t split ) {
    if( ctune_LogWriter_checkInterruptState() == true )
        return;

    bool goto_sleep = ( split == 0 ) || ( ( split - start ) > timeout_val );

    pthread_mutex_lock( &log_writer.sleep_mutex );
    log_writer.sleeping = goto_sleep;
    pthread_mutex_unlock( &log_writer.sleep_mutex );
}

/**
 * [PRIVATE/THREAD SAFE] Checks if log writer is suspended
 */
void ctune_LogWriter_checkSleeping() {
    pthread_mutex_lock( &log_writer.sleep_mutex );
    while ( log_writer.sleeping )
        pthread_cond_wait( &log_writer.wakeup_cond, &log_writer.sleep_mutex );
    pthread_mutex_unlock( &log_writer.sleep_mutex );
}

/**
 * [PRIVATE] Launches the main loop that consumes log messages from the queue and writes them to the output file
 * @param args Obscure pointer to Argument package
 * @return NULL
 */
static void * ctune_LogWriter_runLoop( void * args ) {
    struct LogWriterCfg * cfg      = args;
    LogQueueNode_t      * msg_node = NULL;

    while( ctune_LogWriter_checkInterruptState() == false ) {
        ctune_LogWriter_checkSleeping();

        while( ( msg_node = ctune_LogQueue.dequeue( cfg->msg_queue ) ) != NULL ) {
            fprintf( cfg->out, "%s\n", msg_node->data );
            ctune_LogQueue.freeLogQueueNode( msg_node );
            fflush( cfg->out );
        }

        ctune_LogWriter_checkTired( log_writer.timer.start,
                                    log_writer.timer.timeout,
                                    ctune_LogWriter_getTimeNow() );
    }

    fclose( cfg->out );
    return NULL;
}

/**
 * Starts the log writer
 * @param out_filepath Log file output path
 * @param file_mode    Mode to use when opening the log file
 * @param queue        Pointer to an instantiated LogQueue
 * @return Success
 */
static bool ctune_LogWriter_start( const char * out_filepath, const char * file_mode, LogQueue_t * queue ) {
    if( ctune_LogWriter_checkInterruptState() == true ) {
        syslog( LOG_ERR,
                "[ctune_LogWriter_start( \"%s\", \"%s\", %p )] Log writer loop already active.",
                out_filepath, file_mode, queue
        );

        return false;
    }

    bool error_state = false;

    pthread_mutex_lock( &log_writer.interrupt_mutex );

    log_writer.out       = fopen( out_filepath, file_mode );
    log_writer.msg_queue = queue;
    log_writer.interrupt = false;
    log_writer.sleeping  = false;

    if( log_writer.out == NULL ) {
        syslog( LOG_ERR,
                "[ctune_LogWriter_start( \"%s\", \"%s\", %p )] Log writer could not open file: %s",
                out_filepath, file_mode, queue, strerror( errno )
        );

        error_state = true;
        goto end;
    }

    if( pthread_create( &log_writer_thread, NULL, &ctune_LogWriter_runLoop, (void *) &log_writer ) != 0 ) {
        syslog( LOG_ERR,
                "[ctune_LogWriter_start( \"%s\", \"%s\", %p )] Log writer failed to start consumer thread.",
                out_filepath, file_mode, queue
        );

        ctune_err.set( CTUNE_ERR_THREAD_CREATE );
        log_writer.interrupt = true;
    }

    end:
        pthread_mutex_unlock( &log_writer.interrupt_mutex );
        return !(error_state);
}

/**
 * [THREAD SAFE] Stops the log writer
 * @return 0 for success or negative cTune error number on failure
 */
static int ctune_LogWriter_stop() {
    pthread_mutex_lock( &log_writer.interrupt_mutex );
    log_writer.interrupt = true; //sends interrupt to loop
    pthread_mutex_unlock( &log_writer.interrupt_mutex );

    ctune_LogWriter.resume(); //get the loop going so it can exit

    ctune_Timeout_t timer = ctune_Timeout.init( CTUNE_LOGGER_LOGWRITER_STOP_TIMEOUT, 0, NULL );

    while( ctune_LogWriter_checkInterruptState() == false && !ctune_Timeout.timedOut( &timer ) );

    if( ctune_LogWriter_checkInterruptState() == false ) {
        return -CTUNE_ERR_LOG_STOP_TIMEOUT;
    }

    return CTUNE_ERR_NONE;
}

/**
 * [THREAD SAFE] Wake-up the log writer
 * - (use as callback on the LoqQueue whenever a enqueue operation is made)
 */
void ctune_LogWriter_resume() {
    pthread_mutex_lock( &log_writer.sleep_mutex );
    pthread_cond_broadcast( &log_writer.wakeup_cond );
    pthread_mutex_unlock( &log_writer.sleep_mutex );

    //resets timer everytime a call is made
    log_writer.sleeping    = false;
    log_writer.timer.start = ctune_LogWriter_getTimeNow();
}

/**
 * Namespace constructor
 */
const struct ctune_LogWriter_Instance ctune_LogWriter = {
    .start    = &ctune_LogWriter_start,
    .stop     = &ctune_LogWriter_stop,
    .resume   = &ctune_LogWriter_resume,
};