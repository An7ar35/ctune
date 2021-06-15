#include "RadioPlayer.h"

#include <unistd.h>
#include <signal.h>

#include "../utils/Timeout.h"

/**
 * Argument container for playing streams
 * @param url         Stream url
 * @param init_vol    Starting volume for the stream playback
 * @param timeout_val Timeout value in seconds for connecting/playing
 */
typedef struct ctune_RadioPlayer_PlaybackArgs {
    String_t url;
    int      init_vol;
    int      timeout_val;

} ctune_PlaybackArgs_t;

/**
 * Private vars
 */
struct {
    bool               initialised;
    bool               player_initialised;
    ctune_Player_t   * player_plugin;
    ctune_AudioOut_t * output_plugin;

    struct { /* PLAYER CONTROL */
        pthread_t             thread;
        volatile sig_atomic_t state; //used to interrupt playing of a stream

    } player;

    ctune_PlaybackArgs_t stream_args;

    struct {
        void (* song_change_callback)( const char * str );
        void (* volume_change_event_callback)( const int vol );
        void (* playback_state_change_cb)( bool state );

    } cb;

} radio_player = {
    .initialised        = false,
    .player_initialised = false,
    .player_plugin      = NULL,
    .output_plugin      = NULL,
    .player = {
        .state = false,
    },
    .stream_args = {
        { NULL, 0 },
        0,
        0
    },
    .cb = {
        NULL,
        NULL,
        NULL,
    },
};

/**
 * [PRIVATE/THREAD SAFE] Controls the playback state - use when getting and setting the state must be done as an atomic operation
 * Note: When calling ON/OFF only the callback method is called when `ctrl` matches the current state
 * @param ctrl Control request command (get state/turn on/turn off/switch)
 * @return Playback state (state/switch) or the change state (on/off)
 */
static bool ctune_RadioPlayer_setPlaybackState( enum CTUNE_PLAYBACK_CTRL ctrl ) {
    bool curr_state = CTUNE_PLAYER_STATE_PLAYING;

    curr_state = radio_player.player.state;

    switch( ctrl ) {
        case CTUNE_PLAYBACK_CTRL_OFF: {
            if( curr_state )
                CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_RadioPlayer_setPlaybackState( %i )] %s -> turn OFF", (int) ctrl, ( curr_state ? "ON" : "OFF" ) );

            radio_player.player.state = CTUNE_PLAYER_STATE_STOPPED;
            curr_state = ( curr_state != CTUNE_PLAYER_STATE_STOPPED );

            if( radio_player.cb.playback_state_change_cb != NULL )
                radio_player.cb.playback_state_change_cb( CTUNE_PLAYER_STATE_STOPPED );
        } break;

        case CTUNE_PLAYBACK_CTRL_ON: {
            if( !curr_state )
                CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_RadioPlayer_setPlaybackState( %i )] %s -> turn ON", (int) ctrl, ( curr_state ? "ON" : "OFF" ) );

            radio_player.player.state = CTUNE_PLAYER_STATE_PLAYING;
            curr_state = ( curr_state != CTUNE_PLAYER_STATE_PLAYING );

            if( radio_player.cb.playback_state_change_cb != NULL )
                radio_player.cb.playback_state_change_cb( CTUNE_PLAYER_STATE_PLAYING );
        } break;

        case CTUNE_PLAYBACK_CTRL_SWITCH: {
            CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_RadioPlayer_setPlaybackState( %i )] switch (%i -> %i)", (int) ctrl, curr_state, !curr_state );
            curr_state = radio_player.player.state;
            curr_state = radio_player.player.state = !curr_state;

            if( radio_player.cb.playback_state_change_cb != NULL )
                radio_player.cb.playback_state_change_cb( curr_state );
        } break;

        case CTUNE_PLAYBACK_CTRL_STATE: //fallthrough
        default:
            break;
    }

    return curr_state;
}

/**
 * [PRIVATE/THREAD SAFE] Connects and plays a Radio station's stream
 * @param args Pointer to a ctune_PlaybackArgs_t object
 * @return NULL
 */
static void * ctune_RadioPlayer_launchPlayback( void * args ) {
    if( radio_player.player_plugin != NULL ) {
        ctune_PlaybackArgs_t *cast_args = args;
        radio_player.player_plugin->playRadioStream( cast_args->url._raw, cast_args->init_vol, cast_args->timeout_val );

    } else {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_RadioPlayer_playRadioStream( %p )] No player plugin set!", args );
        ctune_err.set( CTUNE_ERR_IO_PLUGIN_NULL );
    }

    return NULL;
}

/**
 * Initialises the main functionalities
 * @param song_change_callback           Function to call when stream metadata changes (sends the current stream title)
 * @param volume_change_event_callback   Function to call when a volume change request is successful (new volume is passed as arg)
 */
static void ctune_RadioPlayer_init( void(* song_change_callback)( const char * ),
                                    void(* volume_change_event_callback)( const int ) )
{
    if( radio_player.initialised ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_RadioPlayer_init( %p, %p )] RadioPlayer already initialised.",
                   song_change_callback, volume_change_event_callback
        );

        return; //EARLY RETURN
    }

    radio_player.cb.song_change_callback           = song_change_callback;
    radio_player.cb.volume_change_event_callback   = volume_change_event_callback;
    radio_player.initialised                       = true;
}

/**
 * Sets a playback state change callback method
 * @param playback_state_change_cb Callback function
 */
static void ctune_RadioPlayer_setStateChangeCallback( void (* playback_state_change_cb)( bool ) ) {
    radio_player.cb.playback_state_change_cb = playback_state_change_cb;
}

/**
 * Loads a player plugin
 * @param player Pointer to Player plugin
 * @return Success
 */
static bool ctune_RadioPlayer_loadPlayerPlugin( ctune_Player_t * player ) {
    if( player == NULL )
        return false; //EARLY RETURN

    if( radio_player.player_plugin == NULL ) {
        CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_RadioPlayer_loadPlayerPlugin( %p )] New player set.", player );
        radio_player.player_plugin = player;

    } else {
        CTUNE_LOG( CTUNE_LOG_MSG, "[ctune_RadioPlayer_loadPlayerPlugin( %p )] Player replaced.", player );
        radio_player.player_plugin = player;
    }

    radio_player.player_initialised = false;

    if( radio_player.output_plugin != NULL ) {
        radio_player.player_plugin->init( radio_player.output_plugin,
                                          ctune_RadioPlayer_setPlaybackState,
                                          radio_player.cb.song_change_callback );

        radio_player.player_initialised = true;
    }

    return true;
}

/**
 * Loads a sound server plugin
 * @param sound_server Pointer to sound server plugin
 * @return Success
 */
 static bool ctune_RadioPlayer_loadSoundServerPlugin( ctune_AudioOut_t * sound_server ) {
    if( sound_server == NULL )
        return false; //EARLY RETURN

    if( radio_player.output_plugin == NULL ) {
        CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_RadioPlayer_loadSoundServerPlugin( %p )] New sound server set.", sound_server );
        radio_player.output_plugin = sound_server;

    } else {
        CTUNE_LOG( CTUNE_LOG_MSG, "[ctune_RadioPlayer_loadSoundServerPlugin( %p )] Sound server replaced.", sound_server );
        radio_player.output_plugin->shutdown();
        radio_player.output_plugin = sound_server;
    }

    if( radio_player.player_plugin != NULL && radio_player.player_initialised == false ) {
        radio_player.player_plugin->init( radio_player.output_plugin,
                                          ctune_RadioPlayer_setPlaybackState,
                                          radio_player.cb.song_change_callback );

        radio_player.player_initialised = true;
    }

    return true;
 }

/**
 * [THREAD SAFE] Stops the playback of the currently playing stream
 */
static void ctune_RadioPlayer_stopRadioStream( void ) {
    if( ctune_RadioPlayer_setPlaybackState( CTUNE_PLAYBACK_CTRL_OFF ) ) { //i.e. if a switch from ON->OFF occurred
        if( pthread_join( radio_player.player.thread, NULL ) != 0 ) {
            ctune_err.set( CTUNE_ERR_THREAD_JOIN );
        }

        ctune_err.set( ctune_RadioPlayer.getError() );

        if( ctune_err.number() != CTUNE_ERR_NONE )
            CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_stopPlayback] Player encountered an error: %s", ctune_err.strerror() );
    }
}

/**
 * [THREAD SAFE] Connects and plays a Radio station's stream
 * @param url         Radio station stream URL
 * @param volume      Initial playing volume
 * @param timeout_val Timeout value in seconds
 * @return Success (if false the error_no in RadioPlayer will be set accordingly)
 */
static bool ctune_RadioPlayer_playRadioStream( const char * url, const int volume, int timeout_val ) {
    if( ctune_RadioPlayer_setPlaybackState( CTUNE_PLAYBACK_CTRL_STATE ) == CTUNE_PLAYER_STATE_PLAYING )
        ctune_RadioPlayer_stopRadioStream();

    ctune_RadioPlayer_setPlaybackState( CTUNE_PLAYBACK_CTRL_ON );

    //set the playback arguments values
    String.set( &radio_player.stream_args.url, url );
    radio_player.stream_args.init_vol    = volume;
    radio_player.stream_args.timeout_val = timeout_val;

    //start playback
    if( pthread_create( &radio_player.player.thread, NULL, ctune_RadioPlayer_launchPlayback, (void *) &(radio_player.stream_args) ) != 0 ) {
        ctune_RadioPlayer_setPlaybackState( CTUNE_PLAYBACK_CTRL_OFF );

        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_RadioPlayer_playRadioStream( \"%s\", %d, %d )] Failed to create thread for player.",
                   url, volume, timeout_val
        );

        ctune_err.set( CTUNE_ERR_THREAD_CREATE );
        return false;
    }

    return true;
}

/**
 * [THREAD SAFE] Gets the playback state variable's value
 * @return Playback state value
 */
static bool ctune_RadioPlayer_getPlaybackState( void ) {
    return radio_player.player.state;
}

/**
 * [THREAD SAFE] Changes playback volume by a specified amount
 * @param delta Volume change (+/-)
 */
static void ctune_RadioPlayer_modifyVolume( int delta ) {
    if( radio_player.output_plugin ) {
        radio_player.output_plugin->changeVolume( delta );
        radio_player.cb.volume_change_event_callback( radio_player.output_plugin->getVolume() );
    }
}

/**
* Gets the error number set in RadioPlayer
* @return ctune_errno
*/
static int ctune_RadioPlayer_getError( void ) {
    if( radio_player.player_plugin != NULL ) {
        return radio_player.player_plugin->getError();
    }

    return CTUNE_ERR_IO_PLUGIN_NULL;
}

/**
 * Test stream and get its property (can be called independently from the player)
 * @param url         Stream URL
 * @param timeout_val Timeout value in seconds
 * @param codec_str   Pointer to the codec string
 * @param bitrate     Pointer to the bitrate container
 * @return Stream OK
 */
bool ctune_RadioPlayer_testStream( const char * url, int timeout_val, String_t * codec_str, ulong * bitrate ) {
    if( radio_player.player_plugin != NULL ) {
        return radio_player.player_plugin->testStream( url, timeout_val, codec_str, bitrate );
    }

    CTUNE_LOG( CTUNE_LOG_ERROR,
               "[ctune_RadioPlayer_testStream( %s, %d, %p, %lu )] No player plugin set!",
               url, timeout_val, codec_str, bitrate
    );

    ctune_err.set( CTUNE_ERR_IO_PLUGIN_NULL );

    return false;
}

/**
 * Namespace constructor
 */
const struct ctune_RadioPlayer_Namespace ctune_RadioPlayer = {
    .init                   = &ctune_RadioPlayer_init,
    .setStateChangeCallback = &ctune_RadioPlayer_setStateChangeCallback,
    .loadPlayerPlugin       = &ctune_RadioPlayer_loadPlayerPlugin,
    .loadSoundServerPlugin  = &ctune_RadioPlayer_loadSoundServerPlugin,
    .playRadioStream        = &ctune_RadioPlayer_playRadioStream,
    .stopPlayback           = &ctune_RadioPlayer_stopRadioStream,
    .getPlaybackState       = &ctune_RadioPlayer_getPlaybackState,
    .modifyVolume           = &ctune_RadioPlayer_modifyVolume,
    .getError               = &ctune_RadioPlayer_getError,
    .testStream             = &ctune_RadioPlayer_testStream,
};