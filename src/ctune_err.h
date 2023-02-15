#ifndef CTUNE_UTILS_CTUNE_ERROR_H
#define CTUNE_UTILS_CTUNE_ERROR_H

#include <stdlib.h>
#include <stdio.h>

#define ADD(A, B)  ((A) + (B))

#define CTUNE_ERR_NONE                    0 //basic errors
#define CTUNE_ERR_BUFF_UNDERFLOW        ADD( CTUNE_ERR_NONE,  1 )
#define CTUNE_ERR_BUFF_OVERFLOW         ADD( CTUNE_ERR_NONE,  2 )
#define CTUNE_ERR_BUFF_NULL             ADD( CTUNE_ERR_NONE,  3 )
#define CTUNE_ERR_BUFF_ALLOC            ADD( CTUNE_ERR_NONE,  4 )
#define CTUNE_ERR_MALLOC                ADD( CTUNE_ERR_NONE,  5 )
#define CTUNE_ERR_BAD_CAST              ADD( CTUNE_ERR_NONE,  6 )
#define CTUNE_ERR_NO_DATA               ADD( CTUNE_ERR_NONE,  7 )
#define CTUNE_ERR_BAD_FUNC_ARGS         ADD( CTUNE_ERR_NONE,  8 )
#define CTUNE_ERR_FUNC_FAIL             ADD( CTUNE_ERR_NONE,  9 )
#define CTUNE_ERR_LOG                    10 //logging errors
#define CTUNE_ERR_LOG_SERVICE_TIMEOUT   ADD( CTUNE_ERR_LOG, 1 )
#define CTUNE_ERR_LOG_STOP_TIMEOUT      ADD( CTUNE_ERR_LOG, 2 )
#define CTUNE_ERR_LOG_FLUSH_TIMEOUT     ADD( CTUNE_ERR_LOG, 3 )
#define CTUNE_ERR_IO                     20 //IO errors
#define CTUNE_ERR_IO_STDERR_REDIRECT    ADD( CTUNE_ERR_IO,  1 )
#define CTUNE_ERR_IO_STDERR_RESET       ADD( CTUNE_ERR_IO,  2 )
#define CTUNE_ERR_IO_PLAYLOG_OPEN       ADD( CTUNE_ERR_IO,  3 )
#define CTUNE_ERR_IO_AUDIOFILE_OPEN     ADD( CTUNE_ERR_IO,  4 )
#define CTUNE_ERR_IO_AUDIOFILE_OPENED   ADD( CTUNE_ERR_IO,  5 )
#define CTUNE_ERR_IO_PLUGIN_OPEN        ADD( CTUNE_ERR_IO,  6 )
#define CTUNE_ERR_IO_PLUGIN_CLOSE       ADD( CTUNE_ERR_IO,  7 )
#define CTUNE_ERR_IO_PLUGIN_LINK        ADD( CTUNE_ERR_IO,  8 )
#define CTUNE_ERR_IO_PLUGIN_ABI         ADD( CTUNE_ERR_IO,  9 )
#define CTUNE_ERR_IO_PLUGIN_NULL        ADD( CTUNE_ERR_IO, 10 )
#define CTUNE_ERR_IO_DISK_ACCESS_FAIL   ADD( CTUNE_ERR_IO, 11 )
#define CTUNE_ERR_IO_DISK_FULL          ADD( CTUNE_ERR_IO, 12 )
#define CTUNE_ERR_IO_FILE_FULL          ADD( CTUNE_ERR_IO, 13 )
#define CTUNE_ERR_IO_FILE_CLOSE_FAIL    ADD( CTUNE_ERR_IO, 14 )
#define CTUNE_ERR_IO_FILE_WRITE_FAIL    ADD( CTUNE_ERR_IO, 15 )
#define CTUNE_ERR_IO_MOUSE_ENABLE_FAIL  ADD( CTUNE_ERR_IO, 16 )
#define CTUNE_ERR_IO_MOUSE_DISABLE_FAIL ADD( CTUNE_ERR_IO, 17 )
#define CTUNE_ERR_THREAD                 50
#define CTUNE_ERR_THREAD_CREATE         ADD( CTUNE_ERR_THREAD,  1 )
#define CTUNE_ERR_THREAD_JOIN           ADD( CTUNE_ERR_THREAD,  2 )
#define CTUNE_ERR_THREAD_JOIN_TIMEOUT   ADD( CTUNE_ERR_THREAD,  3 )
#define CTUNE_ERR_NETWORK_IO            100 //network related errors
#define CTUNE_ERR_NSLOOKUP_ADDR         ADD( CTUNE_ERR_NETWORK_IO,  1 )
#define CTUNE_ERR_NSLOOKUP_NAME         ADD( CTUNE_ERR_NETWORK_IO,  2 )
#define CTUNE_ERR_SOCK_SETUP            ADD( CTUNE_ERR_NETWORK_IO,  3 )
#define CTUNE_ERR_SOCK_CONNECT          ADD( CTUNE_ERR_NETWORK_IO,  4 )
#define CTUNE_ERR_SOCK_WRITE            ADD( CTUNE_ERR_NETWORK_IO,  5 )
#define CTUNE_ERR_SOCK_READ             ADD( CTUNE_ERR_NETWORK_IO,  6 )
#define CTUNE_ERR_SOCK_SSL_SETUP        ADD( CTUNE_ERR_NETWORK_IO,  7 )
#define CTUNE_ERR_SOCK_SCONNECT         ADD( CTUNE_ERR_NETWORK_IO,  8 )
#define CTUNE_ERR_SOCK_SSL_CERT         ADD( CTUNE_ERR_NETWORK_IO,  9 )
#define CTUNE_ERR_SOCK_SWRITE           ADD( CTUNE_ERR_NETWORK_IO, 10 )
#define CTUNE_ERR_SOCK_SREAD            ADD( CTUNE_ERR_NETWORK_IO, 11 )
#define CTUNE_ERR_INVALID_URL           ADD( CTUNE_ERR_NETWORK_IO, 12 )
#define CTUNE_ERR_RADIO_BROWSER_API     200 //'Radio Browser' web API specific errors
#define CTUNE_ERR_PARSE                 300 //parsing errors
#define CTUNE_ERR_PARSE_UNKNOWN_KEY     ADD( CTUNE_ERR_PARSE, 1 )
#define CTUNE_ERR_PLAYER                400 //player errors
#define CTUNE_ERR_PLAYER_INIT           ADD( CTUNE_ERR_PLAYER,  1 )
#define CTUNE_ERR_STREAM_OPEN           ADD( CTUNE_ERR_PLAYER,  2 )
#define CTUNE_ERR_STREAM_OPEN_TIMEOUT   ADD( CTUNE_ERR_PLAYER,  3 )
#define CTUNE_ERR_STREAM_READ_TIMEOUT   ADD( CTUNE_ERR_PLAYER,  4 )
#define CTUNE_ERR_STREAM_NO_AUDIO       ADD( CTUNE_ERR_PLAYER,  5 )
#define CTUNE_ERR_STREAM_INFO           ADD( CTUNE_ERR_PLAYER,  6 )
#define CTUNE_ERR_STREAM_CODEC          ADD( CTUNE_ERR_PLAYER,  7 )
#define CTUNE_ERR_STREAM_BUFFER_SIZE_0  ADD( CTUNE_ERR_PLAYER,  8 )
#define CTUNE_ERR_STREAM_BUFFER_ALLOC   ADD( CTUNE_ERR_PLAYER,  9 )
#define CTUNE_ERR_STREAM_SWR            ADD( CTUNE_ERR_PLAYER, 10 )
#define CTUNE_ERR_STREAM_FRAME_FETCH    ADD( CTUNE_ERR_PLAYER, 11 )
#define CTUNE_ERR_STREAM_DECODE         ADD( CTUNE_ERR_PLAYER, 12 )
#define CTUNE_ERR_STREAM_RESAMPLE       ADD( CTUNE_ERR_PLAYER, 13 )
#define CTUNE_ERR_AUDIO_OUT             500 //audio output errors
#define CTUNE_ERR_SDL_INIT              ADD( CTUNE_ERR_AUDIO_OUT,  1 )
#define CTUNE_ERR_SDL_OPEN              ADD( CTUNE_ERR_AUDIO_OUT,  2 )
#define CTUNE_ERR_PULSE_INIT            ADD( CTUNE_ERR_AUDIO_OUT,  3 )
#define CTUNE_ERR_PULSE_SHUTDOWN        ADD( CTUNE_ERR_AUDIO_OUT,  4 )
#define CTUNE_ERR_ALSA_INIT             ADD( CTUNE_ERR_AUDIO_OUT,  5 )
#define CTUNE_ERR_SNDIO_INIT            ADD( CTUNE_ERR_AUDIO_OUT,  6 )
#define CTUNE_ERR_SNDIO_NOVOL           ADD( CTUNE_ERR_AUDIO_OUT,  7 )
#define CTUNE_ERR_LAME_INIT             ADD( CTUNE_ERR_AUDIO_OUT,  8 )
#define CTUNE_ERR_UI                    600 //UI
#define CTUNE_ERR_ACTION                700 //failed actions
#define CTUNE_ERR_ACTION_UNSUPPORTED    ADD( CTUNE_ERR_ACTION,  1 )
#define CTUNE_ERR_ACTION_FAV_TOGGLE     ADD( CTUNE_ERR_ACTION,  2 )
#define CTUNE_ERR_ACTION_FAV_UPDATE     ADD( CTUNE_ERR_ACTION,  3 )
#define CTUNE_ERR_ACTION_FETCH          ADD( CTUNE_ERR_ACTION,  4 )


extern const struct ctune_err_Instance {
    /**
     * Prints to stderr the entire list of ctune-centric error numbers and their corresponding descriptions
     * @param FILE descriptor to output list to
     */
    void (* print_errno_list)( FILE * out );

    /**
     * [THREAD SAFE] Sets the errno
     * @param err cTune Error number
     */
    void (* set)( int err );

    /**
     * [THREAD SAFE] Gets the error number
     * @return ctune errno
     */
    int (* number)( void );

    /**
     * [THREAD SAFE] Gets the string description of a cTune specific errno and resets the errno back to `CTUNE_ERR_NONE`
     * @return String description
     */
    const char * (* strerror)( void );

    /**
     * [THREAD SAFE] Gets the string description of a cTune specific errno **without** resetting the errno
     * @param error Error to get the string description of
     * @return String description
     */
    const char * (* print)( int error );

    /**
     * [OPTIONAL] Sets a printing callback for when an error is set
     * - note that if a callback is set, the errno will be reset after every call
     * @param cb Callback method
     */
    void (* setPrintErrCallback)( void(* cb)( const char * str, int err ) );

} ctune_err;


#endif //CTUNE_UTILS_CTUNE_ERROR_H
