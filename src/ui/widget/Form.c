#include "Form.h"

#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "logger/src/Logger.h"
#include "../../ctune_err.h"
#include "../Resizer.h"

/**
 * [PRIVATE]
 * @param form
 * @return
 */
static bool ctune_UI_Widget_Form_initForm( ctune_UI_Form_t * form ) {
    if( form->nc_form != NULL ) {
        unpost_form( form->nc_form );
        free_form( form->nc_form );
        form->nc_form = NULL;
    }

    if( form->cb.initNCursesForm == NULL || form->form_dialog_impl == NULL || !form->cb.initNCursesForm( form->form_dialog_impl ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_Widget_Form_initForm( %p )] Could not initialise ncurses form (callback = `%p( %p )`).",
                   form, form->cb.initNCursesForm, form->form_dialog_impl
        );

        return false; //EARLY RETURN
    }

    if( ( form->nc_form = new_form( form->fields.arr ) ) == NULL ) {
        switch( errno ) {
            case E_BAD_ARGUMENT:
                CTUNE_LOG( CTUNE_LOG_FATAL,
                           "[ctune_UI_Widget_Form_initForm( %p )] "
                           "Failed to create Form: Routine detected an incorrect or out-of-range argument.",
                           form
                );
                break;

            case E_CONNECTED:
                CTUNE_LOG( CTUNE_LOG_FATAL,
                           "[ctune_UI_Widget_Form_initForm( %p )] "
                           "Failed to create Form: The field is already connected to a form.",
                           form
                );
                break;

            case E_SYSTEM_ERROR:
                CTUNE_LOG( CTUNE_LOG_FATAL,
                           "[ctune_UI_Widget_Form_initForm( %p )] "
                           "Failed to create Form: System error occurred, e.g., malloc failure.",
                           form
                );
                break;

            default:
                CTUNE_LOG( CTUNE_LOG_FATAL,
                           "[ctune_UI_Widget_Form_initForm( %p )] "
                           "Failed to create Form: unknown error.",
                           form
                );
        }

        return false; //EARLY RETURN
    }

    scale_form( form->nc_form, &form->form_dimension.rows, &form->form_dimension.cols );

    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_UI_Widget_Form_initForm( %p )] Form size calculated as: rows = %i, cols = %i",
               form, form->form_dimension.rows, form->form_dimension.cols
    );

    return true;
}

/**
 * Create a Form_t object
 * @param parent    Pointer to size property of the parent window
 * @param win_title Display title for the window
 * @return Basic un-initialised ctune_UI_Form_t object
 */
static ctune_UI_Form_t ctune_UI_Widget_Form_create( const WindowProperty_t * parent, const char * win_title  ) {
    return (ctune_UI_Form_t) {
        .initialised    = false,
        .title          = win_title,
        .mouse_ctrl     = false,
        .margins        = { 0, 1, 1, 1 },
        .screen_size    = parent,
        .form_dimension = { 0, 0, 0, 0 },
        .dialog         = ctune_UI_Dialog.init(),
        .nc_form        = NULL,
        .fields = {
            .size = 0,
            .arr  = NULL,
        },
        .cb = {
            .initNCursesForm = NULL,
        },
        .form_dialog_impl = NULL,
    };
}

/**
 * Initialised a ctune_UI_Form_t object
 * @param form        Pointer to the ctune_UI_Form_t object
 * @param mouse_ctrl  Mouse control flag
 * @param field_count Number of fields in the form (everything will be zeroed after allocation)
 * @return Success
 */
static bool ctune_UI_Widget_Form_init( ctune_UI_Form_t * form, bool mouse_ctrl, size_t field_count ) {
    bool error_state = false;

    if( form->initialised == true ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_Widget_Form_init( %p, %s, %lu )] Form has already been initialised!",
                   form, ( mouse_ctrl ? "true" : "false" ), field_count
        );

        error_state = true;
        goto end;
    }

    if( form->screen_size == NULL ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_UI_Widget_Form_init( %p, %s, %lu )] Pointer to screen size is NULL.",
                   form, ( mouse_ctrl ? "true" : "false" ), field_count
        );

        error_state = true;
        goto end;
    }

    if( ( form->fields.arr = malloc( sizeof( FIELD * ) * field_count ) ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_UI_Widget_Form_init( %p, %s, %lu )] Failed to allocate memory for the 'FIELD *' array.",
                   form, ( mouse_ctrl ? "true" : "false" ), field_count
        );

        error_state = true;
        goto end;
    }

    for( size_t i = 0; i < field_count; ++i ) { //init everything to 0 (note: not using `calloc` as does not guarantee pointer to be NULLed)
        form->fields.arr[i] = NULL;
    }

    form->fields.size = field_count;
    ctune_UI_Form.mouse.setMouseCtrl( form, mouse_ctrl );
    form->initialised = true;

    end:
        return !( error_state );
}


/**
 * Switch mouse control UI on/off
 * @param form            Pointer to a ctune_UI_Form_t object
 * @param mouse_ctrl_flag Flag to turn feature on/off
 */
static void ctune_UI_Widget_Form_mouse_setMouseCtrl( ctune_UI_Form_t * form, bool mouse_ctrl_flag ) {
    form->mouse_ctrl = mouse_ctrl_flag;
}

/**
 * Gets the field under the mouse pointer location and sets it as 'current'
 * @param form   Pointer to a ctune_UI_Form object
 * @param from_i Field index range begin (inclusive)
 * @param to_i   Field index range end (exclusive)
 * @param y      Mouse click row position on screen
 * @param x      Mouse click column position on screen
 * @param pos    Pointer to a integer to store the position of the cursor within the selected field (optional)
 * @return Pointer to clicked field (will be set as 'current') or NULL
 */
static FIELD * ctune_UI_Widget_Form_mouse_click( ctune_UI_Form_t * form, size_t from_i, size_t to_i, int y, int x, int * pos ) {
    if( from_i >= to_i || to_i >= form->fields.size ) {
        return NULL; //EARLY RETURN
    }

    int row = y;
    int col = x;

    if( wmouse_trafo( form->dialog.canvas.pad, &row, &col, false ) ) {
        const WindowProperty_t pad_view = ctune_UI_Dialog.getViewProperty( &form->dialog );
        const int              pad_row  = pad_view.pos_y + row;
        const int              pad_col  = pad_view.pos_x + col;

        for( size_t i = from_i; i < to_i; ++i ) {
            FIELD * field = form->fields.arr[ i ];

            if( field ) {
                int field_h = 0;
                int field_w = 0;
                int field_y = 0;
                int field_x = 0;

                field_info( form->fields.arr[ i ], &field_h, &field_w, &field_y, &field_x, NULL, NULL );

                if( pad_row >= field_y && pad_row <= ( field_y + field_h )
                 && pad_col >= field_x && pad_col <= ( field_x + field_w ) )
                {
                    if( pos ) {
                        ( *pos ) = ( pad_col - field_x );
                    }

                    CTUNE_LOG( CTUNE_LOG_TRACE,
                               "[ctune_UI_Widget_Form_mouse_click( %p, %lu, %lu, %i, %i, %p )] "
                               "Field %i clicked { field pos: %i, view box = (%i, %i), pad = (%i, %i) }",
                               form, from_i, to_i, y, x, pos,
                               i, ( pos ? *pos : -1 ), row, col, pad_row, pad_col
                    );

                    ctune_UI_Form.field.setCurrent( form, i );

                    return form->fields.arr[ i ];
                }
            }
        }
    }

    return NULL;
}

/**
 * Sets the auto-scroll threshold offset on the form Dialog
 * @param form Pointer to a ctune_UI_Form_t object
 * @param y    Vertical offset
 * @param x    Horizontal offset
 */
static void ctune_UI_Widget_Form_scrolling_setAutoScroll( ctune_UI_Form_t * form, int y, int x ) {
    ctune_UI_Dialog.setAutoScrollOffset( &form->dialog, y, x );
}

/**
 * Incrementally scroll the window
 * @param form Pointer to a ctune_UI_Form_t object
 * @param mask Scroll mask
 */
static void ctune_UI_Widget_Form_scrolling_incrementalScroll( ctune_UI_Form_t * form, ctune_UI_ScrollMask_m mask ) {
    ctune_UI_Dialog.incrementalScroll( &form->dialog, mask );
}

/**
 * Scroll to the edge
 * @param form Pointer to a ctune_UI_Form_t object
 * @param mask Scroll mask
 */
static void ctune_UI_Widget_Form_scrolling_edgeScroll( ctune_UI_Form_t * form, ctune_UI_ScrollMask_m mask ) {
    ctune_UI_Dialog.edgeScroll( &form->dialog, mask );
}

/**
 * Autoscroll to the current field in the form
 * @param form Pointer to a ctune_UI_Form_t object
 */
static void ctune_UI_Widget_Form_scrolling_autoscroll( ctune_UI_Form_t * form ) {
    int field_pos_y = 0;
    int field_pos_x = 0;

    if( field_info( current_field( form->nc_form ), NULL, NULL, &field_pos_y, &field_pos_x, NULL, NULL ) != E_OK ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_Widget_Form_scrolling_autoscroll( %p )] Failed to get info for field.", form );
        return; //EARLY RETURN
    }

    ctune_UI_Dialog.autoScroll( &form->dialog, field_pos_y, field_pos_x );
}

/**
 * Checks if area at coordinate is a window control
 * @param form Pointer to a ctune_UI_Form_t object
 * @param y    Row location on screen
 * @param x    Column location on screen
 * @return Window control mask
 */
static ctune_UI_WinCtrlMask_m ctune_UI_Widget_Form_mouse_isWinCtrl( ctune_UI_Form_t * form, int y, int x ) {
    return ctune_UI_Dialog.isWinControl( &form->dialog, y, x );
}

/**
 * Starts input functionality on the form (call prior to forwarding things to the form driver)
 * @param form Pointer to a ctune_UI_Form_t object
 */
static void ctune_UI_Widget_Form_input_start( ctune_UI_Form_t * form ) {
    keypad( form->dialog.canvas.pad, TRUE );
}

/**
 * Stops input functionalities on the form
 * @param form Pointer to a ctune_UI_Form_t object
 */
static void ctune_UI_Widget_Form_input_stop( ctune_UI_Form_t * form ) {
    ctune_UI_Dialog.hide( &form->dialog );
    ctune_UI_Resizer.pop();
}

/**
 * Forward a character to the ncurses form driver
 * @param form Pointer to a ctune_UI_Form_t object
 * @param c    Character to forward
 * @return E_OK (Errors: E_BAD_ARGUMENT, E_BAD_STATE, E_NOT_POSTED, E_INVALID_FIELD, E_NOT_CONNECTED, E_REQUEST_DENIED, E_SYSTEM_ERROR, E_UNKNOWN_COMMAND)
 */
static int ctune_UI_Widget_Form_input_fwdToFormDriver( ctune_UI_Form_t * form, int c ) {
    return form_driver( form->nc_form, c );
}

/**
 * Gets a character input from the form's window
 * @param form Pointer to a ctune_UI_Form_t object
 * @return character
 */
static int ctune_UI_Widget_Form_input_getChar( ctune_UI_Form_t * form ) {
    return wgetch( form->dialog.canvas.pad );
}

/**
 * Creates a field
 * @param form       Pointer to a ctune_UI_Form_t object
 * @param field_i    Index of the field to set
 * @param properties Size and placement properties
 * @return Success
 */
static bool ctune_UI_Widget_Form_field_createField( ctune_UI_Form_t * form, size_t field_i, WindowProperty_t properties ) {
    bool error_state = false;

    if( field_i < form->fields.size ) {
        if( ( form->fields.arr[field_i] = new_field( properties.rows, properties.cols, properties.pos_y, properties.pos_x, 0, 0 ) ) == NULL ) {
            CTUNE_LOG( CTUNE_LOG_FATAL,
                       "[ctune_UI_Widget_Form_field_createField( %p, %lu, { %i, %i, %i, %i } )] Failed to create Field (#%i): %s",
                       form, field_i, properties.rows, properties.cols, properties.pos_y, properties.pos_x, field_i, strerror( errno )
            );

            error_state = true;
        }

    } else {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_UI_Widget_Form_field_createField( %p, %lu, { %i, %i, %i, %i } )] Failed to create Field (#%i): index >= length (%lu)",
                   form, field_i, properties.rows, properties.cols, properties.pos_y, properties.pos_x, field_i, form->fields.size
        );

        error_state = true;
    }

    return !( error_state );
}

/**
 * Gets a FIELD
 * @param form Pointer to a ctune_UI_Form_t object
 * @param i    Index of the field
 * @return Pointer to internally stored FIELD (can be NULL if not set or on error)
 */
static FIELD * ctune_UI_Widget_Form_field_get( ctune_UI_Form_t * form, size_t i ) {
    if( i < form->fields.size ) {
        return form->fields.arr[i]; //EARLY RETURN
    }

    CTUNE_LOG( CTUNE_LOG_ERROR,
               "[ctune_UI_Widget_Form_field_get( %p, %lu )] Field index out of bounds (size: %lu).",
               form, i, form->fields.size
    );

    return NULL;
}

/**
 * Clears all fields in a range
 * @param form   Pointer to a ctune_UI_Form_t object
 * @param from_i Index of the beginning of the range to clear
 * @param to_i   Index of the end of the range to clear
 * @return Success
 */
static bool ctune_UI_Widget_Form_field_clearRange( ctune_UI_Form_t * form, int from_i, int to_i ) {
    if( from_i >= 0 && to_i >= 0 && from_i <= to_i ) {
        for( int i = from_i; i < to_i; ++i ) {
            set_current_field( form->nc_form, form->fields.arr[ i ] );
            form_driver( form->nc_form, REQ_CLR_FIELD );
        }

        return true; //EARLY RETURN

    }

    CTUNE_LOG( CTUNE_LOG_ERROR,
               "[ctune_UI_Widget_Form_field_clearRange( %p, %i, %i )] Invalid index/ices given (field count: %lu).",
               form, from_i, to_i, form->fields.size
    );

    return false;
}

/**
 * Sets the current active field on the form
 * @param form    Pointer to a ctune_UI_Form_t object
 * @param field_i Index of the field to set
 */
static void ctune_UI_Widget_Form_field_setCurrent( ctune_UI_Form_t * form, size_t field_i ) {
    if( field_i < form->fields.size ) {
        set_current_field( form->nc_form, form->fields.arr[field_i] );
    }
}

/**
 * Gets the current field in the form
 * @param form Pointer to a ctune_UI_Form_t object
 * @return Pointer to current FIELD (or NULL on error)
 */
static FIELD * ctune_UI_Widget_Form_field_currentField( ctune_UI_Form_t * form ) {
    return current_field( form->nc_form );
}

/**
 * Gets the current form field's index
 * @param form Pointer to a ctune_UI_Form_t object
 * @return Index of current field
 */
static int ctune_UI_Widget_Form_field_currentIndex( ctune_UI_Form_t * form ) {
    return field_index( current_field( form->nc_form ) );
}

/**
 * Checks a field is set as current
 * @param form    Pointer to a ctune_UI_Form_t object
 * @param field_i Index of the field to check
 * @return Current state
 */
static bool ctune_UI_Widget_Form_field_isCurrentField( ctune_UI_Form_t * form, size_t field_i ) {
    if( field_i < form->fields.size ) {
        return ( current_field( form->nc_form ) == form->fields.arr[field_i] );
    }

    return false;
}

/**
 * Set a field's options
 * @param form    Pointer to a ctune_UI_Form_t object
 * @param field_i Index of the field to set
 * @param opts    Option mask
 */
static void ctune_UI_Widget_Form_field_setOptions( ctune_UI_Form_t * form, size_t field_i, Field_Options opts ) {
    if( field_i < form->fields.size ) {
        set_field_opts( form->fields.arr[field_i], opts );
    }
}

/**
 * Sets the field's display buffer (0)
 * @param form    Pointer to a ctune_UI_Form_t object
 * @param field_i Index of the field to set
 * @param value   Value to set
 * @return E_OK (Errors: E_BAD_ARGUMENT, E_SYSTEM_ERROR)
 */
static int ctune_UI_Widget_Form_field_setBuffer( ctune_UI_Form_t * form, size_t field_i, const char * value ) {
    if( field_i < form->fields.size ) {
        return set_field_buffer( form->fields.arr[field_i], 0, value );
    }

    return E_BAD_ARGUMENT;
}

/**
 * Gets the internal field's buffer (0)
 * @param field   Pointer to a ctune_UI_Form_t object
 * @param field_i Index of the field to set
 * @return Pointer to field buffer (NULL on error)
 */
static char * ctune_UI_Widget_Form_field_buffer( ctune_UI_Form_t * form, size_t field_i ) {
    if( field_i < form->fields.size ) {
        return field_buffer( form->fields.arr[field_i], 0 );
    }

    return NULL;
}

/**
 * Gets the modified status of a field
 * @param field   Pointer to a ctune_UI_Form_t object
 * @param field_i Index of the field to set
 * @return Changed state
 */
static bool ctune_UI_Widget_Form_field_status( ctune_UI_Form_t * form, size_t field_i ) {
    if( field_i < form->fields.size ) {
        return field_status( form->fields.arr[field_i] );
    }

    return false;
}

/**
 * Set the field's background colour attributes
 * @param field   Pointer to a ctune_UI_Form_t object
 * @param field_i Index of the field to set
 * @param attr    Field attributes
 * @return E_OK (Errors: E_BAD_ARGUMENT, E_SYSTEM_ERROR)
 */
static int ctune_UI_Widget_Form_field_setBackground( ctune_UI_Form_t * form, size_t field_i, chtype attr ) {
    if( field_i < form->fields.size ) {
        return set_field_back( form->fields.arr[field_i], attr );
    }

    return E_BAD_ARGUMENT;
}

/**
 * Set the field's background colour attributes
 * @param field   Pointer to a ctune_UI_Form_t object
 * @param field_i Index of the field to set
 * @param attr    Field attributes
 * @return E_OK (Errors: E_BAD_ARGUMENT, E_SYSTEM_ERROR)
 */
static int ctune_UI_Widget_Form_field_setForeground( ctune_UI_Form_t * form, size_t field_i, chtype attr ) {
    if( field_i < form->fields.size ) {
        return set_field_fore( form->fields.arr[field_i], attr );
    }

    return E_BAD_ARGUMENT;
}

/**
 * Create and show a populated window with the find form (call from implementation dialog only)
 * @param form Pointer to a ctune_UI_Form_t object
 * @return Success
 */
static bool ctune_UI_Widget_Form_display_show( ctune_UI_Form_t * form ) {
    if( !form->initialised ) {
        ctune_err.set( CTUNE_ERR_UI );
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_Widget_Form_display_show( %p )] Form not initialised prior.", form );
        return false; //EARLY RETURN
    }

    if( !ctune_UI_Widget_Form_initForm( form ) ) {
        ctune_err.set( CTUNE_ERR_UI );
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_Widget_Form_display_show( %p )] Failed to initialise ncurses form",
                   form
        );

        return false; //EARLY RETURN
    }

    ctune_UI_Dialog.free( &form->dialog );

    ctune_UI_Dialog.createScrollWin( &form->dialog,
                                     form->form_dimension.rows,
                                     form->form_dimension.cols );

    ctune_UI_Dialog.createBorderWin( &form->dialog,
                                     form->screen_size,
                                     form->title,
                                     &form->margins,
                                     form->mouse_ctrl );

    set_form_win( form->nc_form, form->dialog.border_win.window );
    set_form_sub( form->nc_form, form->dialog.canvas.pad );
    post_form( form->nc_form );

    ctune_UI_Dialog.show( &form->dialog );
    ctune_UI_Dialog.refreshView( &form->dialog );
    ctune_UI_Resizer.push( ctune_UI_Form.display.resize, form );
    doupdate();

    return true;
}

/**
 * Redraws the dialog (call from implementation dialog only)
 * @param form Pointer to a ctune_UI_Form_t object
 */
static void ctune_UI_Widget_Form_display_resize( void * form ) {
    if( form == NULL ) {
        ctune_err.set( CTUNE_ERR_UI );
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_Widget_Form_display_resize( %p )] Form is NULL.", form );
        return; //EARLY RETURN
    }

    ctune_UI_Form_t * form_dialog = form;

    CTUNE_LOG( CTUNE_LOG_TRACE,
               "[ctune_UI_Widget_Form_display_resize( %p )] Resize event called for dialog: \"%s\"",
               form, form_dialog->title
    );

    if( !form_dialog->initialised ) {
        ctune_err.set( CTUNE_ERR_UI );
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_Widget_Form_display_resize( %p )] Form is not initialised.", form_dialog );
        false; //EARLY RETURN
    }

    ctune_UI_Dialog.edgeScroll( &form_dialog->dialog, CTUNE_UI_SCROLL_TO_HOME );

    ctune_UI_Dialog.createBorderWin( &form_dialog->dialog,
                                     form_dialog->screen_size,
                                     form_dialog->title,
                                     &form_dialog->margins,
                                     form_dialog->mouse_ctrl );

    set_form_win( form_dialog->nc_form, form_dialog->dialog.border_win.window );
    set_form_sub( form_dialog->nc_form, form_dialog->dialog.canvas.pad );
    post_form( form_dialog->nc_form );

    ctune_UI_Dialog.show( &form_dialog->dialog );
    ctune_UI_Dialog.refreshView( &form_dialog->dialog );
}

/**
 * Refresh form's dialog pad
 * @param form Pointer to a ctune_UI_Form_t object
 */
static void ctune_UI_Widget_Form_display_refreshView( ctune_UI_Form_t * form ) {
    ctune_UI_ScrollWin.refreshView( &form->dialog.canvas );
}


/**
 * Sets the callback to use for building the ncurses form
 * @param form     Pointer to a ctune_UI_Form_t object
 * @param data     Pointer to the form dialog implementation to pass to the callback as an argument
 * @param callback Callback method
 */
static void ctune_UI_Widget_Form_setNCursesFormInitCallback( ctune_UI_Form_t * form, void * data, bool (* callback)( void * ) ) {
    if( form ) {
        form->cb.initNCursesForm = callback;
        form->form_dialog_impl   = data;
    }
}

/**
 * De-allocates content from memory
 * @param form Pointer to a ctune_UI_Form_t object
 */
static void ctune_UI_Widget_Form_freeContent( void * form ) {
    if( form ) {
        ctune_UI_Form_t * ptr = form;

        unpost_form( ptr->nc_form );
        free_form( ptr->nc_form );

        if( ptr->fields.arr ) {
            for( size_t i = 0; i < ptr->fields.size; ++i ) {
                free_field( ptr->fields.arr[ i ] );
            }

            free( ptr->fields.arr );
            ptr->fields.arr  = NULL;
            ptr->fields.size = 0;
        }

        ptr->initialised        = false;
        ptr->cb.initNCursesForm = NULL;
        ptr->form_dialog_impl   = NULL;

        ctune_UI_Dialog.free( &ptr->dialog );

        CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_Widget_Form_freeContent( %p )] Form freed.", form );
    }
}


/**
 * Namespace constructor
 */
const struct ctune_UI_Widget_Form_Namespace ctune_UI_Form = {
    .create                = &ctune_UI_Widget_Form_create,
    .init                  = &ctune_UI_Widget_Form_init,
    .mouse = {
        .setMouseCtrl      = &ctune_UI_Widget_Form_mouse_setMouseCtrl,
        .click             = &ctune_UI_Widget_Form_mouse_click,
        .isWinCtrl         = &ctune_UI_Widget_Form_mouse_isWinCtrl,
    },
    .scrolling = {
        .setAutoScroll     = &ctune_UI_Widget_Form_scrolling_setAutoScroll,
        .incrementalScroll = &ctune_UI_Widget_Form_scrolling_incrementalScroll,
        .edgeScroll        = &ctune_UI_Widget_Form_scrolling_edgeScroll,
        .autoscroll        = &ctune_UI_Widget_Form_scrolling_autoscroll,
    },
    .input = {
        .start             = &ctune_UI_Widget_Form_input_start,
        .stop              = &ctune_UI_Widget_Form_input_stop,
        .fwdToFormDriver   = &ctune_UI_Widget_Form_input_fwdToFormDriver,
        .getChar           = &ctune_UI_Widget_Form_input_getChar,
    },
    .field = {
        .create            = &ctune_UI_Widget_Form_field_createField,
        .get               = &ctune_UI_Widget_Form_field_get,
        .setCurrent        = &ctune_UI_Widget_Form_field_setCurrent,
        .current           = &ctune_UI_Widget_Form_field_currentField,
        .currentIndex      = &ctune_UI_Widget_Form_field_currentIndex,
        .isCurrent         = &ctune_UI_Widget_Form_field_isCurrentField,
        .setOptions        = &ctune_UI_Widget_Form_field_setOptions,
        .clearRange        = &ctune_UI_Widget_Form_field_clearRange,
        .setBuffer         = &ctune_UI_Widget_Form_field_setBuffer,
        .buffer            = &ctune_UI_Widget_Form_field_buffer,
        .status            = &ctune_UI_Widget_Form_field_status,
        .setBackground     = &ctune_UI_Widget_Form_field_setBackground,
        .setForeground     = &ctune_UI_Widget_Form_field_setForeground,
    },
    .display = {
        .show              = &ctune_UI_Widget_Form_display_show,
        .resize            = &ctune_UI_Widget_Form_display_resize,
        .refreshView       = &ctune_UI_Widget_Form_display_refreshView,
    },
    .setFormInitCallback   = &ctune_UI_Widget_Form_setNCursesFormInitCallback,
    .freeContent           = &ctune_UI_Widget_Form_freeContent,
};