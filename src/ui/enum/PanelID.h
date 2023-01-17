#ifndef CTUNE_UI_ENUM_PANELID_H
#define CTUNE_UI_ENUM_PANELID_H

typedef enum {
    CTUNE_UI_PANEL_TITLE = 0,   //main edge - always visible
    CTUNE_UI_PANEL_STATUS_1,    //main edge - always visible
    CTUNE_UI_PANEL_STATUS_2,    //main edge - always visible
    CTUNE_UI_PANEL_STATUS_3,    //main edge - always visible
    CTUNE_UI_PANEL_MSG_LINE,    //main edge - always visible
    CTUNE_UI_PANEL_FAVOURITES,  //tabbed window
    CTUNE_UI_PANEL_SEARCH,      //tabbed window
    CTUNE_UI_PANEL_BROWSER,     //paned window

    CTUNE_UI_PANEL_COUNT
} ctune_UI_PanelID_e;


extern const struct ctune_UI_PanelID_Namespace {
    /**
     * Gets corresponding string for a Panel ID
     * @param id PanelID enum type
     * @return String
     */
    const char * (* str)( ctune_UI_PanelID_e id );

} ctune_UI_PanelID;

#endif //CTUNE_UI_ENUM_PANELID_H
