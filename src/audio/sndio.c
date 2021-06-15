#include "AudioOut.h"

#include "../logger/Logger.h"
#include "../ctune_err.h"

#include <sndio.h>

const unsigned abi_version = CTUNE_AUDIOOUT_ABI_VERSION;

/**
 * Audio mixer volume
 */
static volatile int ctune_audio_mix_volume = 0;

/**
 * sndio server information
 */
static struct {
    struct sio_par   param;
    struct sio_hdl * handle;
    bool             vol_enable;
} sndio_audio_server = {
    .handle     = NULL,
    .vol_enable = true,
};

/**
 * Sets a value to the output volume
 * @param vol Volume (0-100)
 */
static void ctune_audio_setVolume( int vol ) {
    if( sndio_audio_server.vol_enable ) {
        int old_vol   = ctune_audio_mix_volume;
        int sndio_vol = 0;

        if( vol <= 0 ) {
            ctune_audio_mix_volume = 0;

        } else if( vol >= 100 ) {
            ctune_audio_mix_volume = 100;
            sndio_vol              = SIO_MAXVOL;

        } else {
            ctune_audio_mix_volume = vol;
            sndio_vol              = ( vol / 100. + 0.005 ) * SIO_MAXVOL;
        }

        if( !sio_setvol( sndio_audio_server.handle, sndio_vol ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_audio_setVolume( %i )] Failed to set sndio volume %i", vol, sndio_vol );
            ctune_audio_mix_volume = old_vol;
        }

        CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_audio_setVolume( %i )] sndio mixing volume: %i%%", vol, ctune_audio_mix_volume );

    } else {
        ctune_err.set( CTUNE_ERR_SNDIO_NOVOL );
    }
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
    if( sndio_audio_server.handle != NULL ) {
        sio_close( sndio_audio_server.handle );
        sndio_audio_server.handle = NULL;
    }
}

/**
 * Initialises sndio
 * @param fmt           Output format
 * @param sample_rate   DSP frequency (samples per second)
 * @param channels      Number of separate sound channels
 * @param samples       Audio buffer size in samples (2^n)
 * @param volume        Pointer to start mixer volume or NULL for restore
 * @return Error code (0 on success)
 */
static int ctune_audio_initAudioOut( ctune_output_fmt_t fmt, int sample_rate, uint channels, uint samples, int volume ) {
    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_audio_initAudioOut( format: %d, sample rate: %i, channels: %u, samples: %u, vol: %i )] Initialising sndio server.",
               fmt, sample_rate, channels, samples, volume
    );

    if( ( sndio_audio_server.handle = sio_open( NULL, SIO_PLAY, 0 ) ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_audio_initAudioOut( %d, %i, %u, %u, %i )] Failed to open sndio.",
                   fmt, sample_rate, channels, samples, volume
        );

        goto failed;
    };

    sio_initpar( &sndio_audio_server.param );

    sndio_audio_server.param.pchan    = channels;
    sndio_audio_server.param.rate     = sample_rate;
    sndio_audio_server.param.sig      = 1; //signed
    sndio_audio_server.param.le       = 1; //little endian
    sndio_audio_server.param.bits     = fmt;
    sndio_audio_server.param.appbufsz = ( sndio_audio_server.param.rate * 300 / 1000 );

    struct sio_par param_cp = sndio_audio_server.param;

    if( !sio_setpar( sndio_audio_server.handle, &sndio_audio_server.param ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_audio_initAudioOut( %d, %i, %u, %u, %i )] Failed to set parameters.",
                   fmt, sample_rate, channels, samples, volume
        );

        goto failed;
    }

    if( !sio_getpar( sndio_audio_server.handle, &sndio_audio_server.param ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_audio_initAudioOut( %d, %i, %u, %u, %i )] Failed to fetch parameters.",
                   fmt, sample_rate, channels, samples, volume
        );

        goto failed;
    }

    bool param_set = param_cp.rate  == sndio_audio_server.param.rate
                  && param_cp.pchan == sndio_audio_server.param.pchan
                  && param_cp.bits  == sndio_audio_server.param.bits
                  && param_cp.le    == sndio_audio_server.param.le
                  && param_cp.sig   == sndio_audio_server.param.sig;

    if( !param_set ){
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_audio_initAudioOut( %d, %i, %u, %u, %i )] Failed check of the parameters set.",
                   fmt, sample_rate, channels, samples, volume
        );

        goto failed;
    }

    if( sio_onvol( sndio_audio_server.handle, NULL, NULL ) == 0 ) {
        CTUNE_LOG( CTUNE_LOG_WARNING,
                   "[ctune_audio_initAudioOut( %d, %i, %u, %u, %i )] Sound attenuation control (volume) is NOT available.",
                   fmt, sample_rate, channels, samples, volume
        );

        sndio_audio_server.vol_enable = false;
    }

    ctune_audio_changeVolume( volume );

    if( !sio_start( sndio_audio_server.handle ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_audio_initAudioOut( %d, %i, %u, %u, %i )] Failed to start sndio.",
                   fmt, sample_rate, channels, samples, volume
        );

        goto failed;
    }

    return 0;

    failed:
        ctune_audio_shutdownAudioOut();
        return -CTUNE_ERR_SNDIO_INIT;
}

/**
 * Sends PCM data to PulseAudio sink (audio output)
 * @param buffer    Pointer to PCM audio data
 * @param buff_size Size of PCM buffer (in bytes)
 */
static void ctune_audio_sendToAudioSink( const void * buffer, int buff_size ) {
    size_t ret = 0;

    if( ( ret = sio_write( sndio_audio_server.handle, buffer, buff_size ) ) != (unsigned) buff_size ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_audio_sendToAudioSink( %p, %i )] Failed to write complete buffer to sndio interface (%d/%lu).",
                   buffer, buff_size, ret, buff_size
        );
    };
}


const struct ctune_AudioOut ctune_AudioOutput = {
    .init         = &ctune_audio_initAudioOut,
    .write        = &ctune_audio_sendToAudioSink,
    .changeVolume = &ctune_audio_changeVolume,
    .getVolume    = &ctune_audio_getVolume,
    .shutdown     = &ctune_audio_shutdownAudioOut
};