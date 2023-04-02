#ifndef CTUNE_UI_WINDOW_MAINWIN_H
#define CTUNE_UI_WINDOW_MAINWIN_H

#include "RSListWin.h"
#include "BrowserWin.h"
#include "../datastructure/WindowProperty.h"
#include "../enum/ActionID.h"
#include "../enum/PanelID.h"
#include "../enum/ContextID.h"
#include "../../dto/UIConfig.h"
#include "../../enum/PlaybackCtrl.h"

typedef struct ctune_UI_Window_MainWin {
    const WindowProperty_t * screen_property;
    bool                     mouse_ctrl;
    WINDOW                 * panel_windows[CTUNE_UI_PANEL_COUNT];
    PANEL                  * panels       [CTUNE_UI_PANEL_COUNT];
    ctune_UI_Context_e       curr_ctx;

    struct ctune_UI_MainWinSizes {
        WindowProperty_t title_bar;
        WindowProperty_t status_bar1;
        WindowProperty_t status_bar2;
        WindowProperty_t status_bar3;
        WindowProperty_t msg_line;
        WindowProperty_t tab_border;
        WindowProperty_t tab_canvas;
        WindowProperty_t browser_left;
        WindowProperty_t browser_right;
        WindowProperty_t browser_line;
    } size;

    struct {
        WindowProperty_t      favourites_tab_selector;
        WindowProperty_t      search_tab_selector;
        WindowProperty_t      browser_tab_selector;
        ctune_UI_RSListWin_t  favourites;
        ctune_UI_RSListWin_t  search;
        ctune_UI_BrowserWin_t browser;
    } tabs;

    struct {
        const char * (* getDisplayText)( ctune_UI_TextID_e );
        unsigned     (* getStationState)( const ctune_RadioStationInfo_t * rsi );
        bool         (* playStation)( const ctune_RadioStationInfo_t * rsi );
        void         (* openInfoDialog)( const ctune_RadioStationInfo_t * rsi );
    } cb;

    struct {
        ctune_UI_PanelID_e         prev_panel;
        ctune_UI_PanelID_e         curr_panel;

        ctune_RadioStationInfo_t * curr_radio_station;
        uint64_t                   curr_radio_station_hash;
        Vector_t                   favourites;

        //vars for painting back previous state post-resizing
        String_t                   curr_song;
        int                        curr_vol;
        ctune_PlaybackCtrl_e       playback_state;

    } cache;
} ctune_UI_MainWin_t;


extern const struct ctune_UI_MainWinClass {
    /**
     * Creates a MainWin
     * @param getDisplayText Callback method to get UI text strings
     * @param screen_size    Pointer to the screen size property
     * @return MainWin object
     */
    ctune_UI_MainWin_t (* create)( const char * (* getDisplayText)( ctune_UI_TextID_e ), WindowProperty_t * screen_size );

    /**
     * Initialises main window
     * @param main            MainWin object
     * @param ui_config       Pointer to the UI configuration
     * @param getStations     Callback method to fetch more stations
     * @param getCatItems     Callback method to fetch station search category items
     * @param getStationsBy   Callback method to fetch stations based on a sub-category
     * @param toggleFavourite Callback method to toggle a station's "favourite" status
     * @return Success
     */
    bool (* init)( ctune_UI_MainWin_t * main,
                   ctune_UIConfig_t   * ui_config,
                   bool (* getStations)( ctune_RadioBrowserFilter_t *, Vector_t * ),
                   bool (* getCatItems)( const ctune_ListCategory_e, const ctune_RadioBrowserFilter_t *, Vector_t * ),
                   bool (* getStationsBy)( const ctune_ByCategory_e, const char *, Vector_t * ),
                   bool (* toggleFavourite)( ctune_RadioStationInfo_t *, ctune_StationSrc_e ) );

    /**
     * Switch mouse control UI on/off
     * @param main            Pointer to MainWin
     * @param mouse_ctrl_flag Flag to turn feature on/off
     */
    void (* setMouseCtrl)( ctune_UI_MainWin_t * win, bool mouse_ctrl_flag );

    /**
     * Handles a mouse event
     * @param main  Pointer to MainWin
     * @param event Mouse event
     * @return ActionID associated with mouse event in MainWin
     */
    ctune_UI_ActionID_e (* handleMouseEvent)( ctune_UI_MainWin_t * main, MEVENT * event );

    /**
     * De-allocates resources used by MainWin
     * @param main MainWin object
     */
    void (* free)( ctune_UI_MainWin_t * main );

    /**
     * Reconstruct the UI for when window sizes change
     * @param main_win Pointer to MainWin
     */
    void (* resize)( void * main_win );

    /**
     * Move a particular pane to the top
     * @param main     Pointer to MainWin
     * @param panel_id PanelID enum
     */
    void (* show)( ctune_UI_MainWin_t * main, ctune_UI_PanelID_e panel_id );

    /**
     * Gets the current context
     * @param main Pointer to MainWin
     * @return Context enum
     */
    ctune_UI_Context_e (* currentContext)( ctune_UI_MainWin_t * main );

    /**
     * Gets the current panel ID selected
     * @param main Pointer to MainWin
     * @return PanelID enum
     */
    ctune_UI_PanelID_e (* currentPanelID)( ctune_UI_MainWin_t * main );

    /**
     * Gets the previously selected panel ID
     * @param main Pointer to MainWin
     * @return PanelID enum
     */
    ctune_UI_PanelID_e (* previousPanelID)( ctune_UI_MainWin_t * main );

    /**
     * Gets the current radio station
     * @param main Pointer to MainWin
     * @return Current radio station
     */
    const ctune_RadioStationInfo_t * (* getCurrStation)( ctune_UI_MainWin_t * main );

    /**
     * Gets the current radio station's hash
     * @param main Pointer to MainWin
     * @return Current radio station's hash value
     */
    uint64_t (* getCurrStationHash)( ctune_UI_MainWin_t * main );

    /**
     * Gets the currently selected station
     * @param main Pointer to MainWin
     * @param tab  PanelID of tab to get selected from
     * @return Pointer to selected radio station or NULL
     */
    const ctune_RadioStationInfo_t * (* getSelectedStation)( ctune_UI_MainWin_t * main, ctune_UI_PanelID_e tab );

    /**
     * Check if the control row is selected in a given tab's listing
     * @param main Pointer to MainWin
     * @param tab  PanelID enum
     * @return Selected ctrl row state
     */
    bool (* isCtrlRowSelected)( ctune_UI_MainWin_t * main, ctune_UI_PanelID_e tab );

    /**
     * Gets a view state from a list
     * @param main Pointer to MainWin
     * @param tab  Tab of the list targeted
     * @return State
     */
    ctune_UI_RSListWin_PageState_t (* getViewState)( ctune_UI_MainWin_t * main, ctune_UI_PanelID_e tab );

    /**
     * Sets a view state for a list
     * @param main  Pointer to MainWin
     * @param tab   Tab of the list targeted
     * @param state State to load
     */
    void (* setViewState)( ctune_UI_MainWin_t * main, ctune_UI_PanelID_e tab, ctune_UI_RSListWin_PageState_t state );

    /**
     * Checks if given browser pane is currently in focus
     * @param main Pointer to MainWin
     * @param pane PaneFocus_e enum ID
     * @return Focussed state
     */
    bool (* browserPaneIsInFocus)( ctune_UI_MainWin_t * main, ctune_UI_BrowserWin_PaneFocus_e pane );

    struct {
        /**
         * Update the cached favourites
         * @param main Pointer to MainWin
         * @param cb   Callback to update collection of favourite stations
         */
        void (* updateFavourites)( ctune_UI_MainWin_t * main, bool (* cb)( Vector_t * ) );

        /**
         * Toggles the favourite state of a selected station
         * @param main Pointer to MainWin
         * @param tab  Panel ID of the tab where to toggle the fav flag on the selected station
         */
        void (* toggleFavourite)( ctune_UI_MainWin_t * main, ctune_UI_PanelID_e tab );

        /**
         * Sets the theming of 'favourite' stations on/off
         * @param main       Pointer to MainWin
         * @param theme_flag Switch to turn on/off theming for rows of favourite stations
         */
        void (* themeFavourites)( ctune_UI_MainWin_t * main, bool theme_flag );

        /**
         * Loads search results into search tab
         * @param main    Pointer to MainWin
         * @param results Collection of stations
         * @param filter  Filter used during search
         */
        void (* loadSearchResults)( ctune_UI_MainWin_t * main, Vector_t * results, ctune_RadioBrowserFilter_t * filter );

        /**
         * Clears results from search tab
         * @param main Pointer to MainWin
         */
        void (* clearSearchResults)( ctune_UI_MainWin_t * main );

        /**
         * Sets big row displaying on/off (false: 1 x line, true: 2 x lines + line delimiter)
         * @param main       Pointer to MainWin
         * @param tab        PanelID enum
         * @param large_flag Flag to turn feature on/off
         */
        void (* setLargeRow)( ctune_UI_MainWin_t * main, ctune_UI_PanelID_e tab, bool large_flag );

        /**
         * Signals a radio station as 'current' (i.e. as queued or playing)
         * @param main Pointer to MainWin
         * @param rsi  Pointer to RadioStationInfo DTO
         */
        void (* setCurrStation)( ctune_UI_MainWin_t * main, ctune_RadioStationInfo_t * rsi );

        /**
         * Start playback of current selected station on current tab view
         * @param main Pointer to MainWin
         * @return Pointer to selected station or NULL
         */
        void (* playSelectedStation)( ctune_UI_MainWin_t * main );
    } ctrl;

    /**
     * Navigation
     */
    struct {
        /**
         * Navigation select for 'ENTER' key
         * @param main Pointer to MainWin
         */
        void (* enter)( ctune_UI_MainWin_t * main );

        /**
         * Navigation select for 'UP' key
         * @param main Pointer to MainWin
         */
        void (* selectUp)( ctune_UI_MainWin_t * main );

        /**
         * Navigation select for 'DOWN' key
         * @param main Pointer to MainWin
         */
        void (* selectDown)( ctune_UI_MainWin_t * main );

        /**
         * Navigation select for 'LEFT' key
         * @param main Pointer to MainWin
         */
        void (* selectLeft)( ctune_UI_MainWin_t * main );

        /**
         * Navigation select for 'RIGHT' key
         * @param main Pointer to MainWin
         */
        void (* selectRight)( ctune_UI_MainWin_t * main );

        /**
         * Navigation select for 'PPAGE' key
         * @param main Pointer to MainWin
         */
        void (* selectPageUp)( ctune_UI_MainWin_t * main );

        /**
         * Navigation select for 'NPAGE' key
         * @param main Pointer to MainWin
         */
        void (* selectPageDown)( ctune_UI_MainWin_t * main );

        /**
         * Navigation select for 'HOME' key
         * @param main Pointer to MainWin
         */
        void (* selectHome)( ctune_UI_MainWin_t * main );

        /**
         * Navigation select for 'END' key
         * @param main Pointer to MainWin
         */
        void (* selectEnd)( ctune_UI_MainWin_t * main );

        /**
         * Navigation select for 'TAB' key
         * @param main Pointer to MainWin
         */
        void (* switchFocus)( ctune_UI_MainWin_t * main );
    } nav;

    /**
     * Printers
     */
    struct {
        /**
         * Prints a string inside the area reserved for song descriptions
         * @param str String to display on screen
         */
        void (* songInfo)( ctune_UI_MainWin_t * main, const char * str );

        /**
         * Prints an integer inside the area reserved to display the current volume
         * @param vol Volume to display on screen
         */
        void (* volume)( ctune_UI_MainWin_t * main, const int vol );

        /**
         * Prints the playback state to the screen
         * @param state Playback state
         */
        void (* playbackState)( ctune_UI_MainWin_t * main, const ctune_PlaybackCtrl_e state );

        /**
         * Prints the search state to the screen
         * @param state Search state
         */
        void (* searchingState)( ctune_UI_MainWin_t * main, const bool state );

        /**
         * Prints an error description string to the screen
         * @param err_str Error string
         */
        void (* error)( ctune_UI_MainWin_t * main, const char * err_str );

        /**
         * Prints a message string to the screen
         * @param info_str Information string
         */
        void (* statusMsg)( ctune_UI_MainWin_t * main, const char * info_str );

        /**
         * Clears the message line of any messages if not used for input
         * @param main Pointer to MainWin
         */
        void (* clearMsgLine)( ctune_UI_MainWin_t * main );
    } print;

    struct {
        /**
         * Sets the callback to use to get a station's state
         * @param main Pointer to MainWin
         * @param cb   Callback method
         */
        void (* setStationStateGetterCallback)( ctune_UI_MainWin_t * main, unsigned (* cb)( const ctune_RadioStationInfo_t * ) );

        /**
         * Sets the callback to use to trigger playback of a station
         * @param main Pointer to MainWin
         * @param cb   Callback method
         */
        void (* setPlayStationCallback)( ctune_UI_MainWin_t * main, bool (* cb)( const ctune_RadioStationInfo_t * ) );

        /**
         * Sets the callback to use to open the station information dialog
         * @param main Pointer to MainWin
         * @param cb   Callback method
         */
        void (* setOpenInfoDialogCallback)( ctune_UI_MainWin_t * main, void (* cb)( const ctune_RadioStationInfo_t * ) );
    } cb;


} ctune_UI_MainWin;

#endif //CTUNE_UI_WINDOW_MAINWIN_H