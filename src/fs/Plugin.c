#include "Plugin.h"

#include <dlfcn.h>

#include "../logger/Logger.h"

static struct {
    bool                audio_server_initialised;
    ctune_AudioOut_t    audio_server;
    bool                player_initialised;
    ctune_Player_t      player;

} private = {
    .player_initialised       = false,
    .audio_server_initialised = false,

    .audio_server = {
        .handle       = NULL,
        .name         = { NULL, 0 },
        .abi_version  = NULL,
        .init         = NULL,
        .getVolume    = NULL,
        .setVolume    = NULL,
        .changeVolume = NULL,
        .write        = NULL,
        .shutdown     = NULL,
    },

    .player = {
        .handle          = NULL,
        .name            = { NULL, 0 },
        .abi_version     = NULL,
        .init            = NULL,
        .playRadioStream = NULL,
        .getError        = NULL,
        .testStream      = NULL,
    },
};

/**
 * [PRIVATE] Closes the plugin handles and frees nested resources
 * @param ao Pointer to AudioOut_t object
 */
static void ctune_Plugin_freeAudioOut( ctune_AudioOut_t * ao ) {
    if( ao != NULL && ao->handle != NULL ) {
        ao->shutdown();

        if( dlclose( ao->handle ) != 0 ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_Plugin_freePlayer( %p )] Failed to close the handle of plugin '%s': %s",
                       ao, ao->name._raw, dlerror()
            );

            ctune_err.set( CTUNE_ERR_IO_PLUGIN_CLOSE );
        }

        String.free( &ao->name );
        ao->abi_version  = NULL;
        ao->init         = NULL;
        ao->getVolume    = NULL;
        ao->setVolume    = NULL;
        ao->changeVolume = NULL;
        ao->write        = NULL;
        ao->shutdown     = NULL;

        private.player_initialised = false;
    }
}

/**
 * [PRIVATE] Closes the plugin handles and frees nested resources
 * @param player Pointer to RadioPlayer_t object
 */
static void ctune_Plugin_freePlayer( ctune_Player_t * player ) {
    if( player != NULL && player->handle != NULL ) {
        if( dlclose( player->handle ) != 0 ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_Plugin_freePlayer( %p )] Failed to close the handle of plugin '%s': %s",
                       player, player->name._raw, dlerror()
            );

            ctune_err.set( CTUNE_ERR_IO_PLUGIN_CLOSE );
        }

        String.free( &player->name );
        player->abi_version     = NULL;
        player->init            = NULL;
        player->playRadioStream = NULL;
        player->getError        = NULL;
        player->testStream      = NULL;

        private.audio_server_initialised = false;
    }
}

/**
 * Loads and links and audio output plugin
 * @param dir_path Directory path of the output plugin(s)
 * @param name     Name of plugin
 * @return Pointer to internal plugin (NULL on error)
 */
static ctune_AudioOut_t * ctune_Plugin_loadAudioOut( const char * dir_path, const char * name ) {
    if( dir_path == NULL || name == NULL ) {
        CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_Plugin_loadAudioOut( %p, %p )] NULL arg(s).", dir_path, name );
        return NULL; //EARLY RETURN
    }

    bool     error_state = false;
    char *   error       = NULL;
    String_t path        = String.init();

    String.append_back( &path, dir_path );
    String.append_back( &path, name );
    String.append_back( &path, ".so" );

    if( ( private.audio_server.handle = dlopen( path._raw, RTLD_NOW ) ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_Plugin_loadAudioOut( \"%s\", \"%s\"  )] Failed to open plugin '%s' (path: '%s'): %s",
                   dir_path, name, name, path._raw, dlerror()
        );

        ctune_err.set( CTUNE_ERR_IO_PLUGIN_OPEN );
        error_state = true;
        goto end;
    }

    String.set( &private.audio_server.name, name );

    //ABI check
    private.audio_server.abi_version = dlsym( private.audio_server.handle, "abi_version" );

    if( ( error = dlerror() ) != NULL ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_Plugin_loadAudioOut( \"%s\", \"%s\" )] Failed to get ABI version for plugin '%s': %s",
                   dir_path, name, name, error
        );

        ctune_err.set ( CTUNE_ERR_IO_PLUGIN_LINK );
        error_state = true;
        goto end;
    }

    if( private.audio_server.abi_version == NULL || *private.audio_server.abi_version != CTUNE_AUDIOOUT_ABI_VERSION ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_Plugin_loadAudioOut( \"%s\", \"%s\" )] "
                   "ABI version for plugin '%s' mismatch (plugin: v%d, required: v%d).",
                   dir_path, name, name,
                   ( private.audio_server.abi_version != NULL ? *private.audio_server.abi_version : 0 ), CTUNE_AUDIOOUT_ABI_VERSION
        );

        ctune_err.set ( CTUNE_ERR_IO_PLUGIN_ABI );
        error_state = true;
        goto end;
    }

    //linkage
    struct ctune_AudioOut * ao = dlsym( private.audio_server.handle, "ctune_AudioOutput" );

    if( ( error = dlerror() ) != NULL ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_Plugin_loadAudioOut( \"%s\", \"%s\" )] Failed to link plugin '%s': %s",
                   dir_path, name, name, error
        );

        ctune_err.set ( CTUNE_ERR_IO_PLUGIN_LINK );
        error_state = true;
        goto end;
    }

    private.audio_server.init         = ao->init;
    private.audio_server.getVolume    = ao->getVolume;
    private.audio_server.setVolume    = ao->setVolume;
    private.audio_server.changeVolume = ao->changeVolume;
    private.audio_server.write        = ao->write;
    private.audio_server.shutdown     = ao->shutdown;

    CTUNE_LOG( CTUNE_LOG_MSG,
               "[ctune_Plugin_loadAudioOut( \"%s\", \"%s\" )] Plugin loaded: %s",
               dir_path, name, private.audio_server.name._raw
    );

    end:
        if( error_state )
            ctune_Plugin_freeAudioOut( &private.audio_server );

        String.free( &path );
        return ( error_state ? NULL : &private.audio_server );

}

/**
 * Loads and links a player plugin
 * @param dir_path Directory path of the player plugin(s)
 * @param name     Name of plugin
 * return Pointer to internal plugin (NULL on error)
 */
static ctune_Player_t * ctune_Plugin_loadPlayer( const char * dir_path, const char * name ) {
    if( dir_path == NULL || name == NULL ) {
        CTUNE_LOG( CTUNE_LOG_FATAL, "[ctune_Plugin_loadPlayer( %p, %p )] NULL arg(s).", dir_path, name );
        return NULL; //EARLY RETURN
    }

    bool     error_state = false;
    char *   error       = NULL;
    String_t path        = String.init();

    String.append_back( &path, dir_path );
    String.append_back( &path, name );
    String.append_back( &path, ".so" );

    if( ( private.player.handle = dlopen( path._raw, RTLD_NOW ) ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_Plugin_loadPlayer( \"%s\", \"%s\" )] Failed to open plugin '%s' (path: '%s'): %s",
                   dir_path, name, name, path._raw, dlerror()
        );

        ctune_err.set( CTUNE_ERR_IO_PLUGIN_OPEN );
        error_state = true;
        goto end;
    }

    String.set( &private.player.name, name );

    //ABI check
    private.player.abi_version = dlsym( private.player.handle, "abi_version" );

    if( ( error = dlerror() ) != NULL ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_Plugin_loadPlayer( \"%s\", \"%s\" )] Failed to get ABI version for plugin '%s': %s",
                   dir_path, name, name, error
        );

        ctune_err.set ( CTUNE_ERR_IO_PLUGIN_LINK );
        error_state = true;
        goto end;
    }

    if( private.player.abi_version == NULL || *private.player.abi_version != CTUNE_PLAYER_ABI_VERSION ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_Plugin_loadPlayer( \"%s\", \"%s\" )] "
                   "ABI version for plugin '%s' mismatch (plugin: v%d, required: v%d).",
                   dir_path, name, name,
                   ( private.player.abi_version != NULL ? *private.player.abi_version : 0 ), CTUNE_PLAYER_ABI_VERSION
        );

        ctune_err.set ( CTUNE_ERR_IO_PLUGIN_ABI );
        error_state = true;
        goto end;
    }

    //linkage
    struct ctune_Player_Interface * p = dlsym( private.player.handle, "ctune_Player" );

    if( ( error = dlerror() ) != NULL ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_Plugin_loadPlayer( \"%s\", \"%s\" )] Failed to link plugin '%s': %s",
                   dir_path, name, name, error
        );

        ctune_err.set ( CTUNE_ERR_IO_PLUGIN_LINK );
        error_state = true;
        goto end;
    }

    private.player.init            = p->init;
    private.player.playRadioStream = p->playRadioStream;
    private.player.getError        = p->getError;
    private.player.testStream      = p->testStream;

    CTUNE_LOG( CTUNE_LOG_MSG,
               "[ctune_Plugin_loadPlayer( \"%s\", \"%s\" )] Plugin loaded: %s",
               dir_path, name, private.player.name._raw
    );

    end:
        if( error_state )
            ctune_Plugin_freePlayer( &private.player );

        String.free( &path );
        return ( error_state ? NULL : &private.player );
}

/**
 * Closes the plugin handles and frees nested resources
 */
static void ctune_Plugin_free( void ) {
    if( private.player_initialised ) {
        ctune_Plugin_freeAudioOut( &private.audio_server );
        private.player_initialised = false;
    }

    if( private.audio_server_initialised ) {
        ctune_Plugin_freePlayer( &private.player );
        private.audio_server_initialised = false;
    }

    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_Plugin_free()] Plugin(s) freed." );
}

/**
 * Namespace constructor
 */
const struct ctune_Plugin_Namespace ctune_Plugin = {
    .loadAudioOut = &ctune_Plugin_loadAudioOut,
    .loadPlayer   = &ctune_Plugin_loadPlayer,
    .free         = &ctune_Plugin_free,
};