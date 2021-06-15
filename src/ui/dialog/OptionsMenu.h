#ifndef CTUNE_UI_DIALOG_OPTIONSMENU_H
#define CTUNE_UI_DIALOG_OPTIONSMENU_H

#include "../../dto/RadioStationInfo.h" //for sorting
#include "../../enum/StationAttribute.h" //for sorting
#include "../datastructure/WindowMargin.h"
#include "../definitions/Language.h"
#include "../enum/FormExit.h"
#include "../enum/PanelID.h"
#include "../widget/SlideMenu.h"
#include "../widget/BorderWin.h"

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
        Vector_t           lvl1_menu_payloads;
        Vector_t           lvl2_menu_payloads;
        bool               input_captured;
    } cache;

    struct Callbacks {
        const char * (* getDisplayText)( ctune_UI_TextID_e );
        void         (* sortStationList)( ctune_UI_PanelID_e tab, ctune_RadioStationInfo_SortBy_e attr );
        void         (* addNewStation)( ctune_UI_PanelID_e tab );
        void         (* editStation)( ctune_UI_PanelID_e tab );
        void         (* toggleFavourite)( ctune_UI_PanelID_e tab );
        void         (* syncUpstream)( ctune_UI_PanelID_e tab );
    } cb;

} ctune_UI_OptionsMenu_t;

/**
 * OptionsMenu namespace
 */
extern const struct ctune_UI_Dialog_OptionsMenu_Namespace {
    /**
     * Creates an uninitialised ctune_UI_OptionsMenu_t
     * @param parent          Canvas property for parent window
     * @param getDisplayText  Callback method to get UI text
     * @param sortStationList Callback method to sort station list
     * @param addNewStation   Callback method to open new station dialog
     * @param editStation     Callback method to open edit station dialog
     * @param toggleFavourite Callback method to toggle a station's "favourite" status
     * @param syncUpstream    Callback method to sync favourites from remote sources with their upstream counterpart
     * @return Initialised ctune_UI_OptionsMenu_t object
     */
    ctune_UI_OptionsMenu_t (* create)( const WindowProperty_t * parent,
                                       const char * (* getDisplayText)( ctune_UI_TextID_e ),
                                       void         (* sortStationList)( ctune_UI_PanelID_e tab, ctune_RadioStationInfo_SortBy_e attr ),
                                       void         (* addNewStation)( ctune_UI_PanelID_e tab ),
                                       void         (* editStation)( ctune_UI_PanelID_e tab ),
                                       void         (* toggleFavourite)( ctune_UI_PanelID_e tab ),
                                       void         (* syncUpstream)( ctune_UI_PanelID_e tab ) );

    /**
     * Initialised a ctune_UI_OptionsMenu_t object
     * @param om Pointer to ctune_UI_OptionsMenu_t object
     * @return Success
     */
    bool (* init)( ctune_UI_OptionsMenu_t * om );

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
    void (* captureInput)( ctune_UI_OptionsMenu_t * om, ctune_UI_PanelID_e tab );

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

} ctune_UI_OptionsMenu;

#endif //CTUNE_UI_DIALOG_OPTIONSMENU_H