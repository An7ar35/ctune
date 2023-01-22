#ifndef CTUNE_UI_DIALOG_RSEDIT_H
#define CTUNE_UI_DIALOG_RSEDIT_H

#include <stdbool.h>
#include <ncurses.h>
#include <regex.h>

#include "../widget/Form.h"
#include "../enum/FormExit.h"
#include "../../dto/RadioStationInfo.h"

/**
 * RSEdit object
 * @param initialised Init flag
 * @param form        Form widget
 * @param cache       Cached variables
 * @param cb          Callback methods
 */
typedef struct ctune_UI_Dialog_RSEdit {
    bool                     initialised;
    ctune_UI_Form_t          form;

    struct {
        ctune_RadioStationInfo_t station;
        size_t                   max_label_width;
        regex_t                  url_regex;

    } cache;

    struct {
        const char * (* getDisplayText)( ctune_UI_TextID_e );
        bool         (* generateUUID)( String_t * uuid );
        bool         (* testStream)( const char * url, String_t * codec, ulong * bitrate );
        bool         (* validateURL)( const char * url );
    } cb;

} ctune_UI_RSEdit_t;

/**
 * RSEdit namespace
 */
extern const struct ctune_UI_RSEdit_Namespace {
    /**
     * Creates a base ctune_UI_RSEdit_t object
     * @param parent         Pointer to size property of the parent window
     * @param getDisplayText Callback method to get text strings for the display
     * @param generateUUID   Callback method to create a unique UUID for new stations
     * @param testStream     Callback method to test and get codec/bitrate from a stream URL
     * @param validateURL    Callback method to check the validity of a URL string
     * @return Basic un-initialised ctune_UI_RSEdit_t object
     */
    ctune_UI_RSEdit_t (* create)( const WindowProperty_t * parent,
                                  const char * (* getDisplayText)( ctune_UI_TextID_e ),
                                  bool         (* generateUUID)( String_t * uuid ),
                                  bool         (* testStream)( const char * url, String_t * codec, ulong * bitrate ),
                                  bool         (* validateURL)( const char * url ) );

    /**
     * Initialises RSFind (mostly checks base values are OK)
     * @param rsfind     Pointer to a ctune_UI_RSFind_t object
     * @param mouse_ctrl Flag to turn init mouse controls
     * @return Success
     */
    bool (* init)( ctune_UI_RSEdit_t * rsedit, bool mouse_ctrl );

    /**
     * Get the initialised state of the instance
     * @param rsedit Pointer to a ctune_UI_RSEdit_t object
     * @return Initialised state
     */
    bool (* isInitialised)( ctune_UI_RSEdit_t * rsedit );

    /**
     * Switch mouse control UI on/off
     * @param rsedit          Pointer to ctune_UI_RSEdit_t object
     * @param mouse_ctrl_flag Flag to turn feature on/off
     */
    void (* setMouseCtrl)( ctune_UI_RSEdit_t * rsedit, bool mouse_ctrl_flag );

    /**
     * Loads a radio station into the form
     * @param rsedit Pointer to a ctune_UI_RSEdit_t object
     * @param rsi    Pointer to RSI to edit
     */
    void (* loadStation)( ctune_UI_RSEdit_t * rsedit, const ctune_RadioStationInfo_t * rsi );

    /**
     * Sets up the form for a new radio station
     * @param rsedit Pointer to a ctune_UI_RSEdit_t object
     * @return Success
     */
    bool (* newStation)( ctune_UI_RSEdit_t * rsedit );

    /**
     * Copy a radio station into the form with a new generated local UUID
     * @param rsedit Pointer to a ctune_UI_RSEdit_t object
     * @param rsi    Pointer to RSI to copy as new local station
     * @return Success
     */
    bool (* copyStation)( ctune_UI_RSEdit_t * rsedit, const ctune_RadioStationInfo_t * rsi );

    /**
     * Create and show a populated window with the find form
     * @param rsedit Pointer to a ctune_UI_RSEdit_t object
     * @return Success
     */
    bool (* show)( ctune_UI_RSEdit_t * rsedit );

    /**
     * Pass keyboard input to the form
     * @param rsedit Pointer to a ctune_UI_RSEdit_t object
     * @return Form exit state
     */
    ctune_FormExit_e (* captureInput)( ctune_UI_RSEdit_t * rsedit );

    /**
     * Gets the internal filter object
     * @param rsedit Pointer to a ctune_UI_RSEdit_t object
     * @return Pointer to internal RadioStationInfo_t DTO
     */
    ctune_RadioStationInfo_t * (* getStation)( ctune_UI_RSEdit_t * rsedit );

    /**
     * De-allocates the form and its fields
     * @param rsedit Pointer to a ctune_UI_RSEdit_t object
     */
    void (* free)( ctune_UI_RSEdit_t * rsedit );

} ctune_UI_RSEdit;

#endif //CTUNE_UI_DIALOG_RSEDIT_H
