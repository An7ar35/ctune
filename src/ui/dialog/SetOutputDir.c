#include "SetOutputDir.h"

#include "logger/src/Logger.h"
#include "../definitions/KeyBinding.h"
#include "../definitions/Theme.h"
#include "../../utils/utilities.h"
#include "ContextHelp.h"

typedef enum {
    LABEL_PATH = 0,
    LABEL_COUNT,
} SetOutPath_Label_e;

typedef enum {
    INPUT_PATH = LABEL_COUNT,

    BUTTON_CANCEL,
    BUTTON_SAVE,

    FIELD_LAST,

    FIELD_COUNT,
} SetOutPath_Input_e;

/**
 * [PRIVATE] Check if field is a button
 * @param sop   Pointer to a ctune_UI_SetOutputDir_t object
 * @param field Form field
 * @return Button field status
 */
static bool ctune_UI_SetOutputDir_isButton( ctune_UI_SetOutputDir_t * sop, const FIELD * field ) {
    return field == ctune_UI_Form.field.get( &sop->form, BUTTON_CANCEL )
        || field == ctune_UI_Form.field.get( &sop->form, BUTTON_SAVE   );
}

/**
 * [PRIVATE] Check if given field is a form exit
 * @param sop       Pointer to a ctune_UI_SetOutputDir_t object
 * @param field     FIELD pointer
 * @param exit_type Variable pointer to set the exit type of the field
 * @return Exit state
 */
static bool ctune_UI_SetOutputDir_isExitState( ctune_UI_SetOutputDir_t * sop, const FIELD * field, ctune_FormExit_e * exit_type ) {
    if( ctune_UI_SetOutputDir_isButton( sop, field ) ) {
        if( field == ctune_UI_Form.field.get( &sop->form, BUTTON_CANCEL ) ) {
            *exit_type = CTUNE_UI_FORM_CANCEL;
            return true;
        }

        if( field == ctune_UI_Form.field.get( &sop->form, BUTTON_SAVE ) ) {
            *exit_type = CTUNE_UI_FORM_SUBMIT;
            return true;
        }
    }

    return false;
}

/**
 * [PRIVATE] Highlights input field where the cursor is at
 * @param sop Pointer to a ctune_UI_SetOutputDir_t object
 */
static void ctune_UI_SetOutputDir_highlightCurrField( ctune_UI_SetOutputDir_t * sop ) {
    const FIELD * curr_field       = ctune_UI_Form.field.current( &sop->form );
    const bool    curr_is_button   = ctune_UI_SetOutputDir_isButton( sop, curr_field );
    const bool    curr_is_editable = !( curr_is_button );

    curs_set( ( curr_is_editable ? 1 : 0 ) );

    for( int i = 0; i < LABEL_COUNT; ++i ) {
        ctune_UI_Form.field.setBackground( &sop->form, i, A_NORMAL );
    }

    for( int i = LABEL_COUNT; i < FIELD_COUNT; ++i ) {
        FIELD * field = ctune_UI_Form.field.get( &sop->form, i );

        if( field == curr_field ) {
            ctune_UI_Form.field.setBackground( &sop->form, i, A_REVERSE );

        } else {
            if( ctune_UI_SetOutputDir_isButton( sop, field ) ) {
                ctune_UI_Form.field.setBackground( &sop->form, i, A_NORMAL );
            } else {
                ctune_UI_Form.field.setBackground( &sop->form, i, A_UNDERLINE );
            }
        }
    }
}

/**
 * [PRIVATE] Initialises fields
 * @param sop Pointer to a ctune_UI_SetOutputDir_t object
 * @return Success
 */
static bool ctune_UI_SetOutputDir_initFields( ctune_UI_SetOutputDir_t * sop ) {
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_SetOutputDir_initFields()] Initialising form fields..." );

    ctune_UI_Form.field.setBuffer( &sop->form, LABEL_PATH,    sop->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_OUTPUT_PATH ) );
    ctune_UI_Form.field.setBuffer( &sop->form, INPUT_PATH,    ctune_fallbackStr( sop->cb.getPath(), "" ) );
    ctune_UI_Form.field.setBuffer( &sop->form, BUTTON_CANCEL, sop->cb.getDisplayText( CTUNE_UI_TEXT_BUTTON_CANCEL ) );
    ctune_UI_Form.field.setBuffer( &sop->form, BUTTON_SAVE,   sop->cb.getDisplayText( CTUNE_UI_TEXT_BUTTON_SUBMIT ) );

    ctune_UI_Form.field.setBackground( &sop->form, BUTTON_CANCEL, ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_DFLT ) );
    ctune_UI_Form.field.setForeground( &sop->form, BUTTON_CANCEL, ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_DFLT ) );
    ctune_UI_Form.field.setBackground( &sop->form, BUTTON_SAVE,   ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_DFLT ) );
    ctune_UI_Form.field.setForeground( &sop->form, BUTTON_SAVE,   ctune_UI_Theme.color( CTUNE_UI_ITEM_BUTTON_DFLT ) );

    return true;
}

/**
 * [PRIVATE] Creates the fields (Called once and cached for the remainder of the runtime)
 * @param sop Pointer to a ctune_UI_SetOutputDir_t object
 * @return Success
 */
static bool ctune_UI_SetOutputDir_createFields( ctune_UI_SetOutputDir_t * sop ) {
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_SetOutputDir_createFields( %p )] Creating form fields...", sop );

//      <label_col>
//      |
//      |      <field_col>
//      |      |
//    .--------------[FORM]---------------.
//    |                                   |
//    | Path:  __________________________ |
//    .                                   .
//    .                                   .

    if( sop->cache.max_label_width == 0 ) {
        sop->cache.max_label_width = ctune_max_ul( sop->cache.max_label_width, strlen( sop->cb.getDisplayText( CTUNE_UI_TEXT_LABEL_OUTPUT_PATH ) ) );
    }

    int label_col_width   = 0;
    int order_field_width = 0;

    if( !ctune_utoi( sop->cache.max_label_width, &label_col_width ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_UI_SetOutputDir_createFields( %p )] Failed size_t->int cast of `label_col_width` (%lu).",
                   sop, sop->cache.max_label_width
        );

        return false; //EARLY RETURN
    }

    const int row_height  = 1;
    const int label_col   = 0;
    const int field_col   = ( label_col_width + 2 );
    const int field_width = ( order_field_width > 50 ? order_field_width : 50 );
    const int form_width  = field_col + field_width;

    bool ret[FIELD_LAST]; //errors
    //Field labels                                                                            rows        cols             y  x
    ret[LABEL_PATH] = ctune_UI_Form.field.create( &sop->form, LABEL_PATH, (WindowProperty_t){ row_height, label_col_width, 2, label_col } );
    //Field inputs                                                                            rows        cols             y  x
    ret[INPUT_PATH] = ctune_UI_Form.field.create( &sop->form, INPUT_PATH, (WindowProperty_t){ row_height, field_width,     2, field_col } );

    const int button_separation = 6;
    const int max_button_width  = (int) ctune_max_ul( strlen( sop->cb.getDisplayText( CTUNE_UI_TEXT_BUTTON_SUBMIT ) ),
                                                      strlen( sop->cb.getDisplayText( CTUNE_UI_TEXT_BUTTON_CANCEL ) ) );
    const int button_line_ln    = max_button_width + button_separation + max_button_width;
    int       button_line_pad   = 0;

    if( button_line_ln < form_width ) {
        button_line_pad = ( form_width - button_line_ln ) / 2;
    }
    //Buttons                                                                                         rows        cols            y   x
    ret[BUTTON_CANCEL] = ctune_UI_Form.field.create( &sop->form, BUTTON_CANCEL, (WindowProperty_t){ row_height, max_button_width, 4, button_line_pad } );
    ret[BUTTON_SAVE  ] = ctune_UI_Form.field.create( &sop->form, BUTTON_SAVE,   (WindowProperty_t){ row_height, max_button_width, 4, ( button_line_pad + max_button_width + button_separation ) } );

    for( int i = 0; i < FIELD_LAST; ++i ) {
        if( !ret[i] ) {
            return false; //EARLY RETURN
        }
    }

    ctune_UI_Form.field.setOptions( &sop->form, LABEL_PATH,    O_VISIBLE | O_PUBLIC | O_AUTOSKIP );
    ctune_UI_Form.field.setOptions( &sop->form, INPUT_PATH,    O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE | O_NULLOK );
    ctune_UI_Form.field.setOptions( &sop->form, BUTTON_CANCEL, O_VISIBLE | O_PUBLIC | O_ACTIVE | O_STATIC );
    ctune_UI_Form.field.setOptions( &sop->form, BUTTON_SAVE,   O_VISIBLE | O_PUBLIC | O_ACTIVE | O_STATIC );

    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_SetOutputDir_createFields()] Fields created." );
    return true;
}

/**
 * [PRIVATE] Initialises the Form (callback)
 * @return Success
 */
static bool ctune_UI_SetOutputDir_initForm( void * sop ) {
    ctune_UI_SetOutputDir_t * self = sop;

    if( self->cache.max_label_width == 0 ) {
        if( !ctune_UI_SetOutputDir_createFields( self ) ) {
            CTUNE_LOG( CTUNE_LOG_FATAL,
                       "[ctune_UI_SetOutputDir_initForm( %p )] Failed to create fields for the form.",
                       sop
            );

            return false; //EARLY RETURN
        }
    }

    if( !ctune_UI_SetOutputDir_initFields( self ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_UI_SetOutputDir_initForm( %p )] Failed to init fields for the form.",
                   sop
        );

        return false;
    }

    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_UI_SetOutputDir_initForm( %p )] Form desc field size = %i",
               sop, self->cache.max_label_width
    );

    return true;
}

/**
 * Calls the directory path setter with the value in the field
 * @param sop Pointer to a ctune_UI_SetOutputDir_t object
 * @return Success
 */
static bool setDirectoryPath( ctune_UI_SetOutputDir_t * sop ) {
    bool ok = false;

    if( ctune_UI_Form.field.status( &sop->form, INPUT_PATH ) ) {
        char * path = ctune_trimspace( ctune_UI_Form.field.buffer( &sop->form, INPUT_PATH ) );
        ok = sop->cb.setPath( path );
        free( path );
    }

    if( !ok ) {
        //TODO highlight field in error colour
    }

    return ok;
}

/**
 * Creates a base ctune_UI_SetOutputDir_t object
 * @param parent         Pointer to size property of the parent window
 * @param getDisplayText Callback method to get text strings for the display
 * @param setPath        Callback method to check and set a directory path
 * @param getPath        Callback method to get a currently set directory path
 * @return Basic un-initialised ctune_UI_SetOutputDir_t object
 */
static ctune_UI_SetOutputDir_t ctune_UI_SetOutputDir_create( const WindowProperty_t * parent,
                                                              const char * (* getDisplayText)( ctune_UI_TextID_e ),
                                                              bool         (* setPath)( const char * ),
                                                              const char * (* getPath)( void ) )
{
    return (ctune_UI_SetOutputDir_t) {
        .initialised = false,
        .form        = ctune_UI_Form.create( parent, getDisplayText( CTUNE_UI_TEXT_WIN_TITLE_SETOUTDIR ) ),
        .cache = {
            .max_label_width = 0,
        },
        .cb = {
            .getDisplayText = getDisplayText,
            .setPath        = setPath,
            .getPath        = getPath,
        }
    };
}

/**
 * Initialises SetOutputDir (mostly checks base values are OK)
 * @param sop        Pointer to a ctune_UI_SetOutputDir_t object
 * @param mouse_ctrl Flag to turn init mouse controls
 * @return Success
 */
static bool ctune_UI_SetOutputDir_init( ctune_UI_SetOutputDir_t * sop, bool mouse_ctrl ) {
    if( ctune_UI_Form.init( &sop->form, mouse_ctrl, FIELD_COUNT ) ) {
        ctune_UI_Form.scrolling.setAutoScroll( &sop->form, 2, 15 ); //TODO change that to saner values
        ctune_UI_Form.setFormInitCallback( &sop->form, sop, ctune_UI_SetOutputDir_initForm );
        return ( sop->initialised = true );
    }

    CTUNE_LOG( CTUNE_LOG_ERROR,
               "[ctune_UI_SetOutputDir_init( %p, %s )] Failed to initialise!",
               sop, ( mouse_ctrl ? "true" : "false" )
    );

    return false;
}

/**
 * Get the initialised state of the instance
 * @param sop Pointer to a ctune_UI_SetOutputDir_t object
 * @return Initialised state
 */
static bool ctune_UI_SetOutputDir_isInitialised( ctune_UI_SetOutputDir_t * sop ) {
    return sop->initialised;
}

/**
 * Switch mouse control UI on/off
 * @param sop             Pointer to ctune_UI_SetOutputDir_t object
 * @param mouse_ctrl_flag Flag to turn feature on/off
 */
static void ctune_UI_SetOutputDir_setMouseCtrl( ctune_UI_SetOutputDir_t * sop, bool mouse_ctrl_flag ) {
    ctune_UI_Form.mouse.setMouseCtrl( &sop->form, mouse_ctrl_flag );
}

/**
 * Create and show a populated window with the find form
 * @param sop Pointer to a ctune_UI_SetOutputDir_t object
 * @return Success
 */
static bool ctune_UI_SetOutputDir_show( ctune_UI_SetOutputDir_t * sop ) {
    if( !ctune_UI_Form.display.show( &sop->form ) ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_SetOutputDir_show( %p )] Failed to show Form.", sop );
        return false; //EARLY RETURN
    }

    return true;
}

/**
 * [PRIVATE] Handle a mouse event
 * @param sop        Pointer to a ctune_UI_SetOutputDir_t object
 * @param event      Mouse event mask
 * @param exit_state Pointer to Form exit state variable
 * @return Exit request
 */
static bool ctune_UI_SetOutputDir_handleMouseEvent( ctune_UI_SetOutputDir_t * sop, MEVENT * event, ctune_FormExit_e * exit_state ) {
    const ctune_UI_WinCtrlMask_m win_ctrl = ctune_UI_Form.mouse.isWinCtrl( &sop->form, event->y, event->x );
    const ctune_UI_ScrollMask_m  scroll   = ctune_UI_WinCtrlMask.scrollMask( win_ctrl );

    if( win_ctrl ) {
        if( scroll ) {
            if( event->bstate & BUTTON1_CLICKED ) {
                ctune_UI_Form.scrolling.incrementalScroll( &sop->form, scroll );

            } else if( event->bstate & BUTTON1_DOUBLE_CLICKED ) {
                ctune_UI_Form.scrolling.incrementalScroll( &sop->form, ctune_UI_ScrollMask.setScrollFactor( scroll, 2 ) );

            } else if( event->bstate & BUTTON1_TRIPLE_CLICKED ) {
                ctune_UI_Form.scrolling.incrementalScroll( &sop->form, ctune_UI_ScrollMask.setScrollFactor( scroll, 3 ) );

            } else if( event->bstate & BUTTON3_CLICKED ) {
                ctune_UI_Form.scrolling.edgeScroll( &sop->form, scroll );
            }

        } else if( win_ctrl & CTUNE_UI_WINCTRLMASK_CLOSE ) {
            if( event->bstate & BUTTON1_CLICKED ) {
                return true; //EARLY RETURN
            }
        }

        return false; //EARLY RETURN
    }

    bool exit = false;
    int  pos  = 0;

    FIELD *   prev_selected_field = ctune_UI_Form.field.current( &sop->form );
    FIELD *   clicked_field       = ctune_UI_Form.mouse.click( &sop->form, LABEL_COUNT, FIELD_LAST, event->y, event->x, &pos );

    if( clicked_field ) {
        if( prev_selected_field == clicked_field ) { //same editable field
            ctune_UI_Form.input.fwdToFormDriver( &sop->form, REQ_BEG_FIELD );

            for( int i = 0; i < pos; ++i ) {
                ctune_UI_Form.input.fwdToFormDriver( &sop->form, REQ_RIGHT_CHAR );
            }

        } else if( ctune_UI_SetOutputDir_isButton( sop, clicked_field ) ) {
            ctune_UI_SetOutputDir_highlightCurrField( sop );
            exit = ctune_UI_SetOutputDir_isExitState( sop, clicked_field, exit_state );

        } else {
            ctune_UI_SetOutputDir_highlightCurrField( sop );
        }
    }

    return exit;
}

/**
 * Pass keyboard input to the form
 * @param sop Pointer to a ctune_UI_SetOutputDir_t object
 * @return Form exit state
 */
static ctune_FormExit_e ctune_UI_SetOutputDir_captureInput( ctune_UI_SetOutputDir_t * sop ) {
    bool             exit       = false;
    ctune_FormExit_e exit_state = CTUNE_UI_FORM_ESC;
    int              character;
    MEVENT           mouse_event;

    ctune_UI_Form.input.start( &sop->form );
    ctune_UI_Form.input.fwdToFormDriver( &sop->form, REQ_FIRST_FIELD );
    ctune_UI_SetOutputDir_highlightCurrField( sop );
    ctune_UI_Form.display.refreshView( &sop->form );

    while( !exit ) {
        character = ctune_UI_Form.input.getChar( &sop->form );

        switch( ctune_UI_KeyBinding.getAction( CTUNE_UI_CTX_SETOUTDIR, character ) ) {
            case CTUNE_UI_ACTION_ERR   : //fallthrough
            case CTUNE_UI_ACTION_RESIZE: break;

            case CTUNE_UI_ACTION_HELP: {
                ctune_UI_ContextHelp.show( CTUNE_UI_CTX_SETOUTDIR );
                ctune_UI_ContextHelp.captureInput();
            } break;

            case CTUNE_UI_ACTION_ESC: {
                exit_state = CTUNE_UI_FORM_ESC;
                exit = true;
            } break;

            case CTUNE_UI_ACTION_FIELD_BEGIN: {
                ctune_UI_Form.input.fwdToFormDriver( &sop->form, REQ_BEG_FIELD );
            } break;

            case CTUNE_UI_ACTION_FIELD_END: {
                ctune_UI_Form.input.fwdToFormDriver( &sop->form, REQ_END_FIELD );
            } break;

            case CTUNE_UI_ACTION_FIELD_FIRST: {
                ctune_UI_Form.input.fwdToFormDriver( &sop->form, REQ_FIRST_FIELD );
                ctune_UI_SetOutputDir_highlightCurrField( sop );
                ctune_UI_Form.scrolling.autoscroll( &sop->form );
            } break;

            case CTUNE_UI_ACTION_FIELD_LAST: {
                ctune_UI_Form.input.fwdToFormDriver( &sop->form, REQ_LAST_FIELD );
                ctune_UI_SetOutputDir_highlightCurrField( sop );
                ctune_UI_Form.scrolling.autoscroll( &sop->form );
            } break;

            case CTUNE_UI_ACTION_FIELD_PREV: {
                ctune_UI_Form.input.fwdToFormDriver( &sop->form, REQ_PREV_FIELD );
                ctune_UI_SetOutputDir_highlightCurrField( sop );
                ctune_UI_Form.input.fwdToFormDriver( &sop->form, REQ_END_LINE );
                ctune_UI_Form.scrolling.autoscroll( &sop->form );
            } break;

            case CTUNE_UI_ACTION_FIELD_NEXT: {
                ctune_UI_Form.input.fwdToFormDriver( &sop->form, REQ_NEXT_FIELD );
                ctune_UI_SetOutputDir_highlightCurrField( sop );
                ctune_UI_Form.input.fwdToFormDriver( &sop->form, REQ_END_LINE );
                ctune_UI_Form.scrolling.autoscroll( &sop->form );
            } break;

            case CTUNE_UI_ACTION_GO_LEFT: {
                ctune_UI_Form.input.fwdToFormDriver( &sop->form, REQ_LEFT_CHAR );
            } break;

            case CTUNE_UI_ACTION_GO_RIGHT: {
                ctune_UI_Form.input.fwdToFormDriver( &sop->form, REQ_RIGHT_CHAR );
            } break;

            case CTUNE_UI_ACTION_TRIGGER: {
                if( ( exit = ctune_UI_SetOutputDir_isExitState( sop, ctune_UI_Form.field.current( &sop->form ), &exit_state ) ) ) {
                    if( exit_state == CTUNE_UI_FORM_SUBMIT && !setDirectoryPath( sop ) ) {
                        exit = false;
                    }

                } else {
                    ctune_UI_Form.input.fwdToFormDriver( &sop->form, REQ_NEXT_FIELD );
                    ctune_UI_SetOutputDir_highlightCurrField( sop );
                    ctune_UI_Form.input.fwdToFormDriver( &sop->form, REQ_END_LINE );
                    ctune_UI_Form.scrolling.autoscroll( &sop->form );
                }
            } break;

            case CTUNE_UI_ACTION_TOGGLE_ALT: { //'space'
                if( ( exit = ctune_UI_SetOutputDir_isExitState( sop, ctune_UI_Form.field.current( &sop->form ), &exit_state ) ) ) {
                    if( exit_state == CTUNE_UI_FORM_SUBMIT && !setDirectoryPath( sop ) ) {
                        exit = false;
                    }

                } else {
                    ctune_UI_Form.input.fwdToFormDriver( &sop->form, character );
                }
            } break;

            case CTUNE_UI_ACTION_CLEAR_ALL: {
                ctune_UI_Form.field.clearRange( &sop->form, LABEL_COUNT, FIELD_COUNT );
                ctune_UI_SetOutputDir_initFields( sop );
                ctune_UI_Form.field.setCurrent( &sop->form, LABEL_COUNT );
                ctune_UI_SetOutputDir_highlightCurrField( sop );
            } break;

            case CTUNE_UI_ACTION_CLEAR_SELECTED: {
                ctune_UI_Form.input.fwdToFormDriver( &sop->form, REQ_CLR_FIELD );
            } break;

            case CTUNE_UI_ACTION_DEL_PREV: {
                ctune_UI_Form.input.fwdToFormDriver( &sop->form, REQ_DEL_PREV );
            } break;

            case CTUNE_UI_ACTION_DEL_NEXT: {
                ctune_UI_Form.input.fwdToFormDriver( &sop->form, REQ_DEL_CHAR );
            } break;

            case CTUNE_UI_ACTION_MOUSE_EVENT: {
                if( getmouse( &mouse_event ) == OK ) {
                    if( ( exit = ctune_UI_SetOutputDir_handleMouseEvent( sop, &mouse_event, &exit_state ) ) ) {
                        if( exit_state == CTUNE_UI_FORM_SUBMIT && !setDirectoryPath( sop ) ) {
                            exit = false;
                        }
                    }
                }
            } break;

            default: {
                ctune_UI_Form.input.fwdToFormDriver( &sop->form, character );
            } break;
        }

        ctune_UI_Form.display.refreshView( &sop->form );
    }

    ctune_UI_Form.input.stop( &sop->form );

    return ( exit_state );
}

/**
 * De-allocates the form and its fields
 * @param sop Pointer to a ctune_UI_SetOutputDir_t object
 */
static void ctune_UI_SetOutputDir_free( ctune_UI_SetOutputDir_t * sop ) {
    ctune_UI_Form.freeContent( &sop->form );
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_SetOutputDir_free( %p )] SetOutputDir freed.", sop );
}

/**
 * Namespace constructor
 */
const struct ctune_UI_Dialog_SetOutputDir_Namespace ctune_UI_SetOutputDir = {
    .create        = &ctune_UI_SetOutputDir_create,
    .init          = &ctune_UI_SetOutputDir_init,
    .isInitialised = &ctune_UI_SetOutputDir_isInitialised,
    .setMouseCtrl  = &ctune_UI_SetOutputDir_setMouseCtrl,
    .show          = &ctune_UI_SetOutputDir_show,
    .captureInput  = &ctune_UI_SetOutputDir_captureInput,
    .free          = &ctune_UI_SetOutputDir_free,
};