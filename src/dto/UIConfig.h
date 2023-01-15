#ifndef CTUNE_DTO_UICFG_H
#define CTUNE_DTO_UICFG_H

#include <stdbool.h>

#include "ColourTheme.h"
#include "../enum/Flag.h"

typedef struct {
    struct {
        bool theme_favourites;
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

    struct {
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
