#ifndef CTUNE_UI_DIALOG_SETOUTPUTDIR_H
#define CTUNE_UI_DIALOG_SETOUTPUTDIR_H

#include <ncurses.h>

#include "../../datastructure/String.h"
#include "../enum/FormExit.h"
#include "../widget/Form.h"

typedef struct ctune_UI_Dialog_SetOutputDir {
    bool            initialised;
    ctune_UI_Form_t form;

    struct {
        size_t max_label_width;
    } cache;

    struct {
        const char * (* getDisplayText)( ctune_UI_TextID_e );
        bool         (* setPath)( const char * );
        const char * (* getPath)( void );
    } cb;
} ctune_UI_SetOutputDir_t;

/**
 * SetOutputDir namespace
 */
extern const struct ctune_UI_Dialog_SetOutputDir_Namespace {
    /**
     * Creates a base ctune_UI_SetOutputDir_t object
     * @param parent         Pointer to size property of the parent window
     * @param getDisplayText Callback method to get text strings for the display
     * @param setPath        Callback method to check and set a directory path
     * @param getPath        Callback method to get a currently set directory path
     * @return Basic un-initialised ctune_UI_SetOutputDir_t object
     */
    ctune_UI_SetOutputDir_t (* create)( const WindowProperty_t * parent,
                                        const char * (* getDisplayText)( ctune_UI_TextID_e ),
                                        bool         (* setPath)( const char * ),
                                        const char * (* getPath)( void ) );

    /**
     * Initialises SetOutputPath (mostly checks base values are OK)
     * @param sop        Pointer to a ctune_UI_SetOutputDir_t object
     * @param mouse_ctrl Flag to turn init mouse controls
     * @return Success
     */
    bool (* init)( ctune_UI_SetOutputDir_t * sop, bool mouse_ctrl );

    /**
     * Get the initialised state of the instance
     * @param sop Pointer to a ctune_UI_SetOutputDir_t object
     * @return Initialised state
     */
    bool (* isInitialised)( ctune_UI_SetOutputDir_t * sop );

    /**
     * Switch mouse control UI on/off
     * @param sop             Pointer to ctune_UI_SetOutputDir_t object
     * @param mouse_ctrl_flag Flag to turn feature on/off
     */
    void (* setMouseCtrl)( ctune_UI_SetOutputDir_t * sop, bool mouse_ctrl_flag );

    /**
     * Create and show a populated window with the find form
     * @param sop Pointer to a ctune_UI_SetOutputDir_t object
     * @return Success
     */
    bool (* show)( ctune_UI_SetOutputDir_t * sop );

    /**
     * Pass keyboard input to the form
     * @param sop Pointer to a ctune_UI_SetOutputDir_t object
     * @return Form exit state
     */
    ctune_FormExit_e (* captureInput)( ctune_UI_SetOutputDir_t * sop );

    /**
     * De-allocates the form and its fields
     * @param sop Pointer to a ctune_UI_SetOutputDir_t object
     */
    void (* free)( ctune_UI_SetOutputDir_t * sop );

} ctune_UI_SetOutputDir;

#endif //CTUNE_UI_DIALOG_SETOUTPUTDIR_H