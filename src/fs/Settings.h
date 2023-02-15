#ifndef CTUNE_FS_SETTINGS_H
#define CTUNE_FS_SETTINGS_H

#include <stdbool.h>

#include "../enum/StationSrc.h"
#include "../datastructure/String.h"
#include "../datastructure/Vector.h"
#include "../dto/RadioStationInfo.h"
#include "../dto/UIConfig.h"
#include "../audio/AudioOut.h"
#include "../player/Player.h"
#include "../audio/FileOut.h"

extern const struct ctune_Settings_Instance {
    /**
     * Initialises all the variables for the Settings instance
     */
    void (* init)( void );

    /**
     * De-allocates anything stored on the heap
     */
    void (* free)( void );

    struct { /* Runtime lock file*/
        /**
         * Creates the lock file
         * @return Success
         */
        bool (* lock)( void );

        /**
         * Deletes the lock file
         * @return Success
         */
        bool (* unlock)( void );

    } rtlock;

    struct { /* Favourite stations */
        /**
         * Loads the setting file
         * @return Success
         */
        bool (* loadFavourites)( void );

        /**
         * Exports a collection of favourite stations to the file
         * @return Success
         */
        bool (* saveFavourites)( void );

        /**
         * Syncs a collection used for display the internal map state
         * @param stations View container
         * @return Success
         */
        bool (* refreshView)( Vector_t * stations );

        /**
         * Sets the sorting attribute for the display list
         * @param attr ctune_RadioStationInfo_SortBy_e ID
         */
        void (* setSortingAttribute)( ctune_RadioStationInfo_SortBy_e attr );

        /**
         * Checks if a station is in the 'favourites' list
         * @param uuid UUID string
         * @param src  Radio station provenance
         * @return Favourite state
         */
        bool (* isFavourite)( const char * uuid, ctune_StationSrc_e src );

        /**
         * Gets the pointer to a favourite RSI inside the HashMap collection
         * @param uuid UUID of the RSI
         * @param src  Radio station provenance
         * @return Pointer to radio station object
         */
        const ctune_RadioStationInfo_t * (* getFavourite)( const char * uuid, ctune_StationSrc_e src );

        /**
         * Adds a new station to the 'favourites' list
         * @param rsi Pointer to a RadioStationInfo_t DTO
         * @param src  Radio station provenance
         * @return Success
         */
        bool (* addStation)( const ctune_RadioStationInfo_t * rsi, ctune_StationSrc_e src );

        /**
         * Removes a station from the 'favourites' list
         * @param rsi Pointer to a RadioStationInfo_t DTO
         * @param src  Radio station provenance
         * @return Success
         */
        bool (* removeStation)( const ctune_RadioStationInfo_t * rsi, ctune_StationSrc_e src );

    } favs;

    struct { /* Application configuration */
        /**
         * Loads the setting file
         * @return Success
         */
        bool (* loadCfg)( void );

        /**
         * Checks if the settings have been loaded
         */
        bool (* isLoaded)( void );

        /**
         * Writes the current settings to the cfg file
         * @return Success
         */
        bool (* writeCfg)( void );

        /**
         * Gets the saved volume
         * @return Volume (0-100)
         */
        int (* getVolume)( void );

        /**
         * Set the volume to save in configuration
         * @param vol Volume (0-100)
         * @return volume post-change
         */
        int (* setVolume)( int );

        /**
         * Modifies the stored configuration volume
         * @param delta Delta value to modify by
         * @return Volume post-modification
         */
        int (* modVolume)( int );

        /**
         * Gets the last station played' UUID
         * @return UUID or NULL if none was saved
         */
        const char * (* getLastPlayedUUID)( void );

        /**
         * Gets hte last station played' source
         * @return ctune_StationSrc_e enum value
         */
        ctune_StationSrc_e (* getLastPlayedSrc)( void );

        /**
         * Sets the UUID of the last station played to be saved in the configuration
         * @param uuid Radio station UUID to save
         * @param src  Source for Station UUID
         */
        void (* setLastPlayedStation)( const char *, ctune_StationSrc_e );

        /**
         * Gets the playback log file overwrite preference
         * @return Overwrite flag
         */
        bool (* playbackLogOverwrite)( void );

        /**
         * Gets the timeout value in seconds for connecting to and playing a stream
         * @return Timeout value in seconds
         */
        int (* getStreamTimeoutVal)( void );

        /**
         * Sets the timeout value in seconds for connecting to and playing a stream
         * @param val Timeout value in seconds (1-10 inclusive)
         * @return Success
         */
        bool (* setStreamTimeoutVal)( int val );

        /**
         * Gets the timout value in seconds for querying a network service
         * @return Timeout value in seconds
         */
        int (* getNetworkTimeoutVal)( void );

        /**
         * Get the output path
         * @return Output path
         */
        const char * (* outputPath)( void );

        /**
         * Sets the output path
         * @param path Output path
         */
        bool (* setOutputPath)( const char * path );

        /**
         * Gets the UI configuration
         * @return ctune_UIConfig object
         */
        ctune_UIConfig_t (* getUIConfig)( void );

        /**
         * Sets the UI configuration
         * @param cfg UI config to copy over
         * @return Success
         */
        bool (* setUIConfig)( const ctune_UIConfig_t * cfg );

    } cfg;

    struct { /* Plugins */
        /**
         * Call to load all available plugins into the engine
         * @return Success
         */
        bool (* loadPlugins)( void );

        /**
         * Gets the currently selected plugin of a given type or the default if it has not been selected yet
         * @param type Plugin type enum
         * @return Pointer to plugin interface of given type or NULL
         */
        void * (* getPlugin)( ctune_PluginType_e type );

        /**
         * Sets a plugin as 'selected'
         * @param type Plugin type enum
         * @param id   Plugin ID
         * @return Success
         */
        bool (* setPlugin)( ctune_PluginType_e type, size_t id );

        /**
         * Gets a list of all the loaded plugins of a specified type
         * @param type Plugin type enum
         * @return Pointer to a heap allocated list of ids, names, descriptions and 'selected' flags
         */
        const Vector_t * (* getPluginList)( ctune_PluginType_e type );

    } plugins;

} ctune_Settings;

#endif //CTUNE_FS_SETTINGS_H