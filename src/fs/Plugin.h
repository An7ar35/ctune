#ifndef CTUNE_FS_PLUGIN_H
#define CTUNE_FS_PLUGIN_H

#include <stdbool.h>

#include "../audio/AudioOut.h"
#include "../audio/FileOut.h"
#include "../player/Player.h"

#include "../dto/PluginInfo.h"
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
     * Validates a named plugin against the loaded plugins
     * @param type Plugin type enum
     * @param name Plugin name
     * @return Validation state
     */
    bool (* validate)( ctune_PluginType_e type, const char * name );

    /**
     * Sets a plugin as 'selected'
     * @param type Plugin type enum
     * @param name Name of plugin to set
     * @return Success
     */
    bool (* setPluginByName)( ctune_PluginType_e type, const char * name );

    /**
     * Sets a plugin as 'selected'
     * @param type Plugin type enum
     * @param id   ID of the plugin inside the list of the type
     * @return Success
     */
    bool (* setPluginByID)( ctune_PluginType_e type, size_t id );

    /**
     * Gets the information of all plugins for a given type
     * @param type Plugin enum type
     * @return Allocated list (or NULL)
     */
    Vector_t * (* getPluginInfoList)( ctune_PluginType_e type );

    /**
     * Gets the currently selected plugin of a specified type
     * @param type Plugin type enum
     * @return Pointer to the selected plugin or NULL
     */
    void * (* getSelectedPlugin)( ctune_PluginType_e type );

    /**
     * Gets the currently selected plugin name of a specified type
     * @param type Plugin type enum
     * @return Name string or NULL
     */
    const char * (* getSelectedPluginName)( ctune_PluginType_e type );

    /**
     * Closes the plugin handles and frees nested resources (calls un-loaders)
     */
    void (* free)( void );

} ctune_Plugin;

#endif //CTUNE_FS_PLUGIN_H
