#ifndef CTUNE_CLI_CLI_H
#define CTUNE_CLI_CLI_H

#include <stdbool.h>
#include "../datastructure/String.h"
#include "../datastructure/Vector.h"

/**
 * [ENUM] CLI argument types
 */
enum CTUNE_CLI_ARG {
    CTUNE_CLI_ARG_UNKNOWN,
    CTUNE_CLI_ARG_VAL_NOT_FOUND,
    CTUNE_CLI_ARG_PROGNAME,
    CTUNE_CLI_ARG_DEBUG,
    CTUNE_CLI_ARG_FAVOURITE,
    CTUNE_CLI_ARG_HELP,
    CTUNE_CLI_ARG_PLAY,
    CTUNE_CLI_ARG_RESUME_PLAYBACK,
    CTUNE_CLI_ARG_SHOW_CURSOR,
    CTUNE_CLI_ARG_VERSION,
};

/**
 * [DTO] Container for CLI arguments and their values
 * @param arg Argument type
 * @param opt Argument string (optional)
 * @param val Argument value string (optional)
 */
typedef struct ctune_CLI_ArgOption {
    enum CTUNE_CLI_ARG arg;
    String_t           opt;
    String_t           val;

} ctune_ArgOption_t;

/**
 * [ENUM] Resulting states of processing cli arguments
 */
enum CTUNE_CLI_STATES {
    CTUNE_CLI_EXIT_OK           = 0,
    CTUNE_CLI_EXIT_ERR          = 1,
    CTUNE_CLI_CONTINUE          = 2,
    CTUNE_CLI_CONTINUE_WITH_OPT = 3
};


extern const struct ctune_CLI_Instance {
    /**
     * Initialises CLI and processes arguments
     * @param argc Argument count
     * @param argv Argument array
     * @return Result of processing
     */
    enum CTUNE_CLI_STATES (* processArgs)( int argc, char * argv[] );

    /**
     * Get a pointer to the internal collection of actionable options for reading only.
     *
     * *Note*: any argument that has a value will have that/these
     * values consecutively arranged in the Vector inside
     * `ArgOption_t` objects. E.g.: ..[opt="-p"][val="uuid"][..]..
     *
     * @return Pointer to vector of actionable options
     */
    Vector_t * (* getActionableOptions)( void );

    /**
     * De-allocates/cleanup internal CLI variables
     */
    void (* free)( void );

} ctune_CLI;

#endif //CTUNE_CLI_CLI_H
