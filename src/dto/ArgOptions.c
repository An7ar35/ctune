#include "ArgOptions.h"

#include "../logger/Logger.h"

/**
 * Sends all options to the log (for debug purposes)
 * @param caller Description of the caller method or stage
 * @param opts   Options DTO
 */
static void ctune_Options_sendToLogger( const char * caller, const ctune_ArgOptions_t * opts ) {
    if( opts == NULL )
        return; //EARLY RETURN

    if( opts->log_level > CTUNE_LOG_MSG )
        CTUNE_LOG( CTUNE_LOG_MSG, "[%s] CMD arg. option: debug on (lvl=%i)", caller, opts->log_level );

    if( opts->ui.show_cursor )
        CTUNE_LOG( CTUNE_LOG_MSG, "[%s] CMD arg, option: show cursor", caller );

    if( opts->playback.favourite_init )
        CTUNE_LOG( CTUNE_LOG_MSG, "[%s] CMD arg, option: add arg station to favourite", caller );

    if( opts->playback.resume_playback )
        CTUNE_LOG( CTUNE_LOG_MSG, "[%s] CMD arg. option: resume playback", caller );

    if( !String.empty( &opts->playback.init_station_uuid ) )
        CTUNE_LOG( CTUNE_LOG_MSG, "[%s] CMD arg. option: play station UUID = <%s>", caller, opts->playback.init_station_uuid._raw );
}

/**
 * De-allocates heap allocated resources inside an Options DTO
 * @param opts Options DTO
 */
static void ctune_Options_freeContent( ctune_ArgOptions_t * opts ) {
    if( opts ) {
        String.free( &opts->playback.init_station_uuid );
    }
}

/**
 * Namespace constructor
 */
const struct ctune_Options_Namespace ctune_ArgOptions = {
    .sendToLogger = &ctune_Options_sendToLogger,
    .freeContent  = &ctune_Options_freeContent
};