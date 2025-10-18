#ifndef CTUNE_UI_WINDOW_RSLISTWIN_H
#define CTUNE_UI_WINDOW_RSLISTWIN_H

#ifdef NO_NCURSESW
    #include <panel.h>
#else
    #include <ncursesw/panel.h>
#endif

#include "../../datastructure/HashMap.h"
#include "../datastructure/WindowProperty.h"
#include "../types/ScrollMask.h"
#include "../../dto/RadioStationInfo.h"
#include "../../dto/RadioBrowserFilter.h"
#include "../../datastructure/Vector.h"
#include "../enum/TextID.h"

typedef struct ctune_UI_Window_RSListWin_PageState {
    size_t first_on_page;
    size_t last_on_page;
    size_t selected;
} ctune_UI_RSListWin_PageState_t;

typedef struct ctune_UI_Window_RSListWin {
    Vector_t entries;  //keeps deep copies of RadioStationInfo_t objects

    const WindowProperty_t * canvas_property;
    PANEL                  * canvas_panel;
    WINDOW                 * canvas_win;
    PANEL                  * indicator_panel;
    WINDOW                 * indicator_win;
    int                      indicator_width;
    bool                     redraw;
    bool                     in_focus;
    bool                     mouse_ctrl;

    struct {
        bool   large_row;
        bool   theme_favs;
        int    row_height;
        size_t first_on_page;
        size_t last_on_page;
        size_t selected;
        bool   show_ctrl_row;
        bool   (* ctrl_row_fn)( struct ctune_UI_Window_RSListWin * win ); //special control row positioned after the `last` RSI row
    } row;

    struct {
        size_t name_ln;
        size_t cc_ln;
        size_t tags_ln;
        size_t kbps_ln;
    } sizes;

    struct {
        ctune_RadioBrowserFilter_t * filter;
    } cache;

    struct {
        bool         (* getStations)( ctune_RadioBrowserFilter_t *, Vector_t * );
        const char * (* getDisplayText)( ctune_UI_TextID_e );
        bool         (* toggleFavourite)( ctune_RadioStationInfo_t *, ctune_StationSrc_e );
        bool         (* isFavourite)( const ctune_RadioStationInfo_t * );
        unsigned     (* getStationState)( const ctune_RadioStationInfo_t * );
    } cb;

} ctune_UI_RSListWin_t;


extern struct ctune_UI_RSListWinClass {
    /**
     * Creates an initialised RSListWin_t
     * @param canvas_property Canvas property to base sizes on
     * @param getDisplayText  Callback method to get UI text
     * @param getStations     Callback method to fetch more stations
     * @param toggleFavourite Callback method to toggle a station's "favourite" status
     * @param getStationState Callback method to get a station's queued/favourite state
     * @return Initialised object
     */
    ctune_UI_RSListWin_t (* init)( const WindowProperty_t * canvas_property,
                                   const char * (* getDisplayText)( ctune_UI_TextID_e ),
                                   bool (* getStations)( ctune_RadioBrowserFilter_t *, Vector_t * ),
                                   bool (* toggleFavourite)( ctune_RadioStationInfo_t *, ctune_StationSrc_e ),
                                   unsigned (* getStationState)( const ctune_RadioStationInfo_t * ) );

    /**
     * Switch mouse control UI on/off
     * @param win             RSListWin_t object
     * @param mouse_ctrl_flag Flag to turn feature on/off
     */
    void (* setMouseCtrl)( ctune_UI_RSListWin_t * win, bool mouse_ctrl_flag );

    /**
     * Sets big row displaying on/off (false: 1 x line, true: 2 x lines + line delimiter)
     * @param win        RSListWin_t object
     * @param large_flag Flag to turn feature on/off
     */
    void (* setLargeRow)( ctune_UI_RSListWin_t * win, bool large_flag );

    /**
     * Sets the theming of 'favourite' stations on/off
     * @param win        RSListWin_t object
     * @param theme_flag Switch to turn on/off theming for rows of favourite stations
     */
    void (* themeFavourites)( ctune_UI_RSListWin_t * win, bool theme_flag );

    /**
     * Sets the internal flag to show a control row
     * @param win           RSListWin_t object
     * @param show_ctrl_row Show control row flag
     */
    void (* showCtrlRow)( ctune_UI_RSListWin_t * win, bool show_ctrl_row );

    /**
     * Loads a set of results into RSListWin's internal store
     * @param win     RSListWin_t object
     * @param results Collection of RadioStationInfo_t objects
     * @param filter  Filter used (used for fetching more with the ctrl row when applicable)
     * @return Success
     */
    bool (* loadResults)( ctune_UI_RSListWin_t * win, Vector_t * results, const ctune_RadioBrowserFilter_t * filter );

    /**
     * Loads a set of results to append to RSListWin's internal store
     * @param win     RSListWin_t object
     * @param results Collection of RadioStationInfo_t objects
     * @return Success
     */
    bool (* appendResults)( ctune_UI_RSListWin_t * win, Vector_t * results );

    /**
     * Clears whatever rows there are present and defaults back to "no results"
     * @param win RSListWin_t object
     */
    void (* loadNothing)( ctune_UI_RSListWin_t * win );

    /**
     * Sets the redraw flag on
     * @param win RSListWin_t object
     */
    void (* setRedraw)( ctune_UI_RSListWin_t * win );

    /**
     * Sets the 'in focus' flag
     * @param win   RSListWin_t object
     * @param focus Flag value
     */
    void (* setFocus)( ctune_UI_RSListWin_t * win, bool focus );

    /**
     * Change selected row to previous entry
     * @param win RSListWin_t object
     */
    void (* selectUp)( ctune_UI_RSListWin_t * win );

    /**
     * Change selected row to next entry
     * @param win RSListWin_t object
     */
    void (* selectDown)( ctune_UI_RSListWin_t * win );

    /**
     * Change selected row to entry a page length away
     * @param win RSListWin_t object
     */
    void (* selectPageUp)( ctune_UI_RSListWin_t * win );

    /**
     * Change selected row to entry a page length away
     * @param win RSListWin_t object
     */
    void (* selectPageDown)( ctune_UI_RSListWin_t * win );

    /**
     * Change selection to the first item in the list
     * @param win RSListWin_t object
     */
    void (* selectFirst)( ctune_UI_RSListWin_t * win );

    /**
     * Change selection to the last item in the list
     * @param win RSListWin_t object
     */
    void (* selectLast)( ctune_UI_RSListWin_t * win );

    /**
     * Select at given coordinates
     * @param win   RSListWin_t object
     * @param y     Row location on screen
     * @param x     Column location on screen
     */
    void (* selectAt)( ctune_UI_RSListWin_t * win, int y, int x );

    /**
     * Checks if area at coordinate is a scroll button
     * @param win RSListWin_t object
     * @param y   Row location on screen
     * @param x   Column location on screen
     * @return Scroll mask
     */
    ctune_UI_ScrollMask_m (* isScrollButton)( ctune_UI_RSListWin_t * win, int y, int x );

    /**
     * Toggles the "favourite" status of the currently selected item
     * @param win RSListWin_t object
     */
    void (* toggleFav)( ctune_UI_RSListWin_t * win );

    /**
     * Show updated window
     * @param win RSListWin_t object
     * @return Success
     */
    bool (* show)( ctune_UI_RSListWin_t * win );

    /**
     * Redraws the window from scratch
     * @param win RSListWin_t object
     */
    void (* resize)( ctune_UI_RSListWin_t * win );

    /**
     * Gets a RSI pointer to the currently selected item in list or if ctrl row then trigger callback
     * @param win RSListWin_t object
     * @return RadioStationInfo_t object pointer or NULL if out of range of the collection/is ctrl row
     */
    const ctune_RadioStationInfo_t * (* getSelected)( ctune_UI_RSListWin_t * win );

    /**
     * Gets the internal index of the currently selected row
     * @param win RSListWin_t object
     * @return Index
     */
    ctune_UI_RSListWin_PageState_t (* getPageState)( ctune_UI_RSListWin_t * win );

    /**
     * Sets the selected row by its index
     * @param win   RSListWin_t object
     * @param index Index to set
     * @return Success
     */
    bool (* setPageState)( ctune_UI_RSListWin_t * win, ctune_UI_RSListWin_PageState_t state );

    /**
     * Checks if the current row selected is the ctrl row
     * @param win RSListWin_t object
     * @return is the Ctrl row
     */
    bool (* isCtrlRowSelected)( ctune_UI_RSListWin_t * win );

    /**
     * Triggers the Ctrl row callback if there is one
     * @param win RSListWin_t object
     * @return Callback's success (also `true` if no callback)
     */
    bool (* triggerCtrlRow)( ctune_UI_RSListWin_t * win );

    /**
     * De-allocates RSListWin variables
     * @param win RSListWin_t object
     */
    void (* free)( ctune_UI_RSListWin_t * win );

} ctune_UI_RSListWin;

#endif //CTUNE_UI_WINDOW_RSLISTWIN_H
