#include "../src/player/Player.h"

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/error.h>

#include "logger/src/Logger.h"
#include "../src/audio/AudioOut.h"
#include "../src/ctune_err.h"
#include "../src/utils/Timeout.h"

const unsigned           abi_version = CTUNE_PLAYER_ABI_VERSION;
const ctune_PluginType_e plugin_type = CTUNE_PLUGIN_IN_STREAM_PLAYER;

enum RADIOPLAYER_INIT_STAGES {
    STAGE_STREAM_INPUT = 0,
    STAGE_INPUT_CODEC,
    STAGE_RESAMPLER,
    STAGE_AUDIO_OUT,

    STAGE_COUNT,
};

/**
 * Player plugin variables
 * @param error              ctune error no
 * @param audio_out          Pointer to the audio output to use
 * @param record_plugin      Plugin to record the PCM audio data
 * @param out_channel_layout Number of channels of the PCM data to be sent to the audio output
 * @param out_sample_rate    Sample rate of the PCM output
 * @param out_sample_fmt     Sample format of the PCM data to be sent to the audio output
 * @param ctune_sample_fmt   Local ctune equivalent of `out_sample_fmt``
 */
struct {
    int                    error;
    ctune_AudioOut_t     * audio_out;
    ctune_FileOut_t      * record_plugin;
    struct AVChannelLayout out_channel_layout;
    int                    out_sample_rate;

    struct {
        const enum AVSampleFormat ffmpeg;
        const ctune_OutputFmt_e   ctune;

    } out_sample_fmt;

    struct {
        bool (* playback_ctrl_callback)( enum CTUNE_PLAYBACK_CTRL );
        void (* song_change_callback)( const char *str );
    } cb;

} ffmpeg_player = {
    .error              = CTUNE_ERR_NONE,
    .audio_out          = NULL,
    .record_plugin      = NULL,
    .out_channel_layout = AV_CH_LAYOUT_STEREO,
    .out_sample_rate    = 44100, //initial value but will be overwritten anyway
    .out_sample_fmt     = {
        .ffmpeg = AV_SAMPLE_FMT_S32,
        .ctune  = CTUNE_AUDIO_OUTPUT_FMT_S32,  //equivalent of above
    },
    .cb = {
        NULL,
        NULL,
    },
};

/**
 * [PRIVATE] AV log callback
 * @param ptr   A pointer to an arbitrary struct of which the first field is a pointer to an AVClass struct.
 * @param level The importance level of the message expressed using a Logging Constant.
 * @param fmt   The format string (printf-compatible) that specifies how subsequent arguments are converted to output.
 * @param vargs The arguments referenced by the format string.
 */
static void ctune_Player_avLogCallback( void * avcl, int level, const char * fmt, va_list vl ) {
    static const int LINE_SIZE = 256;

    int  prefix  = 1;
    int  written = 0;
    char line_buffer[LINE_SIZE];

    if( ( written = av_log_format_line2( avcl, level, fmt, vl, line_buffer, LINE_SIZE, &prefix ) ) < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_Player_avLogCallback( %p, %i, %fmt, ... )] Failed to format FFMPEG log line: ", avcl, level, fmt, written );

    } else {
        if( written > 0 ) {
            line_buffer[written - 1] = '\0'; //removes newline character ffmpeg writes at the end of the line (Seriously, why does it do that?!?)
        }

        switch( level ) {
            case AV_LOG_PANIC: //fallthrough - (0) Something went really wrong, and we will crash now.
            case AV_LOG_FATAL: //(8) Something went wrong and recovery is not possible.
                CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_Player_avLogCallback(..)] FFMPEG: %s", line_buffer );
                break;

            case AV_LOG_ERROR: //(16) Something went wrong and cannot losslessly be recovered.
                CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_Player_avLogCallback(..)] FFMPEG: %s", line_buffer );
                break;

            case AV_LOG_WARNING: //(24) Something somehow does not look correct.
                CTUNE_LOG( CTUNE_LOG_WARNING, "[ctune_Player_avLogCallback(..)] FFMPEG: %s", line_buffer );
                break;

            case AV_LOG_INFO:    //(32) fallthrough - Standard information.
                CTUNE_LOG( CTUNE_LOG_MSG, "[ctune_Player_avLogCallback(..)] FFMPEG: %s", line_buffer );
                break;

            /* these msgs are not that relevant for ctune and just pollute the log mostly */
            case AV_LOG_VERBOSE: //(40) fallthrough - Detailed information.
            case AV_LOG_DEBUG:   //(48) fallthrough - Stuff which is only useful for libav* developers.
            case AV_LOG_TRACE:   //(56) fallthrough
            case AV_LOG_QUIET:   //fallthrough
            default: break;
        }
    }
}

/**
 * [PRIVATE] Callback for when a timout occurs
 * @param err cTune error code
 */
static void ctune_Player_timeoutCallback( int err ) {
    ffmpeg_player.cb.playback_ctrl_callback( CTUNE_PLAYBACK_CTRL_OFF );
    ctune_err.set( err );
}

/**
 * [PRIVATE] (Step 1) Setup the stream input context
 * @param in_format_ctx  Pointer to the `AVFormatContext` to use for the input stream
 * @param in_codec       Pointer to the `AVCodec *` to input the input stream's info into
 * @param audio_stream_i Pointer to integer to store the index of the audio stream
 * @param url            Input stream URL (i.e.: the radio station stream's URL)
 * @return 0 on success, negative number denotes a ctune error number
 */
static int ctune_Player_setupStreamInput( AVFormatContext * in_format_ctx, AVCodec ** in_codec, int * audio_stream_i, const char * url ) {
    in_format_ctx->flags                = AVFMT_FLAG_NONBLOCK;
    in_format_ctx->probesize            = 10000000; //bytes
    in_format_ctx->max_analyze_duration =  8000000; //microseconds

    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_Player_setupStreamInput( %p, %i, %s )] "
               "Setting up stream input (Probe size = %lu bytes, Max analysis time = %lus)...",
               in_format_ctx, *audio_stream_i, url,
               in_format_ctx->probesize, ( in_format_ctx->max_analyze_duration / 1000000 )
    );

    if( avformat_open_input( &in_format_ctx, url, NULL, NULL ) < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Player_setupStreamInput( %p, %i, %s )] Failed open source stream.",
                   in_format_ctx, *audio_stream_i, url
        );

        return -CTUNE_ERR_STREAM_OPEN;
    }

    if( avformat_find_stream_info( in_format_ctx, NULL ) < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Player_setupStreamInput( %p, %i, %s )] Failed to acquire source stream information.",
                   in_format_ctx, *audio_stream_i, url
        );

        return -CTUNE_ERR_STREAM_INFO;
    }

    *audio_stream_i = av_find_best_stream( in_format_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, (const AVCodec **) in_codec, 0 );

    if( *audio_stream_i < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Player_setupStreamInput( %p, %i, %s )] Failed find a valid audio stream in input: %s (%d)",
                   in_format_ctx, *audio_stream_i, url,
                   av_err2str( *audio_stream_i ), AVERROR( *audio_stream_i )
        );

        return -CTUNE_ERR_STREAM_NO_AUDIO;
    }

    const AVCodecParameters * parameters = in_format_ctx->streams[*audio_stream_i]->codecpar; //shortcut pointer

    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_Player_setupStreamInput( %p, %i, %p )] "
               "Input stream setup complete: { codec = '%s', channels = %d, sample-rate = %d, bits per samples = %d, bit-rate = %ld, frame-size = %d }",
               in_format_ctx, *audio_stream_i, url,
               avcodec_get_name( parameters->codec_id ), parameters->ch_layout.nb_channels, parameters->sample_rate, parameters->bits_per_coded_sample, parameters->bit_rate, parameters->frame_size
    );

    return 0;
}

/**
 * [PRIVATE] (Step 2) Setup appropriate codec to decode the input stream
 * @param parameters Pointer to `AVCodecParameters` for the input
 * @param codec      Reference to Pointer to `AVCodec` for the input
 * @param context    Reference to Pointer to `AVCodecContext` for the input
 * @return Success
 */
static bool ctune_Player_setupInputCodec( AVCodecParameters * parameters, AVCodec ** codec, AVCodecContext ** context ) {
    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_Player_setupInputCodec( %p, %p, %p )] Setting up input codec...",
               codec, parameters, context
    );

    if( *codec == NULL ) {  //check codec was found by `av_find_best_stream` in stage 1
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Player_setupInputCodec( %p, %p, %p )] Failed to find appropriate codec (%s) for decoding stream.",
                   codec, parameters, context, avcodec_get_name( parameters->codec_id )
        );
        return false;
    }

    if( ( *context = avcodec_alloc_context3( *codec ) ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Player_setupInputCodec( %p, %p, %p )] Failed to allocate codec context for decoding stream.",
                   codec, parameters, context
        );
        return false;
    }

    if( avcodec_parameters_to_context( *context, parameters ) < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Player_setupInputCodec( %p, %p, %p )] Failed to allocate codec parameters to context for decoding stream.",
                   codec, parameters, context
        );

        return false;
    }

    if( avcodec_open2( *context, *codec, NULL ) < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Player_setupInputCodec( %p, %p, %p )] Failed to open codec (%s) for decoding stream.",
                   codec, parameters, context, avcodec_get_name( parameters->codec_id )
        );
        return false;
    }

    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_Player_setupInputCodec( %p, %p, %p )] Setup complete for codec '%s'.",
               codec, parameters, context, avcodec_get_name( parameters->codec_id )
    );

    return true;
}

/**
 * [PRIVATE] (Step 3) Setup software audio re-sampler
 * @param resample_ctx      Reference to Pointer to `SwrContext` resample context
 * @param codec_params      Pointer to the stream input's `AVCodecParameters`
 * @param in_sample_format  Input sample format (`AV_SAMPLE_FMT_*`)
 * @param out_sample_format Output sample format (`AV_SAMPLE_FMT_*`)
 * @return Success
 */
static bool ctune_Player_setupResampler( SwrContext ** resample_ctx, AVCodecParameters * codec_params, enum AVSampleFormat in_sample_format, enum AVSampleFormat out_sample_format ) {
    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_Player_setupResampler( %p, %p, '%s', '%s' )] "
               "Setting up re-sampler...",
               resample_ctx, codec_params, av_get_sample_fmt_name( in_sample_format ), av_get_sample_fmt_name( out_sample_format )
    );

    if( ( *resample_ctx = swr_alloc() ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Player_setupResampler( %p, %p, '%s', '%s' )] "
                   "Failed to allocate memory to the re-sampler context.",
                   resample_ctx, codec_params, av_get_sample_fmt_name( in_sample_format ), av_get_sample_fmt_name( out_sample_format )
        );
        return false;
    }

    int  ret                     = 0; //reusable returned values container
    char channel_layout_str[200] = { 0 };

    av_channel_layout_default( &ffmpeg_player.out_channel_layout, 2 );
    av_channel_layout_describe( &ffmpeg_player.out_channel_layout, &channel_layout_str[0], 200 );

    ret = swr_alloc_set_opts2( resample_ctx,
                               &ffmpeg_player.out_channel_layout, out_sample_format, codec_params->sample_rate, //out
                               &codec_params->ch_layout, in_sample_format, codec_params->sample_rate,           //in
                               0, NULL );

    ffmpeg_player.out_sample_rate = codec_params->sample_rate;

    if( ret < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Player_setupResampler( %p, %p, '%s', '%s' )] "
                   "Failed allocation options to the re-sampler context: %s (%d)",
                   resample_ctx, codec_params, av_get_sample_fmt_name( in_sample_format ), av_get_sample_fmt_name( out_sample_format ),
                   av_err2str( ret ), AVERROR( ret )
        );
        return false;
    }

    ret = swr_init( *resample_ctx );

    if( ret < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Player_setupResampler( %p, %p, '%s', '%s' )] "
                   "Failed re-sampler initialisation: %s",
                   resample_ctx, codec_params, av_get_sample_fmt_name( in_sample_format ), av_get_sample_fmt_name( out_sample_format ),
                   av_err2str( ret )
        );
        return false;
    };

    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_Player_setupResampler( %p, %p, '%s', '%s' )] Re-sampler setup complete: "
               "{ channel layout: %d, sample format: '%s', sample rate: %d, frame size: %d }->"
               "{ channel layout: %d (%s), sample format: '%s', sample rate: %d }",
               resample_ctx, codec_params, av_get_sample_fmt_name( in_sample_format ), av_get_sample_fmt_name( out_sample_format ),
               codec_params->ch_layout.nb_channels, av_get_sample_fmt_name( in_sample_format ), codec_params->sample_rate, codec_params->frame_size,
               ffmpeg_player.out_channel_layout.nb_channels, channel_layout_str, av_get_sample_fmt_name( out_sample_format ), codec_params->sample_rate
    );

    return true;
}

/**
 * [PRIVATE] Creates a buffer
 * @param buffer_ptr     Pointer to the buffer pointer
 * @param channel_layout Channel layout
 * @param samples        Number of samples
 * @param sample_fmt     Sample format
 * @return Calculated size of the buffer
 */
static int ctune_Player_createBuffer( uint8_t ** buffer_ptr, uint64_t channel_layout, int samples, enum AVSampleFormat sample_fmt ) {
    bool error_state     = false;
    int  out_buffer_size = av_samples_get_buffer_size( NULL, ffmpeg_player.out_channel_layout.nb_channels, samples, sample_fmt, 1 );

    if( buffer_ptr == NULL || *buffer_ptr != NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Player_createBuffer( %p, %lu, %d, '%s' )] Buffer pointer is NULL.",
                   buffer_ptr, channel_layout, samples, av_get_sample_fmt_name( sample_fmt )
        );

        ffmpeg_player.error = CTUNE_ERR_BAD_FUNC_ARGS;
        error_state         = true;
        goto end;
    }

    if( out_buffer_size <= 0 ) {
        CTUNE_LOG( CTUNE_LOG_WARNING,
                   "[ctune_Player_createBuffer( %p, %lu, %d, '%s' )] "
                   "Failed to calculate appropriate buffer size for output: %s (%d)",
                   buffer_ptr, channel_layout, samples, av_get_sample_fmt_name( sample_fmt ),
                   av_err2str( out_buffer_size ), AVERROR( out_buffer_size )
        );

        out_buffer_size = ( CTUNE_MAX_FRAME_SIZE + AV_INPUT_BUFFER_PADDING_SIZE ); //fallback

        CTUNE_LOG( CTUNE_LOG_WARNING,
                   "[ctune_Player_createBuffer( %p, %lu, %d, '%s' )] Using fallback buffer size: %d",
                   buffer_ptr, channel_layout, samples, av_get_sample_fmt_name( sample_fmt ), out_buffer_size
        );
    }

    (* buffer_ptr) = (uint8_t *) av_malloc( out_buffer_size + AV_INPUT_BUFFER_PADDING_SIZE );

    if( (*buffer_ptr) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_DEBUG,
                   "[ctune_Player_createBuffer( %p, %lu, %d, '%s' )] Failed buffer allocation.",
                   buffer_ptr, channel_layout, samples, av_get_sample_fmt_name( sample_fmt )
        );

        ffmpeg_player.error = CTUNE_ERR_STREAM_BUFFER_ALLOC;
        error_state         = true;
        goto end;
    }

    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_Player_createBuffer( %p, %lu, %d, '%s' )] Buffer created (size: %d).",
               buffer_ptr, channel_layout, samples, av_get_sample_fmt_name( sample_fmt ), out_buffer_size
    );

    end:
        return ( error_state ? 0 : out_buffer_size );
}

/**
 * Connects and plays a Radio station's stream
 * @param url         Radio station stream URL
 * @param volume      Initial playing volume
 * @param timeout_val Timeout value in seconds
 * @return Success (if false the error_no in the RadioPlayer_t instance will be set accordingly)
 */
static bool ctune_Player_playRadioStream( const char * url, const int volume, int timeout_val ) {
    char * radio_stream_url = strdup( url ); //creating local copy as ref might disappear in other thread

    ffmpeg_player.error = CTUNE_ERR_NONE;

    CTUNE_LOG( CTUNE_LOG_MSG, "[ctune_Player_playRadioStream( \"%s\", %i, %is )] Playing stream.", radio_stream_url, volume, timeout_val );

    /**
     * Stages:
     * 1. Open input (source) stream and get information
     * 2. Get and open matching codec for source stream to decode the compressed source audio
     * 3. Setup resampling to convert decoded source into mixed PCM data
     * 4. Setup audio sink (output)
     * 5. Decode and resample the frames and send to output sink as they are received
     */

    bool                error_state          = false;
    bool                stages[STAGE_COUNT]  = { [STAGE_STREAM_INPUT] = false,
                                                 [STAGE_INPUT_CODEC ] = false,
                                                 [STAGE_RESAMPLER   ] = false,
                                                 [STAGE_AUDIO_OUT   ] = false };
    int                 ret                  =    0; //reusable returned values container
    AVFormatContext   * in_format_ctx        = avformat_alloc_context();
    int                 audio_stream_index   =   -1;
    AVCodecParameters * in_codec_param       = NULL;
    AVCodecContext    * in_codec_ctx         = NULL;
    AVCodec           * in_codec             = NULL;
    SwrContext        * resample_ctx         = NULL;
    uint8_t	          * out_buffer           = NULL;
    int                 out_buffer_size      =   -1;
    AVPacket          * packet               = NULL;
    AVFrame           * frame                = NULL;
    ctune_Timeout_t     timeout_timer        = ctune_Timeout.init( timeout_val, CTUNE_ERR_STREAM_OPEN_TIMEOUT, ctune_Player_timeoutCallback );
    AVIOInterruptCB     interrupt_callback   = { .callback = ctune_Timeout.timedOut, &timeout_timer };

    if( ffmpeg_player.audio_out == NULL ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_Player_playRadioStream( \"%s\", %i, %is )] Mo sound output plugin set.",
                   radio_stream_url, volume, timeout_val
        );

        ffmpeg_player.error = CTUNE_ERR_IO_PLUGIN_NULL;
        error_state         = true;
        goto end;
    }

    //---(1) setup input---
    in_format_ctx->interrupt_callback = interrupt_callback; //interrupt callback for when connection fails on `avformat_open_input` (e.g. tcp timeout)
    ctune_Timeout.reset( &timeout_timer );

    if( ( ret = ctune_Player_setupStreamInput( in_format_ctx, &in_codec, &audio_stream_index, radio_stream_url ) ) != 0 ) {
        error_state = true;
        ffmpeg_player.error = abs( ret );
        goto end;

    } else {
        stages[STAGE_STREAM_INPUT] = true;
    }

    #ifdef DEBUG
        av_dump_format( in_format_ctx, 0, radio_stream_url, 0 ); //prints all sorts of info about stream
    #endif

    //--(2) find codec--
    in_codec_param = in_format_ctx->streams[audio_stream_index]->codecpar; //codec parameters for the stream

    if( !ctune_Player_setupInputCodec( in_codec_param, &in_codec, &in_codec_ctx ) ) {
        error_state = true;
        ffmpeg_player.error = CTUNE_ERR_STREAM_CODEC;
        goto end;

    } else {
        stages[STAGE_INPUT_CODEC] = true;
    }

    //--(3) setup resampling for output--
    if( !ctune_Player_setupResampler( &resample_ctx, in_codec_param, in_codec_ctx->sample_fmt, ffmpeg_player.out_sample_fmt.ffmpeg ) ) {
        error_state = true;
        ffmpeg_player.error = CTUNE_ERR_STREAM_SWR;
        goto end;

    } else {
        stages[STAGE_RESAMPLER] = true;
    }

    //--(4) setup audio output sink--
    if( ( ret = ffmpeg_player.audio_out->init( ffmpeg_player.out_sample_fmt.ctune, in_codec_param->sample_rate, in_codec_param->ch_layout.nb_channels, in_codec_param->frame_size, volume ) ) != 0 ) {
        ffmpeg_player.error = abs( ret );
        error_state = true;
        goto end;

    } else {
        stages[STAGE_AUDIO_OUT] = true;
    }

    //--(5) decode, resample and send to audio sink frames as they come in--
    packet          = av_packet_alloc();
    frame           = av_frame_alloc();
    out_buffer_size = ctune_Player_createBuffer( &out_buffer, ffmpeg_player.out_channel_layout.nb_channels, in_codec_param->frame_size, ffmpeg_player.out_sample_fmt.ffmpeg );

    if( out_buffer_size <= 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Player_playRadioStream( \"%s\", %i, %is )] Error creating buffer.",
                   radio_stream_url, volume, timeout_val
        );

        error_state = true;
        goto end;
    }

    //(reusing old timer for `av_read_frame(..)`, etc..)
    ctune_Timeout.setFailErr( &timeout_timer, CTUNE_ERR_STREAM_READ_TIMEOUT, ctune_Player_timeoutCallback );
    ctune_Timeout.reset( &timeout_timer );

    while( ffmpeg_player.cb.playback_ctrl_callback( CTUNE_PLAYBACK_CTRL_STATE_REQ )
        && ( ret = av_read_frame( in_format_ctx, packet ) ) >= 0 )
    {
        //check if metadata has been changed (i.e. new song playing)
        if( in_format_ctx->event_flags == AVFMT_EVENT_FLAG_METADATA_UPDATED ) {
            AVDictionaryEntry * title  = av_dict_get( in_format_ctx->metadata, "StreamTitle", NULL, 0 );
            ffmpeg_player.cb.song_change_callback( ( title ? title->value : "n/a" ) );
            in_format_ctx->event_flags = 0;
        }

        //decode compressed frame packet into raw uncompressed frame
        if( ( ret = avcodec_send_packet( in_codec_ctx, packet ) ) < 0 ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_Player_playRadioStream( \"%s\", %i, %is )] Error sending packet to decoder: %s",
                       radio_stream_url, volume, timeout_val, av_err2str( ret )
            );

            ffmpeg_player.error = CTUNE_ERR_STREAM_DECODE;
            error_state = true;
            goto end;
        }

        while( ( ret = avcodec_receive_frame( in_codec_ctx, frame ) ) == 0 ) {
            //resample the decoded frame
            const int sample_count = swr_convert( resample_ctx, &out_buffer, out_buffer_size, (const uint8_t **) frame->data , frame->nb_samples );
            if( sample_count < 0 ) {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_Player_playRadioStream( \"%s\", %i, %is )] Error converting frame data: %s (%d)",
                           radio_stream_url, volume, timeout_val, av_err2str( sample_count ), AVERROR( sample_count )
                );

                ffmpeg_player.error = CTUNE_ERR_STREAM_RESAMPLE;
                error_state = true;
                goto end;
            }

            //get decoded size
            const int data_size = av_samples_get_buffer_size( NULL,
                                                              ffmpeg_player.out_channel_layout.nb_channels,
                                                              sample_count,
                                                              ffmpeg_player.out_sample_fmt.ffmpeg,
                                                              1 );
            if( data_size < 0 ) {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_Player_playRadioStream( \"%s\", %i, %is )] Error calculating converted frame size: %s (%d)",
                           radio_stream_url, volume, timeout_val, av_err2str( data_size ), AVERROR( data_size )
                );

                ffmpeg_player.error = CTUNE_ERR_STREAM_BUFFER_SIZE_0;
                error_state = true;
                goto end;
            }

            ffmpeg_player.audio_out->write( out_buffer, data_size );

            if( ffmpeg_player.record_plugin ) {
                ffmpeg_player.record_plugin->write( out_buffer, data_size );
            }
        }

        av_packet_unref( packet );
        ctune_Timeout.reset( &timeout_timer );
    }

    if( ret < 0 && ret != AVERROR(EAGAIN) ) {
        if( ret == AVERROR_EOF ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_Player_playRadioStream( \"%s\", %i, %is )] Error getting decoded packet from decoder: %s (%d)",
                       radio_stream_url, volume, timeout_val, av_err2str( ret ), AVERROR( ret )
            );

            ffmpeg_player.error = CTUNE_ERR_STREAM_DECODE;

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_Player_playRadioStream( \"%s\", %i, %is )] Failed reading frame: %s (%d).",
                       radio_stream_url, volume, timeout_val, av_err2str( ret ), AVERROR( ret )
            );

            ffmpeg_player.error = CTUNE_ERR_STREAM_FRAME_FETCH;
        }

        error_state = true;
        goto end;
    }

    CTUNE_LOG( CTUNE_LOG_MSG,
               "[ctune_Player_playRadioStream( \"%s\", %i, %is )] Stream playback stopped.",
               radio_stream_url, volume, timeout_val
    );

    end: //cleanup
        CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_Player_playRadioStream( \"%s\", %i, %is )] Shutting down stream.", radio_stream_url, volume, timeout_val );

        if( ffmpeg_player.record_plugin ) {
            if( ( ret = ffmpeg_player.record_plugin->close() ) != CTUNE_ERR_NONE ) {
                ctune_err.set( ret );
            }

            ffmpeg_player.record_plugin = NULL;
        }

        if( error_state ) {
            ctune_err.set( ffmpeg_player.error );
        }

        if( out_buffer ) {
            free( out_buffer );
        }

        if( stages[STAGE_AUDIO_OUT] ) {
            ffmpeg_player.audio_out->shutdown();

            if( packet ) {
                av_packet_unref( packet );
                av_packet_free( &packet );
            }

            if( frame ) {
                av_frame_unref( frame );
                av_frame_free( &frame );
            }
        }

        if( stages[STAGE_RESAMPLER] && resample_ctx ) {
            swr_close( resample_ctx );
            av_channel_layout_uninit( &ffmpeg_player.out_channel_layout );
        }

        if( stages[STAGE_STREAM_INPUT] ) {
            //if there was a timeout on connecting to remote stream then no info would be packed
            //into `in_format_ctx` and this will cause libav to segfault on `avformat_close_input(..)` - bug?
            avformat_close_input( &in_format_ctx );
        }

        if( stages[STAGE_INPUT_CODEC] && in_codec_ctx ) {
            avcodec_free_context( &in_codec_ctx );
        }

        free( radio_stream_url );

        ffmpeg_player.cb.playback_ctrl_callback( CTUNE_PLAYBACK_CTRL_OFF );

        return !( error_state );
}

/**
 * Attach a callback that copies the raw PCM buffer of the decoded stream
 * @param filepath Output filepath
 * @param plugin   File recording plugin
 * @return Success
 */
static bool ctune_Player_startRecording( const char * filepath, ctune_FileOut_t * plugin ) {
    if( ffmpeg_player.record_plugin == NULL ) {
        const int ret = plugin->init( filepath, ffmpeg_player.out_sample_fmt.ctune, ffmpeg_player.out_sample_rate, ffmpeg_player.out_channel_layout.nb_channels, 0 );

        if( ret == CTUNE_ERR_NONE ) {
            ffmpeg_player.record_plugin = plugin;

            CTUNE_LOG( CTUNE_LOG_MSG,
                       "[ctune_Player_startRecording( %s, %p )] '%s' recording started.",
                       filepath, plugin, plugin->name()
            );

            ffmpeg_player.cb.playback_ctrl_callback( CTUNE_PLAYBACK_CTRL_SWITCH_REC_REQ );

            return true; //EARLY RETURN

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_Player_startRecording( \"%s\", %p )] Failed to initialise plugin '%s'",
                       filepath, plugin, plugin->name()
            );

            ctune_err.set( abs( ret ) );
        }
    }

    return false;
}

/**
 * Stops the recording and closes the output file
 */
static void ctune_Player_stopRecording( void ) {
    ctune_FileOut_t * plugin = ffmpeg_player.record_plugin;
    ffmpeg_player.record_plugin = NULL;

    const int ret = plugin->close();

    if( ret != CTUNE_ERR_NONE ) {
        ctune_err.set( ret );
    }

    ffmpeg_player.cb.playback_ctrl_callback( CTUNE_PLAYBACK_CTRL_SWITCH_REC_REQ );

    CTUNE_LOG( CTUNE_LOG_MSG,
               "[ctune_Player_startRecording()] '%s' recording stopped.",
               plugin->name()
    );
}

/**
 * Gets the plugin's name
 * @return Plugin name string
 */
static const char * ctune_Player_name( void ) {
    return "ffmpeg";
}

/**
 * Gets the plugin's description
 * @return Plugin description string
 */
static const char * ctune_Player_description( void ) {
    return "FFMPEG player";
}

/**
 * Initialises the RadioPlayer functionalities
 * @param sound_server                   Pointer to the sound server plugin to use as output
 * @param playback_ctrl_callback         Function to check/set the playback state global flag
 * @param song_change_callback           Function to call when stream metadata changes (sends the current stream title)
 */
static void ctune_Player_init(
    ctune_AudioOut_t * sound_server,
    bool(* playback_ctrl_callback)( enum CTUNE_PLAYBACK_CTRL ),
    void(* song_change_callback)( const char * ) )
{
    if( ctune_Logger.logLevel() <= CTUNE_LOG_DEBUG ) {
        av_log_set_level( AV_LOG_INFO );
    } else {
        av_log_set_level( AV_LOG_QUIET );
    }

    av_log_set_callback( ctune_Player_avLogCallback );

    if( sound_server == NULL || playback_ctrl_callback == NULL || song_change_callback == NULL  ) {
        ctune_err.set( CTUNE_ERR_PLAYER_INIT );
    }

    ffmpeg_player.error                     = CTUNE_ERR_NONE;
    ffmpeg_player.audio_out                 = sound_server;
    ffmpeg_player.cb.playback_ctrl_callback = playback_ctrl_callback,
    ffmpeg_player.cb.song_change_callback   = song_change_callback;
}

/**
 * Gets the error number set in RadioPlayer
 * @return ctune_errno
 */
static int ctune_Player_errno() {
    return ffmpeg_player.error;
}

/**
 * Test stream and get its property
 * @param url         Stream URL
 * @param timeout_val Timeout value in seconds
 * @param codec_str   Pointer to the codec string pointer
 * @param bitrate     Pointer to the bitrate container
 * @return Stream OK
 */
bool ctune_Player_testStream( const char * url, int timeout_val, String_t * codec_str, ulong * bitrate ) {
    CTUNE_LOG( CTUNE_LOG_TRACE, "[ctune_Player_testStream( \"%s\", %d, %p, %p )] Testing stream...", url, timeout_val, codec_str, bitrate );

    if( url == NULL || codec_str == NULL || bitrate == NULL ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_Player_testStream( \"%s\", %d, %p, %p )] NULL arg(s)!",
                   url, timeout_val, codec_str, bitrate
        );

        ctune_err.set( CTUNE_ERR_BAD_FUNC_ARGS );
        return false; //EARLY RETURN
    }

    /**
     * Stages:
     * 1. Open input (source) stream and get information
     * 2. Get and open matching codec for source stream to decode the compressed source audio
     */

    char              * radio_stream_url     = strdup( url ); //local copy
    int                 err_code             = CTUNE_ERR_NONE;
    bool                stages[STAGE_COUNT]  = { [STAGE_STREAM_INPUT] = false, [STAGE_INPUT_CODEC] = false };
    int                 ret                  =    0;
    AVFormatContext   * in_format_ctx        = avformat_alloc_context();
    int                 audio_stream_index   =   -1;
    AVCodecParameters * in_codec_param       = NULL;
    AVCodecContext    * in_codec_ctx         = NULL;
    AVCodec           * in_codec             = NULL;
    ctune_Timeout_t     timeout_timer        = ctune_Timeout.init( timeout_val, CTUNE_ERR_STREAM_OPEN_TIMEOUT, ctune_Player_timeoutCallback );
    AVIOInterruptCB     interrupt_callback   = { .callback = ctune_Timeout.timedOut, &timeout_timer };

    //---(1) setup input---
    in_format_ctx->interrupt_callback = interrupt_callback; //interrupt callback for when connection fails on `avformat_open_input` (e.g. tcp timeout)
    ctune_Timeout.reset( &timeout_timer );

    if( ( ret = ctune_Player_setupStreamInput( in_format_ctx, &in_codec, &audio_stream_index, radio_stream_url ) ) != 0 ) {
        err_code = abs( ret );
        goto end;

    } else {
        stages[STAGE_STREAM_INPUT] = true;
    }

    //--(2) find codec--
    in_codec_param = in_format_ctx->streams[audio_stream_index]->codecpar; //codec parameters for the stream

    if( !ctune_Player_setupInputCodec( in_codec_param, &in_codec, &in_codec_ctx ) ) {
        err_code = CTUNE_ERR_STREAM_CODEC;
        goto end;

    } else {
        stages[STAGE_INPUT_CODEC] = true;
        String.set( codec_str, avcodec_get_name( in_codec_param->codec_id ) );
        *bitrate = ( in_codec_param->bit_rate / 1000 ); //get kbps
    }

    end: //cleanup
        CTUNE_LOG( CTUNE_LOG_TRACE,
                   "[ctune_Player_testStream( \"%s\", %d, %p, %p )] Cleaning up.",
                   url, timeout_val, codec_str, bitrate
        );

        if( err_code != CTUNE_ERR_NONE )
            ctune_err.set( err_code );

        if( stages[STAGE_STREAM_INPUT] ) {
            //if there was a timeout on connecting to remote stream then no info would be packed
            //into `in_format_ctx` and this will cause libav to segfault on `avformat_close_input(..)` - bug?
            avformat_close_input( &in_format_ctx );
        }

        if( stages[STAGE_INPUT_CODEC] && in_codec_ctx ) {
            avcodec_free_context( &in_codec_ctx );
        }

        free( radio_stream_url );

        return ( err_code == CTUNE_ERR_NONE );
}

/**
 * Constructor
 */
const struct ctune_Player_Interface ctune_Player = {
    .name            = &ctune_Player_name,
    .description     = &ctune_Player_description,
    .init            = &ctune_Player_init,
    .playRadioStream = &ctune_Player_playRadioStream,
    .startRecording  = &ctune_Player_startRecording,
    .stopRecording   = &ctune_Player_stopRecording,
    .getError        = &ctune_Player_errno,
    .testStream      = &ctune_Player_testStream,
};