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

    struct ctune_ColourTheme theme;

} ctune_UIConfig_t;

extern const struct ctune_UIConfig_Namespace {
    /**
     * Copy a ctune_UIConfig object into another
     * @param from Origin
     * @param to   Destination
     * @return Success
     */
    bool (* copy)( const ctune_UIConfig_t * from, ctune_UIConfig_t * to );

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
