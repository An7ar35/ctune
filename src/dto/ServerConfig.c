#include "ServerConfig.h"

/**
 * Initialise fields in the struct
 * @param stats ServerConfig DTO pointer
 */
static void ctune_ServerConfig_init( struct ctune_ServerConfig * cfg ) {
    cfg->check_enabled                                 = NULL;
    cfg->prometheus_exporter_enabled                   = NULL;
    cfg->pull_servers                                  = StrList.init();
    cfg->tcp_timeout_seconds                           = 0;
    cfg->broken_stations_never_working_timeout_seconds = 0;
    cfg->broken_stations_timeout_seconds               = 0;
    cfg->checks_timeout_seconds                        = 0;
    cfg->click_valid_timeout_seconds                   = 0;
    cfg->clicks_timeout_seconds                        = 0;
    cfg->mirror_pull_interval_seconds                  = 0;
    cfg->update_caches_interval_seconds                = 0;
    cfg->server_name                                   = NULL;
    cfg->check_retries                                 = 0;
    cfg->check_batchsize                               = 0;
    cfg->check_pause_seconds                           = 0;
    cfg->api_threads                                   = 0;
    cfg->cache_type                                    = NULL;
    cfg->cache_ttl                                     = 0;
}

/**
 * Frees the content of a ServerConfig DTO
 * @param cat_item ServerConfig DTO
 */
static void ctune_ServerConfig_freeContent( struct ctune_ServerConfig * cfg ) {
    if( cfg == NULL )
        return; //EARLY RETURN

    if( cfg->check_enabled ) {
        free( cfg->check_enabled );
        cfg->check_enabled = NULL;
    }

    if( cfg->prometheus_exporter_enabled ) {
        free( cfg->prometheus_exporter_enabled );
        cfg->prometheus_exporter_enabled = NULL;
    }

    StrList.free_strlist( &cfg->pull_servers );

    if( cfg->server_name ) {
        free( cfg->server_name );
        cfg->server_name = NULL;
    }

    if( cfg->cache_type ) {
        free( cfg->cache_type );
        cfg->cache_type = NULL;
    }
}

/**
 * Prints a ServerConfig
 * @param out Output
 * @param cfg ServerConfig instance
 */
static void ctune_ServerConfig_print( FILE * out, const struct ctune_ServerConfig * cfg ) {
    fprintf( out, "Check enabled .......................: %s\n", cfg->check_enabled );
    fprintf( out, "Prometheus exporter enabled .........: %s\n", cfg->prometheus_exporter_enabled );
    fprintf( out, "Pull servers ........................: " );
    struct StrListNode * curr = cfg->pull_servers._front;

    while( curr ) {
        fprintf( out, "%s\n", curr->data );
        curr = curr->next;
        if( curr )
            fprintf( out, "                                       " );
    }

    fprintf( out, "TCP timeout .........................: %lus\n", cfg->tcp_timeout_seconds );
    fprintf( out, "Broken stations never working timeout: %lus\n", cfg->broken_stations_never_working_timeout_seconds );
    fprintf( out, "Broken stations timeout .............: %lus\n", cfg->broken_stations_timeout_seconds );
    fprintf( out, "Checks timeout ......................: %lus\n", cfg->checks_timeout_seconds );
    fprintf( out, "Click valid timeout .................: %lus\n", cfg->click_valid_timeout_seconds );
    fprintf( out, "Clicks timeout ......................: %lus\n", cfg->clicks_timeout_seconds );
    fprintf( out, "Mirror pull interval ................: %lus\n", cfg->mirror_pull_interval_seconds );
    fprintf( out, "Update cache interval ...............: %lus\n", cfg->update_caches_interval_seconds );
    fprintf( out, "Server name .........................: %s\n", cfg->server_name );
    fprintf( out, "Check retries .......................: %lu\n", cfg->check_retries );
    fprintf( out, "Check batch size ....................: %lu\n", cfg->check_batchsize );
    fprintf( out, "Check pause .........................: %lus\n", cfg->check_pause_seconds );
    fprintf( out, "API threads .........................: %lu\n", cfg->api_threads );
    fprintf( out, "Cache type ..........................: %s\n", cfg->cache_type );
    fprintf( out, "Cache TTL ...........................: %lu\n", cfg->cache_ttl );
}


/**
 * Namespace constructor
 */
const struct ctune_ServerConfig_Namespace ctune_ServerConfig = {
   .init        = &ctune_ServerConfig_init,
   .freeContent = &ctune_ServerConfig_freeContent,
   .print       = &ctune_ServerConfig_print,
};