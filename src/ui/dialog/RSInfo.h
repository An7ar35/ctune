#ifndef CTUNE_UI_DIALOG_RSINFO_H
#define CTUNE_UI_DIALOG_RSINFO_H

#include <ncurses.h>
#include <sys/ioctl.h>

#include "../enum/TextID.h"
#include "../datastructure/WindowProperty.h"
#include "../datastructure/WindowMargin.h"
#include "../widget/ScrollBar.h"
#include "../widget/Dialog.h"
#include "../../dto/RadioStationInfo.h"

#define CTUNE_UI_DIALOG_RSINFO_FIELD_COUNT 33

/**
 * RSInfo object
 * @param initialised       Init flag
 * @param mouse_ctrl       Flag to show mouse controls
 * @param screen_size       Pointer to size property of the parent window
 * @param margins           Margin properties of the form window
 * @param dialog            ctune_UI_Dialog widget
 * @param label_txt         Cached text for the labels
 * @param col_separator_str String to use as separator between the labels and the fields
 * @param max_label_width   Cached max label width
 * @param cache             Cached last used window title for borders
 * @param cb                Callback(s)
 */
typedef struct ctune_UI_Dialog_RSInfo {
    bool                     initialised;
    bool                     mouse_ctrl;
    const WindowProperty_t * screen_size;
    WindowMargin_t           margins;
    ctune_UI_Dialog_t        dialog;

    const char             * label_txt[CTUNE_UI_DIALOG_RSINFO_FIELD_COUNT];
    const char             * col_separator_str;

    struct {
        size_t   max_label_width;
        String_t win_title;
    } cache;

    struct {
        const char * (* getDisplayText)( ctune_UI_TextID_e );
    } cb;

} ctune_UI_RSInfo_t;

/**
 * RSInfo namespace
 */
extern const struct ctune_UI_Dialog_RSInfo_Namespace {
    /**
     * Creates a base ctune_UI_RSInfo_t object
     * @param parent         Pointer to size property of the parent window
     * @param getDisplayText Callback method to get text strings for the display
     * @param col_sep        Text to use as separation between the label and field column
     * @return Basic un-initialised ctune_UI_RSInfo_t object
     */
    ctune_UI_RSInfo_t (* create)( const WindowProperty_t * parent, const char * (* getDisplayText)( ctune_UI_TextID_e ), const char * col_sep );

    /**
     * Initialises a ctune_UI_RSInfo_t object
     * @param rsinfo Un-initialised ctune_UI_RSInfo_t object
     * @param mouse_ctrl Flag to turn init mouse controls
     * @return Success
     */
    bool (* init)( ctune_UI_RSInfo_t * rsinfo, bool mouse_ctrl );

    /**
     * Get the initialised state of the instance
     * @param rsinfo Pointer to ctune_UI_RSInfo_t object
     * @return Initialised state
     */
    bool (* isInitialised)( const ctune_UI_RSInfo_t * rsinfo );

    /**
     * Switch mouse control UI on/off
     * @param rsinfo          Pointer to ctune_UI_RSInfo_t object
     * @param mouse_ctrl_flag Flag to turn feature on/off
     */
    void (* setMouseCtrl)( ctune_UI_RSInfo_t * rsinfo, bool mouse_ctrl_flag );

    /**
     * Create and show a populated window with the radio station's info
     * @param rsinfo Pointer to ctune_UI_RSInfo_t object
     * @param title  Title for the RSInfo dialog
     * @param rsi    RadioStationInfo_t object
     * @return Success
     */
    bool (* show)( ctune_UI_RSInfo_t * rsinfo, const char * title, const ctune_RadioStationInfo_t * rsi );

    /**
     * Resize the dialog
     * @param rsinfo Pointer to ctune_UI_RSInfo_t object
     */
    void (* resize)( void * rsinfo );

    /**
     * Pass keyboard input to the form
     * @param rsinfo Pointer to ctune_UI_RSInfo_t object
     */
    void (* captureInput)( ctune_UI_RSInfo_t * rsinfo );

    /**
     * De-allocates resources
     * @param rsinfo Pointer to ctune_UI_RSInfo_t object
     */
    void (* free)( ctune_UI_RSInfo_t * rsinfo );

} ctune_UI_RSInfo;

#endif //CTUNE_UI_DIALOG_RSINFO_H