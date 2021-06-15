#include "Player.h"

#include <vlc/vlc.h>
#include <unistd.h>

#include "../logger/Logger.h"
#include "../audio/AudioOut.h"
#include "../ctune_err.h"
#include "../utils/Timeout.h"

const unsigned abi_version = CTUNE_PLAYER_ABI_VERSION;

/**
 * Player plugin variables
 * @param error             ctune error no
 * @param audio_out         Pointer to the audio output to use
 * @param out_sample_fmt    Sample format of the PCM data to be sent to the audio output
 * @param out_sample_rate   Sample rate in Hz of the PCM data to be sent to the audio output
 * @param out_channels      Number of channels of the PCM data to be sent to the audio output
 */
struct ctune_RadioPlayer {
    int                     error;
    ctune_AudioOut_t      * audio_out;

    struct {
        const char        * vlc;
        ctune_output_fmt_t  ctune;
    } out_sample_fmt;

    const int               out_sample_rate;
    const int               out_channels;

    libvlc_instance_t     * vlc_instance;
    libvlc_media_player_t * vlc_media_player;

    struct {
        bool (* playback_ctrl_callback)( enum CTUNE_PLAYBACK_CTRL );
        void (* song_change_callback)( const char * str );
    } cb;

} vlc_player = {
    .error               = CTUNE_ERR_NONE,
    .audio_out           = NULL,
    .out_sample_fmt   = {
        .vlc   = "s16l", //signed 16bit little endian ('s32n' on VLC v3.0.14 doesn't work as expected)
        .ctune = CTUNE_AUDIO_OUTPUT_FMT_S16, //equivalent of above
    },
    .out_sample_rate     = 44100, //Hz
    .out_channels        = 2,     //stereo
    .cb = {
        NULL,
        NULL,
    },
};

/**
 * [PRIVATE] LibVLC event handling callback
 * @param p_event libVLC event
 * @param p_data  Opaque pointer
 */
static void handleVlcStreamEventCallback( const struct libvlc_event_t * p_event, void * p_data ) {
    CTUNE_LOG( CTUNE_LOG_TRACE,
               "[handleVlcStreamEventCallback( %p, %p )] Event: %s",
               p_event, p_data, libvlc_event_type_name( p_event->type )
    );

    switch( p_event->type ) {
        case libvlc_MediaPlayerEncounteredError: {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[handleVlcStreamEventCallback( %p, %p )] VLC encountered an error: %s",
                       p_event, p_data, libvlc_errmsg()
            );
        } break;

        case libvlc_MediaPlayerPlaying: //fallthrough
        case libvlc_MediaMetaChanged:   //fallthrough
        case libvlc_MediaPlayerTitleChanged: {
            //TODO uncomment code once the stream title update issue is sorted in LibVLC

//            libvlc_media_t * media = libvlc_media_player_get_media( (libvlc_media_player_t *) p_data );
//            const char     * title = libvlc_media_get_meta( media, libvlc_meta_NowPlaying );
//
//            if( title != NULL )
//                vlc_player.cb.song_change_callback( title );
        } break;

        default: break;
    }
}

/**
 * [PRIVATE] Shutdown and clean up VLC instance
 * @param instance_ptr     Pointer to the VLC instance pointer
 * @param media_player_ptr Pointer to the initialise a media player pointer
 */
static void ctune_Player_shutdownVLC( libvlc_instance_t ** instance_ptr, libvlc_media_player_t ** media_player_ptr ) {
    if( media_player_ptr != NULL && *media_player_ptr != NULL ) {
        if( libvlc_media_player_is_playing( *media_player_ptr ) ) {
            libvlc_media_player_stop( *media_player_ptr );
        }

        libvlc_media_player_release( *media_player_ptr );
        *media_player_ptr = NULL;
    }

    if( instance_ptr != NULL && *instance_ptr != NULL ) {
        libvlc_release( *instance_ptr );
        *instance_ptr = NULL;
    }
}

/**
 * [PRIVATE] VLC audio playback to sound output callback
 * @param data    Data pointer as passed to libvlc_audio_set_callbacks() [IN]
 * @param samples Pointer to a table of audio samples to play back [IN]
 * @param count   Number of audio samples to play back
 * @param pts     Expected play time stamp (see libvlc_delay())
 */
static void sendToSoundOutCallback( void * data, const void * samples, unsigned count, int64_t pts ) {
    if( vlc_player.cb.playback_ctrl_callback( CTUNE_PLAYBACK_CTRL_STATE ) == CTUNE_PLAYER_STATE_PLAYING ) {
        const unsigned long bytes = ( count * 4 /* bytes (16bits * 2 channels) */ );

        if( bytes > INT_MAX ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[sendToSoundOutCallback( %p, %p, %ud, %ld )] Cast error: sample count (%ud) > INT_MAX (%d)",
                       data, samples, count, pts, count, INT_MAX
            );
            //TODO implement if this comes up as an issue.
            //     (break count into int manageable chunks and advance buff ptr accordingly as chunks are sent)
        } else {
            vlc_player.audio_out->write( samples, (int) bytes );
        }
    }
}

/**
 * [PRIVATE] Start a VLC instance
 * @param instance_ptr     Pointer to the VLC instance pointer
 * @param media_player_ptr Pointer where to initialise a media player at
 * @return Success
 */
static bool ctune_Player_initVLC( libvlc_instance_t ** instance_ptr, libvlc_media_player_t ** media_player_ptr ) {
    bool error_state = false;

    if( instance_ptr == NULL || media_player_ptr == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Player_initVLC( %p, %p )] Error: NULL arg(s).",
                   instance_ptr, media_player_ptr
        );

        error_state = true;
        goto end;
    }

    //libVLC instance (only 1 can exist)
    if( *instance_ptr == NULL ) {
        const char * vlc_argv[] = { "--quiet", "--no-video" };
        const int    vlc_argc   = sizeof( vlc_argv ) / sizeof( *vlc_argv );

        if( ( (*instance_ptr) = libvlc_new( vlc_argc, vlc_argv ) ) == NULL ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_Player_initVLC( %p, %p )] Failed to start a VLC instance.",
                       instance_ptr, media_player_ptr
            );

            error_state = true;
            goto end;
        }

    } else {
        CTUNE_LOG( CTUNE_LOG_DEBUG,
                   "[ctune_Player_initVLC( %p, %p )] LibVLC instance already initialised.",
                   instance_ptr, media_player_ptr
        );
    }

    //Media player instance
    if( *media_player_ptr == NULL ) {
        if( ( *media_player_ptr = libvlc_media_player_new( *instance_ptr ) ) == NULL ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_Player_initVLC( %p, %p )] Failed to create a VLC media player object.",
                       instance_ptr, media_player_ptr
            );

            error_state = true;
            goto end;
        }

    } else {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Player_initVLC( %p, %p )] Error: VLC media player already initialised.",
                   instance_ptr, media_player_ptr
        );

        error_state = true;
        goto end;
    }

    end:
        return !( error_state );
}

/**
 * [PRIVATE] Attaches a callback to a select set of VLC events
 * @param media_player Pointer to a VLC media player instance
 * @param cb           Universal callback method for event processing ( cb(event,media_player) )
 */
static void ctune_Player_attachEventCallbacks( libvlc_media_player_t * media_player, void(* cb)( const struct libvlc_event_t *, void * ) ) {
    libvlc_event_manager_t * event_manager = libvlc_media_player_event_manager( media_player );

    libvlc_event_attach( event_manager, libvlc_MediaPlayerEncounteredError, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaPlayerPlaying, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaMetaChanged, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaPlayerTitleChanged, cb, media_player );

#ifndef NDEBUG
    libvlc_event_attach( event_manager, libvlc_MediaSubItemAdded, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaDurationChanged, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaParsedChanged, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaFreed, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaStateChanged, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaSubItemTreeAdded, cb, media_player );

    libvlc_event_attach( event_manager, libvlc_MediaPlayerMediaChanged, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaPlayerNothingSpecial, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaPlayerOpening, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaPlayerPaused, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaPlayerStopped, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaPlayerForward, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaPlayerBackward, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaPlayerEndReached, cb, media_player );

    libvlc_event_attach( event_manager, libvlc_MediaPlayerSeekableChanged, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaPlayerPausableChanged, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaPlayerSnapshotTaken, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaPlayerLengthChanged, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaPlayerVout, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaPlayerScrambledChanged, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaPlayerESAdded, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaPlayerESDeleted, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaPlayerESSelected, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaPlayerCorked, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaPlayerUncorked, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaPlayerChapterChanged, cb, media_player );

    libvlc_event_attach( event_manager, libvlc_MediaListItemAdded, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaListWillAddItem, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaListItemDeleted, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaListWillDeleteItem, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaListEndReached, cb, media_player );

    libvlc_event_attach( event_manager, libvlc_MediaListViewItemAdded, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaListViewWillAddItem, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaListViewItemDeleted, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaListViewWillDeleteItem, cb, media_player );

    libvlc_event_attach( event_manager, libvlc_MediaListPlayerPlayed, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaListPlayerNextItemSet, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_MediaListPlayerStopped, cb, media_player );

    libvlc_event_attach( event_manager, libvlc_RendererDiscovererItemAdded, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_RendererDiscovererItemDeleted, cb, media_player );

    libvlc_event_attach( event_manager, libvlc_VlmMediaAdded, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_VlmMediaRemoved, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_VlmMediaChanged, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_VlmMediaInstanceStarted, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_VlmMediaInstanceStopped, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_VlmMediaInstanceStatusInit, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_VlmMediaInstanceStatusOpening, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_VlmMediaInstanceStatusPlaying, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_VlmMediaInstanceStatusPause, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_VlmMediaInstanceStatusEnd, cb, media_player );
    libvlc_event_attach( event_manager, libvlc_VlmMediaInstanceStatusError, cb, media_player );
#endif
}

/**
 * Connects and plays a Radio station's stream
 * @param url         Radio station stream URL
 * @param volume      Initial playing volume
 * @param timeout_val Timeout value in seconds
 * @return Success (if false the error_no in the RadioPlayer_t instance will be set accordingly)
 */
static bool ctune_Player_playRadioStream( const char * url, const int volume, int timeout_val ) {
    char * radio_stream_url = strdup( url ); //creating local copy as ref might disappear in other thread

    CTUNE_LOG( CTUNE_LOG_MSG, "[ctune_Player_playRadioStream( \"%s\", %i, %is )] Playing stream.", radio_stream_url, volume, timeout_val );

    vlc_player.error = CTUNE_ERR_NONE;

    int              ret         = 0;
    bool             error_state = false;
    libvlc_media_t * vlc_media   = NULL;
    String_t         last_song   = String.init();

    if( vlc_player.audio_out == NULL ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_Player_playRadioStream( \"%s\", %i, %is )] Mo sound output plugin set.",
                   radio_stream_url, volume, timeout_val
        );

        vlc_player.error = CTUNE_ERR_IO_PLUGIN_NULL;
        error_state      = true;
        goto end;
    }

    if( !ctune_Player_initVLC( &vlc_player.vlc_instance, &vlc_player.vlc_media_player ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Player_playRadioStream( \"%s\", %i, %is )] Failed to start VLC.",
                   radio_stream_url, volume, timeout_val
        );

        vlc_player.error = CTUNE_ERR_PLAYER_INIT;
        error_state      = true;
        goto end;
    }

    ctune_Player_attachEventCallbacks( vlc_player.vlc_media_player, handleVlcStreamEventCallback );

    libvlc_audio_set_format( vlc_player.vlc_media_player, vlc_player.out_sample_fmt.vlc, vlc_player.out_sample_rate, vlc_player.out_channels );
    libvlc_audio_set_callbacks( vlc_player.vlc_media_player, sendToSoundOutCallback, NULL, NULL, NULL, NULL, NULL );

    if( ( vlc_media = libvlc_media_new_location( vlc_player.vlc_instance, url ) ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Player_playRadioStream( \"%s\", %i, %is )] Failed to open URL: %s",
                   radio_stream_url, volume, timeout_val, libvlc_errmsg()
        );

        vlc_player.error = CTUNE_ERR_STREAM_OPEN;
        error_state      = true;
        goto end;
    }

    if( ( ret = vlc_player.audio_out->init( vlc_player.out_sample_fmt.ctune, vlc_player.out_sample_rate, vlc_player.out_channels, 1152, volume ) ) != 0 ) {
        vlc_player.error = abs( ret );                                                          //Might cause problems with SDL2  ^^^^
        error_state = true;
        goto end;
    }

    libvlc_media_player_set_media( vlc_player.vlc_media_player, vlc_media );

    if( libvlc_media_player_play( vlc_player.vlc_media_player ) < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Player_playRadioStream( \"%s\", %i, %is )] Failed to start playback: %s",
                   radio_stream_url, volume, timeout_val, libvlc_errmsg()
        );

        vlc_player.error = CTUNE_ERR_STREAM_OPEN;
        error_state = true;
        goto end;
    }

    while( vlc_player.cb.playback_ctrl_callback( CTUNE_PLAYBACK_CTRL_STATE ) == CTUNE_PLAYER_STATE_PLAYING ) {
        /**
         * FIXME Since LibVLC does not trigger a `libvlc_MediaPlayerTitleChanged` event after the initial start of the
         *       stream there is no way to have the current song properly displayed after that save from checking at
         *       regular interval.. It's a hack but as long as this issue isn't sorted in LibVLC... we're stuck with this.
         */
        libvlc_media_t * media = libvlc_media_player_get_media( vlc_player.vlc_media_player );
        const char     * title = libvlc_media_get_meta( media, libvlc_meta_NowPlaying );

        if( title != NULL && ( String.empty( &last_song ) || strcmp( last_song._raw, title ) != 0 ) ) {
            String.set( &last_song, title );
            vlc_player.cb.song_change_callback( title );
        }

        usleep( 800 ); //FIXME could do with some improvement once the above issue is sorted as this is a bit hacky as a model
    }

    end: //cleanup
        CTUNE_LOG( CTUNE_LOG_DEBUG,
                   "[ctune_Player_playRadioStream( \"%s\", %i, %is )] Shutting down stream.",
                   radio_stream_url, volume, timeout_val
        );

        ctune_Player_shutdownVLC( &vlc_player.vlc_instance, &vlc_player.vlc_media_player );
        vlc_player.audio_out->shutdown();

        if( error_state ) {
            ctune_err.set( vlc_player.error );
        }

        free( radio_stream_url );

        vlc_player.cb.playback_ctrl_callback( CTUNE_PLAYBACK_CTRL_OFF );

        return !(error_state);
}

/**
 * Initialises the RadioPlayer functionalities
 * @param sound_server                   Pointer to the sound server plugin to use as output
 * @param playback_ctrl_callback         Function to check/set the playback state global flag
 * @param song_change_callback           Function to call when stream metadata changes (sends the current stream title)
 */
static void ctune_Player_init(
    ctune_AudioOut_t * sound_server,
    bool(* playback_ctrl_callback)( enum CTUNE_PLAYBACK_CTRL ),
    void(* song_change_callback)( const char * ) )
{
    if( sound_server == NULL || playback_ctrl_callback == NULL || song_change_callback == NULL ) {
        ctune_err.set( CTUNE_ERR_PLAYER_INIT );
    }

    vlc_player.error                             = CTUNE_ERR_NONE;
    vlc_player.audio_out                         = sound_server;
    vlc_player.cb.playback_ctrl_callback         = playback_ctrl_callback;
    vlc_player.cb.song_change_callback           = song_change_callback;
}

/**
 * Gets the error number set in RadioPlayer
 * @return ctune_errno
 */
static int ctune_Player_errno() {
    return vlc_player.error;
}

/**
 * Test stream and get its property
 * @param url         Stream URL
 * @param timeout_val Timeout value in seconds
 * @param codec_str   Pointer to the codec string pointer
 * @param bitrate     Pointer to the bitrate container
 * @return Stream OK
 */
bool ctune_Player_testStream( const char * url, int timeout_val, String_t * codec_str, ulong * bitrate ) {
    CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_Player_testStream( \"%s\", %d, %p, %p )] Testing stream...", url, timeout_val, codec_str, bitrate );

    if( url == NULL || codec_str == NULL || bitrate == NULL ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_Player_testStream( \"%s\", %d, %p, %p )] NULL arg(s)!",
                   url, timeout_val, codec_str, bitrate
        );

        ctune_err.set( CTUNE_ERR_BAD_FUNC_ARGS );
        return false; //EARLY RETURN
    }

    char                  * radio_stream_url = strdup( url ); //local copy
    int                     err_code         = CTUNE_ERR_NONE;
    libvlc_instance_t     * vlc_instance     = NULL;
    libvlc_media_player_t * vlc_media_player = NULL;
    libvlc_media_t        * vlc_media        = NULL;

    if( !ctune_Player_initVLC( &vlc_instance, &vlc_media_player ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Player_testStream( \"%s\", %d, %p, %p )] Failed to start VLC.",
                   radio_stream_url, timeout_val, codec_str, bitrate
        );

        err_code = CTUNE_ERR_PLAYER_INIT;
        goto end;
    }

    libvlc_event_manager_t * event_manager = libvlc_media_player_event_manager( vlc_media_player );
    libvlc_event_attach( event_manager, libvlc_MediaParsedChanged, handleVlcStreamEventCallback, vlc_media_player );

    if( ( vlc_media = libvlc_media_new_location( vlc_instance, url ) ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Player_testStream( \"%s\", %d, %p, %p )] Failed to open URL: %s",
                   radio_stream_url, timeout_val, codec_str, bitrate, libvlc_errmsg()
        );

        err_code = CTUNE_ERR_STREAM_OPEN;
        goto end;
    }

    libvlc_media_player_set_media( vlc_media_player, vlc_media );

    if( ( libvlc_media_parse_with_options( vlc_media, libvlc_media_parse_network, ( timeout_val * 1000 ) ) ) < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Player_testStream( \"%s\", %d, %p, %p )] Failed acquire source stream information: %s",
                   radio_stream_url, timeout_val, codec_str, bitrate, libvlc_errmsg()
        );

        err_code = CTUNE_ERR_STREAM_INFO;
        goto end;
    };

    ctune_Timeout_t timeout = ctune_Timeout.init( timeout_val, CTUNE_ERR_STREAM_INFO, NULL );

    while( !ctune_Timeout.timedOut( &timeout ) && libvlc_media_get_parsed_status( vlc_media ) != libvlc_media_parsed_status_done ) {
        usleep( 100 );
    }

    switch( libvlc_media_get_parsed_status( vlc_media ) ) {
        case libvlc_media_parsed_status_skipped: //fallthrough
        case libvlc_media_parsed_status_failed: {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_Player_testStream( \"%s\", %d, %p, %p )] Failed to acquire source stream information: parse failure.",
                       radio_stream_url, timeout_val, codec_str, bitrate
            );

            err_code = CTUNE_ERR_STREAM_INFO;
        } break;

        case libvlc_media_parsed_status_timeout: {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_Player_testStream( \"%s\", %d, %p, %p )] Failed to acquire source stream information: parse timeout.",
                       radio_stream_url, timeout_val, codec_str, bitrate
            );

            err_code = CTUNE_ERR_STREAM_OPEN_TIMEOUT;
        } break;

        case libvlc_media_parsed_status_done: {
            libvlc_media_track_t ** tracks;
            unsigned int            stream_count = libvlc_media_tracks_get( vlc_media, &tracks );

            if( stream_count > 0 ) {
                *bitrate = tracks[ 0 ]->i_bitrate / 1000; //kbps
                String.set( codec_str, libvlc_media_get_codec_description( libvlc_track_audio, tracks[ 0 ]->i_codec ) );

            } else {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_Player_testStream( \"%s\", %d, %p, %p )] Failed to acquire source stream information: no tracks found (%ud).",
                           radio_stream_url, timeout_val, codec_str, bitrate, stream_count
                );

                err_code = CTUNE_ERR_STREAM_NO_AUDIO;
            }

            libvlc_media_tracks_release( tracks, stream_count );
        } break;
    }

    end:
        CTUNE_LOG( CTUNE_LOG_TRACE,
                   "[ctune_Player_testStream( \"%s\", %d, %p, %p )] Cleaning up.",
                   url, timeout_val, codec_str, bitrate
        );

        libvlc_media_release( vlc_media );
        ctune_Player_shutdownVLC( &vlc_instance, &vlc_media_player );
        free( radio_stream_url );

        if( err_code != CTUNE_ERR_NONE ) {
            ctune_err.set( err_code );
        }

        return ( err_code == CTUNE_ERR_NONE );
}


/**
 * Constructor
 */
const struct ctune_Player_Interface ctune_Player = {
    .init            = &ctune_Player_init,
    .getError        = &ctune_Player_errno,
    .playRadioStream = &ctune_Player_playRadioStream,
    .testStream      = &ctune_Player_testStream,
};