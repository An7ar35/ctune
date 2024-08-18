#include "../src/audio/AudioOut.h"

#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include <pipewire/pipewire.h> //https://docs.pipewire.org/page_tutorial.html
#include <spa/param/audio/format-utils.h>
#include <spa/param/props.h>


#include "logger/src/Logger.h"
#include "../src/ctune_err.h"
#include "../src/utils/Timeout.h"
#include "../src/datastructure/CircularBuffer.h"
#include "project_version.h"

const unsigned           abi_version = CTUNE_AUDIOOUT_ABI_VERSION;
const ctune_PluginType_e plugin_type = CTUNE_PLUGIN_OUT_AUDIO_SERVER;

/**
 * Audio mixer volume
 */
static volatile int ctune_audio_mix_volume = 0;


static void ctune_audio_pipewire_onStateChanged( void *, enum pw_stream_state, enum pw_stream_state, const char * );
static void ctune_audio_pipewire_onParamChanged( void *, uint32_t, const struct spa_pod * );
static void ctune_audio_pipewire_onControlInfo( void *, uint32_t, const struct pw_stream_control * );
static void ctune_audio_pipewire_sendDataToStream( void * );

static const struct pw_stream_events stream_events = {
    PW_VERSION_STREAM_EVENTS,
    .state_changed = ctune_audio_pipewire_onStateChanged,
    .param_changed = ctune_audio_pipewire_onParamChanged,
    .control_info  = ctune_audio_pipewire_onControlInfo,
    .process       = ctune_audio_pipewire_sendDataToStream,
};

static void ctune_audio_setVolume( int );
static int  ctune_audio_getVolume();

/**
 * PipeWire server information
 */
static struct {
    struct pw_core        * core;
    struct pw_thread_loop * loop;
    int                     main_loop_ret_val;
    struct pw_stream      * stream;
    struct pw_context     * context;
    struct pw_properties  * properties;
    struct spa_hook         stream_listener;

    int frame_size;
    int channels;
    int rate;
    int volume;

    void(* vol_change_cb)( int );

    volatile sig_atomic_t ready;

    /* Holds data until PipeWire is ready and actually asks for data */
    CircularBuffer_t * buffer;

} pipewire_server = {
    .core              = NULL,
    .loop              = NULL,
    .main_loop_ret_val = 0,
    .stream            = NULL,
    .context           = NULL,
    .properties        = NULL,
    .stream_listener   = NULL,
    .frame_size        = 0,
    .vol_change_cb     = NULL,
};



/**
 * [PRIVATE] Get the equivalent SPA format from a ctune output format
 * @param fmt ctune output format
 * @return Equivalent SPA audio format
 */
static enum spa_audio_format ctune_audio_translateToSPAFormat( ctune_OutputFmt_e fmt ) {
    switch( fmt ) {
        case CTUNE_AUDIO_OUTPUT_FMT_S16:
            return SPA_AUDIO_FORMAT_S16;

        case CTUNE_AUDIO_OUTPUT_FMT_S32: //fallthrough
        default:
            return SPA_AUDIO_FORMAT_S32;
    }
}

/**
 * [PRIVATE] Averages the volumes found an a array of channel volumes
 * @param volumes  Array of volumes
 * @param channels Number of channels in the array
 * @return Average
 */
static float ctune_audio_avgVolume( const float * volumes, uint32_t channels ) {
    float sum = 0.0f;

    for( uint32_t i = 0; i < channels; i++ ) {
        sum += volumes[i];
    }

    return sum / (float) channels;
}

/**
 * [PRIVATE] State change callback
 * @param userdata  User data pointer
 * @param old_state Old pw stream state
 * @param new_state New pw stream state
 * @param error     Error string
 */
static void ctune_audio_pipewire_onStateChanged( void               * userdata,
                                                 enum pw_stream_state old_state,
                                                 enum pw_stream_state new_state,
                                                 const char         * error )
{
    CTUNE_LOG(CTUNE_LOG_DEBUG,
              "[ctune_audio_pipewire_onStateChanged( %p, %d, %d, \'%s\')] State change registered: %s -> %s",
              userdata, old_state, new_state, error,
              pw_stream_state_as_string( old_state ), pw_stream_state_as_string( new_state )
    );

    if( new_state == PW_STREAM_STATE_ERROR ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_audio_pipewire_onStateChanged( %p, %d, %d, \'%s\')] Error in stream state.",
                   userdata, old_state, new_state, error
        )
    }
}

/**
 * [PRIVATE] Param change callback
 * @param userdata User data pointer
 * @param id       Param ID
 * @param param    Pointer to param object
 */
static void ctune_audio_pipewire_onParamChanged( void * userdata, uint32_t id, const struct spa_pod * param ) {
    CTUNE_LOG(CTUNE_LOG_TRACE, "[ctune_audio_pipewire_onParamChanged( %p, %lu, %p )]", userdata, id, param );
}

/**
 * [PRIVATE] Control info change callback
 * @param userdata User data pointer
 * @param id       SPA_PROP_* enum
 * @param control  Pointer to control object
 */
static void ctune_audio_pipewire_onControlInfo( void * userdata, uint32_t id, const struct pw_stream_control * control ) {
    switch( id ) {
        case SPA_PROP_mute:
            CTUNE_LOG(CTUNE_LOG_TRACE, "[ctune_audio_pipewire_onControlInfo( %p, SPA_PROP_mute, %p )] ...", userdata, control );
            break;

        case SPA_PROP_volume:
            CTUNE_LOG(CTUNE_LOG_TRACE, "[ctune_audio_pipewire_onControlInfo( %p, SPA_PROP_volume, %p )] ...", userdata, control );
            break;

        case SPA_PROP_channelVolumes:
            CTUNE_LOG(CTUNE_LOG_TRACE, "[ctune_audio_pipewire_onControlInfo( %p, SPA_PROP_channelVolumes, %p )] ...", userdata, control );
            const float avg     = ctune_audio_avgVolume( control->values, control->n_values );
            const int   percent = (int) (avg * 100);

            ctune_audio_setVolume( percent );

            if( pipewire_server.vol_change_cb != NULL ) {
                pipewire_server.vol_change_cb( percent );
            }

            break;

        case SPA_PROP_softMute:
            CTUNE_LOG(CTUNE_LOG_TRACE, "[ctune_audio_pipewire_onControlInfo( %p, SPA_PROP_softMute, %p )] ...", userdata, control );
            break;

        case SPA_PROP_softVolumes:
            CTUNE_LOG(CTUNE_LOG_TRACE, "[ctune_audio_pipewire_onControlInfo( %p, SPA_PROP_softVolumes, %p )] ...", userdata, control );
            break;

        default:
            CTUNE_LOG(CTUNE_LOG_TRACE, "[ctune_audio_pipewire_onControlInfo( %p, %lu, %p )] ...", userdata, id, control );
            break;
    }
}

/**
 * [PRIVATE]
 * @param userdata User data pointer
 */
static void ctune_audio_pipewire_sendDataToStream( void * userdata ) {
    struct pw_buffer * pw_buffer = NULL;

    if( ( pw_buffer = pw_stream_dequeue_buffer( pipewire_server.stream ) ) == NULL ) {
        CTUNE_LOG(CTUNE_LOG_WARNING, "[ctune_audio_pipewire_sendDataToStream( %p )] Out of buffers", userdata );
        return; //EARLY RETURN
    }

    uint8_t           * stream_buffer = NULL;
    struct spa_buffer * spa_buffer    = pw_buffer->buffer;

    if( ( stream_buffer = spa_buffer->datas[0].data ) == NULL ) {
        CTUNE_LOG(CTUNE_LOG_ERROR, "[ctune_audio_pipewire_sendDataToStream( %p )] Failed to grab stream buffer", userdata );
        return; //EARLY RETURN
    }

    const int n_frames   = SPA_MIN( pw_buffer->requested, spa_buffer->datas[0].maxsize / pipewire_server.frame_size );
    const int chunk_size = ( n_frames * pipewire_server.frame_size );

    pw_stream_queue_buffer( pipewire_server.stream, pw_buffer );

    spa_buffer->datas[0].chunk->offset = 0;
    spa_buffer->datas[0].chunk->stride = pipewire_server.frame_size;
    spa_buffer->datas[0].chunk->size   = CircularBuffer.readChunk( pipewire_server.buffer, stream_buffer, chunk_size );

    pw_stream_queue_buffer( pipewire_server.stream, pw_buffer );
}

/**
 * Sets the volume refresh callback method (to update the UI/internal state on external vol change events)
 * @param cb Callback method
 */
static void ctune_audio_setVolumeChangeCallback( void(* cb)( int ) ) {
    pipewire_server.vol_change_cb = cb;
}

/**
 * Gets current mixing volume (0-100)
 * @return Output volume as a percentage
 */
static int ctune_audio_getVolume() {
    const struct pw_stream_control * ctrl    = pw_stream_get_control( pipewire_server.stream, SPA_PROP_channelVolumes );
    const float                      avg     = ctune_audio_avgVolume( ctrl->values, ctrl->n_values );
    const uint32_t                   percent = avg * 100;

    ctune_audio_mix_volume = (int) percent;

    CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_audio_getVolume()] avg: %f, percent: %lu%%", avg, percent );

    return ctune_audio_mix_volume;
}

/**
 * Sets a value to the output volume
 * @param vol Volume (0-100)
 */
static void ctune_audio_setVolume( int vol ) {
    CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_audio_setVolume( %i )] Pipewire mixing volume: %i%%", vol, ctune_audio_mix_volume );

    float channel_volumes[pipewire_server.channels];
    float new_vol = 0.f;

    if( vol <= 0 ) {
        ctune_audio_mix_volume = 0;

    } else if( vol >= 100 ) {
        ctune_audio_mix_volume = 100;
        new_vol = 1.f;

    } else {
        ctune_audio_mix_volume = vol;
        new_vol = ( vol / 100.f + 0.005f );
    }

    for( int i = 0; i < pipewire_server.channels; ++i ) {
        channel_volumes[ i ] = new_vol;
    }

    CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_audio_setVolume()] new_vol: %f, percent: %lu%%", new_vol, vol );

    ctune_audio_mix_volume = vol;

    pw_thread_loop_lock( pipewire_server.loop );
    pw_stream_set_control( pipewire_server.stream, SPA_PROP_channelVolumes, pipewire_server.channels, channel_volumes, 0 );
    pw_thread_loop_unlock( pipewire_server.loop );
}

/**
 * Modify the output volume
 * @param delta Percent change of volume
 * @return Volume change state
 */
static bool ctune_audio_changeVolume( int delta ) {
    if( delta && pipewire_server.ready ) {
        ctune_audio_setVolume( ctune_audio_mix_volume + delta );
        return true;
    }

    return false;
}

/**
 * Calls all the cleaning/closing/shutdown functions for the PipeWire server
 */
static void ctune_audio_shutdownAudioOut() {
    if( pipewire_server.loop ) {
        pw_thread_loop_stop( pipewire_server.loop );
    }

    if( pipewire_server.stream ) {
        pw_stream_destroy( pipewire_server.stream );
        spa_hook_remove( &pipewire_server.stream_listener );
        spa_zero( pipewire_server.stream_listener );
        pipewire_server.stream  = NULL;
    }

    if( pipewire_server.loop ) {
        pw_thread_loop_destroy( pipewire_server.loop );
        pipewire_server.loop = NULL;
    }

    pipewire_server.ready = false;

    if( pipewire_server.buffer ) {
        CircularBuffer.free( pipewire_server.buffer );
        free( pipewire_server.buffer );
        pipewire_server.buffer = NULL;
    }

    pw_deinit();
    pipewire_server.core    = NULL;
    pipewire_server.context = NULL;
}

/**
 * Gets the plugin's name
 * @return Plugin name string
 */
static const char * ctune_audio_name( void ) {
    return "pipewire";
}

/**
 * Gets the plugin's description
 * @return Plugin description string
 */
static const char * ctune_audio_description( void ) {
    return "PipeWire sound server";
}

/**
 * Initialises PipeWire
 * @param fmt         Output format
 * @param sample_rate DSP frequency (samples per second)
 * @param channels    Number of separate sound channels
 * @param samples     Audio buffer size in samples (2^n)`
 * @param volume      Start mixer volume
 * @return 0 on success or negative ctune error number
 */
static int ctune_audio_initAudioOut( ctune_OutputFmt_e fmt, int sample_rate, uint channels, uint samples, const int volume ) {
    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_audio_initAudioOut( format: %d, sample rate: %i, channels: %u, samples: %u, vol: %i )] Initialising PipeWire server.",
               fmt, sample_rate, channels, samples, volume
    );

    pipewire_server.ready                 = false;
    pipewire_server.main_loop_ret_val     = 0;
    pipewire_server.frame_size            = fmt * channels;
    pipewire_server.channels              = channels;
    pipewire_server.volume                = volume;
    pipewire_server.properties            = pw_properties_new( PW_KEY_CONFIG_NAME, "client-rt.conf",
                                                               PW_KEY_MEDIA_TYPE, "Audio",
                                                               PW_KEY_MEDIA_CATEGORY, "Playback",
                                                               PW_KEY_MEDIA_ROLE, "Music",
                                                               PW_KEY_NODE_NAME, CTUNE_APPNAME,
                                                               PW_KEY_NODE_DESCRIPTION, CTUNE_APPNAME,
                                                               PW_KEY_APP_NAME, CTUNE_APPNAME,
                                                               PW_KEY_APP_ID, CTUNE_APPNAME,
                                                               PW_KEY_APP_PROCESS_ID, CTUNE_APPNAME,
                                                               PW_KEY_APP_ICON_NAME, CTUNE_APPNAME,
                                                               PW_KEY_NODE_ALWAYS_PROCESS, "true",
                                                               NULL );

    pw_init(NULL, NULL);

    int     ret = 0;
    uint8_t buffer[1024];
    float   channel_volumes[channels];

    struct       spa_pod_builder pw_spa_pod_builder = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
    const struct spa_pod       * pod_parameters[1];

    if( ( pipewire_server.loop = pw_thread_loop_new( "ctune/output/pw", NULL ) ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_audio_initAudioOut( %d, %i, %u, %u, %i )] Failed to create threaded loop.",
                   fmt, sample_rate, channels, samples, volume
        );

        goto failed;
    }

    pw_thread_loop_lock( pipewire_server.loop );

    if( ( ret = pw_thread_loop_start( pipewire_server.loop ) ) < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_audio_initAudioOut( %d, %i, %u, %u, %i )] Failed to start threaded loop: %s",
                   fmt, sample_rate, channels, samples, volume, strerror( ret )
        );

        goto failed_while_locked;
    }

    struct pw_context * context = pw_context_new( pw_thread_loop_get_loop( pipewire_server.loop ), pipewire_server.properties, 0 );

    if( context == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_audio_initAudioOut( %d, %i, %u, %u, %i )] Failed to create context.",
                   fmt, sample_rate, channels, samples, volume
        );

        goto failed_while_locked;
    }

    if( !( pipewire_server.core = pw_context_connect( context, pipewire_server.properties, 0 ) ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_audio_initAudioOut( %d, %i, %u, %u, %i )] Failed to connect to context: %s",
                   fmt, sample_rate, channels, samples, volume, strerror( errno )
        );

        goto failed_while_locked;
    }

    //TODO audio volume

    pipewire_server.stream = pw_stream_new( pipewire_server.core, "ctune-pw-out", pipewire_server.properties );

    pw_stream_add_listener( pipewire_server.stream, &pipewire_server.stream_listener, &stream_events, NULL);

    struct spa_pod_builder    pw_pod     = SPA_POD_BUILDER_INIT( buffer, sizeof( buffer ) );
    struct spa_audio_info_raw audio_info = SPA_AUDIO_INFO_RAW_INIT( .format   = ctune_audio_translateToSPAFormat( fmt ),
                                                                    .channels = channels,
                                                                    .rate     = sample_rate );

    pod_parameters[0] = spa_format_audio_raw_build( &pw_pod, SPA_PARAM_EnumFormat, &audio_info );

    pw_stream_connect( pipewire_server.stream,
                       PW_DIRECTION_OUTPUT,
                       PW_ID_ANY,
                       PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS | PW_STREAM_FLAG_RT_PROCESS,
                       pod_parameters, 1 );

    //Set channel volumes
//    ctune_audio_pipewire_setVolume( volume, &channel_volumes[0] );
//    ctune_audio_mix_volume = volume;
//    pw_stream_set_control( pipewire_server.stream, SPA_PROP_channelVolumes, pipewire_server.channels, channel_volumes, 0 );

    //Setup buffer for pipewire to feed from
    pipewire_server.buffer = malloc( sizeof( CircularBuffer_t ) );
    (*pipewire_server.buffer) = CircularBuffer.create();
    CircularBuffer.init( pipewire_server.buffer, ( ( fmt * sample_rate * channels ) * 4 ), true );

    pw_thread_loop_unlock( pipewire_server.loop );

    ctune_audio_setVolume( volume );

    pipewire_server.ready = true;

    return 0;

    failed_while_locked:
        pw_thread_loop_unlock( pipewire_server.loop );

    failed:
        ctune_audio_shutdownAudioOut();
        return -CTUNE_ERR_PULSE_INIT;
}

/**
 * Sends PCM data to PipeWire sink (audio output)
 * @param buffer    Pointer to PCM audio data
 * @param buff_size Size of PCM buffer (in bytes)
 */
static void ctune_audio_sendToAudioSink( const void * buffer, int buff_size ) {
    CircularBuffer.writeChunk( pipewire_server.buffer, buffer, buff_size );
}

/**
 * Constructor
 */
const struct ctune_AudioOut ctune_AudioOutput = {
    .name                    = &ctune_audio_name,
    .description             = &ctune_audio_description,
    .init                    = &ctune_audio_initAudioOut,
    .write                   = &ctune_audio_sendToAudioSink,
    .setVolumeChangeCallback = &ctune_audio_setVolumeChangeCallback,
    .setVolume               = &ctune_audio_setVolume,
    .changeVolume            = &ctune_audio_changeVolume,
    .getVolume               = &ctune_audio_getVolume,
    .shutdown                = &ctune_audio_shutdownAudioOut
};