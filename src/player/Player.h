#ifndef CTUNE_PLAYER_PLAYER_H
#define CTUNE_PLAYER_PLAYER_H

#include "logger/src/Logger.h"
#include "../enum/PluginType.h"
#include "../enum/PlaybackCtrl.h"
#include "../enum/SearchCtrl.h"
#include "../network/RadioBrowser.h"
#include "../datastructure/String.h"
#include "../audio/AudioOut.h"

#define CTUNE_PLAYER_ABI_VERSION 2

#define CTUNE_PLAYER_STATE_PLAYING 1
#define CTUNE_PLAYER_STATE_STOPPED 0

#define CTUNE_MAX_FRAME_SIZE 192000 //default fallback for output frame buffer

typedef struct ctune_Player_Interface {
    /**
     * Player plugin file handle
     */
    void * handle;

    /**
     * Pointer to output plugin ABI version number
     */
    const unsigned int * abi_version;

    /**
     * Pointer to the plugin's type
     */
    const ctune_PluginType_e * plugin_type;

    /**
     * Gets the plugin's name
     * @return Plugin name string
     */
    const char * (* name)( void );

    /**
     * Gets the plugin's description
     * @return Plugin description string
     */
    const char * (* description)( void );

    /**
     * Initialises the RadioPlayer functionalities
     * @param sound_server                   Pointer to the sound server plugin to use as output
     * @param playback_ctrl_callback         Function to check/set the playback state global flag
     * @param song_change_callback           Function to call when stream metadata changes (sends the current stream title)
     */
    void (* init)( ctune_AudioOut_t * sound_server,
                   bool(* playback_ctrl_callback)( enum CTUNE_PLAYBACK_CTRL ),
                   void(* song_change_callback)( const char * str )
    );

    /**
     * Connects and plays a Radio station's stream
     * @param url         Radio station stream URL
     * @param volume      Initial playing volume
     * @param timeout_val Timeout value in seconds
     * @return Success (if false the error_no in the RadioPlayer_t instance will be set accordingly)
     */
    bool (* playRadioStream)( const char * url, const int volume, int timeout_val );

    /**
     * Gets the error number set in RadioPlayer
     * @return ctune_errno
     */
    int  (* getError)( void );

    /**
     * Test stream and get its property (independent from the player. i.e.: no side effects on player)
     * @param url         Stream URL
     * @param timeout_val Timeout value in seconds
     * @param codec_str   Pointer to the codec string
     * @param bitrate     Pointer to the bitrate container
     * @return Stream OK
     */
    bool (* testStream)( const char * url, int timeout_val, String_t * codec_str, ulong * bitrate );

} ctune_Player_t;

#endif //CTUNE_PLAYER_PLAYER_H
