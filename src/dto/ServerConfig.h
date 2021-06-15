#ifndef CTUNE_DTO_SERVERCONFIG_H
#define CTUNE_DTO_SERVERCONFIG_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "../datastructure/StrList.h"

typedef struct ctune_ServerConfig {
    char *         check_enabled;
    char *         prometheus_exporter_enabled;
    struct StrList pull_servers;
    ulong          tcp_timeout_seconds;
    ulong          broken_stations_never_working_timeout_seconds;
    ulong          broken_stations_timeout_seconds;
    ulong          checks_timeout_seconds;
    ulong          click_valid_timeout_seconds;
    ulong          clicks_timeout_seconds;
    ulong          mirror_pull_interval_seconds;
    ulong          update_caches_interval_seconds;
    char *         server_name;
    ulong          check_retries;
    ulong          check_batchsize;
    ulong          check_pause_seconds;
    ulong          api_threads;
    char *         cache_type;
    ulong          cache_ttl;

} ctune_ServerConfig_t;


extern const struct ctune_ServerConfig_Namespace {
    /**
     * Initialise fields in the struct
     * @param stats ServerConfig DTO pointer
     */
    void (* init)( struct ctune_ServerConfig *cfg );

    /**
     * Frees the content of a ServerConfig DTO
     * @param cat_item ServerConfig DTO
     */
    void (* freeContent)( struct ctune_ServerConfig *cfg );

    /**
     * Prints a ServerConfig
     * @param out Output
     * @param cfg ServerConfig instance
     */
    void (* print)( FILE *out, const struct ctune_ServerConfig *cfg );

} ctune_ServerConfig;

#endif //CTUNE_DTO_SERVERCONFIG_H
