#include "../src/audio/FileOut.h"

#include "logger/src/Logger.h"
#include "../src/ctune_err.h"

#include <lame/lame.h>

const unsigned           abi_version = CTUNE_FILEOUT_ABI_VERSION;
const ctune_PluginType_e plugin_type = CTUNE_PLUGIN_OUT_AUDIO_FILE;

/**
 * Gets the plugin's name
 * @return Plugin name string
 */
static const char * ctune_FileOut_name( void ) {
    return "mp3lame";
}

/**
 * Gets the plugin's description
 * @return Plugin description string
 */
static const char * ctune_FileOut_description( void ) {
    return "Lame MP3 encoder";
}

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
static int ctune_FileOut_init( const char * path, ctune_OutputFmt_e fmt, int sample_rate, uint channels, uint samples, uint8_t buff_size_MB ) {

}

/**
 * Sends PCM data to sink buffer
 * @param buffer    Pointer to PCM audio data
 * @param buff_size Size of PCM buffer (in bytes)
 * @return 0 on success or negative ctune error number
 */
static int ctune_FileOut_write( const void * buffer, int buff_size ) {

}

/**
 * Calls all the cleaning/closing/shutdown functions
 */
void ctune_FileOut_close( void ) {

}


const struct ctune_FileOut ctune_FileOutput = {
    .name        = &ctune_FileOut_name,
    .description = &ctune_FileOut_description,
    .init        = &ctune_FileOut_init,
    .write       = &ctune_FileOut_write,
    .close       = &ctune_FileOut_close,
};