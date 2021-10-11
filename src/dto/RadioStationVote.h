#ifndef CTUNE_DTO_RADIOSTATIONVOTE_H
#define CTUNE_DTO_RADIOSTATIONVOTE_H

#include <stdlib.h>
#include <stdio.h>

#include "Field.h"

typedef struct ctune_RadioStationVote {
    char * ok;
    char * message;

} ctune_RadioStationVote_t;


extern const struct ctune_RadioStationVote_Namespace {
    /**
     * Initialise fields in the struct
     * @param stats RadioStationVote DTO pointer
     */
    void (* init)( void *rsv );

    /**
     * Frees the content of a RadioStationVote DTO
     * @param stats RadioStationVote DTO
     */
    void (* freeContent)( void *rsv );

    /**
     * Prints a RadioStationVote
     * @param out   Output
     * @param stats RadioStationVote instance
     */
    void (* print)( FILE *out, const struct ctune_RadioStationVote *rsv );

    /**
     * Gets a field by its name string
     * @param rsi ClickCounter_t object
     * @param api_name Name string
     * @return Field
     */
    ctune_Field_t (* getField)( struct ctune_RadioStationVote *rsv, const char *api_name );

} ctune_RadioStationVote;

#endif //CTUNE_DTO_RADIOSTATIONVOTE_H
