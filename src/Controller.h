#ifndef CTUNE_UI_CONTROLLER_H
#define CTUNE_UI_CONTROLLER_H

#include <stdbool.h>

#include "datastructure/Vector.h"
#include "dto/ArgOptions.h"
#include "dto/RadioBrowserFilter.h"
#include "dto/RadioStationInfo.h"
#include "dto/UIConfig.h"
#include "enum/StationSrc.h"
#include "enum/PlaybackCtrl.h"
#include "enum/SearchCtrl.h"
#include "enum/ByCategory.h"
#include "enum/ListCategory.h"

extern const struct ctune_Controller_Instance {
    /**
     * Initiate cTune
     * @return Success
     */
    bool (* init)( void );

    /**
     * Loads controller level options and act on these
     * @param opts Options
     */
    void (* load)( ctune_ArgOptions_t * opts );

    /**
     * Call the resize callback method if set
     */
    void (* resizeUI)( void );

    /**
     * Shutdown and cleanup cTune
     */
    void (* free)( void );

    /**
     * Playback controls
     */
    struct {
        /**
         * [THREAD SAFE] Gets the playback state variable's value
         * @return Playback state value
         */
        bool (* getPlaybackState)( void );

        /**
         * [THREAD SAFE] Starts playback of a radio station
         * @param station Pointer to a RadioStationInfo DTO
         * @return Success
         */
        bool (* start)( const ctune_RadioStationInfo_t * station );

        /**
         * [THREAD SAFE] Stops the playback of the currently playing stream
         */
        void (* stop)( void );

        /**
         * [THREAD SAFE] Changes playback volume by a specified amount
         * @param delta Volume change (+/-)
         */
        void (* modifyVolume)( int delta );

        /**
         * Test a stream and get properties
         * @param url     Stream URL
         * @param codec   Pointer to store codec string value into
         * @param bitrate Pointer to store bitrate value into
         * @return Success
         */
        bool (* testStream)( const char * url, String_t * codec, ulong * bitrate );

        /**
         * Tests the validity of a URL string
         * @param url URL string
         * @return Valid state
         */
        bool (* validateURL)( const char * url );

    } playback;

    /**
     * Information download control
     */
    struct {
        /**
         * Search for all stations matching the criteria in filter
         * @param filter   Filter
         * @param stations RadioStationInfo container
         * @return Success
         */
        bool (* getStations)( ctune_RadioBrowserFilter_t * filter, Vector_t * stations );

        /**
         * Search for all stations matching the criteria
         * @param category    Category
         * @param search_term Search term (optional)
         * @param stations    Container for stations
         * @return Success
         */
        bool (* getStationsBy)( const ctune_ByCategory_e category, const char * search_term, Vector_t * stations );

        /**
         * Download items within a specified category
         * @param category       Category for which to download items from
         * @param filter         Search filter
         * @param category_items Data-structure to store the category items (`ctune_CategoryItem_t`) into
         */
        bool (* getCategoryItems)( const ctune_ListCategory_e category, const ctune_RadioBrowserFilter_t * filter, Vector_t * categories );

    } search;

    /**
     * Application setting transparent access methods
     */
    struct {
        /**
         * Add/Remove a station to the list of favourites
         * @param rsi RadioStationInfo_t object
         * @param src Radio station origin
         * @return Success
         */
        bool (* toggleFavourite)( ctune_RadioStationInfo_t * rsi, ctune_StationSrc_e src );

        /**
         * Updates a favourite station's information
         * @param rsi RadioStationInfo_t object
         * @param src Radio station origin
         * @return Success
         */
        bool (* updateFavourite)( ctune_RadioStationInfo_t * rsi, ctune_StationSrc_e src );

        /**
         * Check if a station is in the list of favourites
         * @param rsi RadioStationInfo_t object
         * @param src  Radio station origin
         * @return Favourite state
         */
        bool (* isFavourite)( const ctune_RadioStationInfo_t * rsi, ctune_StationSrc_e src );

        /**
         * Checks if a UUID is used by the favourite stations
         * @param uuid UUID string
         * @param src  Radio station UUID origin
         * @return In-use state
         */
        bool (* isFavouriteUUID)( const char * uuid, ctune_StationSrc_e src );

        /**
         * Saves the favourite stations to file
         * @return Success
         */
        bool (* saveFavourites)( void );

        /**
         * Update the collection of stations so that it contains only the current favourites
         * @param stations Vector of stations
         */
        bool (* getListOfFavourites)( Vector_t * stations );

        /**
         * Sets the sorting attribute for the list of favourite stations
         * @param attr ctune_RadioStationInfo_SortBy_e ID
         */
        void (* setFavouriteSorting)( ctune_RadioStationInfo_SortBy_e attr );

        /**
         * Gets the loaded UI theme
         * @return Colour theme
         */
        struct ctune_ColourTheme * (* getUiTheme)( void );

        /**
         * Gets the theming requirements for the stations inside the 'Favourites' tab
         * @returns Flag state to use theming
         */
        bool (* favTabThemingState)( void );

        /**
         * Gets the large row usage requirements for the Favourites tab
         * @return Flag state to use large row
         */
        bool (* largeRowsForFavTab)( void );

        /**
         * Gets the large row usage requirements for the Search Results tab
         * @return Flag state to use large row
         */
        bool (* largeRowsForSearchTab)( void );

        /**
         * Gets the large row usage requirements for the Browser tab
         * @return Flag state to use large row
         */
        bool (* largeRowsForBrowserTab)( void );

        /**
         * Gets a pointer to the internal UIConfig object
         * @return Pointer to ctune_UIConfig_t object
         */
        ctune_UIConfig_t * (* getUIConfig)( void );

        /**
         * Saves the internal UIConfig_t object to the Settings component
         */
        void (* saveUIConfig)( void );

    } cfg;

    /**
     * Sets a callback for resize events
     * @param cb Callback method
     */
    void (* setResizeUIEventCallback)( void(* cb)( void ) );

    /**
     * [OPTIONAL] Sets a callback for the volume change event
     * @param cb Callback method
     */
    void (* setVolumeChangeEventCallback)( void(* cb)( int ) );

    /**
     * [OPTIONAL] Sets a callback for the song change event
     * @param cb Callback method
     */
    void (* setSongChangeEventCallback)( void(* cb)( const char *) );

    /**
     * [OPTIONAL] Sets a callback for the radio station change event
     * @param cb Callback method
     */
    void (* setStationChangeEventCallback)( void(* cb)( const ctune_RadioStationInfo_t * rsi ) );

    /**
     * [OPTIONAL] Assigns a function as the playback state change callback
     * @param cb Callback function pointer
     */
    void (* setPlaybackStateChangeEventCallback)( void(* cb)( bool ) );

    /**
     * [OPTIONAL] Assigns a function as the search state change callback
     * @param cb Callback function pointer
     */
    void (* setSearchStateChangeEventCallback)( void(* cb)( bool ) );

} ctune_Controller;

#endif //CTUNE_UI_CONTROLLER_H
