#ifndef CTUNE_PLAYER_RADIOPLAYER_H
#define CTUNE_PLAYER_RADIOPLAYER_H

#include "Player.h"
#include "../audio/AudioOut.h"
#include "../audio/OutputFormat.h"

extern const struct ctune_RadioPlayer_Namespace {
    /**
     * Initialises the main functionalities
     * @param song_change_callback           Function to call when stream metadata changes (sends the current stream title)
     * @param volume_change_event_callback   Function to call when a volume change request is successful (new volume is passed as arg)
     */
    void (* init)( void(* song_change_callback)( const char * str ),
                   void(* volume_change_event_callback)( const int vol )
    );

    /**
     * Sets a playback state change callback method
     * @param playback_state_change_cb Callback function
     */
    void (* setStateChangeCallback)( void (* playback_state_change_cb)( bool ) );

    /**
     * Loads a player plugin
     * @param player Pointer to Player plugin
     * @return Success
     */
    bool (* loadPlayerPlugin)( ctune_Player_t * player );

    /**
     * Loads a sound server plugin
     * @param sound_server Pointer to sound server plugin
     * @return Success
     */
    bool (* loadSoundServerPlugin)( ctune_AudioOut_t * sound_server );

    /**
     * [THREAD SAFE] Connects and plays a Radio station's stream
     * @param url         Radio station stream URL
     * @param volume      Initial playing volume
     * @param timeout_val Timeout value in seconds
     * @return Success (if false the error_no in RadioPlayer will be set accordingly)
     */
    bool (* playRadioStream)( const char * url, const int volume, int timeout_val );

    /**
     * [THREAD SAFE] Stops the playback of the currently playing stream
     */
    void (* stopPlayback)( void );

    /**
     * [THREAD SAFE] Gets the playback state variable's value
     * @return Playback state value
     */
    bool (* getPlaybackState)( void );

    /**
     * [THREAD SAFE] Changes playback volume by a specified amount
     * @param delta Volume change (+/-)
     */
    void (* modifyVolume)( int delta );

    /**
    * [THREAD SAFE] Gets the error number set in RadioPlayer
    * @return ctune_errno
    */
    int  (* getError)( void );

    /**
     * Test stream and get its property (independent from main player)
     * @param url         Stream URL
     * @param timeout_val Timeout value in seconds
     * @param codec_str   Pointer to the codec string
     * @param bitrate     Pointer to the bitrate container
     * @return Stream OK
     */
    bool (* testStream)( const char * url, int timeout_val, String_t * codec_str, ulong * bitrate );

} ctune_RadioPlayer;

#endif //CTUNE_RADIOPLAYER_H
