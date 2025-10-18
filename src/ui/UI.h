#ifndef CTUNE_UI_UI_MAIN_H
#define CTUNE_UI_UI_MAIN_H

#ifdef NO_NCURSESW
    #include <ncurses.h>
#else
    #include <ncursesw/ncurses.h>
#endif

#include <stdbool.h>

#include "../dto/ArgOptions.h"
#include "../dto/RadioStationInfo.h"
#include "../enum/PlaybackCtrl.h"
#include "enum/PanelID.h"
#include "definitions/Language.h"

struct ctune_UI_Instance {
    /**
     * Initialises the UI and its internal variables
     * @param show_cursor Flag to show the UI cursor
     * @return Success
     */
    bool (* setup)( bool show_cursor );

    /**
     * Start the UI loop
     */
    void (* start)( void );

    /**
     * Terminates the UI
     */
    void (* teardown)( void );

    /**
     * Reconstruct the UI for when window sizes change
     */
    void (* resize)();

    /**
     * Signals a radio station as 'current' (i.e. as queued or playing)
     * @param rsi Pointer to RadioStationInfo DTO
     */
    void (* setCurrStation)( const ctune_RadioStationInfo_t * rsi );

    /**
     * Prints a string inside the area reserved for song descriptions
     * @param str String to display on screen
     */
    void (* printSongInfo)( const char * str );

    /**
     * Prints an integer inside the area reserved to display the current volume
     * @param vol Volume to display on screen
     */
    void (* printVolume)( const int vol );

    /**
     * Prints the playback state to the screen
     * @param state Playback state
     */
    void (* printPlaybackState)( const ctune_PlaybackCtrl_e state );

    /**
     * Prints the search state to the screen
     * @param state Search state
     */
    void (* printSearchingState)( const bool state );

    /**
     * Prints an error description string to the screen
     * @param err_str Error string
     * @param err_no  Error number (cTune specific)
     */
    void (* printError)( const char * err_str, int err_no );

    /**
     * Prints a message string to the screen
     * @param info_str Information string
     */
    void (* printStatusMsg)( const char * info_str );

    /**
     * Sets the callback to use when a volume change occurs without anything playing
     * @param cb Callback method
     */
    void (* setQuietVolChangeCallback)( int(* cb)( int ) );
};

extern const struct ctune_UI_Instance ctune_UI;

#endif //CTUNE_UI_UI_MAIN_H
