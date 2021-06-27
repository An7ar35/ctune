#include <alsa/asoundlib.h> //https://www.alsa-project.org/alsa-doc/alsa-lib/
#include <limits.h>

#include "AudioOut.h"
#include "../ctune_err.h"
#include "../logger/Logger.h"

const unsigned abi_version = CTUNE_AUDIOOUT_ABI_VERSION;

/**
 * Audio mixer volume
 */
static volatile int ctune_audio_mix_volume = 0;

/**
 * PulseAudio server information
 * @param device_name Name of output sound card to use
 * @param access          PCM access type
 * @param hw_params       Hardware parameters
 * @param pcm_handle      Pointer to PCM device handle
 * @param mixer_name      Name of the mixer to use
 * @param mixer_handle    Pointer to mixer handle
 * @param mixer_id        Mixer simple element identifier
 * @param mixer_element   Pointer to mixer element handle
 * @param frame_byte_size Size of a frame in bytes
 */
static struct {
    const char           * device_name;
    const snd_pcm_access_t access;
    snd_pcm_hw_params_t  * hw_params;
    snd_pcm_t            * pcm_handle;
    const char           * mixer_name;
    snd_mixer_t          * mixer_handle;
    snd_mixer_selem_id_t * mixer_id;
    snd_mixer_elem_t     * mixer_element;
    unsigned               frame_byte_size;
} alsa_audio_server = {
    .device_name     = "default",
    .access          = SND_PCM_ACCESS_RW_INTERLEAVED,
    .hw_params       = NULL,
    .pcm_handle      = NULL,
    .mixer_name      = "PCM",
    .mixer_handle    = NULL,
    .mixer_id        = NULL,
    .mixer_element   = NULL,
    .frame_byte_size = 0,
};

/**
 * [PRIVATE] Get the equivalent SLD format from a ctune output format
 * @param fmt ctune output format
 * @return Equivalent SDL format
 */
static snd_pcm_format_t ctune_audio_translateToALSAFormat( ctune_OutputFmt_e fmt ) {
    switch( fmt ) {
        case CTUNE_AUDIO_OUTPUT_FMT_S16:
            return SND_PCM_FORMAT_S16_LE;

        case CTUNE_AUDIO_OUTPUT_FMT_S32: //fallthrough
        default:
            return SND_PCM_FORMAT_S32_LE;
    }
}

/**
 * Sets a value to the output volume
 * @param vol Volume (0-100)
 */
static void ctune_audio_setVolume( int vol ) {
    long alsa_min = 0;
    long alsa_max = 0;
    long alsa_vol = 0;

    snd_mixer_selem_get_playback_volume_range( alsa_audio_server.mixer_element, &alsa_min, &alsa_max );

    if( vol <= 0 ) {
        ctune_audio_mix_volume = 0;
        alsa_vol               = alsa_min;
    } else if( vol >= 100 ) {
        ctune_audio_mix_volume = 100;
        alsa_vol               = alsa_max;
    } else {
        alsa_vol               = ( vol * alsa_max / 100 );
        ctune_audio_mix_volume = vol;
    }

    snd_mixer_selem_set_playback_volume_all( alsa_audio_server.mixer_element, alsa_vol );

    CTUNE_LOG( CTUNE_LOG_TRACE,
               "[ctune_audio_setVolume( %i )] ALSA mixing volume: %i (%i%%)",
               vol, alsa_vol, ctune_audio_mix_volume
    );
}

/**
 * Gets current mixing volume (0-100)
 * @return Output volume as a percentage
 */
static int ctune_audio_getVolume() {
    return ctune_audio_mix_volume;
}

/**
 * Modify the output volume
 * @param delta Percent change of volume
 * @return Volume change state
 */
static bool ctune_audio_changeVolume( int delta ) {
    if( delta ) {
        ctune_audio_setVolume( ctune_audio_mix_volume + delta );
        return true;
    }

    return false;
}

/**
 * Calls all the cleaning/closing/shutdown functions for the Alsa server
 */
static void ctune_audio_shutdownAudioOut() {
    if( alsa_audio_server.pcm_handle != NULL ) {
        int ret = 0;

        if( ( ret = snd_pcm_drain( alsa_audio_server.pcm_handle ) ) < 0 ) {
            CTUNE_LOG( CTUNE_LOG_WARNING,
                       "[ctune_audio_shutdownAudioOut()] Failed to drain ALSA device: %s",
                       snd_strerror( ret )
            );
        };

        if( ( ret = snd_pcm_close( alsa_audio_server.pcm_handle ) ) < 0 ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_audio_shutdownAudioOut()] Failed to close ALSA device: %s",
                       snd_strerror( ret )
            );
        };

        snd_pcm_hw_free( alsa_audio_server.pcm_handle );
        alsa_audio_server.pcm_handle = NULL;
    }

    if( alsa_audio_server.hw_params != NULL ) {
        snd_pcm_hw_params_free( alsa_audio_server.hw_params );
        alsa_audio_server.hw_params = NULL;
    }

    if( alsa_audio_server.mixer_handle ) {
        snd_mixer_close( alsa_audio_server.mixer_handle );
        alsa_audio_server.mixer_handle  = NULL;
        alsa_audio_server.mixer_id      = NULL;
        alsa_audio_server.mixer_element = NULL;
    }
}

/**
 * [PRIVATE] Sets the hardware parameters
 * @param format      Sample format
 * @param sample_rate Sample rate
 * @param channels    Number of channels
 * @return Success
 */
static bool setAlsaHwParams( snd_pcm_format_t format, int sample_rate, uint channels ) {
    int ret = 0;

    if( ( ret = snd_pcm_hw_params_malloc( &alsa_audio_server.hw_params ) ) < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[setAlsaHwParams( '%s', %i, %u )] Failed to allocate hardware parameter structure: %s",
                   snd_pcm_format_name( format ), sample_rate, channels, snd_strerror( ret )
        );

        return false; //EARLY RETURN
    }

    if( ( ret = snd_pcm_hw_params_any( alsa_audio_server.pcm_handle, alsa_audio_server.hw_params ) ) < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[setAlsaHwParams( '%s', %i, %u )] Failed to initialise hardware parameter structure: %s",
                   snd_pcm_format_name( format ), sample_rate, channels, snd_strerror( ret )
        );

        return false; //EARLY RETURN
    }

    if( ( ret = snd_pcm_hw_params_set_access( alsa_audio_server.pcm_handle, alsa_audio_server.hw_params, alsa_audio_server.access ) ) < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[setAlsaHwParams( '%s', %i, %u )] Failed to set access type: %s",
                   snd_pcm_format_name( format ), sample_rate, channels, snd_strerror( ret )
        );

        return false; //EARLY RETURN
    }

    if( ( ret = snd_pcm_hw_params_set_format( alsa_audio_server.pcm_handle, alsa_audio_server.hw_params, format ) ) < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[setAlsaHwParams( '%s', %i, %u )] Failed to set sample format: %s",
                   snd_pcm_format_name( format ), sample_rate, channels, snd_strerror( ret )
        );

        return false; //EARLY RETURN
    }

    if( ( ret = snd_pcm_hw_params_set_channels( alsa_audio_server.pcm_handle, alsa_audio_server.hw_params, channels ) ) < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[setAlsaHwParams( '%s', %i, %u )] Failed to set channel count: %s",
                   snd_pcm_format_name( format ), sample_rate, channels, snd_strerror( ret )
        );

        return false; //EARLY RETURN
    }

    if( ( ret = snd_pcm_hw_params_set_rate( alsa_audio_server.pcm_handle, alsa_audio_server.hw_params, sample_rate, 0 ) ) < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[setAlsaHwParams( '%s', %i, %u )] Failed to set sample rate: %s",
                   snd_pcm_format_name( format ), sample_rate, channels, snd_strerror( ret )
        );

        return false; //EARLY RETURN
    }

    if( ( ret = snd_pcm_hw_params( alsa_audio_server.pcm_handle, alsa_audio_server.hw_params ) ) < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[setAlsaHwParams( '%s', %i, %u )] Failed to set parameters: %s",
                   snd_pcm_format_name( format ), sample_rate, channels, snd_strerror( ret )
        );

        return false; //EARLY RETURN
    }

    return true;
}

/**
 * [PRIVATE] Initialises the ALSA mixer
 * @return Success
 */
static bool initAlsaMixer() {
    int ret = 0;

    if( ( ret = snd_mixer_open( &alsa_audio_server.mixer_handle, 0 ) ) < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[initAlsaMixer()] Failed to open mixer: %s", snd_strerror( ret ) );
        return false; //EARLY RETURN
    }

    if( ( ret = snd_mixer_attach( alsa_audio_server.mixer_handle, alsa_audio_server.device_name ) ) < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[initAlsaMixer()] Failed to attach mixer to '%s' device: %s",
                   alsa_audio_server.device_name, snd_strerror( ret )
        );

        return false; //EARLY RETURN
    }

    if( ( ret = snd_mixer_selem_register( alsa_audio_server.mixer_handle, NULL, NULL ) ) < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[initAlsaMixer()] Failed to register mixer element: %s", snd_strerror( ret ) );
        return false; //EARLY RETURN
    }

    if( ( ret = snd_mixer_load( alsa_audio_server.mixer_handle ) ) < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[initAlsaMixer()] Failed to load mixer: %s", snd_strerror( ret ) );
        return false; //EARLY RETURN
    }

    snd_mixer_selem_id_alloca( &alsa_audio_server.mixer_id );
    snd_mixer_selem_id_set_index( alsa_audio_server.mixer_id, 0 );
    snd_mixer_selem_id_set_name( alsa_audio_server.mixer_id, alsa_audio_server.mixer_name );

    if( ( alsa_audio_server.mixer_element = snd_mixer_find_selem( alsa_audio_server.mixer_handle, alsa_audio_server.mixer_id ) ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[initAlsaMixer()] Failed to find mixer element." );
        return false; //EARLY RETURN
    };

    return true;
}

/**
 * Initialises ALSA
 * @param fmt         Output format
 * @param sample_rate DSP frequency (samples per second)
 * @param channels    Number of separate sound channels
 * @param samples     Audio buffer size in samples (i.e. size of 1 frame in bytes)
 * @param volume      Pointer to start mixer volume or NULL for restore
 * @return Error code (0 on success)
 */
static int ctune_audio_initAudioOut( ctune_OutputFmt_e fmt, int sample_rate, uint channels, uint samples, int volume ) {
    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_audio_initAudioOut( format: %d, sample rate: %i, channels: %u, samples: %u, vol: %i )] Initialising ALSA server.",
               fmt, sample_rate, channels, samples, volume
    );

    int ret = 0;

    if( ( ret = snd_pcm_open( &alsa_audio_server.pcm_handle, alsa_audio_server.device_name, SND_PCM_STREAM_PLAYBACK, 0 ) ) < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_audio_initAudioOut( %d, %i, %u, %u, %i )] Failed to open '%s' device: %s",
                   fmt, sample_rate, channels, samples, volume, alsa_audio_server.device_name, snd_strerror( ret )
        );

        goto failed;
    }

    snd_pcm_format_t format = ctune_audio_translateToALSAFormat( fmt );

    if( !setAlsaHwParams( format, sample_rate, channels ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_audio_initAudioOut( %d, %i, %u, %u, %i )] Failed to set HW parameters for '%s' device.",
                   fmt, sample_rate, channels, samples, volume, alsa_audio_server.device_name
        );

        goto failed;
    }

    alsa_audio_server.frame_byte_size = snd_pcm_frames_to_bytes( alsa_audio_server.pcm_handle, 1 );

    if( !initAlsaMixer() ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_audio_initAudioOut( %d, %i, %u, %u, %i )] Failed to initialise the mixer for '%s' device.",
                   fmt, sample_rate, channels, samples, volume, alsa_audio_server.device_name
        );

        goto failed;
    }

    if( ( ret = snd_pcm_prepare( alsa_audio_server.pcm_handle ) ) < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_audio_initAudioOut( %d, %i, %u, %u, %i )] Failed to prepare sound device '%s': %s",
                   fmt, sample_rate, channels, samples, volume, alsa_audio_server.device_name, snd_strerror( ret )
        );

        goto failed;
    };

    ctune_audio_changeVolume( volume );

    return 0;

    failed:
        ctune_audio_shutdownAudioOut();
        return -CTUNE_ERR_PULSE_INIT;
}

/**
 * Sends PCM data to PulseAudio sink (audio output)
 * @param buffer    Pointer to PCM audio data
 * @param buff_size Size of PCM buffer (in bytes)
 */
static void ctune_audio_sendToAudioSink( const void * buffer, int buff_size ) {
    long     written     = 0;
    int      recovered   = 0;
    unsigned frame_count = ( buff_size / alsa_audio_server.frame_byte_size );

    start: {
        written = snd_pcm_writei( alsa_audio_server.pcm_handle, buffer, frame_count );

        if( written < 0 ) {
            if( !recovered && ( written == -EPIPE || written == -EINTR || written == -ESTRPIPE ) ) {
                CTUNE_LOG( CTUNE_LOG_WARNING,
                           "[ctune_audio_sendToAudioSink( %p, %i )] Write failure: %s",
                           buffer, buff_size, snd_strerror( written )
                );

                ++recovered;

                if( written < INT_MIN || written > INT_MAX ) {
                    CTUNE_LOG( CTUNE_LOG_ERROR,
                               "[ctune_audio_sendToAudioSink( %p, %i )] Can't cast long->int (%ld)",
                               buff_size, buff_size, written
                    );

                    return; //EARLY RETURN
                }

                if( ( written = snd_pcm_recover( alsa_audio_server.pcm_handle, (int) written, 0 ) ) == 0 ) {
                    CTUNE_LOG( CTUNE_LOG_DEBUG,
                               "[ctune_audio_sendToAudioSink( %p, %i )] Recovery successful: %s",
                               buffer, buff_size, snd_strerror( written )
                    );

                    goto start;

                } else {
                    CTUNE_LOG( CTUNE_LOG_DEBUG,
                               "[ctune_audio_sendToAudioSink( %p, %i )] Failed recovery: %s",
                               buffer, buff_size, snd_strerror( written )
                    );
                };
            }

        } else if( labs( written ) != frame_count ) {
            CTUNE_LOG( CTUNE_LOG_WARNING,
                       "[ctune_audio_sendToAudioSink( %p, %i )] Failed to write complete buffer to ALSA interface (%d/%lu)",
                       buffer, buff_size, written, frame_count
            );
        }
    }
}


const struct ctune_AudioOut ctune_AudioOutput = {
    .init         = &ctune_audio_initAudioOut,
    .write        = &ctune_audio_sendToAudioSink,
    .changeVolume = &ctune_audio_changeVolume,
    .getVolume    = &ctune_audio_getVolume,
    .shutdown     = &ctune_audio_shutdownAudioOut
};