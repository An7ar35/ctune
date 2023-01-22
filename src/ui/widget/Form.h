#ifndef CTUNE_UI_WIDGET_FORM_H
#define CTUNE_UI_WIDGET_FORM_H

#include <form.h>

#include "Dialog.h"
#include "../enum/TextID.h"

/**
 * UI_Form Widget convenience wrapper for ncurses forms
 */
typedef struct ctune_UI_Widget_Form {
    bool                     initialised;
    const char *             title;
    bool                     mouse_ctrl;
    WindowMargin_t           margins;
    const WindowProperty_t * screen_size;
    WindowProperty_t         form_dimension;
    ctune_UI_Dialog_t        dialog;
    FORM                   * nc_form;

    struct {
        size_t   size;
        FIELD ** arr;
    } fields;

    struct {
        bool (* initNCursesForm)( void * dialog_impl );
    } cb;

    void * form_dialog_impl;

} ctune_UI_Form_t;

/**
 * UI_Form widget namespace methods
 */
extern const struct ctune_UI_Widget_Form_Namespace {
    /**
     * Create a Form_t object
     * @param parent    Pointer to size property of the parent window
     * @param win_title Display title for the window
     * @return Basic un-initialised ctune_UI_Form_t object
     */
    ctune_UI_Form_t (* create)( const WindowProperty_t * parent, const char * win_title );

    /**
     * Initialised a ctune_UI_Form_t object
     * @param form        Pointer to the ctune_UI_Form_t object
     * @param mouse_ctrl  Mouse control flag
     * @param field_count Number of fields in the form (everything will be zeroed after allocation)
     * @return Success
     */
    bool (* init)( ctune_UI_Form_t * form, bool mouse_ctrl, size_t field_count );

    /**
     * Mouse related actions
     */
    struct {
        /**
         * Switch mouse control UI on/off
         * @param form            Pointer to a ctune_UI_Form_t object
         * @param mouse_ctrl_flag Flag to turn feature on/off
         */
        void (* setMouseCtrl)( ctune_UI_Form_t * form, bool mouse_ctrl_flag );

        /**
         * Gets the field under the mouse pointer location and sets it as 'current'
         * @param form   Pointer to a ctune_UI_Form object
         * @param from_i Field index range begin (inclusive)
         * @param to_i   Field index range end (exclusive)
         * @param y      Mouse click row position on screen
         * @param x      Mouse click column position on screen
         * @param pos    Pointer to a integer to store the position of the cursor within the selected field (optional)
         * @return Pointer to clicked field (or NULL)
         */
        FIELD * (* click)( ctune_UI_Form_t * form, size_t from_i, size_t to_i, int y, int x, int * pos );

        /**
         * Checks if area at coordinate is a window control
         * @param form Pointer to a ctune_UI_Form_t object
         * @param y    Row location on screen
         * @param x    Column location on screen
         * @return Window control mask
         */
        ctune_UI_WinCtrlMask_m (* isWinCtrl)( ctune_UI_Form_t * form, int y, int x );

    } mouse;

    /**
     * Scrolling related actions
     */
    struct {
        /**
         * Sets the auto-scroll threshold offset on the form Dialog
         * @param form Pointer to a ctune_UI_Form_t object
         * @param y    Vertical offset
         * @param x    Horizontal offset
         */
        void (* setAutoScroll)( ctune_UI_Form_t * form, int y, int x );

        /**
         * Incrementally scroll the window
         * @param form Pointer to a ctune_UI_Form_t object
         * @param mask Scroll mask
         */
        void (* incrementalScroll)( ctune_UI_Form_t * form, ctune_UI_ScrollMask_m mask );

        /**
         * Scroll to the edge
         * @param form Pointer to a ctune_UI_Form_t object
         * @param mask Scroll mask
         */
        void (* edgeScroll)( ctune_UI_Form_t * form, ctune_UI_ScrollMask_m mask );

        /**
         * Autoscroll to the current field in the form
         * @param form Pointer to a ctune_UI_Form_t object
         */
        void (* autoscroll)( ctune_UI_Form_t * form );

    } scrolling;

    /**
     * Form input control methods
     */
    struct {
        /**
         * Starts input functionality on the form (call prior to forwarding things to the form driver)
         * @param form Pointer to a ctune_UI_Form_t object
         */
        void (* start)( ctune_UI_Form_t * form );

        /**
         * Stops input functionalities on the form
         * @param form Pointer to a ctune_UI_Form_t object
         */
        void (* stop)( ctune_UI_Form_t * form );

        /**
         * Forward a character to the ncurses form driver
         * @param form Pointer to a ctune_UI_Form_t object
         * @param c    Character to forward
         * @return E_OK (Errors: E_BAD_ARGUMENT, E_BAD_STATE, E_NOT_POSTED, E_INVALID_FIELD, E_NOT_CONNECTED, E_REQUEST_DENIED, E_SYSTEM_ERROR, E_UNKNOWN_COMMAND)
         */
        int (* fwdToFormDriver)( ctune_UI_Form_t * form, int c );

        /**
         * Gets a character input from the form's window
         * @param form Pointer to a ctune_UI_Form_t object
         * @return character
         */
        int (* getChar)( ctune_UI_Form_t * form );

    } input;

    /**
     * Form FIELD actions
     */
    struct {
        /**
         * Creates a field
         * @param form       Pointer to a ctune_UI_Form_t object
         * @param field_i    Index of the field to set
         * @param properties Size and placement properties
         * @return Success
         */
        bool (* create)( ctune_UI_Form_t * form, size_t field_i, WindowProperty_t properties );

        /**
         * Gets a FIELD
         * @param form Pointer to a ctune_UI_Form_t object
         * @param i    Index of the field
         * @return Pointer to internally stored FIELD (can be NULL if not set or on error)
         */
        FIELD * (* get)( ctune_UI_Form_t * form, size_t i );

        /**
         * Sets the current active field on the form
         * @param form    Pointer to a ctune_UI_Form_t object
         * @param field_i Index of the field to set
         */
        void (* setCurrent)( ctune_UI_Form_t * form, size_t field_i );

        /**
         * Gets the current field in the form
         * @param form Pointer to a ctune_UI_Form_t object
         * @return Pointer to current FIELD (or NULL on error)
         */
        FIELD * (* current)( ctune_UI_Form_t * form );

        /**
         * Gets the current form field's index
         * @param form Pointer to a ctune_UI_Form_t object
         * @return Index of current field
         */
        int (* currentIndex)( ctune_UI_Form_t * form );

        /**
         * Checks a field is set as current
         * @param form    Pointer to a ctune_UI_Form_t object
         * @param field_i Index of the field to check
         * @return Current state
         */
        bool (* isCurrent)( ctune_UI_Form_t * form, size_t field_i );

        /**
         * Set a field's options
         * @param form    Pointer to a ctune_UI_Form_t object
         * @param field_i Index of the field to set
         * @param opts    Option mask
         */
        void (* setOptions)( ctune_UI_Form_t * form, size_t field_i, Field_Options opts );

        /**
         * Sets the field's display buffer (0)
         * @param form    Pointer to a ctune_UI_Form_t object
         * @param field_i Index of the field to set
         * @param value   Value to set
         * @return E_OK (Errors: E_BAD_ARGUMENT, E_SYSTEM_ERROR)
         */
        int (* setBuffer)( ctune_UI_Form_t * form, size_t field_i, const char * value );

        /**
         * Gets the internal field's buffer (0)
         * @param form    Pointer to a ctune_UI_Form_t object
         * @param field_i Index of the field to set
         * @return Pointer to field buffer
         */
        char * (* buffer)( ctune_UI_Form_t * form, size_t field_i );

        /**
         * Gets the modified status of a field
         * @param form    Pointer to a ctune_UI_Form_t object
         * @param field_i Index of the field to set
         * @return Changed state
         */
        bool (* status)( ctune_UI_Form_t * form, size_t field_i );

        /**
         * Set the field's background colour attributes
         * @param form    Pointer to a ctune_UI_Form_t object
         * @param field_i Index of the field to set
         * @param attr    Field attributes
         * @return
         */
        int (* setBackground)( ctune_UI_Form_t * form, size_t field_i, chtype attr );

        /**
         * Set the field's background colour attributes
         * @param form    Pointer to a ctune_UI_Form_t object
         * @param field_i Index of the field to set
         * @param attr    Field attributes
         * @return E_OK (Errors: E_BAD_ARGUMENT, E_SYSTEM_ERROR)
         */
        int (* setForeground)( ctune_UI_Form_t * form, size_t field_i, chtype attr );

        /**
         * Clears all fields in a range
         * @param form   Pointer to a ctune_UI_Form_t object
         * @param from_i Index of the beginning of the range to clear (inclusive)
         * @param to_i   Index of the end of the range to clear (exclusive)
         * @return Success
         */
        bool (* clearRange)( ctune_UI_Form_t * form, int from_i, int to_i );

    } field;

    /**
     * Display related methods
     */
    struct {
        /**
         * Create and show a populated window with the find form
         * @param form Pointer to a ctune_UI_Form_t object
         * @return Success
         */
        bool (* show)( ctune_UI_Form_t * form );

        /**
         * Redraws the dialog
         * @param form Pointer to a ctune_UI_Form_t object
         */
        void (* resize)( void * form );

        /**
         * Refresh form's dialog pad
         * @param form Pointer to a ctune_UI_Form_t object
         */
        void (* refreshView)( ctune_UI_Form_t * form );

    } display;

    /**
     * Sets the callback to use for building the ncurses form
     * @param form     Pointer to a ctune_UI_Form_t object
     * @param data     Pointer to the form dialog implementation to pass to the callback as an argument
     * @param callback Callback method
     */
    void (* setFormInitCallback)( ctune_UI_Form_t * form, void * data, bool (* callback)( void * ) );

    /**
     * De-allocates content from memory
     * @param form Pointer to a ctune_UI_Form_t object
     */
    void (* freeContent)( void * form );

} ctune_UI_Form;


#endif //CTUNE_UI_WIDGET_FORM_H