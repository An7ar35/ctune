#include "../src/audio/FileOut.h"

#include "logger/src/Logger.h"
#include "../src/ctune_err.h"

#include <lame/lame.h>

#define MP3_MODE    1
#define MP3_QUALITY 2

const unsigned           abi_version = CTUNE_FILEOUT_ABI_VERSION;
const ctune_PluginType_e plugin_type = CTUNE_PLUGIN_OUT_AUDIO_RECORDER;

static struct {
    lame_global_flags * gfp;
} output = {
    .gfp = NULL,
};

static void logError( const char * fmt, va_list ap ) {
    //TODO
}

static void logDebug( const char * fmt, va_list ap ) {
    //TODO
}

static void logMessage( const char * fmt, va_list ap ) {
    //TODO
}

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
 * Gets the plugin's file extension
 * @return Plugin file extension
 */
static const char * ctune_FileOut_extension( void ) {
    return "mp3";
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
    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_FileOut_init( \"%s\", %d, %d, %d, %d, %dMB )]",
               path, fmt, sample_rate, channels, samples, buff_size_MB
    );

    if( ( output.gfp = lame_init() ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_FileOut_init( \"%s\", %d, %d, %d, %d, %dMB )] Failed to initialise Lame.",
                   path, fmt, sample_rate, channels, samples, buff_size_MB
        );

        return -CTUNE_ERR_LAME_INIT;
    }

    lame_set_errorf( output.gfp, logError );
    lame_set_debugf( output.gfp, logDebug );
    lame_set_msgf( output.gfp, logMessage );

    lame_set_num_channels( output.gfp, (int) channels );
    lame_set_in_samplerate( output.gfp, sample_rate );
    lame_set_brate( output.gfp, fmt );
    lame_set_mode( output.gfp, MP3_MODE );
    lame_set_quality( output.gfp, MP3_QUALITY );

    const int ret_code = lame_init_params( output.gfp );

    if( ret_code < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_FileOut_init( \"%s\", %d, %d, %d, %d, %dMB )] Failed to initialise Lame parameters (err=%d).",
                   path, fmt, sample_rate, channels, samples, buff_size_MB, ret_code
        );

        lame_close( output.gfp );
        return -CTUNE_ERR_LAME_INIT;
    }

    return 0;
}

/**
 * Sends PCM data to sink buffer
 * @param buffer    Pointer to PCM audio data
 * @param buff_size Size of PCM buffer (in bytes)
 * @return 0 on success or negative ctune error number
 */
static int ctune_FileOut_write( const void * buffer, int buff_size ) { //TODO

    return 0;
}

/**
 * Calls all the cleaning/closing/shutdown functions
 * @return 0 on success or negative ctune error number
 */
static int ctune_FileOut_close( void ) {
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_FileOut_close()]" );
    lame_close( output.gfp );
    //TODO
}


const struct ctune_FileOut ctune_FileOutput = {
    .name        = &ctune_FileOut_name,
    .description = &ctune_FileOut_description,
    .extension   = &ctune_FileOut_extension,
    .init        = &ctune_FileOut_init,
    .write       = &ctune_FileOut_write,
    .close       = &ctune_FileOut_close,
};