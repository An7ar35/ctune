#ifndef CTUNE_DTO_SERVERSTATS_H
#define CTUNE_DTO_SERVERSTATS_H

#include <stdlib.h>
#include <stdio.h>

typedef struct ctune_ServerStats {
    char * supported_version;
    char * software_version;
    char * status;
    ulong  stations;
    ulong  stations_broken;
    ulong  tags;
    ulong  clicks_last_hour;
    ulong  clicks_last_day;
    ulong  languages;
    ulong  countries;

} ctune_ServerStats_t;


extern const struct ctune_ServerStats_Namespace {
    /**
     * Initialise fields in the struct
     * @param stats ServerStats DTO pointer
     */
    void (* init)( struct ctune_ServerStats *stats );

    /**
     * Frees the content of a ServerStats DTO
     * @param stats ServerStats DTO
     */
    void (* freeContent)( struct ctune_ServerStats *stats );

    /**
     * Prints a ServerStats
     * @param out   Output
     * @param stats ServerStats instance
     */
    void (* print)( FILE *out, const struct ctune_ServerStats *stats );

} ctune_ServerStats;

#endif //CTUNE_DTO_SERVERSTATS_H
