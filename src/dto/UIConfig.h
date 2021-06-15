#ifndef CTUNE_DTO_UICFG_H
#define CTUNE_DTO_UICFG_H

#include <stdbool.h>

#include "ColourTheme.h"

typedef struct {
    struct {
        bool hide_fav_theming;
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
    bool (* copy)( const ctune_UIConfig_t * from, ctune_UIConfig_t * to );

} ctune_UIConfig;



#endif //CTUNE_DTO_UICFG_H
