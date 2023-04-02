#include "../src/audio/AudioOut.h"

#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include <pipewire/pipewire.h> //https://docs.pipewire.org/page_tutorial.html

#include "logger/src/Logger.h"
#include "../src/ctune_err.h"
#include "../src/utils/Timeout.h"
#include "project_version.h"

const unsigned           abi_version = CTUNE_AUDIOOUT_ABI_VERSION;
const ctune_PluginType_e plugin_type = CTUNE_PLUGIN_OUT_AUDIO_SERVER;

/**
 * Audio mixer volume
 */
static volatile int ctune_audio_mix_volume = 0;

/**
 * PipeWire server information
 */
static struct {

    volatile sig_atomic_t  ready;

    struct LatencyChanges {
        uint64_t count;
        float    low;
        float    high;
    } latency;

} pulse_audio_server;

///**
// * [PRIVATE] Get the equivalent SLD format from a ctune output format
// * @param fmt ctune output format
// * @return Equivalent PulseAudio format
// */
//static pa_sample_format_t ctune_audio_translateToALSAFormat( ctune_OutputFmt_e fmt ) {
//    switch( fmt ) {
//        case CTUNE_AUDIO_OUTPUT_FMT_S16:
//            return PA_SAMPLE_S16LE;
//
//        case CTUNE_AUDIO_OUTPUT_FMT_S32: //fallthrough
//        default:
//            return PA_SAMPLE_S32LE;
//    }
//}

/**
 * Gets current mixing volume (0-100)
 * @return Output volume as a percentage
 */
static int ctune_audio_getVolume() { //TODO
//    pa_volume_t avg     = pa_cvolume_avg( &pulse_audio_server.channel_volume );
//    uint32_t    percent = ( (double) avg / PA_VOLUME_NORM ) * 100;
//
//    ctune_audio_mix_volume = (int) percent;

    return ctune_audio_mix_volume;
}

/**
 * Sets a value to the output volume
 * @param vol Volume (0-100)
 */
static void ctune_audio_setVolume( int vol ) { //TODO
//    pa_volume_t new_vol = PA_VOLUME_MUTED;
//
//    if( vol <= 0 ) {
//        ctune_audio_mix_volume = 0;
//
//    } else if( vol >= 100 ) {
//        ctune_audio_mix_volume = 100;
//        new_vol = PA_VOLUME_NORM;
//
//    } else {
//        ctune_audio_mix_volume = vol;
//        new_vol = ( vol / 100. + 0.005 ) * PA_VOLUME_NORM;
//    }
//
//    pa_cvolume_set( &pulse_audio_server.channel_volume, pulse_audio_server.channel_map.channels, new_vol );
//
//    CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_audio_setVolume( %i )] Pulse mixing volume: %i%%", vol, ctune_audio_mix_volume );
//
//
//    pa_threaded_mainloop_lock( pulse_audio_server.main_loop );
//
//    pa_context_set_sink_input_volume( pulse_audio_server.context,
//                                      pa_stream_get_index( pulse_audio_server.stream ),
//                                      &pulse_audio_server.channel_volume,
//                                      NULL,
//                                      NULL );
//
//    pa_threaded_mainloop_unlock( pulse_audio_server.main_loop );
}

/**
 * Modify the output volume
 * @param delta Percent change of volume
 * @return Volume change state
 */
static bool ctune_audio_changeVolume( int delta ) { //TODO
//    if( delta && pulse_audio_server.ready ) {
//        ctune_audio_setVolume( ctune_audio_mix_volume + delta );
//        return true;
//    }

    return false;
}

/**
 * Calls all the cleaning/closing/shutdown functions for the PulseAudio server
 */
static void ctune_audio_shutdownAudioOut() { //TODO
//    if( pulse_audio_server.overflow_count ) {
//        CTUNE_LOG( CTUNE_LOG_WARNING,
//                   "[ctune_audio_shutdownAudioOut()] PulseAudio stream overflowed %lu times.",
//                   pulse_audio_server.overflow_count
//        );
//    }
//
//    if( pulse_audio_server.latency.count ) {
//        CTUNE_LOG( CTUNE_LOG_MSG,
//                   "[ctune_audio_shutdownAudioOut()] PulseAudio stream latency changed %lu times (%0.0f <-> %0.0f useconds).",
//                   pulse_audio_server.latency.count, pulse_audio_server.latency.low, pulse_audio_server.latency.high
//        );
//    }
//
//    pulse_audio_server.ready = false;
//
//    if( pulse_audio_server.main_loop ) {
//        pa_threaded_mainloop_stop( pulse_audio_server.main_loop );
//    }
//
//    if( pulse_audio_server.context ) {
//        pa_context_disconnect( pulse_audio_server.context );
//        pa_context_unref( pulse_audio_server.context );
//        pulse_audio_server.context = NULL;
//    }
//
//    if( pulse_audio_server.stream ) {
//        pa_stream_disconnect( pulse_audio_server.stream );
//        pa_stream_unref( pulse_audio_server.stream );
//        pulse_audio_server.stream = NULL;
//    }
//
//    if( pulse_audio_server.main_loop ) {
//        pa_threaded_mainloop_free( pulse_audio_server.main_loop );
//        pulse_audio_server.main_loop = NULL;
//    }
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

//    pa_proplist * property_list = pa_proplist_new();
//    pa_proplist_sets( property_list, PA_PROP_APPLICATION_ID, CTUNE_APPNAME );
//    pa_proplist_sets( property_list, PA_PROP_APPLICATION_NAME, CTUNE_APPNAME );
//
//    pulse_audio_server.overflow_count        = 0;
//    pulse_audio_server.latency               = (struct LatencyChanges){ 0, 0.0f, 0.0f };
//    pulse_audio_server.ready                 = false;
//    pulse_audio_server.main_loop             = pa_threaded_mainloop_new();
//    pulse_audio_server.main_loop_ret_val     = 0;
//    pulse_audio_server.mainloop_api          = pa_threaded_mainloop_get_api( pulse_audio_server.main_loop );
//    pulse_audio_server.context               = pa_context_new_with_proplist( pulse_audio_server.mainloop_api, NULL, property_list );
//
//    pa_proplist_free( property_list );
//
//    pa_channel_map_init_stereo( &pulse_audio_server.channel_map );
//    pulse_audio_server.sample_specs.format   = ctune_audio_translateToALSAFormat( fmt );
//    pulse_audio_server.sample_specs.rate     = sample_rate;
//    pulse_audio_server.sample_specs.channels = channels;
//
//    pa_context_set_state_callback( pulse_audio_server.context, &notifyContextStateChangeCallback, pulse_audio_server.main_loop );
//
//    //create context
//    if( pa_context_connect( pulse_audio_server.context, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL ) < 0 ) {
//        CTUNE_LOG( CTUNE_LOG_ERROR,
//                   "[ctune_audio_initAudioOut( %d, %i, %u, %u, %i )] Failed to connect context to PulseAudio server.",
//                   fmt, sample_rate, channels, samples, volume
//        );
//
//        goto failed;
//    }
//
//    pa_threaded_mainloop_lock( pulse_audio_server.main_loop );
//
//    if( pa_threaded_mainloop_start( pulse_audio_server.main_loop ) < 0 ) {
//        CTUNE_LOG( CTUNE_LOG_ERROR,
//                   "[ctune_audio_initAudioOut( %d, %i, %u, %u, %i )] Failed to start PulseAudio mainloop.",
//                   fmt, sample_rate, channels, samples, volume
//        );
//
//        pa_threaded_mainloop_unlock( pulse_audio_server.main_loop );
//        goto failed;
//    }
//
//
//    //wait for the context to be ready
//    bool context_ready = false;
//
//    while( !context_ready ) {
//        pa_context_state_t context_state = pa_context_get_state( pulse_audio_server.context );
//
//        if( PA_CONTEXT_IS_GOOD( context_state ) == 0 ) {
//            CTUNE_LOG( CTUNE_LOG_ERROR,
//                       "[ctune_audio_initAudioOut( %d, %i, %u, %u, %i )] Failed to init PulseAudio context.",
//                       fmt, sample_rate, channels, samples, volume
//            );
//            pa_threaded_mainloop_unlock( pulse_audio_server.main_loop );
//            goto failed;
//
//        } else if( context_state == PA_CONTEXT_READY ) {
//            context_ready = true;
//
//        } else {
//            pa_threaded_mainloop_wait( pulse_audio_server.main_loop );
//        }
//    }
//
//    pulse_audio_server.stream = pa_stream_new( pulse_audio_server.context, "playback", &pulse_audio_server.sample_specs, &pulse_audio_server.channel_map );
//
//    if( !pulse_audio_server.stream ) {
//        CTUNE_LOG( CTUNE_LOG_ERROR, "Pulse audio error: %s", pa_strerror( pa_context_errno( pulse_audio_server.context ) ) );
//        pa_threaded_mainloop_unlock( pulse_audio_server.main_loop );
//        goto failed;
//    }
//
//    //create playback stream
//    pa_stream_set_state_callback( pulse_audio_server.stream, notifyStreamStateChangeCallBack, pulse_audio_server.main_loop );
//    pa_stream_set_underflow_callback( pulse_audio_server.stream, notifyStreamUnderflowCallback, pulse_audio_server.main_loop );
//    pa_stream_set_overflow_callback( pulse_audio_server.stream, notifyStreamOverflowCallback, pulse_audio_server.main_loop );
//    pa_stream_set_latency_update_callback( pulse_audio_server.stream, notifyLatencyUpdateCallback, pulse_audio_server.main_loop );
//    pa_context_set_state_callback( pulse_audio_server.context, &notifyContextStateChangeCallback, pulse_audio_server.main_loop );
//
//    pa_cvolume   * init_volume = &pulse_audio_server.channel_volume;
//    pa_buffer_attr buffer_attributes = { .tlength   = -1,
//        .minreq    = -1,
//        .maxlength = -1,
//        .prebuf    = -1,
//        .fragsize  = -1, };
//
//    ctune_audio_setVolume( volume );
//
//    const pa_stream_flags_t flags = PA_STREAM_ADJUST_LATENCY | PA_STREAM_VARIABLE_RATE | PA_STREAM_AUTO_TIMING_UPDATE | PA_STREAM_INTERPOLATE_TIMING;
//
//    if( pa_stream_connect_playback( pulse_audio_server.stream, NULL, &buffer_attributes, flags, init_volume, NULL ) ) {
//        CTUNE_LOG( CTUNE_LOG_ERROR,
//                   "[ctune_audio_initAudioOut( %d, %i, %u, %u, %i )] Failed to connect PulseAudio stream to the default audio output sink.",
//                   fmt, sample_rate, channels, samples, volume
//        );
//        pa_threaded_mainloop_unlock( pulse_audio_server.main_loop );
//        goto failed;
//    }
//
//    //wait for the stream to be ready
//    bool stream_ready = false;
//
//    while( !stream_ready ) {
//        pa_stream_state_t stream_state = pa_stream_get_state( pulse_audio_server.stream );
//
//        if( PA_STREAM_IS_GOOD( stream_state ) == 0 ) {
//            CTUNE_LOG( CTUNE_LOG_ERROR,
//                       "[ctune_audio_initAudioOut( %d, %i, %u, %u, %i )] Failed to init PulseAudio stream.",
//                       fmt, sample_rate, channels, samples, volume
//            );
//            pa_threaded_mainloop_unlock( pulse_audio_server.main_loop );
//            goto failed;
//
//        } else if( stream_state == PA_STREAM_READY ) {
//            stream_ready = true;
//
//        } else {
//            pa_threaded_mainloop_wait( pulse_audio_server.main_loop );
//        }
//    }
//
//    const pa_buffer_attr * buff_attr = pa_stream_get_buffer_attr( pulse_audio_server.stream );
//
//    if( buff_attr ) {
//        CTUNE_LOG( CTUNE_LOG_DEBUG,
//                   "Pulse audio stream buffer attributes = { maxlength: %lu, fragsize: %lu, minreq: %lu, tlength: %lu }",
//                   buff_attr->maxlength, buff_attr->fragsize, buff_attr->minreq, buff_attr->prebuf,
//                   buff_attr->tlength );
//    }
//
//    pa_threaded_mainloop_unlock( pulse_audio_server.main_loop );
//
//    return 0;
//
//    failed:
//    ctune_audio_shutdownAudioOut();
//    return -CTUNE_ERR_PULSE_INIT;
}

/**
 * Sends PCM data to PulseAudio sink (audio output)
 * @param buffer    Pointer to PCM audio data
 * @param buff_size Size of PCM buffer (in bytes)
 */
static void ctune_audio_sendToAudioSink( const void * buffer, int buff_size ) {
    //TODO
}

/**
 * Constructor
 */
const struct ctune_AudioOut ctune_AudioOutput = {
    .name         = &ctune_audio_name,
    .description  = &ctune_audio_description,
    .init         = &ctune_audio_initAudioOut,
    .write        = &ctune_audio_sendToAudioSink,
    .setVolume    = &ctune_audio_setVolume,
    .changeVolume = &ctune_audio_changeVolume,
    .getVolume    = &ctune_audio_getVolume,
    .shutdown     = &ctune_audio_shutdownAudioOut
};