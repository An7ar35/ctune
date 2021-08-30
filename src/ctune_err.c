#include "ctune_err.h"
#include "logger/Logger.h"

static struct {
    volatile int    errno;
    pthread_mutex_t errno_mutex;
    void (* print_callback )( const char * str, int err );

} ctuneErr = {
    .errno          = CTUNE_ERR_NONE,
    .errno_mutex    = PTHREAD_MUTEX_INITIALIZER,
    .print_callback = NULL,
};

struct ctune_error {
    int          num;
    const char * str;
};

static const struct ctune_error ctune_error_descs[] = {
    { CTUNE_ERR_NONE,                 "" },
    { CTUNE_ERR_BUFF_OVERFLOW,        "Buffer overflow" },
    { CTUNE_ERR_BUFF_ALLOC,           "Buffer allocation failed - see log" },
    { CTUNE_ERR_MALLOC,               "Memory allocation failed - see log" },
    { CTUNE_ERR_BAD_CAST,             "Bad cast" },
    { CTUNE_ERR_BAD_FUNC_ARGS,        "Bad function arguments - see log" },
    { CTUNE_ERR_LOG,                  "Logger error" },
    { CTUNE_ERR_LOG_SERVICE_TIMEOUT,  "Logger service timout" },
    { CTUNE_ERR_LOG_STOP_TIMEOUT,     "Logger service termination timeout" },
    { CTUNE_ERR_LOG_FLUSH_TIMEOUT,    "Logger queue flush timeout" },
    { CTUNE_ERR_IO,                   "IO error" },
    { CTUNE_ERR_IO_STDERR_REDIRECT,   "Redirecting stderr failed" },
    { CTUNE_ERR_IO_STDERR_RESET,      "Resetting stderr output to default failed" },
    { CTUNE_ERR_IO_PLAYLOG_OPEN,      "Opening playback log file failed" },
    { CTUNE_ERR_IO_PLUGIN_OPEN,       "Could not open plugin" },
    { CTUNE_ERR_IO_PLUGIN_CLOSE,      "Could not close plugin" },
    { CTUNE_ERR_IO_PLUGIN_LINK,       "Failed linking to plugin" },
    { CTUNE_ERR_IO_PLUGIN_ABI,        "Plugin ABI version mismatch" },
    { CTUNE_ERR_IO_PLUGIN_NULL,       "No plugin set" },
    { CTUNE_ERR_THREAD,               "Thread error" },
    { CTUNE_ERR_THREAD_CREATE,        "Error in launching thread" },
    { CTUNE_ERR_THREAD_JOIN,          "Error in terminating thread" },
    { CTUNE_ERR_THREAD_JOIN_TIMEOUT,  "Busy thread timeout reached." },
    { CTUNE_ERR_NETWORK_IO,           "Network IO error" },
    { CTUNE_ERR_NSLOOKUP_ADDR,        "NS address lookup failure" },
    { CTUNE_ERR_NSLOOKUP_NAME,        "NS name lookup failure" },
    { CTUNE_ERR_SOCK_SETUP,           "Socket setup error" },
    { CTUNE_ERR_SOCK_CONNECT,         "Socket connection error" },
    { CTUNE_ERR_SOCK_WRITE,           "Socket write error" },
    { CTUNE_ERR_SOCK_READ,            "Socket read error" },
    { CTUNE_ERR_SOCK_SSL_SETUP,       "SSL setup error" },
    { CTUNE_ERR_SOCK_SCONNECT,        "SSL Socket connect error" },
    { CTUNE_ERR_SOCK_SSL_CERT,        "SSL certificate error" },
    { CTUNE_ERR_SOCK_SWRITE,          "SSL socket write error" },
    { CTUNE_ERR_SOCK_SREAD,           "SSL socket read error" },
    { CTUNE_ERR_INVALID_URL,          "Invalid URL string" },
    { CTUNE_ERR_RADIO_BROWSER_API,    "Radio Browser API error" },
    { CTUNE_ERR_PARSE,                "Parsing error" },
    { CTUNE_ERR_PLAYER,               "Player error" },
    { CTUNE_ERR_PLAYER_INIT,          "Failed to init player instance - see log" },
    { CTUNE_ERR_STREAM_OPEN,          "Can't open source stream" },
    { CTUNE_ERR_STREAM_OPEN_TIMEOUT,  "Open stream timeout" },
    { CTUNE_ERR_STREAM_READ_TIMEOUT,  "Stream reading timeout" },
    { CTUNE_ERR_STREAM_NO_AUDIO,      "No audio stream found in source" },
    { CTUNE_ERR_STREAM_INFO,          "Can't get source stream information" },
    { CTUNE_ERR_STREAM_CODEC,         "Failed stream codec setup" },
    { CTUNE_ERR_STREAM_BUFFER_SIZE_0, "Failed to create resampling output buffer - insufficient info" },
    { CTUNE_ERR_STREAM_BUFFER_ALLOC,  "Failed to allocate resampling output buffer" },
    { CTUNE_ERR_STREAM_SWR,           "Failed to setup audio resampler" },
    { CTUNE_ERR_STREAM_FRAME_FETCH,   "Failed to fetch next audio frame" },
    { CTUNE_ERR_STREAM_DECODE,        "Failed to decode audio frame" },
    { CTUNE_ERR_STREAM_RESAMPLE,      "Failed to resample decoded frame" },
    { CTUNE_ERR_AUDIO_OUT,            "Audio output error" },
    { CTUNE_ERR_SDL_INIT,             "Could not initialise SDL" },
    { CTUNE_ERR_SDL_OPEN,             "Could not open SDL audio" },
    { CTUNE_ERR_PULSE_INIT,           "Could not initialise Pulse audio" },
    { CTUNE_ERR_ALSA_INIT,            "Could not initialise ALSA" },
    { CTUNE_ERR_SNDIO_INIT,           "Could not initialise SNDIO" },
    { CTUNE_ERR_SNDIO_NOVOL,          "Volume control is not available ('sndio')" },
    { CTUNE_ERR_UI,                   "UI error - see log" },
    { CTUNE_ERR_ACTION,               "Failed action - see log" },
    { CTUNE_ERR_ACTION_UNSUPPORTED,   "Unsupported action" },
    { CTUNE_ERR_ACTION_FAV_TOGGLE,    "Favourite state toggle failed - see log" },
    { CTUNE_ERR_ACTION_FAV_UPDATE,    "Update failed - see log" },
    { CTUNE_ERR_ACTION_FETCH,         "Fetch failed - see log" }
};

/**
 * Prints to stderr the entire list of ctune-centric error numbers and their corresponding descriptions
 * @param FILE descriptor to output list to
 */
static void ctune_err_printErrnoList( FILE * out ) {
    static const size_t ARR_LENGTH = sizeof( ctune_error_descs ) / sizeof( ctune_error_descs[0] );
    fprintf( out, "=== CTUNE ERROR NUMBER LIST BEGIN ===\n" );
    for( size_t i = 0; i < ARR_LENGTH; ++i ) {
        fprintf( out, "%i\t%s\n", ctune_error_descs[i].num, ctune_error_descs[i].str );
    }
    fprintf( out, "=== CTUNE ERROR NUMBER LIST END ===\n" );
}

/**
 * [THREAD SAFE] Gets the string description of a cTune specific errno and resets the errno back to `CTUNE_ERR_NONE`
 * @return String description
 */
static const char * ctune_err_printResetErrStr() {
    static const size_t ARR_LENGTH    = sizeof( ctune_error_descs ) / sizeof( ctune_error_descs[0] );
    static const char * INVALID_ERRNO = "Unrecognised ctune_errno";

    for( size_t i = 0; i < ARR_LENGTH; ++i ) {
        if( ctune_error_descs[i].num == ctuneErr.errno ) {
            pthread_mutex_lock( &ctuneErr.errno_mutex );
            ctuneErr.errno = CTUNE_ERR_NONE; //resets
            pthread_mutex_unlock( &ctuneErr.errno_mutex );
            return ctune_error_descs[i].str;
        }
    }

    CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_strerror()] %s: %i", INVALID_ERRNO, ctuneErr.errno );
    return INVALID_ERRNO;
}

/**
 * [THREAD SAFE] Gets the string description of a cTune specific errno _without_ resetting the errno
 * @param error Error to get the string description of
 * @return String description
 */
static const char * ctune_err_print( int error ) {
    static const size_t ARR_LENGTH    = sizeof( ctune_error_descs ) / sizeof( ctune_error_descs[0] );
    static const char * INVALID_ERRNO = "Unrecognised ctune_errno";

    for( size_t i = 0; i < ARR_LENGTH; ++i ) {
        if( ctune_error_descs[i].num == error ) {
            return ctune_error_descs[i].str;
        }
    }

    CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_strerror()] %s: %i", INVALID_ERRNO, error );
    return INVALID_ERRNO;
}

/**
 * [THREAD SAFE] Gets the error number
 * @return ctune errno
 */
static int ctune_err_errorno() {
    int e = CTUNE_ERR_NONE;

    pthread_mutex_lock( &ctuneErr.errno_mutex );
    e = ctuneErr.errno;
    pthread_mutex_unlock( &ctuneErr.errno_mutex );

    return e;
}

/**
 * [THREAD SAFE] Sets the errno
 * @param err cTune Error number
 */
static void ctune_err_set_errno( int err ) {
    pthread_mutex_lock( &ctuneErr.errno_mutex );

    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_set_errno( %i )] %s", err, ctune_err.print( err ) );

    int old = ctuneErr.errno;

    if( ctuneErr.errno != CTUNE_ERR_NONE )
        CTUNE_LOG( CTUNE_LOG_WARNING, "[ctune_set_errno( %i )] An error code was previously set: (%i) %s", err, old, ctune_err.strerror() );

    ctuneErr.errno = err;

    if( ctuneErr.print_callback != NULL ) {
        ctuneErr.print_callback( ctune_err.print( err ), err );
        ctuneErr.errno = CTUNE_ERR_NONE; //resets errno
    }

    pthread_mutex_unlock( &ctuneErr.errno_mutex );
}

/**
 * [OPTIONAL] Sets a printing callback for when an error is set
 * - note that if a callback is set, the errno will be reset after every call
 * @param cb Callback method
 */
void ctune_err_setPrintErrCallback( void(* cb)( const char * str, int err ) ) {
    ctuneErr.print_callback = cb;
}

/**
 * Namespace constructor
 */
const struct ctune_err_Instance ctune_err = {
    .print_errno_list    = &ctune_err_printErrnoList,
    .set                 = &ctune_err_set_errno,
    .number              = &ctune_err_errorno,
    .strerror            = &ctune_err_printResetErrStr,
    .print               = &ctune_err_print,
    .setPrintErrCallback = &ctune_err_setPrintErrCallback,
};