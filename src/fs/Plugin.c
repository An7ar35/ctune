#include "Plugin.h"

#include <dlfcn.h>
#include <dirent.h>

#include "logger/src/Logger.h"

static struct {
    bool initialised;

    struct {
        Vector_t         list;
        ctune_Player_t * selected;
    } audio_players;

    struct {
        Vector_t           list;
        ctune_AudioOut_t * selected;
    } audio_servers;

    struct {
        Vector_t          list;
        ctune_FileOut_t * selected;
    } audio_file_writers;

} private = {
    .initialised                 = false,
    .audio_players.selected      = NULL,
    .audio_file_writers.selected = NULL,
    .audio_servers.selected      = NULL,
};

/**
 * [PRIVATE] Loads a plugin
 * @param handle File handle
 * @return Name of plugin on success or NULL
 */
static const char * ctune_Plugin_loadPlugin( void * handle ) {
    if( handle == NULL ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_Plugin_loadPlugin( %p )] NULL handle.",
                   handle
        );

        return NULL; //EARLY RETURN
    }

    const char               * plugin_name = NULL;
    char                     * error       = NULL;
    const unsigned int       * abi_version = dlsym( handle, "abi_version" );
    const ctune_PluginType_e * plugin_type = dlsym( handle, "plugin_type" );

    if( plugin_type == NULL || abi_version == NULL ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_Plugin_loadPlugin( %p )] NULL fields (abi_version: %p, plugin_type: %p).",
                   handle, abi_version, plugin_type
        );
        
        return NULL;
    }

    switch( *plugin_type ) {
        case CTUNE_PLUGIN_IN_STREAM_PLAYER: {
            if( *abi_version != CTUNE_PLAYER_ABI_VERSION ) {
                CTUNE_LOG( CTUNE_LOG_FATAL,
                           "[ctune_Plugin_loadPlugin( %p )] ABI version mismatch: plugin=%d, ctune=%d.",
                           handle, *abi_version, CTUNE_PLAYER_ABI_VERSION
                );

                return NULL; //EARLY RETURN
            }

            ctune_Player_t * plugin = Vector.emplace_back( &private.audio_players.list );

            if( plugin ) {
                plugin->handle      = handle;
                plugin->abi_version = abi_version;
                plugin->plugin_type = plugin_type;

                ctune_Player_t * p = dlsym( plugin->handle, "ctune_Player" );

                if( ( error = dlerror() ) != NULL ) {
                    CTUNE_LOG( CTUNE_LOG_FATAL,
                               "[ctune_Plugin_loadPlugin( %p )] Failed linking: %s ('%s')",
                               handle, error, ctune_PluginType.str( *plugin_type )
                    );

                    ctune_err.set ( CTUNE_ERR_IO_PLUGIN_LINK );
                    Vector.remove( &private.audio_file_writers.list, Vector.size( &private.audio_players.list ) - 1 );

                } else {
                    plugin->name            = p->name;
                    plugin->description     = p->description;
                    plugin->init            = p->init;
                    plugin->playRadioStream = p->playRadioStream;
                    plugin->getError        = p->getError;
                    plugin->testStream      = p->testStream;

                    plugin_name = plugin->name();
                }
            }
        } break;

        case CTUNE_PLUGIN_OUT_AUDIO_SERVER: {
            if( *abi_version != CTUNE_FILEOUT_ABI_VERSION ) {
                CTUNE_LOG( CTUNE_LOG_FATAL,
                           "[ctune_Plugin_loadPlugin( %p )] ABI version mismatch: plugin=%d, ctune=%d.",
                           handle, *abi_version, CTUNE_FILEOUT_ABI_VERSION
                );

                return NULL; //EARLY RETURN
            }

            ctune_AudioOut_t * plugin = Vector.emplace_back( &private.audio_servers.list );

            if( plugin ) {
                plugin->handle      = handle;
                plugin->abi_version = abi_version;
                plugin->plugin_type = plugin_type;

                ctune_AudioOut_t * ao = dlsym( plugin->handle, "ctune_AudioOutput" );

                if( ( error = dlerror() ) != NULL ) {
                    CTUNE_LOG( CTUNE_LOG_FATAL,
                               "[ctune_Plugin_loadPlugin( %p )] Failed linking: %s ('%s')",
                               handle, error, ctune_PluginType.str( *plugin_type )
                    );

                    ctune_err.set ( CTUNE_ERR_IO_PLUGIN_LINK );
                    Vector.remove( &private.audio_servers.list, Vector.size( &private.audio_servers.list ) - 1 );

                } else {
                    plugin->name         = ao->name;
                    plugin->description  = ao->description;
                    plugin->init         = ao->init;
                    plugin->write        = ao->write;
                    plugin->setVolume    = ao->setVolume;
                    plugin->changeVolume = ao->changeVolume;
                    plugin->getVolume    = ao->getVolume;
                    plugin->shutdown     = ao->shutdown;

                    plugin_name = plugin->name();
                }
            }
        } break;

        case CTUNE_PLUGIN_OUT_AUDIO_FILE: {
            if( *abi_version != CTUNE_AUDIOOUT_ABI_VERSION ) {
                CTUNE_LOG( CTUNE_LOG_FATAL,
                           "[ctune_Plugin_loadPlugin( %p )] ABI version mismatch: plugin=%d, ctune=%d.",
                           handle, *abi_version, CTUNE_AUDIOOUT_ABI_VERSION
                );

                return NULL; //EARLY RETURN
            }

            ctune_FileOut_t * plugin = Vector.emplace_back( &private.audio_file_writers.list );

            if( plugin ) {
                plugin->handle      = handle;
                plugin->abi_version = abi_version;
                plugin->plugin_type = plugin_type;

                struct ctune_FileOut * fo = dlsym( plugin->handle, "ctune_FileOutput" );

                if( ( error = dlerror() ) != NULL ) {
                    CTUNE_LOG( CTUNE_LOG_FATAL,
                               "[ctune_Plugin_loadPlugin( %p )] Failed linking: %s ('%s')",
                               handle, error, ctune_PluginType.str( *plugin_type )
                    );

                    ctune_err.set ( CTUNE_ERR_IO_PLUGIN_LINK );
                    Vector.remove( &private.audio_file_writers.list, Vector.size( &private.audio_file_writers.list ) - 1 );

                } else {
                    plugin->name        = fo->name;
                    plugin->description = fo->description;
                    plugin->init        = fo->init;
                    plugin->write       = fo->write;
                    plugin->close       = fo->close;

                    plugin_name = plugin->name();
                }
            }
        } break;
    }

    return plugin_name;
}

/**
 * [PRIVATE] Closes the plugin handles and frees nested resources
 * @param ao Pointer to AudioOut_t object
 */
static void ctune_Plugin_freeAudioOut( void * ao ) {
    ctune_AudioOut_t * ptr = ao;

    if( ptr != NULL && ptr->handle != NULL ) {
        ptr->shutdown();

        if( dlclose( ptr->handle ) != 0 ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_Plugin_freePlayer( %p )] Failed to close the handle of plugin '%s': %s",
                       ptr, ptr->name(), dlerror()
            );

            ctune_err.set( CTUNE_ERR_IO_PLUGIN_CLOSE );
        }

        ptr->abi_version  = NULL;
        ptr->init         = NULL;
        ptr->getVolume    = NULL;
        ptr->setVolume    = NULL;
        ptr->changeVolume = NULL;
        ptr->write        = NULL;
        ptr->shutdown     = NULL;
    }
}

/**
 * [PRIVATE] Closes the plugin handles and frees nested resources
 * @param player Pointer to RadioPlayer_t object
 */
static void ctune_Plugin_freePlayer( void * player ) {
    ctune_Player_t * ptr = player;

    if( ptr != NULL && ptr->handle != NULL ) {
        if( dlclose( ptr->handle ) != 0 ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_Plugin_freePlayer( %p )] Failed to close the handle of plugin '%s': %s",
                       ptr, ptr->name(), dlerror()
            );

            ctune_err.set( CTUNE_ERR_IO_PLUGIN_CLOSE );
        }

        ptr->abi_version     = NULL;
        ptr->init            = NULL;
        ptr->playRadioStream = NULL;
        ptr->getError        = NULL;
        ptr->testStream      = NULL;
    }
}

/**
 * [PRIVATE] Closes the plugin handles and frees nested resources
 * @param player Pointer to FileOut_t object
 */
static void ctune_Plugin_freeSoundFileOutput( void * fileout ) {
    ctune_FileOut_t * ptr = fileout;

    if( ptr != NULL && ptr->handle != NULL ) {
        if( dlclose( ptr->handle ) != 0 ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_Plugin_freeSoundFileOutput( %p )] Failed to close the handle of plugin '%s': %s",
                       ptr, ptr->name(), dlerror()
            );

            ctune_err.set( CTUNE_ERR_IO_PLUGIN_CLOSE );
        }

        ptr->abi_version = NULL;
        ptr->plugin_type = NULL;
        ptr->name        = NULL;
        ptr->description = NULL;
        ptr->init        = NULL;
        ptr->write       = NULL;
        ptr->close       = NULL;
    }
}

/**
 * Initialises plugin engine
 */
static void ctune_Plugin_init( void ) {
    private.audio_players.list      = Vector.init( sizeof( ctune_Player_t ), ctune_Plugin_freePlayer );
    private.audio_servers.list      = Vector.init( sizeof( ctune_AudioOut_t ), ctune_Plugin_freeAudioOut );
    private.audio_file_writers.list = Vector.init( sizeof( ctune_FileOut_t ), ctune_Plugin_freeSoundFileOutput );
}

/**
 * Loads all compatible plugins found in a directory
 * @param dir_path Directory path
 * @return Success
 */
bool ctune_Plugin_loadPlugins( const char * dir_path ) {
    if( dir_path == NULL ) {
        CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_Plugin_loadPlugins( %p )] NULL arg(s).", dir_path );
        return 0; //EARLY RETURN
    }

    bool            error_state = false;
    DIR           * directory;
    struct dirent * directory_entry;

    if( ( directory = opendir( dir_path ) ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_Plugin_loadPlugins( \"%s\" )] Failed to open directory.",
                   dir_path );
        error_state = true;
        goto end;
    }

    size_t filename_ln = 0;

    while( ( directory_entry = readdir( directory ) ) ) {
        if( directory_entry->d_type != DT_REG || ( filename_ln = strlen( directory_entry->d_name ) ) <= 3 ) {
            continue; //i.e. not a regular file and not enough chars in name for a '.so' extension
        }

        const char * extension = ctune_substr( directory_entry->d_name, ( filename_ln - 3 ), 3 );

        if( extension && strcmp( extension, ".so" ) == 0 ) {
            String_t filepath = String.init();
            String.append_back( &filepath, dir_path );
            String.append_back( &filepath, directory_entry->d_name );

            const char * plugin_name = ctune_Plugin_loadPlugin( dlopen( filepath._raw, RTLD_NOW ) );

            if( plugin_name ) {
                CTUNE_LOG( CTUNE_LOG_MSG,
                           "[ctune_Plugin_loadPlugins( \"%s\" )] Plugin loaded: %s",
                           dir_path, plugin_name
                );

            } else {
                CTUNE_LOG( CTUNE_LOG_FATAL,
                           "[ctune_Plugin_loadPlugins( \"%s\" )] Failed to link plugin '%s'.",
                           dir_path, directory_entry->d_name
                );

                error_state = true;
            }

            String.free( &filepath );
        }
    }

    end:
        closedir( directory );
        return !( error_state );
}

/**
 * Sets a plugin as 'selected'
 * @param type Plugin type enum
 * @param name Name of plugin to set
 * @return Success
 */
static bool ctune_Plugin_setPlugin( ctune_PluginType_e type, const char * name ) {
    switch( type ) {
        case CTUNE_PLUGIN_IN_STREAM_PLAYER: {
            for( size_t i = 0; i < Vector.size( &private.audio_players.list ); ++i ) {
                ctune_Player_t * p = Vector.at( &private.audio_players.list, i );

                if( strcmp( p->name(), name ) == 0 ) {
                    CTUNE_LOG( CTUNE_LOG_DEBUG,
                               "[ctune_Plugin_setPlugin( '%s', \"%s\" )] Plugin set as selected: %p",
                               ctune_PluginType.str( type ), name, p
                    );

                    private.audio_players.selected = p;
                    return true;
                }
            }
        } break;

        case CTUNE_PLUGIN_OUT_AUDIO_SERVER: {
            for( size_t i = 0; i < Vector.size( &private.audio_servers.list ); ++i ) {
                ctune_AudioOut_t * p = Vector.at( &private.audio_servers.list, i );

                if( strcmp( p->name(), name ) == 0 ) {
                    CTUNE_LOG( CTUNE_LOG_DEBUG,
                               "[ctune_Plugin_setPlugin( '%s', \"%s\" )] Plugin set as selected: %p",
                               ctune_PluginType.str( type ), name, p
                    );

                    private.audio_servers.selected = p;
                    return true;
                }
            }
        } break;

        case CTUNE_PLUGIN_OUT_AUDIO_FILE: {
            for( size_t i = 0; i < Vector.size( &private.audio_file_writers.list ); ++i ) {
                ctune_FileOut_t * p = Vector.at( &private.audio_file_writers.list, i );

                if( strcmp( p->name(), name ) == 0 ) {
                    CTUNE_LOG( CTUNE_LOG_DEBUG,
                               "[ctune_Plugin_setPlugin( '%s', \"%s\" )] Plugin set as selected: %p",
                               ctune_PluginType.str( type ), name, p
                    );

                    private.audio_file_writers.selected = p;
                    return true;
                }
            }
        } break;

        default: break;
    }

    CTUNE_LOG( CTUNE_LOG_ERROR,
               "[ctune_Plugin_setPlugin( '%s', \"%s\" )] Failed to set selected.",
               ctune_PluginType.str( type ), name
    );

    return false;
}

/**
 * Gets all the loaded plugins of a specified type
 * @param type Plugin type enum
 * @return Pointer to internal plugin list of specified type (or NULL on error)
 */
static const Vector_t * ctune_Plugin_getPluginList( ctune_PluginType_e type ) {
    switch( type ) {
        case CTUNE_PLUGIN_IN_STREAM_PLAYER: return &private.audio_players.list;
        case CTUNE_PLUGIN_OUT_AUDIO_SERVER: return &private.audio_servers.list;
        case CTUNE_PLUGIN_OUT_AUDIO_FILE  : return &private.audio_file_writers.list;
        default                           : return NULL;
    }
}

/**
 * Gets the currently selected plugin of a specified type
 * @param type Plugin type enum
 * @return Pointer to the selected plugin or NULL
 */
static void * ctune_Plugin_getSelectedPlugin( ctune_PluginType_e type ) {
    switch( type ) {
        case CTUNE_PLUGIN_IN_STREAM_PLAYER: return private.audio_players.selected;
        case CTUNE_PLUGIN_OUT_AUDIO_SERVER: return private.audio_servers.selected;
        case CTUNE_PLUGIN_OUT_AUDIO_FILE  : return private.audio_file_writers.selected;
        default                           : return NULL;
    }
}

/**
 * Closes the plugin handles and frees nested resources
 */
static void ctune_Plugin_free( void ) {
    Vector.clear_vector( &private.audio_players.list );
    Vector.clear_vector( &private.audio_file_writers.list );
    Vector.clear_vector( &private.audio_servers.list );
    private.audio_players.selected      = NULL;
    private.audio_file_writers.selected = NULL;
    private.audio_servers.selected      = NULL;

    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_Plugin_free()] Plugin(s) freed." );
}

/**
 * Namespace constructor
 */
const struct ctune_Plugin_Namespace ctune_Plugin = {
    .init              = &ctune_Plugin_init,
    .loadPlugins       = &ctune_Plugin_loadPlugins,
    .setPlugin         = &ctune_Plugin_setPlugin,
    .getPluginList     = &ctune_Plugin_getPluginList,
    .getSelectedPlugin = &ctune_Plugin_getSelectedPlugin,
    .free              = &ctune_Plugin_free,
};