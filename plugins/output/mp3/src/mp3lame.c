#include "../src/audio/FileOut.h"

#include "logger/src/Logger.h"
#include "../src/ctune_err.h"

#include <lame/lame.h>
#include <sys/statvfs.h>

#define MP3_QUALITY        2
#define MP3_DFLT_BUFF_SIZE 5000000 //5MB

const unsigned           abi_version = CTUNE_FILEOUT_ABI_VERSION;
const ctune_PluginType_e plugin_type = CTUNE_PLUGIN_OUT_AUDIO_RECORDER;

static struct {
    FILE              * file;
    String_t            path;
    lame_global_flags * gfp;
    int                 frame_bytes;
    ctune_OutputFmt_e   in_fmt;

    struct Buffer {
        int             size;
        size_t          i;
        uint8_t       * data;
    } buffer;

} output = {
    .file        = NULL,
    .path        = { NULL, 0 },
    .gfp         = NULL,
    .frame_bytes = 0,
    .in_fmt      = 0,
    .buffer      = {
        .size = MP3_DFLT_BUFF_SIZE,
        .i    = 0,
        .data = NULL,
    },
};

/**
 * [PRIVATE] Checks available disk space on path
 * @param path      Target path
 * @param req_bytes Required bytes
 * @return Error code
 */
static int hasAvailableSpace( const char * path, size_t req_bytes ) {
    struct statvfs stat;

    if( statvfs( path, &stat ) != 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[hasAvailableSpace( \"%s\" )] Failed to get file system stats.",
                   path
        );

        return CTUNE_ERR_IO_DISK_ACCESS_FAIL; //EARLY RETURN
    }

    if( ( stat.f_bsize * stat.f_bavail ) < req_bytes ) {
        return CTUNE_ERR_IO_DISK_FULL; //EARLY RETURN
    }

    return 0;
}

/**
 * [PRIVATE] Writes the content of buffer to a file
 * @param out    Pointer to file handler
 * @param buffer Buffer container
 * @return Bytes written
 */
static size_t writeBufferToFile( FILE * out, struct Buffer * buffer ) {
    const size_t written = fwrite( buffer->data, sizeof( uint8_t ), buffer->i, out );
    buffer->i = 0;
    return written;
}

static void logError( const char * fmt, va_list ap ) {
    CTUNE_LOG( CTUNE_LOG_ERROR, fmt, ap );
}

static void logDebug( const char * fmt, va_list ap ) {
    CTUNE_LOG( CTUNE_LOG_DEBUG, fmt, ap );
}

static void logMessage( const char * fmt, va_list ap ) {
    CTUNE_LOG( CTUNE_LOG_MSG, fmt, ap );
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
 * @param buff_size_MB Size of the file buffer in Megabytes (0: set to default)
 * @return 0 on success or negative ctune error number
 */
static int ctune_FileOut_init( const char * path, ctune_OutputFmt_e fmt, int sample_rate, uint channels, uint8_t buff_size_MB ) {
    int error = CTUNE_ERR_NONE;

    if( fmt != CTUNE_AUDIO_OUTPUT_FMT_S16 && fmt != CTUNE_AUDIO_OUTPUT_FMT_S32 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_FileOut_init( \"%s\" %d, %d, %d, %dMB )] "
                   "Format '%d' is not implemented in plugin (see `write(..)` function).",
                   path, fmt, sample_rate, channels, buff_size_MB, fmt
        );

        error = CTUNE_ERR_BAD_FUNC_ARGS;
        goto fail;
    }

    output.in_fmt      = fmt;
    output.buffer.size = ( buff_size_MB > 0 ? ( buff_size_MB * 1000000 ) : MP3_DFLT_BUFF_SIZE );
    output.buffer.data = malloc( sizeof( uint8_t ) * output.buffer.size );
    output.frame_bytes = (int) ( ( fmt * channels ) / 8 );

    if( output.buffer.data == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_FileOut_init( \"%s\" %d, %d, %d, %dMB )] "
                   "Failed allocation of PCM data buffer.",
                   path, fmt, sample_rate, channels, buff_size_MB
        );

        error = CTUNE_ERR_BUFF_ALLOC;
        goto fail;
    }

    String.set( &output.path, path );

    if( ( output.file = fopen( output.path._raw , "wb+" ) ) == NULL ) {
        error = CTUNE_ERR_IO_AUDIOFILE_OPEN;
        goto fail;
    }

    if( ( output.gfp = lame_init() ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_FileOut_init( \"%s\", %d, %d, %d, %dMB )] Failed to initialise Lame.",
                   path, fmt, sample_rate, channels, buff_size_MB
        );

        error = CTUNE_ERR_LAME_INIT;
        goto fail;
    }

    lame_set_errorf( output.gfp, logError );
    lame_set_debugf( output.gfp, logDebug );
    lame_set_msgf( output.gfp, logMessage );
    lame_set_num_channels( output.gfp, (int) channels );
    lame_set_in_samplerate( output.gfp, sample_rate );
    lame_set_out_samplerate( output.gfp, sample_rate );
    lame_set_mode( output.gfp, STEREO );
    lame_set_quality( output.gfp, MP3_QUALITY );
    lame_set_VBR( output.gfp, vbr_default );

    const int ret_code = lame_init_params( output.gfp );

    if( ret_code < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_FileOut_init( \"%s\", %d, %d, %d, %dMB )] Failed to initialise Lame parameters (err=%d).",
                   path, fmt, sample_rate, channels, buff_size_MB, ret_code
        );

        lame_close( output.gfp );
        error = CTUNE_ERR_LAME_INIT;
        goto fail;
    }

    CTUNE_LOG( CTUNE_LOG_MSG,
               "[ctune_FileOut_init( \"%s\", %d, %d, %d, %dMB )] MP3 recorder initialised (buffer = %luMB).",
               path, fmt, sample_rate, channels, buff_size_MB, ( output.buffer.size / 1000000 )
    );

    return 0;

    fail:
        String.free( &output.path );
        free( output.buffer.data );
        return -error;
}

/**
 * Sends PCM data to sink buffer
 * @param buffer    Pointer to PCM audio data
 * @param buff_size Size of PCM buffer (in bytes)
 * @return 0 on success or negative ctune error number
 */
static int ctune_FileOut_write( const void * buffer, int buff_size ) {
    int error = CTUNE_ERR_NONE;

    if( output.buffer.data == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_FileOut_write( %p, %lu )] Failed to write data to PCM buffer (%p): buffer not allocated",
                   buffer, buff_size, output.buffer.data
        );

        error = CTUNE_ERR_BUFF_NULL;
        goto end;
    }

    if( buff_size <= 0 ) {
        CTUNE_LOG( CTUNE_LOG_WARNING,
                   "[ctune_FileOut_write( %p, %lu )] Failed to write data to PCM buffer (%p): no data (size=%d)",
                   buffer, buff_size, output.buffer.data, buff_size
        );

        error = CTUNE_ERR_NO_DATA;
        goto end;
    }

    const int frames_n = ( buff_size / output.frame_bytes );

    switch( output.in_fmt ) {
        case CTUNE_AUDIO_OUTPUT_FMT_S16: {
            output.buffer.i = lame_encode_buffer_interleaved( output.gfp, (short *) buffer, frames_n, output.buffer.data, output.buffer.size );
        } break;

        case CTUNE_AUDIO_OUTPUT_FMT_S32: {
            output.buffer.i = lame_encode_buffer_interleaved_int( output.gfp, (int *) buffer, frames_n, output.buffer.data, output.buffer.size );
        } break;
    }

    if( ( error = hasAvailableSpace( output.path._raw, output.buffer.i ) ) == CTUNE_ERR_NONE ) {
        writeBufferToFile( output.file, &output.buffer );
    } else {
        goto end;
    }

    end:
        return error;
}

/**
 * Calls all the cleaning/closing/shutdown functions
 * @return 0 on success or negative ctune error number
 */
static int ctune_FileOut_close( void ) {
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_FileOut_close()] Closing MP3 recorder." );

    int error = CTUNE_ERR_NONE;

    if( output.file != NULL ) {
        output.buffer.i = lame_encode_flush( output.gfp, output.buffer.data, output.buffer.size );

        if( ( error = hasAvailableSpace( output.path._raw, output.buffer.i ) ) == CTUNE_ERR_NONE ) {
            writeBufferToFile( output.file, &output.buffer );
        }

        fflush( output.file );

        if( fclose( output.file ) != 0 ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[closeCurrFile()] Failed to close file: %s",
                       output.path._raw
            );

            error = CTUNE_ERR_IO_FILE_CLOSE_FAIL;
        };

        output.file = NULL;
    }

    lame_close( output.gfp );
    String.free( &output.path );
    free( output.buffer.data );

    return -error;
}


const struct ctune_FileOut ctune_FileOutput = {
    .name        = &ctune_FileOut_name,
    .description = &ctune_FileOut_description,
    .extension   = &ctune_FileOut_extension,
    .init        = &ctune_FileOut_init,
    .write       = &ctune_FileOut_write,
    .close       = &ctune_FileOut_close,
};