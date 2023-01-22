#ifndef CTUNE_UI_WINDOW_BROWSERWIN_H
#define CTUNE_UI_WINDOW_BROWSERWIN_H

#include "../../enum/ListCategory.h"
#include "../../enum/ByCategory.h"
#include "../../datastructure/Vector.h"
#include "../../dto/CategoryItem.h"

#include "../widget/SlideMenu.h"
#include "RSListWin.h"

//            BrowserWin
// +--------------+-------------------------+
// |              |                         |
// |              |                         |
// |  SlideMenu   |       RSListWin         |
// |              |                         |
// |              |                         |
// +--------------+-------------------------+

typedef enum ctune_UI_BrowserWin_PaneFocus {
    FOCUS_PANE_RIGHT,
    FOCUS_PANE_LEFT
} ctune_UI_BrowserWin_PaneFocus_e;

/**
 * BrowserWin
 * @param pane_focus Current display focus
 * @param left_pane  Slide menu pane
 * @param right_pane Result list pane
 * @param cache      Stateful cache for the browser window
 * @param cb         Callback functions
 */
typedef struct ctune_UI_BrowserWin {
    ctune_UI_BrowserWin_PaneFocus_e pane_focus;
    ctune_UI_SlideMenu_t            left_pane;
    ctune_UI_RSListWin_t            right_pane;

    bool cache_menu;

    struct {
        ctune_UI_TextID_e  cat2ui_text_enum[RADIOBROWSER_CATEGORY_COUNT];
        ctune_ByCategory_e cat2bycat       [RADIOBROWSER_CATEGORY_COUNT];
        Vector_t           lvl1_menu_payloads;
        Vector_t           lvl2_menu_payloads;
        Vector_t           rsi_results;
    } cache;

    struct {
        const char * (* getDisplayText)( ctune_UI_TextID_e );
        bool         (* getStations)( ctune_RadioBrowserFilter_t *, Vector_t * );
        bool         (* getCatItems)( const ctune_ListCategory_e, const ctune_RadioBrowserFilter_t *, Vector_t * );
        bool         (* getStationsBy)( const ctune_ByCategory_e, const char *, Vector_t * );
    } cb;

} ctune_UI_BrowserWin_t;

/**
 * ctune_UI_BrowserWin class namespace
 */
extern const struct ctune_UI_BrowserWin_Namespace {
    /**
     * Creates an initialised ctune_UI_BrowserWin_t
     * @param left_canvas     Canvas property for the left pane
     * @param right_canvas    Canvas property for the right pane
     * @param getDisplayText  Callback method to get UI text
     * @param getStations     Callback method to fetch more stations
     * @param getCatItems     Callback method to fetch station search category items
     * @param getStationsBy   Callback method to fetch stations based on a sub-category
     * @param toggleFavourite Callback method to toggle a station's "favourite" status
     * @param getStationState Callback method to get a station's queued/favourite state
     * @return Initialised ctune_UI_BrowserWin_t object
     */
    ctune_UI_BrowserWin_t (* init)( const WindowProperty_t * left_canvas,
                                    const WindowProperty_t * right_canvas,
                                    const char * (* getDisplayText)( ctune_UI_TextID_e ),
                                    bool         (* getStations)( ctune_RadioBrowserFilter_t *, Vector_t * ),
                                    bool         (* getCatItems)( const ctune_ListCategory_e, const ctune_RadioBrowserFilter_t *, Vector_t * ),
                                    bool         (* getStationsBy)( const ctune_ByCategory_e, const char *, Vector_t * ),
                                    bool         (* toggleFavourite)( ctune_RadioStationInfo_t *, ctune_StationSrc_e ),
                                    unsigned     (* getStationState)( const ctune_RadioStationInfo_t * ) );

    /**
     * Switch mouse control UI on/off
     * @param win             ctune_UI_BrowserWin_t object
     * @param mouse_ctrl_flag Flag to turn feature on/off
     */
    void (* setMouseCtrl)( ctune_UI_BrowserWin_t * win, bool mouse_ctrl_flag );

    /**
     * Sets big row displaying on/off in right pane
     * @param win        ctune_UI_BrowserWin_t object
     * @param large_flag Flag to turn feature on/off
     */
    void (* setLargeRow)( ctune_UI_BrowserWin_t * win, bool large_flag );

    /**
     * Sets the theming of 'favourite' stations on/off
     * @param win        RSListWin_t object
     * @param theme_flag Switch to turn on/off theming for rows of favourite stations
     */
    void (* themeFavourites)( ctune_UI_BrowserWin_t * win, bool theme_flag );

    /**
     * Show the control row in the right pane
     * @param win           ctune_UI_BrowserWin_t object
     * @param show_ctrl_row Show control row flag
     */
    void (* showCtrlRow)( ctune_UI_BrowserWin_t * win, bool show_ctrl_row );

    /**
     * Creates root menu items
     * @param win ctune_UI_BrowserWin_t object
     * @return Success
     */
    bool (* populateRootMenu)( ctune_UI_BrowserWin_t * win );

    /**
     * Sets the redraw flag on
     * @param win        ctune_UI_SlideMenu_t object
     * @param redraw_all Flag to hard-set redraw on both panes
     */
    void (* setRedraw)( ctune_UI_BrowserWin_t * win, bool redraw_all );

    /**
     * Sets the pane focus
     * @param win  ctune_UI_SlideMenu_t object
     * @param pane PaneFocus_e value
     */
    void (* setFocus)( ctune_UI_BrowserWin_t * win, ctune_UI_BrowserWin_PaneFocus_e pane );

    /**
     * Swaps pane focus
     * @param win ctune_UI_SlideMenu_t object
     */
    void (* switchFocus)( ctune_UI_BrowserWin_t * win );

    /**
     * Checks if given pane is currently in focus
     * @param win ctune_UI_BrowserWin_t object
     * @param pane PaneFocus_e enum ID
     * @return Focussed state
     */
    bool (* isInFocus)( ctune_UI_BrowserWin_t * win, ctune_UI_BrowserWin_PaneFocus_e pane );

    /**
     * Show updated window
     * @param win ctune_UI_BrowserWin_t object
     * @return Success
     */
    bool (* show)( ctune_UI_BrowserWin_t * win );

    /**
     * Resize the BrowserWindow
     * @param win Pointer to ctune_UI_BrowserWin_t object
     */
    void (* resize)( void * win );

    /**
     * Change selected row to previous entry
     * @param win ctune_UI_BrowserWin_t object
     */
    void (* navKeyUp)( ctune_UI_BrowserWin_t * win );

    /**
     * Change selected row to next entry
     * @param win ctune_UI_BrowserWin_t object
     */
    void (* navKeyDown)( ctune_UI_BrowserWin_t * win );

    /**
     * Change pane focus/trigger menu control function of row depending on circumstances
     * @param win ctune_UI_BrowserWin_t object
     */
    void (* navKeyRight)( ctune_UI_BrowserWin_t * win );

    /**
     * Change to the parent win/item of currently selected row
     * @param win ctune_UI_BrowserWin_t object
     */
    void (* navKeyLeft)( ctune_UI_BrowserWin_t * win );

    /**
     * Change selected row to entry a page length away
     * @param win ctune_UI_BrowserWin_t object
     */
    void (* navKeyPageUp)( ctune_UI_BrowserWin_t * win );

    /**
     * Change selected row to entry a page length away
     * @param win ctune_UI_BrowserWin_t object
     */
    void (* navKeyPageDown)( ctune_UI_BrowserWin_t * win );

    /**
     * Change selection to the first item in the list
     * @param win ctune_UI_BrowserWin_t object
     */
    void (* navKeyHome)( ctune_UI_BrowserWin_t * win );

    /**
     * Change selection to the last item in the list
     * @param win ctune_UI_BrowserWin_t object
     */
    void (* navKeyEnd)( ctune_UI_BrowserWin_t * win );

    /**
     * Activate selection
     * @param win ctune_UI_BrowserWin_t object
     */
    void (* navKeyEnter)( ctune_UI_BrowserWin_t * win );

    /**
     * Toggles the "favourite" status of the currently selected radio station
     * @param win ctune_UI_BrowserWin_t object
     */
    void (* toggleFav)( ctune_UI_BrowserWin_t * win );

    /**
     * Select at given coordinates
     * @param win   ctune_UI_BrowserWin_t object
     * @param y     Row location on screen
     * @param x     Column location on screen
     */
    void (* selectAt)( ctune_UI_BrowserWin_t * win, int y, int x );

    /**
     * Checks if area at coordinate is a scroll button
     * @param win ctune_UI_BrowserWin_t object
     * @param y   Row location on screen
     * @param x   Column location on screen
     * @return Scroll mask
     */
    ctune_UI_ScrollMask_m (* isScrollButton)( ctune_UI_BrowserWin_t * win, int y, int x );

    /**
     * Gets a RSI pointer to the currently selected item in the right pane or if ctrl row then trigger callback
     * @param  win ctune_UI_BrowserWin_t object
     * @return RadioStationInfo_t object pointer or NULL if out of range of the collection/is ctrl row
     */
    const ctune_RadioStationInfo_t * (* getSelectedStation)( ctune_UI_BrowserWin_t * win );

    /**
     * Checks if the current row selected in the right pane is the ctrl row
     * @param win ctune_UI_BrowserWin_t object
     * @return is the Ctrl row
     */
    bool (* isCtrlRowSelected)( ctune_UI_BrowserWin_t * win );

    /**
     * De-allocates all content of BrowserWin object
     * @param win ctune_UI_BrowserWin_t object
     */
    void (* free)( ctune_UI_BrowserWin_t * win );

} ctune_UI_BrowserWin;

#endif //CTUNE_UI_WINDOW_BROWSERWIN_H
