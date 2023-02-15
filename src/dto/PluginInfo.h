#ifndef CTUNE_DTO_PLUGININFO_H
#define CTUNE_DTO_PLUGININFO_H

#include <stdbool.h>

#include "../enum/PluginType.h"

/**
 * Container for a Plugin's information
 * @param id          Position in the internal list for the type
 * @param type        Plugin type
 * @param name        Name of the plugin
 * @param description Description string
 * @param extension   File extension
 * @param selected    Flag
 */
typedef struct {
    size_t             id;
    ctune_PluginType_e type;
    const char *       name;
    const char *       description;
    const char *       extension;
    bool               selected;
} ctune_PluginInfo_t;

#endif //CTUNE_DTO_PLUGININFO_H