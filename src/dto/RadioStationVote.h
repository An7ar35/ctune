#ifndef CTUNE_DTO_RADIOSTATIONVOTE_H
#define CTUNE_DTO_RADIOSTATIONVOTE_H

#include <stdlib.h>
#include <stdio.h>

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

} ctune_RadioStationVote;

#endif //CTUNE_DTO_RADIOSTATIONVOTE_H
