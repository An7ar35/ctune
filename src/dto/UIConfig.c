#include "UIConfig.h"

#include <stdlib.h>

/**
 * Copies the content of a UIConfig object into another
 * @param from Origin of content to copy from
 * @param to   Destination to copy to
 * @return Success
 */
static bool ctune_UIConfig_copy( const ctune_UIConfig_t * from, ctune_UIConfig_t * to ) {
    if( from == NULL || to == NULL )
        return false;

    to->fav_tab    = from->fav_tab;
    to->search_tab = from->search_tab;
    to->browse_tab = from->browse_tab;
    to->theme      = from->theme;

    return true;
}

/**
 * Namespace constructor
 */
const struct ctune_UIConfig_Namespace ctune_UIConfig = {
    .copy = &ctune_UIConfig_copy,
};