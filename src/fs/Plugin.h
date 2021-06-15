#ifndef CTUNE_FS_PLUGIN_H
#define CTUNE_FS_PLUGIN_H

#include <stdbool.h>

#include "../audio/AudioOut.h"
#include "../player/Player.h"


#include "../datastructure/Vector.h"


extern const struct ctune_Plugin_Namespace {
    /**
     * Loads and links an audio output plugin
     * @param dir_path Directory path of the output plugin(s)
     * @param name     Name of plugin
     * @return Pointer to internal plugin (NULL on error)
     */
    ctune_AudioOut_t * (* loadAudioOut)( const char * dir_path, const char * name );

    /**
     * Loads and links a player plugin
     * @param dir_path Directory path of the player plugin(s)
     * @param name     Name of plugin
     * return Pointer to internal plugin (NULL on error)
     */
    ctune_Player_t * (* loadPlayer)( const char * dir_path, const char * name );

    /**
     * Closes the plugin handles and frees nested resources (calls un-loaders)
     */
    void (* free)( void );

} ctune_Plugin;

#endif //CTUNE_FS_PLUGIN_H
