#include "CLI.h"

#include <curl/curlver.h>
#include <json-c/json_c_version.h>
#include <openssl/opensslv.h>
#include <ncurses.h>

#ifdef USING_PULSEAUDIO
#include <pulse/version.h>
#endif

#ifdef USING_SDL
#include <SDL2/SDL_version.h>
#endif

#ifdef USING_ALSA
#include <alsa/version.h>
#endif

#ifdef USING_SNDIO
#include <sndio.h>
#endif

#ifdef USING_FFMPEG
#include <libavformat/version.h>
#include <libavcodec/version.h>
#include <libavdevice/version.h>
#include <libswresample/version.h>
#endif

#ifdef USING_VLC
#include <vlc/libvlc_version.h>
#endif

#include "../audio/AudioOut.h"
#include "project_version.h"

#define STR(x) #x
#define XSTR(x) STR(x)

/**
 * Internal storage for CLI
 */
static struct {
    Vector_t all_options;
    Vector_t actionable_options;

} cli;

/**
 * [PRIVATE] Gets the cTune version information string
 * @return information string
 */
static const char * ctune_CLI_version() {
    static const char * version = CTUNE_APPNAME " v" CTUNE_VERSION "\n"
                                  "====================================================\n"
                                  "Copyright .....: " CTUNE_YEARS " " CTUNE_AUTHOR " (" CTUNE_HOMEPAGE_URL ")\n"
                                  "Released under : " CTUNE_LICENSE "\n"
                                  "----------------------------------------------------\n"
                                  "Libraries:\n"
#ifdef USING_FFMPEG
                                  "  FFMpeg (libavformat " XSTR(LIBAVFORMAT_VERSION_MAJOR) "." XSTR(LIBAVFORMAT_VERSION_MINOR) "." XSTR(LIBAVFORMAT_VERSION_MICRO) ", "
"libavcodec " XSTR(LIBAVCODEC_VERSION_MAJOR) "." XSTR(LIBAVCODEC_VERSION_MINOR) "." XSTR(LIBAVCODEC_VERSION_MICRO) ", "
"libswresample " XSTR(LIBSWRESAMPLE_VERSION_MAJOR) "." XSTR(LIBSWRESAMPLE_VERSION_MINOR) "." XSTR(LIBSWRESAMPLE_VERSION_MICRO) ")\n"
#endif
#ifdef USING_VLC
                                  "  VLC " XSTR(LIBVLC_VERSION_MAJOR) "." XSTR(LIBVLC_VERSION_MINOR) "." XSTR(LIBVLC_VERSION_REVISION) "\n"
#endif
#ifdef USING_PULSEAUDIO
                                  "  PulseAudio (" XSTR(PA_MAJOR) "." XSTR(PA_MINOR) "." XSTR(PA_MICRO) ")\n"
#endif
#ifdef USING_SDL
                                  "  SDL (" XSTR(SDL_MAJOR_VERSION) "." XSTR(SDL_MINOR_VERSION) "." XSTR(SDL_PATCHLEVEL) ")\n"
#endif
#ifdef USING_ALSA
                                  "  ALSA (" XSTR(SND_LIB_MAJOR) "." XSTR(SND_LIB_MINOR) "." XSTR(SND_LIB_SUBMINOR) ")\n"
#endif
#ifdef USING_SNDIO
                                  "  sndio\n"
#endif
                                  "  json-c " XSTR(JSON_C_MAJOR_VERSION) "." XSTR(JSON_C_MINOR_VERSION) "." XSTR(JSON_C_MICRO_VERSION) "\n"
                                  "  " OPENSSL_VERSION_TEXT "\n"
                                  "  Curl " LIBCURL_VERSION "\n"
                                  "  nCurses " XSTR(NCURSES_VERSION_MAJOR) "." XSTR(NCURSES_VERSION_MINOR) "." XSTR(NCURSES_VERSION_PATCH) "\n"
                                  "----------------------------------------------------\n";

    return version;
}

/**
 * [PRIVATE] Gets the cTune usage information format string
 * @return information format string
 */
static const char * ctune_CLI_usage() {
    static const char * usage_fmt = "Usage: %s [OPTION]...\n"
                                    "Curses based internet radio player.\n"
                                    "\n"
                                    "       --debug          prints out all debug messages to the log\n"
                                    "   -f  --favourite      add station to favourites when used in conjunction with \"--play\"\n"
                                    "   -h  --help           display this help and exits\n"
                                    "   -p  --play \"UUID\"    plays the radio stream matching the RadioBrowser UUID\n"
                                    "   -r  --resume         resumes station playback of the last session\n"
                                    "       --show-cursor    always visible cursor\n"
                                    "   -v  --version        prints version information and exits\n";
    return usage_fmt;
}

/**
 * [PRIVATE] Parse cli arguments to a vector collection
 * @param argc Argument count
 * @param argv Argument array
 * @param opts Container Vector to store parsed options in
 * @return Success w/o errors
 */
static bool ctune_CLI_parse( int argc, char *argv[], Vector_t * opts ) {
    ctune_ArgOption_t * curr_option       = NULL;
    bool                error_state       = false;
    bool                awaiting_val_curr = false;
    bool                awaiting_val_next = false;

    //program name
    curr_option = Vector.emplace_back( opts );
    curr_option->arg = CTUNE_CLI_ARG_PROGNAME;
    curr_option->opt = String.init();
    curr_option->val = String.init();
    String.set( &curr_option->val, argv[0] );

    //program arguments
    for( int i = 1; i < argc; ++i ) {
        awaiting_val_curr = awaiting_val_next;
        awaiting_val_next = false;

        enum CTUNE_CLI_ARG curr_arg = CTUNE_CLI_ARG_UNKNOWN;

        if( strcmp( argv[i], "--help" ) == 0 || strcmp( argv[i], "-h" ) == 0 ) {
            curr_arg = CTUNE_CLI_ARG_HELP;

        } else if( strcmp( argv[i], "-f" ) == 0 || strcmp( argv[i], "--favourite" ) == 0 ) {
            curr_arg = CTUNE_CLI_ARG_FAVOURITE;

        } else if( strcmp( argv[i], "-p" ) == 0 || strcmp( argv[i], "--play" ) == 0 ) {
            curr_arg = CTUNE_CLI_ARG_PLAY;
            awaiting_val_next = true;

        } else if( strcmp( argv[i], "--debug" ) == 0 ) {
            curr_arg = CTUNE_CLI_ARG_DEBUG;

        } else if( strcmp( argv[i], "--resume" ) == 0 || strcmp( argv[i], "-r" ) == 0 ) {
            curr_arg = CTUNE_CLI_ARG_RESUME_PLAYBACK;

        } else if( strcmp( argv[i], "--show-cursor" ) == 0 ) {
            curr_arg = CTUNE_CLI_ARG_SHOW_CURSOR;

        } else if( strcmp( argv[i], "--version" ) == 0 || strcmp( argv[i], "-v" ) == 0 ) {
            curr_arg = CTUNE_CLI_ARG_VERSION;

        } else {
            curr_arg = CTUNE_CLI_ARG_UNKNOWN;
        }

        ctune_ArgOption_t * prev_opt = curr_option;
        curr_option = Vector.emplace_back( opts );
        curr_option->opt = String.init();
        curr_option->val = String.init();

        if( awaiting_val_curr ) {
            if( curr_arg == CTUNE_CLI_ARG_UNKNOWN && prev_opt != NULL ) { //potential candidate for a value
                curr_option->arg = prev_opt->arg;
                String.set( &curr_option->val, argv[i] );

            } else { //value missing!
                prev_opt->arg = CTUNE_CLI_ARG_VAL_NOT_FOUND;
                curr_option->arg = curr_arg;
                String.set( &curr_option->opt, argv[i] );
                error_state = true;
            }

        } else {
            if( curr_arg == CTUNE_CLI_ARG_UNKNOWN )
                error_state = true;

            curr_option->arg = curr_arg;
            String.set( &curr_option->opt, argv[i] );
        }
    }

    return !( error_state );
}

/**
 * [PRIVATE] Memory de-allocation of an ArgOption_t struct
 * @param arg_option Object to free
 */
static void ctune_CLI_free_ArgOption( void * arg_option ) {
    String.free( &( (ctune_ArgOption_t *) arg_option)->opt );
    String.free( &( (ctune_ArgOption_t *) arg_option)->val );
}

/**
 * Initialises CLI and processes arguments
 * @param argc Argument count
 * @param argv Argument array
 * @return Result of processing
 */
static enum CTUNE_CLI_STATES ctune_CLI_processArgs( int argc, char * argv[] ) {
    cli.all_options        = Vector.init( sizeof( ctune_ArgOption_t ), ctune_CLI_free_ArgOption );
    cli.actionable_options = Vector.init( sizeof( ctune_ArgOption_t ), ctune_CLI_free_ArgOption );

    bool success = ctune_CLI_parse( argc, argv, &cli.all_options );

    if( !success || Vector.empty( &cli.all_options ) ) { //ERROR
        fprintf( stdout, "Error parsing argument(s):\n" );

        for( size_t i = 0; i < Vector.size( &cli.all_options ); ++i ) {
            ctune_ArgOption_t * data = Vector.at( &cli.all_options, i );

            if( data->arg == CTUNE_CLI_ARG_UNKNOWN )
                fprintf( stdout, "  Option not recognised : %s\n", ( String.empty( &data->opt ) ? data->val._raw : data->opt._raw ) );
            else if( data->arg == CTUNE_CLI_ARG_VAL_NOT_FOUND )
                fprintf( stdout, "  Expected value missing: %s\n", ( String.empty( &data->opt ) ? data->val._raw : data->opt._raw ) );
        }

        return CTUNE_CLI_EXIT_ERR; //EARLY RETURN
    }

    ctune_ArgOption_t * prog_name = Vector.at( &cli.all_options, 0 );

    for( size_t i = 1; i < Vector.size( &cli.all_options ); ++i ) {
        ctune_ArgOption_t * data = Vector.at( &cli.all_options, i );

        switch( data->arg ) {
            case CTUNE_CLI_ARG_HELP:
                fprintf( stdout, ctune_CLI_usage(), prog_name->val );
                return CTUNE_CLI_EXIT_OK; //EARLY RETURN

            case CTUNE_CLI_ARG_DEBUG:           //fallthrough
            case CTUNE_CLI_ARG_FAVOURITE:       //fallthrough
            case CTUNE_CLI_ARG_PLAY:            //fallthrough
            case CTUNE_CLI_ARG_RESUME_PLAYBACK: //fallthrough
            case CTUNE_CLI_ARG_SHOW_CURSOR: {
                ctune_ArgOption_t * el = Vector.emplace_back( &cli.actionable_options );

                if( el != NULL ) {
                    el->arg = data->arg;
                    el->opt = String.init();
                    el->val = String.init();
                    String.copy( &el->opt, &data->opt );
                    String.copy( &el->val, &data->val );
                }
            } break;

            case CTUNE_CLI_ARG_VERSION:
                fprintf( stdout, "%s", ctune_CLI_version() );
                return CTUNE_CLI_EXIT_OK; //EARLY RETURN

            case CTUNE_CLI_ARG_UNKNOWN:       //fallthrough (should not come up here)
            case CTUNE_CLI_ARG_VAL_NOT_FOUND: //fallthrough (should not come up here)
            case CTUNE_CLI_ARG_PROGNAME:      //fallthrough (should not come up here as always i=0 in vector)
            default:
                fprintf( stderr, "[ctune_CLI_processArgs(..)] Unexpected/Unknown `CTUNE_CLI_ARG` enum (%i).\n", data->arg );
                return CTUNE_CLI_EXIT_ERR; //EARLY RETURN
        }
    }

    return Vector.empty( &cli.actionable_options )
           ? CTUNE_CLI_CONTINUE
           : CTUNE_CLI_CONTINUE_WITH_OPT;
}

/**
 * Get a pointer to the collection of actionable options
 * @return Pointer to vector of actionable options
 */
static Vector_t * ctune_CLI_getActionableOptions() {
    return &cli.actionable_options;
}

/**
 * De-allocates/cleanup internal CLI variables
 */
static void ctune_CLI_free() {
    Vector.clear_vector( &cli.all_options );
    Vector.clear_vector( &cli.actionable_options );
}

/**
 * Namespace constructor
 */
const struct ctune_CLI_Instance ctune_CLI = {
    .processArgs          = &ctune_CLI_processArgs,
    .getActionableOptions = &ctune_CLI_getActionableOptions,
    .free                 = &ctune_CLI_free
};