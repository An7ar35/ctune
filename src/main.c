#include <signal.h>
#include <string.h>
#include <locale.h>
#include <langinfo.h>

#include "logger/src/Logger.h"
#include "ctune_err.h"
#include "cli/CLI.h"
#include "fs/Settings.h"
#include "fs/XDG.h"
#include "fs/PlaybackLog.h"
#include "Controller.h"
#include "ui/UI.h"
#include "ui/Resizer.h"

//TODO [major functionality] playlist tab + functionality (load csv files from cmd args/dialog box?)
//TODO [medium functionality] Playlog viewer tab with stations played by descending order on the left and tracks played on the right inc timestamp
//                            + must auto refresh on view
//                            + ability to view station info (cache it?)

/* setup/teardown and system */
static bool ctune_init( const ctune_ArgOptions_t * options );
static void ctune_shutdown();
static void ctune_setupSigHandler();
static void ctune_handleSignal( int signo, siginfo_t * info, void * context );

/**
 * Program entry point
 * @param argc Number of arguments
 * @param argv Pointer to arguments array
 * @return Exit state
 */
int main( int argc, char * argv[] ) {
    if( !ctune_Settings.rtlock.lock() ) {
        exit( ERR ); //EARLY EXIT
    };

    ctune_ArgOptions_t options = {
        #ifndef NDEBUG
            .log_level         = CTUNE_LOG_TRACE,
        #else
            .log_level         = CTUNE_LOG_MSG,
        #endif

        .ui = {
            .show_cursor       = false,
        },

        .playback = {
            .init_station_uuid = { ._raw = NULL, ._length = 0 },
            .favourite_init    = false,
            .resume_playback   = false
        }
    };

    /* CLI ARGUMENTS PROCESSING */
    switch( ctune_CLI.processArgs( argc, argv ) ) {

        case CTUNE_CLI_EXIT_OK:
            ctune_CLI.free();
            ctune_Settings.rtlock.unlock();
            exit( OK ); //EARLY EXIT

        case CTUNE_CLI_EXIT_ERR:
            ctune_CLI.free();
            ctune_Settings.rtlock.unlock();
            exit( ERR ); //EARLY EXIT

        case CTUNE_CLI_CONTINUE_WITH_OPT: {
            Vector_t * cli_options = ctune_CLI.getActionableOptions(); //freed via `ctune_CLI.free()`

            for( size_t i = 0; i < Vector.size( cli_options ); ++i ) {
                ctune_ArgOption_t * opt = (ctune_ArgOption_t *) Vector.at( cli_options, i );

                if( opt->arg == CTUNE_CLI_ARG_DEBUG ) {
                    options.log_level = CTUNE_LOG_TRACE;

                } else if( opt->arg == CTUNE_CLI_ARG_SHOW_CURSOR ) {
                    options.ui.show_cursor = true;

                } else if( opt->arg == CTUNE_CLI_ARG_FAVOURITE ) {
                    options.playback.favourite_init = true;

                } else if( opt->arg == CTUNE_CLI_ARG_PLAY && Vector.size( cli_options ) > ( i + 1 ) ) {
                    opt = (ctune_ArgOption_t *) Vector.at( cli_options, ++i ); //advance to option value
                    String.copy( &options.playback.init_station_uuid, &opt->val );

                } else if( opt->arg == CTUNE_CLI_ARG_RESUME_PLAYBACK ) {
                    options.playback.resume_playback = true;
                }
            }
        } //fallthrough

        case CTUNE_CLI_CONTINUE:
            ctune_CLI.free();
            break;
    }

    //setup locale
    const char * locale  = setlocale( LC_ALL, "" );
    const char * charset = nl_langinfo( CODESET );

    CTUNE_LOG( CTUNE_LOG_DEBUG, "Locale (LC_ALL): %s", locale );

    if ( !charset || !charset[0] || strcmp( charset, "UTF-8" ) != 0 ) {
        CTUNE_LOG( CTUNE_LOG_FATAL, "Codeset is not UTF-8." );
        ctune_ArgOptions.freeContent( &options );
        ctune_shutdown();
        fprintf( stderr, "Locale is not UTF-8 (%s)\n", charset );
        exit( ERR ); //EARLY EXIT
    }

    /* CTUNE INITIALISATION */
    if( !ctune_init( &options ) ) {
        ctune_ArgOptions.freeContent( &options );
        ctune_shutdown();
        exit( ERR ); //EARLY EXIT
    }

    ctune_setupSigHandler();

    ctune_Controller.load( &options );
    ctune_ArgOptions.freeContent( &options );
    ctune_UI.start(); //(UI loop)

    ctune_shutdown();
    exit_curses( OK );
    return OK;
}

/**
 * Initialise cTune
 * @return Success
 */
static bool ctune_init( const ctune_ArgOptions_t * options ) {
    bool     error_state       = false;
    String_t err_log_path      = String.init();
    String_t playback_log_path = String.init();

    CTUNE_LOG( CTUNE_LOG_TRACE, "[INIT] cTune initialising..." );

    /* ERROR LOG */
    ctune_Settings.init();
    ctune_XDG.resolveDataFilePath( "ctune.log", &err_log_path );

    if( !ctune_Logger.init( err_log_path._raw, "w", options->log_level ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[INIT] Failed to initialise the logger." );
        error_state = true;
    }

    /* Print the args passed */
    ctune_ArgOptions.sendToLogger( "INIT", options );

    if( !ctune_Settings.plugins.loadPlugins() ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[INIT] Failed to load all available plugins." );
    }

    /* CONFIGURATION */
    if( !ctune_Settings.cfg.loadCfg() ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[INIT] Failed to load configuration file - using defaults." );
    }

    /* FAVOURITES */
    if( !ctune_Settings.favs.loadFavourites() ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[INIT] Failed to load favourites." );
    }

    /* PLAY LOG */
    ctune_XDG.resolveDataFilePath( "playlog.txt", &playback_log_path );

    if( !ctune_PlaybackLog.open( playback_log_path._raw, ctune_Settings.cfg.playbackLogOverwrite() ) ) {
        ctune_err.set( CTUNE_ERR_IO_PLAYLOG_OPEN );
        CTUNE_LOG( CTUNE_LOG_ERROR, "[INIT] Error: %s", ctune_err.strerror() );
    }

    /* CONTROLLER */
    if( !ctune_Controller.init() ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[INIT] Failed to initialise the controller." );
        error_state = true;
    }

    ctune_Controller.setStationChangeEventCallback( ctune_UI.setCurrStation );
    ctune_Controller.setSongChangeEventCallback( ctune_UI.printSongInfo );
    ctune_Controller.setVolumeChangeEventCallback( ctune_UI.printVolume );
    ctune_Controller.setPlaybackStateChangeEventCallback( ctune_UI.printPlaybackState );
    ctune_Controller.setSearchStateChangeEventCallback( ctune_UI.printSearchingState );
    ctune_Controller.setResizeUIEventCallback( ctune_UI_Resizer.resize );

    /* UI */
    if( !ctune_UI.setup( options->ui.show_cursor ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL, "[INIT] Failed to setup the UI." );
        error_state = true;
    }

    ctune_UI.setQuietVolChangeCallback( ctune_Settings.cfg.modVolume ); //used when playback is off and access to the audio server is not available
    ctune_UI.printVolume( ctune_Settings.cfg.getVolume() ); //to get the initial volume to display in the UI

    ctune_err.setPrintErrCallback( ctune_UI.printError );

    String.free( &playback_log_path );
    String.free( &err_log_path );

    if( !error_state )
        CTUNE_LOG( CTUNE_LOG_TRACE, "[INIT] cTune initialisation complete." );

    return !( error_state );
}

/**
 * Shutdown and cleanup cTune
 */
static void ctune_shutdown() {
    static bool initiated = false;

    if( initiated ) {
        //This is in case SIGABRT was raised in one of the 3rd party libraries
        //so that everything exits cleanly regardless (looking at you pulseaudio!).
        CTUNE_LOG( CTUNE_LOG_WARNING, "[ctune_shutdown()] Shutdown already initiated." );
        return; //EARLY RETURN
    }

    initiated = true;

    ctune_Controller.playback.stop();
    ctune_Controller.free();
    ctune_PlaybackLog.close();
    ctune_UI.teardown();
    ctune_Settings.cfg.writeCfg();
    ctune_Settings.rtlock.unlock();
    ctune_Settings.free();

    if( ctune_err.number() != CTUNE_ERR_NONE ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_shutdown()] cTune encountered an error: %s", ctune_err.strerror() );
    } else {
        CTUNE_LOG( CTUNE_LOG_MSG, "[ctune_shutdown()] cTune shutdown successful." );
    }

    ctune_Logger.close();
}

/**
 * Sets-up the signal handler (- call once)
 */
void ctune_setupSigHandler() {
    static struct sigaction signal_handler;

    memset( &signal_handler, 0, sizeof( signal_handler ) );
    signal_handler.sa_sigaction = ctune_handleSignal;
    signal_handler.sa_flags     = SA_SIGINFO;

    sigaction( SIGWINCH, &signal_handler, NULL );
    sigaction( SIGINT,   &signal_handler, NULL );
    sigaction( SIGTERM,  &signal_handler, NULL );
    sigaction( SIGQUIT,  &signal_handler, NULL );
    sigaction( SIGTSTP,  &signal_handler, NULL );
    sigaction( SIGHUP,   &signal_handler, NULL );
    sigaction( SIGABRT,  &signal_handler, NULL );
}

/**
 * Handles system signals
 * @param signo Signal to handle
 * @param info Pointer to signal information struct
 * @param context Pointer to context
 */
void ctune_handleSignal( int signo, siginfo_t * info, void * context ) {
    int exit_state = OK;

    switch( signo ) {
        case SIGWINCH: { //terminal resize event
            CTUNE_LOG( CTUNE_LOG_MSG, "[ctune_handleSignal( %d )] Caught interrupt signal (SIGWINCH).", signo );
            ctune_Controller.resizeUI();
        } break;

        case SIGABRT: //fallthrough
        case SIGINT : exit_state = ERR; //fallthrough
        case SIGQUIT:
        case SIGTSTP:
        case SIGTERM: //fallthrough
        case SIGHUP : {
            CTUNE_LOG( CTUNE_LOG_MSG, "[ctune_handleSignal( %d )] Caught signal: %s", signo, strsignal( signo ) );
            goto shutdown;
        } break;

        default: {
            CTUNE_LOG( CTUNE_LOG_MSG, "[ctune_handleSignal( %d )] Forwarding signal: %s", signo, strsignal( signo ) );
            signal( signo, SIG_DFL );
        } break;
    }

    return;

    shutdown:
        ctune_shutdown();
        exit_curses( exit_state );
}