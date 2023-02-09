#ifndef CTUNE_FS_PLUGIN_H
#define CTUNE_FS_PLUGIN_H

#include <stdbool.h>

#include "../audio/AudioOut.h"
#include "../audio/FileOut.h"
#include "../player/Player.h"


#include "../datastructure/Vector.h"

extern const struct ctune_Plugin_Namespace {
    /**
     * Initialises plugin engine
     */
    void (* init)( void );

    /**
     * Loads all compatible plugins found in a directory
     * @param dir_path Directory path
     * @return Success
     */
    bool (* loadPlugins)( const char * dir_path );

    /**
     * Sets a plugin as 'selected'
     * @param type Plugin type enum
     * @param name Name of plugin to set
     * @return Success
     */
    bool (* setPlugin)( ctune_PluginType_e type, const char * name );

    /**
     * Gets all the loaded plugins of a specified type
     * @param type Plugin type enum
     * @return Pointer to internal plugin list of specified type
     */
    const Vector_t * (* getPluginList)( ctune_PluginType_e type );

    /**
     * Gets the currently selected plugin of a specified type
     * @param type Plugin type enum
     * @return Pointer to the selected plugin or NULL
     */
    void * (* getSelectedPlugin)( ctune_PluginType_e type );

    /**
     * Closes the plugin handles and frees nested resources (calls un-loaders)
     */
    void (* free)( void );

} ctune_Plugin;

#endif //CTUNE_FS_PLUGIN_H
