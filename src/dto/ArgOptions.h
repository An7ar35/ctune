#ifndef CTUNE_DTO_ARGOPTIONS_H
#define CTUNE_DTO_ARGOPTIONS_H

#include <stdbool.h>

#include "../logger/Logger.h"
#include "../datastructure/String.h"

/**
 * Container for the runtime options
 */
struct ctune_ArgOptions {
    ctune_LogLevel_e log_level;

    struct {
        bool     show_cursor;

    } ui;

    struct {
        String_t init_station_uuid;
        bool     favourite_init;
        bool     resume_playback;

    } playback;
};

typedef struct ctune_ArgOptions ctune_ArgOptions_t;


extern const struct ctune_Options_Namespace {
    /**
     * Sends all options to the log (for debug purposes)
     * @param caller Description of the caller method or stage
     * @param opts   Options DTO
     */
    void (* sendToLogger)( const char * caller, const ctune_ArgOptions_t * opts );

    /**
     * De-allocates heap allocated resources inside an Options DTO
     * @param opts Options DTO
     */
    void (* freeContent)( ctune_ArgOptions_t * opts );

} ctune_ArgOptions;

#endif //CTUNE_DTO_ARGOPTIONS_H
