#ifndef CTUNE_ENUM_PLUGINTYPE_H
#define CTUNE_ENUM_PLUGINTYPE_H

#define CTUNE_INPUT_PLUGIN  0b00100000
#define CTUNE_OUTPUT_PLUGIN 0b00010000

typedef enum {
    CTUNE_PLUGIN_IN_STREAM_PLAYER = CTUNE_INPUT_PLUGIN  | 0b0,

    CTUNE_PLUGIN_OUT_AUDIO_SERVER = CTUNE_OUTPUT_PLUGIN | 0b0,
    CTUNE_PLUGIN_OUT_AUDIO_FILE   = CTUNE_OUTPUT_PLUGIN | 0b1,

} ctune_PluginType_e;


/**
 * PluginType namespace
 */
extern const struct ctune_PluginType_Namespace {
    /**
     * Gets the string representation of the enum
     * @param type PluginType enum
     * @return String representation
     */
    const char * (* str)( ctune_PluginType_e type );

} ctune_PluginType;

#endif //CTUNE_ENUM_PLUGINTYPE_H