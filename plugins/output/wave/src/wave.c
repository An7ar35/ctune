#include "../src/audio/FileOut.h"

#include "logger/src/Logger.h"
#include "../src/ctune_err.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/statvfs.h>

#include "../src/datastructure/String.h"

//docs: http://soundfile.sapp.org/doc/WaveFormat/

#define DATA_HEADER_SIZE      8 //4+4 bytes
#define CHUNKSIZE_OFFSET      4
#define SUBCHUNK2SIZE_OFFSET 40
#define BUFFER_CHRONO_SIZE   30 //in seconds

const unsigned           abi_version = CTUNE_FILEOUT_ABI_VERSION;
const ctune_PluginType_e plugin_type = CTUNE_PLUGIN_OUT_AUDIO_RECORDER;

/**
 * Output variables
 * @param path       File path + root name + count + extension
 * @param file       File handle
 * @param info       Output information
 * @param buffer     PCM data buffer
 */
static struct {
    String_t path;
    FILE   * file;

    /**
     * Output information
     * @param nb_channels     Number of channels
     * @param sample_rate     Sample rate
     * @param bits_per_sample Bits per samples
     * @param data_size       Number of bytes of data
     */
    struct Info {
        uint16_t nb_channels;
        uint32_t sample_rate;
        uint16_t bits_per_sample;
        uint32_t data_size;
    } info;

    /**
     * Output buffer
     * @param size Size of buffer in Bytes
     * @param i    Current index
     * @param data Data buffer
     */
    struct Buffer {
        size_t    size;
        size_t    i;
        uint8_t * data;
    } buffer;

} output = {
    .file                = NULL,
    .path                = { NULL, 0 },

    .info = {
        .nb_channels     = 0,
        .sample_rate     = 0,
        .bits_per_sample = 0,
        .data_size       = 0,

    },

    .buffer = {
        .size            = 10000000, //10MB
        .i               = 0,
        .data            = NULL,
    }
};

/**
 * [PRIVATE] Writes a u16 into a u8 buffer in LSB order
 * @param buffer Target buffer
 * @param data   u16 data to write
 * @return Number of bytes writen (2)
 */
static size_t write16LSB( uint8_t * buffer, uint16_t data ) {
    buffer[0] = (uint8_t) ( data      );
    buffer[1] = (uint8_t) ( data >> 8 );

    return 2;
}

/**
 * [PRIVATE] Writes a u16 into a u8 buffer in LSB order
 * @param buffer Target buffer
 * @param data   u16 data to write
 * @return Number of bytes writen (2)
 */
static size_t write16MSB( uint8_t * buffer, uint16_t data ) {
    buffer[0] = (uint8_t) ( data >> 8 );
    buffer[1] = (uint8_t) ( data      );

    return 2;
}

/**
 * [PRIVATE] Writes a u32 into a u8 buffer in LSB order
 * @param buffer Target buffer
 * @param data   u32 data to write
 * @return Number of bytes writen (4)
 */
static size_t write32LSB( uint8_t * buffer, uint32_t data ) {
    buffer[0] = (uint8_t) ( data       );
    buffer[1] = (uint8_t) ( data >>  8 );
    buffer[2] = (uint8_t) ( data >> 16 );
    buffer[3] = (uint8_t) ( data >> 24 );

    return 4;
}

/**
 * [PRIVATE] Writes a u32 into a u8 buffer in LSB order
 * @param buffer Target buffer
 * @param data   u32 data to write
 * @return Number of bytes writen (4)
 */
static size_t write32MSB( uint8_t * buffer, uint32_t data ) {
    buffer[0] = (uint8_t) ( data >> 24 );
    buffer[1] = (uint8_t) ( data >> 16 );
    buffer[2] = (uint8_t) ( data >>  8 );
    buffer[3] = (uint8_t) ( data       );

    return 4;
}

/**
 * [PRIVATE] Calculates the byte rate
 * @param info Output information
 * @return Byte rate value
 */
static uint32_t calcByteRate( const struct Info * info ) {
    return ( info->sample_rate * info->nb_channels * info->bits_per_sample / 8 );
}

/**
 * [PRIVATE] Calculates the block alignment
 * @param info Output information
 * @return Block alignment value
 */
static uint16_t calcBlockAlignment( const struct Info * info ) {
    return ( info->nb_channels * info->bits_per_sample / 8 );
}

/**
 * [PRIVATE]
 * @param buffer Pointer to buffer of at least 36 bytes in size
 * @param info   Output information
 * @return Bytes packed into the buffer
 */
static size_t packHeader( uint8_t * buffer, struct Info * info ) {
    if( buffer == NULL ) {
        return 0; //EARLY RETURN
    }

    const uint32_t RIFF = 0x52494646; //Big-endian "RIFF"
    const uint32_t WAVE = 0x57415645; //Big-endian "WAVE"
    const uint32_t FMT  = 0x666d7420; //Big-endian "fmt"
    const uint32_t PCM  = 1;
    const uint32_t DATA = 0x64617461; //Big-endian "data"

    size_t i = 0;

    i += write32MSB( &buffer[ i ], RIFF );                       //ChunkID
    i += write32MSB( &buffer[ i ], 0 );                          //ChunkSize
    i += write32MSB( &buffer[ i ], WAVE );                       //Format
    i += write32MSB( &buffer[ i ], FMT );                        //SubChunk1ID
    i += write32LSB( &buffer[ i ], 16 );                         //SubChunk1Size
    i += write16LSB( &buffer[ i ], PCM );                        //AudioFormat
    i += write16LSB( &buffer[ i ], info->nb_channels );          //NumChannels
    i += write32LSB( &buffer[ i ], info->sample_rate );          //SampleRate
    i += write32LSB( &buffer[ i ], calcByteRate( info ) );       //ByteRate
    i += write16LSB( &buffer[ i ], calcBlockAlignment( info ) ); //BlockAlign
    i += write16LSB( &buffer[ i ], info->bits_per_sample );      //BitsPerSample

    i += write32MSB( &buffer[ i ], DATA );                       //SubChunk2ID
    i += write32LSB( &buffer[ i ], 0 );                          //SubChunk2Size

    return i;
}

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
        return CTUNE_ERR_IO_DISK_FULL;  //EARLY RETURN
    }

    return 0;
}

/**
 * [PRIVATE] Flushed content of buffer to a file
 * @param out    Pointer to file handler
 * @param buffer Buffer container
 * @param err    Pointer to variable to set in case of error
 * @return Bytes written
 */
static size_t flushBufferToFile( FILE * out, struct Buffer * buffer, int * err ) {
    const size_t free_to_write = ( ULONG_MAX - output.info.data_size );
    const size_t to_write      = buffer->i;

    size_t written = 0;

    if( to_write > free_to_write ) {
        written = fwrite( buffer->data, sizeof( uint8_t ), free_to_write, out );

        if( err ) {
            (*err) = CTUNE_ERR_IO_FILE_FULL;
        }

    } else {
        written = fwrite( buffer->data, sizeof( uint8_t ), to_write, out );
    }

    CTUNE_LOG( CTUNE_LOG_TRACE,
               "[flushBufferToFile( %p, %p, %p )] Flushing buffer to file: writen %lu/%lu bytes.",
               out, buffer, err, written, to_write );

    fflush( out );
    buffer->i = 0;
    return written;
}

/**
 * [PRIVATE] Write the final data size to the file
 * @param out       File handle
 * @param buffer    Data buffer
 * @param data_size Size of data in bytes
 * @return Success
 */
static bool writeSizeToFile( FILE * out, struct Buffer * buffer, size_t data_size ) {
    bool error_state = false;

    if( data_size < 8 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[writeSizeToFile( %p, %p, %lu )] Size is too small: %lu",
                   out, buffer, data_size, data_size
        );

        error_state = true;
        goto end;
    }

    const size_t i = buffer->i;

    { //ChunkSize
        const size_t bytes = write32LSB( &buffer->data[i], ( data_size - 8 ) );

        if( fseek( out, CHUNKSIZE_OFFSET, SEEK_SET ) != 0 ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[writeSizeToFile( %p, %p, %lu )] Failed to seek 'ChunkSize' in output file: %s",
                       out, buffer, data_size, strerror( errno )
            );

            error_state = true;
            goto end;
        }

        if( fwrite( &buffer->data[i], sizeof( uint8_t ), bytes, out ) != bytes ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[writeSizeToFile( %p, %p, %lu )] Failed to write 'ChunkSize' in output file: %s",
                       out, buffer, data_size, strerror( errno )
            );

            error_state = true;
            goto end;
        }
    }

    { //SubChunk2Size
        const size_t bytes = write32LSB( &buffer->data[i], data_size );

        if( fseek( out, SUBCHUNK2SIZE_OFFSET, SEEK_SET ) != 0 ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[writeSizeToFile( %p, %p, %lu )] Failed to seek 'SubChunk2Size' in output file: %s",
                       out, buffer, data_size, strerror( errno )
            );

            error_state = true;
            goto end;
        }

        if( fwrite( &buffer->data[i], sizeof( uint8_t ), bytes, out ) != bytes ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[writeSizeToFile( %p, %p, %lu )] Failed to write 'SubChunk2Size' in output file: %s",
                       out, buffer, data_size, strerror( errno )
            );

            error_state = true;
            goto end;
        }
    }

    end:
        return !( error_state );
}

/**
 * Gets the plugin's name
 * @return Plugin name string
 */
static const char * ctune_FileOut_name( void ) {
    return "wave";
}

/**
 * Gets the plugin's description
 * @return Plugin description string
 */
static const char * ctune_FileOut_description( void ) {
    return "Writes raw PCM data in a 'wav' format.";
}

/**
 * Gets the plugin's file extension
 * @return Plugin file extension
 */
static const char * ctune_FileOut_extension( void ) {
    return "wav";
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
    int error = CTUNE_ERR_NONE;

    if( output.file ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_FileOut_init( \"%s\", %d, %d, %d, %d, %dMB )] "
                   "Previous opened 'wav' file handle not closed.",
                   path, fmt, sample_rate, channels, samples, buff_size_MB
        );

        error = CTUNE_ERR_IO_AUDIOFILE_OPENED;
        goto fail;
    }

    output.info.data_size       = 0;
    output.info.sample_rate     = sample_rate;
    output.info.nb_channels     = (uint16_t)( channels & 0xFFFF );
    output.info.bits_per_sample = fmt;
    String.set( &output.path, path );

    const size_t bytes_per_second  = ( sample_rate * fmt * channels ) / 8;
    const size_t buff_size_B = ( buff_size_MB * 1000000 );

    if( buff_size_MB && buff_size_B <= bytes_per_second ) {
        CTUNE_LOG( CTUNE_LOG_WARNING,
                   "[ctune_FileOut_init( \"%s\", %d, %d, %d, %d, %dMB )] "
                   "Custom buffer size too small (<= 1s).",
                   path, fmt, sample_rate, channels, samples, buff_size_MB
        );

        buff_size_MB = 0; //force to auto-calculated buffer size
    }

    output.buffer.size = ( buff_size_MB > 0 ? buff_size_B : ( bytes_per_second * BUFFER_CHRONO_SIZE ) );
    output.buffer.i    = 0;
    output.buffer.data = malloc( sizeof( uint8_t ) * output.buffer.size );

    if( output.buffer.data == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_FileOut_init( \"%s\", %d, %d, %d, %d, %dMB )] "
                   "Failed allocation of PCM data buffer.",
                   path, fmt, sample_rate, channels, samples, buff_size_MB
        );

        error = CTUNE_ERR_BUFF_ALLOC;
        goto fail;
    }

    if( ( output.file = fopen( output.path._raw , "wb+" ) ) == NULL ) {
        error = CTUNE_ERR_IO_AUDIOFILE_OPEN;
        goto fail;
    }

    output.buffer.i += packHeader( &output.buffer.data[0], &output.info );

    if( ( error = hasAvailableSpace( output.path._raw, output.buffer.i ) ) != CTUNE_ERR_NONE ) {
        goto fail;
    }

    flushBufferToFile( output.file, &output.buffer, &error );

    if( error != CTUNE_ERR_NONE ) {
        goto fail;
    }

    CTUNE_LOG( CTUNE_LOG_MSG,
               "[ctune_FileOut_init( \"%s\", %d, %d, %d, %d, %dMB )] Wave recorder initialised (buffer = %luMB).",
               path, fmt, sample_rate, channels, samples, buff_size_MB, ( output.buffer.size / 1000000 )
    );

    return 0;

    fail:
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_FileOut_init( \"%s\", %d, %d, %d, %d, %dMB )] Failed to initialise wave plugin: %s (%d)",
                   path, fmt, sample_rate, channels, samples, buff_size_MB, ctune_err.print( error ), error
        );

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
    if( output.buffer.data == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_FileOut_write( %p, %lu )] Failed to write data to PCM buffer (%p): buffer not allocated",
                   buffer, buff_size, output.buffer.data
        );

        return -CTUNE_ERR_BUFF_NULL; //EARLY RETURN
    }

    if( buff_size <= 0 ) {
        CTUNE_LOG( CTUNE_LOG_WARNING,
                   "[ctune_FileOut_write( %p, %lu )] Failed to write data to PCM buffer (%p): no data (size=%d)",
                   buffer, buff_size, output.buffer.data, buff_size
        );

        return -CTUNE_ERR_NO_DATA; //EARLY RETURN
    }

    int error = CTUNE_ERR_NONE;

    if( ( output.buffer.size - buff_size ) < output.buffer.i &&
        ( error = hasAvailableSpace( output.path._raw, output.buffer.i ) ) == 0 )
    {
        output.info.data_size += flushBufferToFile( output.file, &output.buffer, &error );

        if( error != CTUNE_ERR_NONE ) {
            goto end;
        }
    }

    if( (unsigned) buff_size <= ( output.buffer.size - DATA_HEADER_SIZE ) ) {
        memcpy( &output.buffer.data[output.buffer.i], buffer, buff_size );
        output.buffer.i += buff_size;

    } else { //Should never happen but, just in case...
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_FileOut_write( %p, %lu )] PCM buffer too small (%dkB).",
                   buffer, buff_size, ( output.buffer.size / 1000 )
        );
    }

    end:
        return error;
}

/**
 * Calls all the cleaning/closing/shutdown functions
 * @return 0 on success or negative ctune error number
 */
static int ctune_FileOut_close( void ) {
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_FileOut_close()] Closing wave recorder." );

    int error = CTUNE_ERR_NONE;

    if( output.file != NULL ) {
        if( output.buffer.i != 0 ) {
            output.info.data_size += flushBufferToFile( output.file, &output.buffer, &error );

            if( error != CTUNE_ERR_NONE ) {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[closeCurrFile()] Failed to completely flush buffer to file: %s",
                           output.path._raw
                );
            }
        }

        if( output.file != NULL ) {
            if( !writeSizeToFile( output.file, &output.buffer, output.info.data_size ) ) {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[closeCurrFile()] Failed to write size to file: %s",
                           output.path._raw
                );

                error = CTUNE_ERR_IO_FILE_WRITE_FAIL;
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
    }

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
    .close       = &ctune_FileOut_close
};