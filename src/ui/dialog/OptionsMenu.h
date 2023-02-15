#ifndef CTUNE_UI_DIALOG_OPTIONSMENU_H
#define CTUNE_UI_DIALOG_OPTIONSMENU_H

#include "../../dto/RadioStationInfo.h" //for sorting
#include "../../dto/UIConfig.h"
#include "../../enum/PluginType.h"
#include "../../enum/StationAttribute.h" //for sorting
#include "../datastructure/WindowMargin.h"
#include "../definitions/Language.h"
#include "../enum/FormExit.h"
#include "../enum/PanelID.h"
#include "../widget/SlideMenu.h"
#include "../widget/BorderWin.h"

/**
 * Callback method signature used in ctune_UI_Dialog_OptionsMenu to trigger menu item events
 */
typedef int (* OptionsMenuCb_fn)( ctune_UI_PanelID_e, int );

/**
 * OptionsMenu object
 * @param initialised Initialisation flag
 * @param parent      Parent window properties
 * @param margins     Margins between the border and the content of the OptionsMenu
 * @param border_win  BorderWin widget
 * @param menu        SlideMenu widget
 * @param cache       Cache
 * @param cb          Callback methods
 */
typedef struct ctune_UI_Dialog_OptionsMenu {
    bool                     initialised;
    const WindowProperty_t * parent;
    WindowMargin_t           margins;
    ctune_UI_BorderWin_t     border_win;
    ctune_UI_SlideMenu_t     menu;

    struct {
        WindowProperty_t   slide_menu_property;
        WindowProperty_t   border_win_property;
        ctune_UI_PanelID_e curr_panel_id;
        Vector_t           payloads;
        bool               input_captured;
    } cache;

    struct Callbacks {
        const char *       (* getDisplayText)( ctune_UI_TextID_e );
        int                (* sortStationList)( ctune_UI_PanelID_e tab, int sort_by_e );
        int                (* addNewStation)( ctune_UI_PanelID_e tab, int /* unused */ );
        int                (* editStation)( ctune_UI_PanelID_e tab, int /* unused */ );
        int                (* toggleFavourite)( ctune_UI_PanelID_e tab, int /* unused */ );
        int                (* syncUpstream)( ctune_UI_PanelID_e tab, int /* unused */ );
        int                (* favTabTheming)( ctune_UI_PanelID_e tab, int action_flag_e );
        int                (* favTabCustomTheming)( ctune_UI_PanelID_e tab, int action_flag_e );
        int                (* listRowSizeLarge)( ctune_UI_PanelID_e tab, int action_flag_e );
        ctune_UIConfig_t * (* getUIConfig)( void );
        int                (* setUIPreset)( ctune_UI_PanelID_e tab, int preset_e );
        int                (* mouseSupport)( ctune_UI_PanelID_e tab, int action_flag_e );
        int                (* setMouseResolution)( ctune_UI_PanelID_e tab, int value );
        int                (* unicodeIcons)( ctune_UI_PanelID_e tab, int action_flag_e );
        int                (* streamTimeout)( ctune_UI_PanelID_e tab, int value );
        const Vector_t *   (* pluginList)( ctune_PluginType_e type );
        int                (* setPlayPlugin)( ctune_UI_PanelID_e tab, int id );
        int                (* setSrvPlugin)( ctune_UI_PanelID_e tab, int id );
        int                (* setRecPlugin)( ctune_UI_PanelID_e tab, int id );
    } cb;

} ctune_UI_OptionsMenu_t;

/**
 * OptionsMenu namespace
 */
extern const struct ctune_UI_Dialog_OptionsMenu_Namespace {
    /**
     * Creates an uninitialised ctune_UI_OptionsMenu_t
     * @param parent          Canvas property for parent window
     * @param parent_id       Parent panel ID
     * @param getDisplayText  Callback method to get UI text
     * @return Initialised ctune_UI_OptionsMenu_t object
     */
    ctune_UI_OptionsMenu_t (* create)( const WindowProperty_t * parent, ctune_UI_PanelID_e parent_id, const char * (* getDisplayText)( ctune_UI_TextID_e ) );

    /**
     * Initialised a ctune_UI_OptionsMenu_t object
     * @param om         Pointer to ctune_UI_OptionsMenu_t object
     * @param mouse_ctrl Flag to turn init mouse controls
     * @return Success
     */
    bool (* init)( ctune_UI_OptionsMenu_t * om, bool mouse_ctrl );

    /**
     * Sets the redraw flag on
     * @param win Pointer to ctune_UI_OptionsMenu_t object
     */
    void (* setRedraw)( ctune_UI_OptionsMenu_t * om );

    /**
     * Show updated window
     * @param om Pointer to ctune_UI_OptionsMenu_t object
     * @return Success
     */
    bool (* show)( ctune_UI_OptionsMenu_t * om );

    /**
     * Redraws the dialog
     * @param om Pointer to ctune_UI_OptionsMenu_t object
     */
    void (* resize)( void * om );

    /**
     * Pass keyboard input to the form
     * @param om Pointer to ctune_UI_OptionsMenu_t object
     */
    void (* captureInput)( ctune_UI_OptionsMenu_t * om );

    /**
     * Exit method to break the input capture loop (can be used as external callback)
     * @param om Pointer to ctune_UI_OptionsMenu_t object
     */
    void (* close)( ctune_UI_OptionsMenu_t * om );

    /**
     * De-allocates all content of OptionsMenu object
     * @param om Pointer to ctune_UI_OptionsMenu_t object
     */
    void (* free)( ctune_UI_OptionsMenu_t * om );

    struct {
        /**
         * Sets the callback method to sort the station list
         * @param om       Pointer to ctune_UI_OptionsMenu_t object
         * @param callback Callback function
         */
        void (* setSortStationListCallback)( ctune_UI_OptionsMenu_t * om, OptionsMenuCb_fn callback );

        /**
         * Sets the callback method to open new station dialog
         * @param om       Pointer to ctune_UI_OptionsMenu_t object
         * @param callback Callback function
         */
        void (* setAddNewStationCallback)( ctune_UI_OptionsMenu_t * om, OptionsMenuCb_fn callback );

        /**
         * Sets the callback method to open edit station dialog
         * @param om       Pointer to ctune_UI_OptionsMenu_t object
         * @param callback Callback function
         */
        void (* setEditStationCallback)( ctune_UI_OptionsMenu_t * om, OptionsMenuCb_fn callback );

        /**
         * Sets the callback method to toggle a station's "favourite" status
         * @param om       Pointer to ctune_UI_OptionsMenu_t object
         * @param callback Callback function
         */
        void (* setToggleFavouriteCallback)( ctune_UI_OptionsMenu_t * om, OptionsMenuCb_fn callback );

        /**
         * Sets the callback method to sync favourites from remote sources with their upstream counterpart
         * @param om       Pointer to ctune_UI_OptionsMenu_t object
         * @param callback Callback function
         */
        void (* setSyncCurrSelectedStationCallback)( ctune_UI_OptionsMenu_t * om, OptionsMenuCb_fn callback );

        /**
         * Sets the callback method to set/get the "favourite" tab's theming
         * @param om       Pointer to ctune_UI_OptionsMenu_t object
         * @param callback Callback function
         */
        void (* setFavouriteTabThemingCallback)( ctune_UI_OptionsMenu_t * om, OptionsMenuCb_fn callback );

        /**
         * Sets the callback method to set/get the state of custom colour theming on the favourite's tab
         * @param om       Pointer to ctune_UI_OptionsMenu_t object
         * @param callback Callback function
         */
        void (* setFavTabCustomThemingCallback)( ctune_UI_OptionsMenu_t * om, OptionsMenuCb_fn callback );

        /**
         * Sets the callback method to set/get the current tab station list's row 'large' property
         * @param om       Pointer to ctune_UI_OptionsMenu_t object
         * @param callback Callback function
         */
        void (* setListRowSizeLargeCallback)( ctune_UI_OptionsMenu_t * om, OptionsMenuCb_fn callback );

        /**
         * Sets the callback method to get a pointer to the UIConfig object
         * @param om       Pointer to ctune_UI_OptionsMenu_t object
         * @param callback Callback function
         */
        void (* setGetUIConfigCallback)( ctune_UI_OptionsMenu_t * om, ctune_UIConfig_t * (* callback)( void ) );

        /**
         * Sets the callback method to set a UI colour pallet preset
         * @param om       Pointer to ctune_UI_OptionsMenu_t object
         * @param callback Callback function
         */
        void (* setSetUIPresetCallback)( ctune_UI_OptionsMenu_t * om, OptionsMenuCb_fn callback );

        /**
         * Sets the callback method to set the mouse support
         * @param om       Pointer to ctune_UI_OptionsMenu_t object
         * @param callback Callback function
         */
        void (* setMouseSupportCallback)( ctune_UI_OptionsMenu_t * om, OptionsMenuCb_fn callback );

        /**
         * Sets the callback method to set the mouse's click-interval resolution in the configuration
         * @param om       Pointer to ctune_UI_OptionsMenu_t object
         * @param callback Callback function
         */
        void (* setMouseResolutionCallback)( ctune_UI_OptionsMenu_t * om, OptionsMenuCb_fn callback );

        /**
         * Sets the callback method to set unicode icons on/off
         * @param om       Pointer to ctune_UI_OptionsMenu_t object
         * @param callback Callback function
         */
        void (* setUnicodeIconsCallback)( ctune_UI_OptionsMenu_t * om, OptionsMenuCb_fn callback );

        /**
         * Sets the callback method to set/get the stream timeout value in the configuration
         * @param om       Pointer to ctune_UI_OptionsMenu_t object
         * @param callback Callback function
         */
        void (* setStreamTimeoutValueCallback)( ctune_UI_OptionsMenu_t * om, OptionsMenuCb_fn callback );

        /**
         * Sets the callback method to get a list of plugins
         * @param om       Pointer to ctune_UI_OptionsMenu_t object
         * @param callback Callback function
         */
        void (* setPluginListCallback)( ctune_UI_OptionsMenu_t * om, const Vector_t * (* callback)( ctune_PluginType_e ) );

        /**
         * Sets the callback methods to set plugins in the configuration
         * @param om                       Pointer to ctune_UI_OptionsMenu_t object
         * @param setPlayPlugin_callback   Plugin setter callback function
         * @param setSndSrvPlugin_callback Plugin setter callback function
         * @param setRecPlugin_callback    Plugin setter callback function
         */
        void (* setPluginSetterCallbacks)( ctune_UI_OptionsMenu_t * om,
                                           OptionsMenuCb_fn setPlayPlugin_callback,
                                           OptionsMenuCb_fn setSndSrvPlugin_callback,
                                           OptionsMenuCb_fn setRecPlugin_callback );
    } cb;

} ctune_UI_OptionsMenu;

#endif //CTUNE_UI_DIALOG_OPTIONSMENU_H