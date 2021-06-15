#ifndef CTUNE_AUDIO_AUDIOOUT_H
#define CTUNE_AUDIO_AUDIOOUT_H

#include <stdbool.h>
#include "output_format.h"
#include "../datastructure/String.h"

#define CTUNE_AUDIOOUT_ABI_VERSION 1

typedef unsigned int uint;

typedef struct ctune_AudioOut {
    /**
     * Output plugin name
     */
    String_t name;

    /**
     * Output plugin file handle
     */
    void * handle;

    /**
     * Pointer to output plugin ABI version number
     */
    unsigned int * abi_version;

    /**
     * Initialises sound server
     * @param fmt           Output format
     * @param sample_rate   DSP frequency (samples per second)
     * @param channels      Number of separate sound channels
     * @param samples       Audio buffer size in samples (i.e. frame size)
     * @param volume        Start mixer volume
     * @return 0 on success or negative ctune error number
     */
    int (* init)( ctune_output_fmt_t fmt, int sample_rate, uint channels, uint samples, int volume );

    /**
     * Sends PCM data to sink buffer
     * @param buffer    Pointer to PCM audio data
     * @param buff_size Size of PCM buffer (in bytes)
     */
    void (* write)( const void * buffer, int buff_size );

    /**
     * Sets a value to the output volume
     * @param vol Volume (0-100)
     */
    void (* setVolume)( int vol );

    /**
     * Modify the output volume
     * @param delta Percent change of volume
     * @return Volume change state
     */
    bool (* changeVolume)( int delta );

    /**
     * Gets current mixing volume (0-100)
     * @return Output volume as a percentage
     */
    int (* getVolume)( void );

    /**
     * Calls all the cleaning/closing/shutdown functions audio server
     */
    void (* shutdown)( void );

} ctune_AudioOut_t;

#endif //CTUNE_AUDIO_AUDIOOUT_H
