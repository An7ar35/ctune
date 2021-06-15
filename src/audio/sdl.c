#include "AudioOut.h"

#include <SDL2/SDL.h> //https://wiki.libsdl.org/

#include "../logger/Logger.h"
#include "../ctune_err.h"

const unsigned abi_version = CTUNE_AUDIOOUT_ABI_VERSION;

static SDL_AudioSpec sdl_audio_specs;

/**
 * Audio buffer information
 *
 * buffer |-----------|------------|
 *        chunk-------pos---length-|
 */
static struct {
    Uint8  * chunk;
    int      length;
    Uint8  * pos;

} audio_buff_info;

/**
 * Audio mixer volume
 */
static volatile int ctune_audio_mix_volume  =  0;
static volatile int sdl_audio_mix_volume    =  0;

/**
 * [PRIVATE] Get the equivalent SLD format from a ctune output format
 * @param fmt ctune output format
 * @return Equivalent SDL format
 */
static SDL_AudioFormat ctune_audio_translateToSDLFormat( ctune_output_fmt_t fmt ) {
    switch( fmt ) {
        case CTUNE_AUDIO_OUTPUT_FMT_S16:
            return AUDIO_S16;

        case CTUNE_AUDIO_OUTPUT_FMT_S32: //fallthrough
        default:
            return AUDIO_S32;
    }
}

/**
 * Callback function to fill the buffer with audio data
 * @param user_data Application-specific parameter saved in the SDL_AudioSpec structure's userdata field
 * @param stream    Pointer to the audio data buffer filled in by SDL_AudioCallback()
 * @param length    Audio data buffer length in bytes
 */
static void fillAudioCallbackFunc( void * user_data, Uint8 * stream, int length ) {
    SDL_memset( stream, 0, length );

    if( audio_buff_info.length == 0 )
        return;

    //Fill buffer with as much audio data as possible
    length = ( length > audio_buff_info.length
               ? audio_buff_info.length
               : length );

    SDL_MixAudio( stream, audio_buff_info.pos, length, sdl_audio_mix_volume );
    audio_buff_info.pos    += length;
    audio_buff_info.length -= length;
}

/**
 * Sets a value to the output volume
 * @param vol Volume (0-100)
 */
static void ctune_audio_setVolume( int vol ) {
    if( vol <= 0 ) {
        ctune_audio_mix_volume = 0;
        sdl_audio_mix_volume   = 0;
    } else if( vol >= 100 ) {
        ctune_audio_mix_volume = 100;
        sdl_audio_mix_volume   = SDL_MIX_MAXVOLUME;
    } else {
        sdl_audio_mix_volume   = (int) ( ( vol / 100. ) * SDL_MIX_MAXVOLUME );
        ctune_audio_mix_volume = vol;
    }

    CTUNE_LOG( CTUNE_LOG_TRACE,
               "[ctune_audio_setVolume( %i )] SDL mixing volume: %i (%i%%)",
               vol, sdl_audio_mix_volume, ctune_audio_mix_volume
    );
}

/**
 * Modify the output volume
 * @param delta Change of volume
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
 * Gets current mixing volume (0-100)
 * @return Output volume as a percentage
 */
static int ctune_audio_getVolume() {
    return ctune_audio_mix_volume;
}

/**
 * Initialises SDL
 * @param fmt           Output format
 * @param sample_rate   DSP frequency (samples per second)
 * @param channels      Number of separate sound channels
 * @param samples       Audio buffer size in samples (2^n)
 * @param volume        Pointer to start mixer volume or NULL for restore
 * @return 0 on success or negative ctune error number
 */
static int ctune_audio_initAudioOut( ctune_output_fmt_t fmt, int sample_rate, uint channels, uint samples, const int volume ) {
    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_audio_initAudioOut( format: %d, sample rate: %i, channels: %u, samples: %u, vol: %i )] Initialising SDL server.",
               fmt, sample_rate, channels, samples, volume
    );

    if( SDL_Init( SDL_INIT_AUDIO | SDL_INIT_TIMER ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_audio_initAudioOut( %d, %i, %u, %u, %d )] Failed SLD init: %s\n",
                   fmt, sample_rate, channels, samples, volume, SDL_GetError()
        );

        return -CTUNE_ERR_SDL_INIT;
    }

    SDL_AudioSpec obtained_audio_specs;

    SDL_zero( sdl_audio_specs );
    sdl_audio_specs.freq     = sample_rate;
    sdl_audio_specs.format   = ctune_audio_translateToSDLFormat( fmt );
    sdl_audio_specs.channels = channels;
    sdl_audio_specs.silence  = 0;
    sdl_audio_specs.samples  = samples;
    sdl_audio_specs.callback = fillAudioCallbackFunc;

    ctune_audio_setVolume( volume );

    if( SDL_OpenAudio( &sdl_audio_specs, &obtained_audio_specs ) < 0 ) {
        return -CTUNE_ERR_SDL_OPEN;
    }

    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_audio_initAudioOut( %d, %i, %u, %u, %d )] "
               "Using SDL. Audio specifications set as: { Sample rate = %i, Channels = %u, Samples = %u }",
               fmt, sample_rate, channels, samples, volume,
               obtained_audio_specs.freq, obtained_audio_specs.channels, obtained_audio_specs.samples
    );

    SDL_PauseAudio( 0 );
    return 0;
}

/**
 * Sends PCM data to SDL sink (audio output)
 * @param buffer    Pointer to PCM audio data
 * @param buff_size Size of PCM buffer (in bytes)
 */
static void ctune_audio_sendToAudioSink( const void * buffer, int buff_size ) {
    while( audio_buff_info.length > 0 )
        SDL_Delay( 1 ); //wait for the buffer to be consumed

    audio_buff_info.chunk  = (Uint8 *) buffer; //PCM audio buffer
    audio_buff_info.length = buff_size;
    audio_buff_info.pos    = audio_buff_info.chunk;
}

/**
 * Calls all the cleaning/closing/shutdown functions for the SDL audio output
 */
static void ctune_audio_shutdownAudioOut() {
    SDL_CloseAudio();
    SDL_Quit();
}

const struct ctune_AudioOut ctune_AudioOutput = {
    .init         = &ctune_audio_initAudioOut,
    .write        = &ctune_audio_sendToAudioSink,
    .setVolume    = &ctune_audio_setVolume,
    .changeVolume = &ctune_audio_changeVolume,
    .getVolume    = &ctune_audio_getVolume,
    .shutdown     = &ctune_audio_shutdownAudioOut
};