#ifndef CTUNE_DTO_UICFG_H
#define CTUNE_DTO_UICFG_H

#include <stdbool.h>

#include "ColourTheme.h"
#include "../enum/Flag.h"
#include "../enum/MouseInterval.h"
#include "../enum/StationSrc.h"

typedef struct {
    bool unicode_icons;

    struct {
        bool                  enabled;
        ctune_MouseInterval_e interval_preset;
    } mouse;

    struct {
        bool theme_favourites;
        bool custom_theming;
        bool large_rows;
    } fav_tab;

    struct {
        bool large_rows;
    } search_tab;

    struct {
        bool large_rows;
    } browse_tab;

    struct {
        ctune_UIPreset_e         preset;
        struct ctune_ColourTheme preset_pallet;
        struct ctune_ColourTheme custom_pallet;
    } theme;

} ctune_UIConfig_t;


extern const struct ctune_UIConfig_Namespace {
    /**
     * Initialise fields in the struct with defaults
     * @return Initialised ctune_UIConfig_t object
     */
    ctune_UIConfig_t (* create)( void );

    /**
     * Copy a ctune_UIConfig object into another overwriting everything
     * @param from Origin
     * @param to   Destination
     * @return Success
     */
    bool (* copy)( const ctune_UIConfig_t * from, ctune_UIConfig_t * to );

    /**
     * Gets the unicode icon flag property
     * @param cfg  Pointer to ctune_UIConfig_t object
     * @param flag Flag action
     * @return Flag state
     */
    bool (* unicodeIcons)( ctune_UIConfig_t * cfg, ctune_Flag_e flag );

    struct {
        /**
         * Set/Gets the mouse flag
         * @param cfg  Pointer to ctune_UIConfig_t object
         * @param flag Flag action
         * @return Property value after operation
         */
        bool (* enabled)( ctune_UIConfig_t * cfg, ctune_Flag_e flag );

        /**
         * Gets the mouse resolution value
         * @param cfg Pointer to ctune_UIConfig_t object
         * @param Mouse Interval value in milliseconds between press and release to be recognised as a click
         */
        int (* clickIntervalResolution)( ctune_UIConfig_t * cfg );

        /**
         * Gets the associated enum of the resolution value stored
         * @param cfg Pointer to ctune_UIConfig_t object
         * @return MouseResolution enum
         */
        ctune_MouseInterval_e (* clickIntervalPreset)( ctune_UIConfig_t * cfg );

        /**
         * Sets the mouse resolution preset
         * @param cfg Pointer to ctune_UIConfig_t object
         * @param res MouseResolution preset
         * @param Success
         */
        bool (* setResolutionPreset)( ctune_UIConfig_t * cfg, ctune_MouseInterval_e res );
    } mouse;

    struct {
        /**
         * Gets the current preset
         * @param cfg Pointer to ctune_UIConfig_t object
         * @return Theme preset
         */
        ctune_UIPreset_e (* currentPreset)( ctune_UIConfig_t * cfg );

        /**
         * Gets currently active theme pallet
         * @param cfg Pointer to ctune_UIConfig_t object
         * @return Pointer to active theme pallet or NULL if the pointer to the config is NULL
         */
        struct ctune_ColourTheme * (* getCurrentThemePallet)( ctune_UIConfig_t * cfg );

        /**
         * Sets the current active theme pallet
         * @param cfg Pointer to ctune_UIConfig_t object
         * @param preset ctune_UIPreset_e preset
         * @return Success
         */
        bool (* setPreset)( ctune_UIConfig_t * cfg, ctune_UIPreset_e preset );
    } theming;

    struct {
        /**
         * Get/Set "Favourites" tab theming
         * @param cfg Pointer to ctune_UIConfig_t object
         * @param flag Flag action
         * @return Property value after operation
         */
        bool (* theming)( ctune_UIConfig_t * cfg, ctune_Flag_e flag );

        /**
         * Get/Set "Favourites" tab custom theming for the station sources
         * @param cfg Pointer to ctune_UIConfig_t object
         * @param flag Flag action
         * @return Property value after operation
         */
        bool (* customTheming)( ctune_UIConfig_t * cfg, ctune_Flag_e flag );

        /**
         * Get the custom colouring used for a station source
         * @param cfg         Pointer to ctune_UIConfig_t object
         * @param station_src ctune_StationSrc_e enum value
         * @return Colour value
         */
        short (* getCustomThemingColour)( ctune_UIConfig_t * cfg, ctune_StationSrc_e station_src );

        /**
         * Set the custom colouring used for a station source
         * @param cfg         Pointer to ctune_UIConfig_t object
         * @param station_src ctune_StationSrc_e enum value
         * @param colour      Colour code
         */
        void (* setCustomThemingColour)( ctune_UIConfig_t * cfg, ctune_StationSrc_e station_src, short colour_code );

        /**
         * Get/Set "Favourites" tab's large row property
         * @param cfg Pointer to ctune_UIConfig_t object
         * @param flag Flag action
         * @return Property value after operation
         */
        bool (* largeRowSize)( ctune_UIConfig_t * cfg, ctune_Flag_e flag );
    } fav_tab;

    struct {
        /**
         * Get/Set "Search" tab's large row property
         * @param cfg Pointer to ctune_UIConfig_t object
         * @param flag Flag action
         * @return Property value after operation
         */
        bool (* largeRowSize)( ctune_UIConfig_t * cfg, ctune_Flag_e flag );
    } search_tab;

    struct {
        /**
         * Get/Set "Browser" tab's large row property
         * @param cfg Pointer to ctune_UIConfig_t object
         * @param flag Flag action
         * @return Property value after operation
         */
        bool (* largeRowSize)( ctune_UIConfig_t * cfg, ctune_Flag_e flag );
    } browse_tab;

} ctune_UIConfig;



#endif //CTUNE_DTO_UICFG_H
