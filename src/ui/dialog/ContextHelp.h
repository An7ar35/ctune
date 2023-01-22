#ifndef CTUNE_UI_DIALOG_CONTEXTHELP_H
#define CTUNE_UI_DIALOG_CONTEXTHELP_H

#include <stdbool.h>
#include <sys/ioctl.h>

#include "../enum/ContextID.h"
#include "../datastructure/WindowProperty.h"
#include "../definitions/Language.h"
#include "../widget/ScrollWin.h"

/**
 * Creates (`.init(..)`) and holds all the contextual help
 * popup windows ready to be displayed (`.show(..)`)
 */
extern const struct ctune_UI_ContextHelp_Instance {
    /**
     * Initialises ContextHelp dialog content
     * @param parent         Pointer to parent window size properties
     * @param getDisplayText Callback method to get text strings for fields
     */
    bool (* init)( const WindowProperty_t * parent, const char * (* getDisplayText)( ctune_UI_TextID_e ) );

    /**
     * Switch mouse control UI on/off
     * @param mouse_ctrl_flag Flag to turn feature on/off
     */
    void (* setMouseCtrl)( bool mouse_ctrl_flag );

    /**
     * Show the help dialog box for the given context
     * @param ctx Context ID enum
     */
    void (* show)( ctune_UI_Context_e ctx );

    /**
     * Resize current help context
     */
    void (* resize)();

    /**
     * Pass keyboard input to the form
     */
    void (* captureInput)( void );

    /**
     * De-allocates resources
     */
    void (* free)( void );

} ctune_UI_ContextHelp;

#endif //CTUNE_UI_DIALOG_CONTEXTHELP_H
