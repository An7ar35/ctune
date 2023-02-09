#include "../src/audio/FileOut.h"

#include "logger/src/Logger.h"
#include "../src/ctune_err.h"

#include <stdlib.h>
#include <stdio.h>

//docs: http://soundfile.sapp.org/doc/WaveFormat/

#define DATA_HEADER_SIZE 8 //4+4 bytes

const unsigned           abi_version = CTUNE_FILEOUT_ABI_VERSION;
const ctune_PluginType_e plugin_type = CTUNE_PLUGIN_OUT_AUDIO_FILE;

/**
 * Output variables
 * @param file File handle
 * @param info Output information
 * @param buffer PCM data buffer
 */
static struct {
    FILE * file;

    /**
     * Output information
     * @param path            File path
     * @param nb_channels     Number of channels
     * @param sample_rate     Sample rate
     * @param bits_per_sample Bits per samples
     */
    struct Info {
        String_t path;
        uint16_t nb_channels;
        uint32_t sample_rate;
        uint16_t bits_per_sample;
    } info;

    /**
     * Output buffer
     * @param size               Size of buffer in Bytes
     * @param i                  Current index
     * @param data_size_offset_i Index offset where the size of the data chunk is located
     * @param data_offset_i      Index offset where the audio data actually begins
     * @param data               Data buffer
     */
    struct Buffer {
        size_t    size;
        size_t    i;
        size_t    data_size_offset_i;
        size_t    data_offset_i;
        uint8_t * data;
    } buffer;

} output = {
    .file = NULL,
    .info = {
        .path            = { NULL, 0 },
        .nb_channels     = 0,
        .sample_rate     = 0,
        .bits_per_sample = 0,
    },
    .buffer = {
        .size               = 10000000, //10MB
        .i                  = 0,
        .data_size_offset_i = 0,
        .data_offset_i      = 0,
        .data               = NULL,
    }
};

/**
 * [PRIVATE] Writes a u16 into a u8 buffer
 * @param buffer Target buffer
 * @param data   u16 data to write
 * @return Number of bytes writen (2)
 */
static size_t write16( uint8_t * buffer, uint16_t data ) {
    buffer[0] = (uint8_t) ( data >> 8 );
    buffer[1] = (uint8_t) data;

    return 2;
}

/**
 * [PRIVATE] Writes a u32 into a u8 buffer
 * @param buffer Target buffer
 * @param data   u32 data to write
 * @return Number of bytes writen (4)
 */
static size_t write32( uint8_t * buffer, uint32_t data ) {
    buffer[0] = (uint8_t) ( data >> 24 );
    buffer[1] = (uint8_t) ( data >> 16 );
    buffer[2] = (uint8_t) ( data >>  8 );
    buffer[3] = (uint8_t) data;

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

    size_t i = 0;

    i += write32( &buffer[i], RIFF );                       //ChunkID
    i += write32( &buffer[i], 0 );                          //ChunkSize
    i += write32( &buffer[i], WAVE );                       //Format
    i += write32( &buffer[i], FMT );                        //SubChunk1ID
    i += write32( &buffer[i], 16 );                         //SubChunk1Size
    i += write16( &buffer[i], PCM );                        //AudioFormat
    i += write16( &buffer[i], info->nb_channels );          //NumChannels
    i += write32( &buffer[i], info->sample_rate );          //SampleRate
    i += write32( &buffer[i], calcByteRate( info ) );       //ByteRate
    i += write16( &buffer[i], calcBlockAlignment( info ) ); //BlockAlign
    i += write16( &buffer[i], info->bits_per_sample );      //BitsPerSample

    return i;
}

/**
 * [PRIVATE] Flushed content of buffer to a file
 * @param out    Pointer to file handler
 * @param buffer Buffer
 */
static void flushToFile( FILE * out, struct Buffer * buffer ) { //TODO check for space? what happens if disk is full or file handle is fucked? - return error code?
    memcpy( out, buffer, buffer->i );
    buffer->i = 0;
}

/**
 * [PRIVATE] Writes the data chunk header to a buffer
 * @param buffer Target buffer
 */
static void beginDataChunk( struct Buffer * buffer ) {
    const uint32_t DATA = 0x64617461; //Big-endian "data"

    buffer->i                 += write32( &output.buffer.data[buffer->i], DATA ); //SubChunk2ID
    buffer->data_size_offset_i = buffer->i;                                       //SubChunk2Size index position
    buffer->i                 += write32( &output.buffer.data[buffer->i], 0 );    //SubChunk2Size
    buffer->data_offset_i      = buffer->i;                                       //Audio data index position
}

/**
 * [PRIVATE] Writes the size of the current data chunk into the buffer
 * @param buffer Target buffer
 */
static void endDataChunk( struct Buffer * buffer ) {
    write32( &buffer->data[buffer->data_size_offset_i], ( buffer->i - buffer->data_offset_i ) );
}

/**
 * Gets the plugin's name
 */
static const char * ctune_FileOut_name( void ) {
    return "wave";
}

/**
 * Gets the plugin's description
 */
static const char * ctune_FileOut_description( void ) {
    return "Writes raw PCM data in a 'wav' format.";
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
    if( output.file ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_FileOut_init( \"%s\", %d, %d, %d, %d, %dMB )] "
                   "Previous opened 'wav' file handle not closed.",
                   path, fmt, sample_rate, channels, samples, buff_size_MB
        );

        return -CTUNE_ERR_IO_AUDIOFILE_OPENED; //EARLY RETURN
    }

    output.info.sample_rate     = sample_rate;
    output.info.nb_channels     = (uint16_t)( channels & 0xFFFF );
    output.info.bits_per_sample = fmt;
    String.set( &output.info.path, path );

    if( buff_size_MB > 0 ) {
        output.buffer.size = ( (size_t) buff_size_MB * 1000000 );
    }

    output.buffer.i                  = 0;
    output.buffer.data_size_offset_i = 0;
    output.buffer.data_offset_i      = 0;
    output.buffer.data               = malloc( sizeof( uint8_t ) * output.buffer.size );

    if( output.buffer.data == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_FileOut_init( \"%s\", %d, %d, %d, %d, %dMB )] "
                   "Failed allocation of PCM data buffer.",
                   path, fmt, sample_rate, channels, samples, buff_size_MB
        );

        return -CTUNE_ERR_BUFF_ALLOC; //EARLY RETURN
    }

    if( ( output.file = fopen( path , "a" ) ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_FileOut_init( \"%s\", %d, %d, %d, %d, %dMB )] "
                   "Failed to open PCM output file: %s",
                   path, fmt, sample_rate, channels, samples, buff_size_MB, path
        );

        free( output.buffer.data );
        return -CTUNE_ERR_IO_AUDIOFILE_OPEN; //EARLY RETURN
    }

    output.buffer.i += packHeader( &output.buffer.data[0], &output.info );

    flushToFile( output.file, &output.buffer );

    return 0;
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

    if( output.buffer.size - buff_size < output.buffer.i ) {
        endDataChunk( &output.buffer );
        flushToFile( output.file, &output.buffer );
    }

    if( output.buffer.i == 0 ) {
        beginDataChunk( &output.buffer );
    }

    if( (unsigned) buff_size <= ( output.buffer.size - DATA_HEADER_SIZE ) ) {
        memcpy( output.buffer.data, buffer, buff_size );

    } else {
        CTUNE_LOG( CTUNE_LOG_WARNING,
                   "[ctune_FileOut_write( %p, %lu )] PCM buffer too small (%dkB): data will be split (%dkB)",
                   buffer, buff_size, output.buffer.data, ( output.buffer.size / 1000 ), ( buff_size * 1000 )
        );

        size_t offset = 0;

        { //Fills remainder of file buffer
            const size_t remaining_space = ( output.buffer.size - output.buffer.i );

            memcpy( output.buffer.data, (uint8_t *) &buffer[offset], remaining_space );
            endDataChunk( &output.buffer );
            flushToFile( output.file, &output.buffer );

            offset += remaining_space;
        }

        const size_t remaining_pending_data = ( buff_size - offset );
        const size_t chunk_size             = ( output.buffer.size - DATA_HEADER_SIZE );
        const size_t whole_chunks           = ( remaining_pending_data / chunk_size );
        const size_t remainder_size         = ( remaining_pending_data % chunk_size );

        for( size_t chunk = 0; chunk < whole_chunks; ++chunk ) {
            beginDataChunk( &output.buffer );
            memcpy( output.buffer.data, (uint8_t *) &buffer[offset], chunk_size );
            offset += chunk_size;
            endDataChunk( &output.buffer );
            flushToFile( output.file, &output.buffer );
        }

        if( remainder_size > 0 ) {
            beginDataChunk( &output.buffer );
            memcpy( output.buffer.data, (uint8_t *) &buffer[offset], remainder_size );
            endDataChunk( &output.buffer );
            flushToFile( output.file, &output.buffer );
        }
    }

    return 0;
}

/**
 * Calls all the cleaning/closing/shutdown functions
 */
static void ctune_FileOut_close( void ) {
    if( output.buffer.i != 0 ) {
        endDataChunk( &output.buffer );
        flushToFile( output.file, &output.buffer );
    }

    fflush( output.file );

    if( output.file != NULL ) {
        fclose( output.file );
        output.file = NULL;
    }

    String.free( &output.info.path );
    free( output.buffer.data );
}


const struct ctune_FileOut ctune_FileOutput = {
    .name        = &ctune_FileOut_name,
    .description = &ctune_FileOut_description,
    .init        = &ctune_FileOut_init,
    .write       = &ctune_FileOut_write,
    .close       = &ctune_FileOut_close
};