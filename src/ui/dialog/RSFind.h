#ifndef CTUNE_UI_DIALOG_RSFIND_H
#define CTUNE_UI_DIALOG_RSFIND_H

#include <ncurses.h>

#include "../../dto/RadioBrowserFilter.h"
#include "../enum/FormExit.h"
#include "../widget/Form.h"

/**
 * RSFind object
 * @param initialised Init flag
 * @param form        Form widget
 * @param cache       Reusable computed variable cache
 * @param cb          Callback methods
 */
typedef struct ctune_UI_Dialog_RSFind {
    bool                     initialised;
    ctune_UI_Form_t          form;

    struct {
        ctune_RadioBrowserFilter_t filter;
        size_t                     max_label_width;
        const char               * order_items[STATION_ATTR_COUNT];
        size_t                     order_width;
        size_t                     order_selection;

    } cache;

    struct {
        const char * (* getDisplayText)( ctune_UI_TextID_e );
    } cb;

} ctune_UI_RSFind_t;


/**
 * RSFind namespace
 */
extern const struct ctune_UI_RSFind_Namespace {
    /**
     * Creates a base ctune_UI_RSFind_t object
     * @param parent         Pointer to size property of the parent window
     * @param getDisplayText Callback method to get text strings for the display
     * @return Basic un-initialised ctune_UI_RSFind_t object
     */
    ctune_UI_RSFind_t (* create)( const WindowProperty_t * parent, const char * (* getDisplayText)( ctune_UI_TextID_e ) );

    /**
     * Initialises RSFind (mostly checks base values are OK)
     * @param rsfind     Pointer to a ctune_UI_RSFind_t object
     * @param mouse_ctrl Flag to turn init mouse controls
     * @return Success
     */
    bool (* init)( ctune_UI_RSFind_t * rsfind, bool mouse_ctrl );

    /**
     * Get the initialised state of the instance
     * @param rsfind Pointer to a ctune_UI_RSFind_t object
     * @return Initialised state
     */
    bool (* isInitialised)( ctune_UI_RSFind_t * rsfind );

    /**
     * Switch mouse control UI on/off
     * @param rsfind          Pointer to ctune_UI_RSFind_t object
     * @param mouse_ctrl_flag Flag to turn feature on/off
     */
    void (* setMouseCtrl)( ctune_UI_RSFind_t * rsfind, bool mouse_ctrl_flag );

    /**
     * Create and show a populated window with the find form
     * @param rsfind Pointer to a ctune_UI_RSFind_t object
     * @return Success
     */
    bool (* show)( ctune_UI_RSFind_t * rsfind );

    /**
     * Pass keyboard input to the form
     * @param rsfind Pointer to a ctune_UI_RSFind_t object
     * @return Form exit state
     */
    ctune_FormExit_e (* captureInput)( ctune_UI_RSFind_t * rsfind );

    /**
     * Gets the internal filter object
     * @param rsfind Pointer to a ctune_UI_RSFind_t object
     * @return Pointer to internal RadioStationFilter_t DTO
     */
    ctune_RadioBrowserFilter_t * (* getFilter)( ctune_UI_RSFind_t * rsfind );

    /**
     * De-allocates the form and its fields
     * @param rsfind Pointer to a ctune_UI_RSFind_t object
     */
    void (* free)( ctune_UI_RSFind_t * rsfind );

} ctune_UI_RSFind;

#endif //CTUNE_UI_DIALOG_RSFIND_H
