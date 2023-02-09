#include "PluginType.h"

/**
 * Gets the string representation of the enum
 * @param type PluginType enum
 * @return String representation
 */
static const char * ctune_PluginType_str( ctune_PluginType_e type ) {
    switch( type ) {
        case CTUNE_PLUGIN_IN_STREAM_PLAYER: return "stream player";
        case CTUNE_PLUGIN_OUT_AUDIO_SERVER: return "audio server";
        case CTUNE_PLUGIN_OUT_AUDIO_FILE  : return "audio file writer";
        default                           : return "unknown";
    }
}

/**
 * Namespace declaration
 */
const struct ctune_PluginType_Namespace ctune_PluginType = {
    .str = &ctune_PluginType_str,
};