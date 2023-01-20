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

    free( cfg->check_enabled );
    cfg->check_enabled = NULL;

    free( cfg->prometheus_exporter_enabled );
    cfg->prometheus_exporter_enabled = NULL;

    StrList.free_strlist( &cfg->pull_servers );

    free( cfg->server_name );
    cfg->server_name = NULL;

    free( cfg->cache_type );
    cfg->cache_type = NULL;
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
 * Gets a field by its name string
 * @param rsi ServerConfig object
 * @param api_name Name string
 * @return Field
 */
inline static ctune_Field_t ctune_ServerConfig_getField( struct ctune_ServerConfig *cfg, const char *api_name ) {
    if( strcmp( api_name, "check_enabled" ) == 0 ) {
        return (ctune_Field_t){ ._field = &cfg->check_enabled, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "prometheus_exporter_enabled" ) == 0 ) {
        return (ctune_Field_t){ ._field = &cfg->prometheus_exporter_enabled, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "pull_servers" ) == 0 ) {
        return (ctune_Field_t){ ._field = &cfg->pull_servers, ._type = CTUNE_FIELD_STRLIST };

    } else if( strcmp( api_name, "tcp_timeout_seconds" ) == 0 ) {
        return (ctune_Field_t){ ._field = &cfg->tcp_timeout_seconds, ._type = CTUNE_FIELD_UNSIGNED_LONG };

    } else if( strcmp( api_name, "broken_stations_never_working_timeout_seconds" ) == 0 ) {
        return (ctune_Field_t){ ._field = &cfg->broken_stations_never_working_timeout_seconds, ._type = CTUNE_FIELD_UNSIGNED_LONG };

    } else if( strcmp( api_name, "broken_stations_timeout_seconds" ) == 0 ) {
        return (ctune_Field_t){ ._field = &cfg->broken_stations_timeout_seconds, ._type = CTUNE_FIELD_UNSIGNED_LONG };

    } else if( strcmp( api_name, "checks_timeout_seconds" ) == 0 ) {
        return (ctune_Field_t){ ._field = &cfg->checks_timeout_seconds, ._type = CTUNE_FIELD_UNSIGNED_LONG };

    } else if( strcmp( api_name, "click_valid_timeout_seconds" ) == 0 ) {
        return (ctune_Field_t){ ._field = &cfg->click_valid_timeout_seconds, ._type = CTUNE_FIELD_UNSIGNED_LONG };

    } else if( strcmp( api_name, "clicks_timeout_seconds" ) == 0 ) {
        return (ctune_Field_t){ ._field = &cfg->clicks_timeout_seconds, ._type = CTUNE_FIELD_UNSIGNED_LONG };

    } else if( strcmp( api_name, "mirror_pull_interval_seconds" ) == 0 ) {
        return (ctune_Field_t){ ._field = &cfg->mirror_pull_interval_seconds, ._type = CTUNE_FIELD_UNSIGNED_LONG };

    } else if( strcmp( api_name, "update_caches_interval_seconds" ) == 0 ) {
        return (ctune_Field_t){ ._field = &cfg->update_caches_interval_seconds, ._type = CTUNE_FIELD_UNSIGNED_LONG };

    } else if( strcmp( api_name, "server_name" ) == 0 ) {
        return (ctune_Field_t){ ._field = &cfg->server_name, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "check_retries" ) == 0 ) {
        return (ctune_Field_t){ ._field = &cfg->check_retries, ._type = CTUNE_FIELD_UNSIGNED_LONG };

    } else if( strcmp( api_name, "check_batchsize" ) == 0 ) {
        return (ctune_Field_t){ ._field = &cfg->check_batchsize, ._type = CTUNE_FIELD_UNSIGNED_LONG };

    } else if( strcmp( api_name, "check_pause_seconds" ) == 0 ) {
        return (ctune_Field_t){ ._field = &cfg->check_pause_seconds, ._type = CTUNE_FIELD_UNSIGNED_LONG };

    } else if( strcmp( api_name, "api_threads" ) == 0 ) {
        return (ctune_Field_t){ ._field = &cfg->api_threads, ._type = CTUNE_FIELD_UNSIGNED_LONG };

    } else if( strcmp( api_name, "cache_type" ) == 0 ) {
        return (ctune_Field_t){ ._field = &cfg->cache_type, ._type = CTUNE_FIELD_CHAR_PTR };

    } else if( strcmp( api_name, "cache_ttl" ) == 0 ) {
        return (ctune_Field_t){ ._field = &cfg->cache_ttl, ._type = CTUNE_FIELD_UNSIGNED_LONG };

    } else {
        return (ctune_Field_t) { ._field = NULL, ._type = CTUNE_FIELD_UNKNOWN };
    }
}


/**
 * Namespace constructor
 */
const struct ctune_ServerConfig_Namespace ctune_ServerConfig = {
   .init        = &ctune_ServerConfig_init,
   .freeContent = &ctune_ServerConfig_freeContent,
   .print       = &ctune_ServerConfig_print,
   .getField    = &ctune_ServerConfig_getField,
};