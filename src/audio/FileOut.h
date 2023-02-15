#ifndef CTUNE_AUDIO_FILEOUT_H
#define CTUNE_AUDIO_FILEOUT_H

#include <stdbool.h>
#include <stdint.h>
#include "OutputFormat.h"
#include "../enum/PluginType.h"
#include "../datastructure/String.h"

#define CTUNE_FILEOUT_ABI_VERSION 2

typedef unsigned int uint;

typedef struct ctune_FileOut {
    /**
     * Output plugin file handle
     */
    void * handle;

    /**
     * Pointer to output plugin ABI version number
     */
    const unsigned int * abi_version;

    /**
     * Pointer to the plugin's type
     */
    const ctune_PluginType_e * plugin_type;

    /**
     * Gets the plugin's name
     * @return Plugin name string
     */
    const char * (* name)( void );

    /**
     * Gets the plugin's description
     * @return Plugin description string
     */
    const char * (* description)( void );

    /**
     * Gets the plugin's file extension
     * @return Plugin file extension
     */
    const char * (* extension)( void );

    /**
     * Initialises file output
     * @param path         Output path and filename
     * @param fmt          Output format
     * @param sample_rate  DSP frequency (samples per second)
     * @param channels     Number of separate sound channels
     * @param samples      Audio buffer size in samples (i.e. frame size)
     * @param buff_size_MB Size of the file buffer in Megabytes (0: set to default)
     * @return 0 on success or negative ctune error number
     */
    int (* init)( const char * path, ctune_OutputFmt_e fmt, int sample_rate, uint channels, uint samples, uint8_t buff_size_MB );

    /**
     * Sends PCM data to sink buffer
     * @param buffer    Pointer to PCM audio data
     * @param buff_size Size of PCM buffer (in bytes)
     * @return 0 on success or negative ctune error number
     */
    int (* write)( const void * buffer, int buff_size );

    /**
     * Calls all the cleaning/closing/shutdown functions
     * @return 0 on success or negative ctune error number
     */
    int (* close)( void );

} ctune_FileOut_t;

#endif //CTUNE_AUDIO_FILEOUT_H