#include "ContextHelp.h"

#include <panel.h>

#include "logger/src/Logger.h"
#include "../../datastructure/Vector.h"
#include "../../utils/utilities.h"
#include "../definitions/KeyBinding.h"
#include "../definitions/Theme.h"
#include "../datastructure/WindowMargin.h"
#include "../widget/Dialog.h"
#include "../Resizer.h"

/**
 * Internal and cached variables
 */
static struct {
    bool                     init;
    bool                     mouse_ctrl;
    const int                col_padding; //space between the key and description columns
    const WindowProperty_t * screen_size;
    const WindowMargin_t     margins;     //space around the content

    struct {
        ctune_UI_Context_e curr_ctx; //current active context
        ctune_UI_Dialog_t  dialogs[CTUNE_UI_CTX_COUNT];
    } cache;

    struct {
        const char * (* getDisplayText)( ctune_UI_TextID_e );
    } cb;

} private = {
    .init        = false,
    .mouse_ctrl  = false,
    .col_padding = 2,
    .cache       = { .curr_ctx = CTUNE_UI_CTX_MAIN, },
    .margins     = { 0, 1, 0, 1 },
    .cb          = { .getDisplayText = NULL },
};

/**
 * [PRIVATE] Helper DTO for the entry gatherer callback method
 * @param entries   Pointer to the Vector_t where ctune_UI_KeyBinding_t are copied
 * @param col1_size Minimum size of the first column (key text)
 * @param col2_size Minimum size of the second column (action description)
 * @param row_count Required number of rows to fit all entries
 */
typedef struct {
    Vector_t * entries;
    size_t     col1_size;
    size_t     col2_size;
    int        row_count;

} BindingProperties_t;

/**
 * [PRIVATE] Key binding entry gatherer callback
 * @param binding  Pointer to a current ctune_UI_KeyBinding_t entry
 * @param userdata Generic pointer to a BindingProperties_t
 */
static void ctune_UI_ContextHelp_gatherEntries( const ctune_UI_KeyBinding_t * binding, void * userdata ) {
    BindingProperties_t   * properties = userdata;
    ctune_UI_KeyBinding_t * el         = Vector.emplace_back( properties->entries );

    *el = *binding;

    if( binding->entry_type != CTUNE_UI_KEYBINDING_TYPE_HIDDEN ) {
        size_t key_ln = strlen( ctune_UI_KeyBinding.getKeyText( binding->key ) );
        size_t description_ln = strlen( private.cb.getDisplayText( binding->description ) );

        if( properties->col1_size < key_ln )
            properties->col1_size = key_ln;

        if( properties->col2_size < description_ln )
            properties->col2_size = description_ln;

        ++properties->row_count;
    }
}

/**
 * [PRIVATE] Creates the scroll window and its content for a context
 * @param ctx Context ID enum
 */
static void ctune_UI_ContextHelp_printScroll( ctune_UI_Context_e ctx ) {
    Vector_t            entries    = Vector.init( sizeof( ctune_UI_KeyBinding_t ), NULL );
    BindingProperties_t properties = { &entries, 0 ,0, 0 };

    if( !ctune_UI_KeyBinding.processEntries( ctx, &properties, ctune_UI_ContextHelp_gatherEntries ) ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_UI_ContextHelp_printScroll( %i )] Failed to process entries for context %i.",
                   ctx, ctx
        );

        goto end;
    }

    int key_len  = 0;
    int desc_len = 0;

    if( !ctune_utoi( properties.col1_size, &key_len ) ) {
        ctune_err.set( CTUNE_ERR_BAD_CAST );

        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_UI_ContextHelp_printScroll( %i )] Integer overflow on key definition text size: %lu",
                   ctx, properties.col1_size
        );
    }

    if( !ctune_utoi( properties.col2_size, &desc_len ) ) {
        ctune_err.set( CTUNE_ERR_BAD_CAST );

        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_UI_ContextHelp_printScroll( %i )] Integer overflow on description text size: %lu",
                   ctx, properties.col2_size
        );
    }

    const int col1_size         = ( key_len + private.col_padding );
    const int total_width       = ( col1_size + desc_len );

    CTUNE_LOG( CTUNE_LOG_TRACE,
               "[ctune_UI_ContextHelp_printScroll( %i )] Printing ScrollWin: { rows: %i, cols: %i }",
               ctx, properties.row_count, total_width, col1_size
    );

    ctune_UI_Dialog.createScrollWin( &private.cache.dialogs[ctx], properties.row_count, total_width );

    int row = 0;

    for( size_t i = 0; i < Vector.size( &entries ); ++i) {
        const ctune_UI_KeyBinding_t * binding = Vector.at( &entries, i );

        if( binding != NULL ) {
            switch( binding->entry_type ) {
                case CTUNE_UI_KEYBINDING_TYPE_HIDDEN: break;
                case CTUNE_UI_KEYBINDING_TYPE_NORMAL: //fallthrough
                case CTUNE_UI_KEYBINDING_TYPE_EMPTY: {
                    mvwprintw( private.cache.dialogs[ ctx ].canvas.pad,
                               row, 0,
                               "%-*s%s",
                               col1_size,
                               ctune_UI_KeyBinding.getKeyText( binding->key ),
                               ctune_UI_Language.text( binding->description )
                    );

                    ++row;
                } break;

                case CTUNE_UI_KEYBINDING_TYPE_HEADING: {
                    wattron( private.cache.dialogs[ ctx ].canvas.pad, ctune_UI_Theme.color( CTUNE_UI_ITEM_TXT_HELP_HEADING ) );

                    mvwprintw( private.cache.dialogs[ ctx ].canvas.pad,
                               row, 0,
                               "%s",
                               ctune_UI_Language.text( binding->description )
                    );

                    wattroff( private.cache.dialogs[ ctx ].canvas.pad, ctune_UI_Theme.color( CTUNE_UI_ITEM_TXT_HELP_HEADING ) );

                    ++row;
                } break;

                default: {
                    CTUNE_LOG( CTUNE_LOG_ERROR,
                               "[ctune_UI_ContextHelp_printScroll( %i )] Binding entry type unrecognised/not implemented (%i)!",
                               ctx, binding->entry_type
                    );
                } break;
            }

        } else {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_UI_ContextHelp_printScroll( %i )] Failed to get binding in Vector_t (i: %lu).",
                       ctx, i
            );
        }
    }

    end:
        Vector.clear_vector( &entries );
}

/**
 * [PRIVATE] Creates the border window for a given contextual help
 * @param ctx Context ID
 */
static void ctune_UI_ContextHelp_createBorderWin( ctune_UI_Context_e ctx ) {
    switch( ctx ) {
        case CTUNE_UI_CTX_MAIN: {
            ctune_UI_Dialog.createBorderWin( &private.cache.dialogs[CTUNE_UI_CTX_MAIN],
                                             private.screen_size,
                                             private.cb.getDisplayText( CTUNE_UI_TEXT_WIN_TITLE_HELP ),
                                             &private.margins,
                                             private.mouse_ctrl );

            ctune_UI_Dialog.hide( &private.cache.dialogs[CTUNE_UI_CTX_MAIN] );
        } break;

        case CTUNE_UI_CTX_FAV_TAB: {
            ctune_UI_Dialog.createBorderWin( &private.cache.dialogs[CTUNE_UI_CTX_FAV_TAB],
                                             private.screen_size,
                                             private.cb.getDisplayText( CTUNE_UI_TEXT_WIN_TITLE_HELP_FAV ),
                                             &private.margins,
                                             private.mouse_ctrl );

            ctune_UI_Dialog.hide( &private.cache.dialogs[CTUNE_UI_CTX_FAV_TAB] );
        } break;

        case CTUNE_UI_CTX_SEARCH_TAB: {
            ctune_UI_Dialog.createBorderWin( &private.cache.dialogs[CTUNE_UI_CTX_SEARCH_TAB],
                                             private.screen_size,
                                             private.cb.getDisplayText( CTUNE_UI_TEXT_WIN_TITLE_HELP_SEARCH ),
                                             &private.margins,
                                             private.mouse_ctrl );

            ctune_UI_Dialog.hide( &private.cache.dialogs[CTUNE_UI_CTX_SEARCH_TAB] );
        } break;

        case CTUNE_UI_CTX_BROWSE_TAB: {
            ctune_UI_Dialog.createBorderWin( &private.cache.dialogs[CTUNE_UI_CTX_BROWSE_TAB],
                                             private.screen_size,
                                             private.cb.getDisplayText( CTUNE_UI_TEXT_WIN_TITLE_HELP_BROWSE ),
                                             &private.margins,
                                             private.mouse_ctrl );

            ctune_UI_Dialog.hide( &private.cache.dialogs[CTUNE_UI_CTX_BROWSE_TAB] );
        } break;

        case CTUNE_UI_CTX_RSFIND: {
            ctune_UI_Dialog.createBorderWin( &private.cache.dialogs[CTUNE_UI_CTX_RSFIND],
                                             private.screen_size,
                                             private.cb.getDisplayText( CTUNE_UI_TEXT_WIN_TITLE_HELP_RSFIND ),
                                             &private.margins,
                                             private.mouse_ctrl );

            ctune_UI_Dialog.hide( &private.cache.dialogs[CTUNE_UI_CTX_RSFIND] );
        } break;

        case CTUNE_UI_CTX_RSINFO: {
            ctune_UI_Dialog.createBorderWin( &private.cache.dialogs[CTUNE_UI_CTX_RSINFO],
                                             private.screen_size,
                                             private.cb.getDisplayText( CTUNE_UI_TEXT_WIN_TITLE_HELP_RSINFO ),
                                             &private.margins,
                                             private.mouse_ctrl );

            ctune_UI_Dialog.hide( &private.cache.dialogs[CTUNE_UI_CTX_RSINFO] );
        } break;

        case CTUNE_UI_CTX_RSEDIT: {
            ctune_UI_Dialog.createBorderWin( &private.cache.dialogs[CTUNE_UI_CTX_RSEDIT],
                                             private.screen_size,
                                             private.cb.getDisplayText( CTUNE_UI_TEXT_WIN_TITLE_HELP_RSINFO ),
                                             &private.margins,
                                             private.mouse_ctrl );

            ctune_UI_Dialog.hide( &private.cache.dialogs[CTUNE_UI_CTX_RSEDIT] );
        } break;

        case CTUNE_UI_CTX_OPT_MENU: {
            ctune_UI_Dialog.createBorderWin( &private.cache.dialogs[CTUNE_UI_CTX_OPT_MENU],
                                             private.screen_size,
                                             private.cb.getDisplayText( CTUNE_UI_TEXT_WIN_TITLE_HELP_OPTIONMENU ),
                                             &private.margins,
                                             private.mouse_ctrl );

            ctune_UI_Dialog.hide( &private.cache.dialogs[CTUNE_UI_CTX_OPT_MENU] );
        } break;

        case CTUNE_UI_CTX_SETOUTDIR: {
            ctune_UI_Dialog.createBorderWin( &private.cache.dialogs[CTUNE_UI_CTX_SETOUTDIR],
                                             private.screen_size,
                                             private.cb.getDisplayText( CTUNE_UI_TEXT_WIN_TITLE_HELP_SETOUTDIR ),
                                             &private.margins,
                                             private.mouse_ctrl );

            ctune_UI_Dialog.hide( &private.cache.dialogs[CTUNE_UI_CTX_SETOUTDIR] );
        } break;

        default: break;
    }
}

/**
 * Initialises ShowHelp dialog content
 * @param parent         Pointer to parent window size properties
 * @param getDisplayText Callback method to get text strings for fields
 */
static bool ctune_UI_ContextHelp_init( const WindowProperty_t * parent, const char * (* getDisplayText)( ctune_UI_TextID_e ) ) {
    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_UI_ContextHelp_init( %p, %p )] Initialising the contextual help...",
               getDisplayText
    );

    if( parent == NULL ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_UI_ContextHelp_init( %p, %p )] Parent screen properties is NULL.",
                   parent, getDisplayText
        );

        return false;
    }

    if( getDisplayText == NULL ) {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_UI_ContextHelp_init( %p, %p )] language callback method for getting field's text is NULL.",
                   getDisplayText
        );

        return false; //EARLY RETURN
    }

    private.screen_size            = parent;
    private.cb.getDisplayText = getDisplayText;

    private.cache.dialogs[CTUNE_UI_CTX_MAIN      ] = ctune_UI_Dialog.init();
    private.cache.dialogs[CTUNE_UI_CTX_FAV_TAB   ] = ctune_UI_Dialog.init();
    private.cache.dialogs[CTUNE_UI_CTX_SEARCH_TAB] = ctune_UI_Dialog.init();
    private.cache.dialogs[CTUNE_UI_CTX_BROWSE_TAB] = ctune_UI_Dialog.init();
    private.cache.dialogs[CTUNE_UI_CTX_RSFIND    ] = ctune_UI_Dialog.init();
    private.cache.dialogs[CTUNE_UI_CTX_RSINFO    ] = ctune_UI_Dialog.init();
    private.cache.dialogs[CTUNE_UI_CTX_RSEDIT    ] = ctune_UI_Dialog.init();
    private.cache.dialogs[CTUNE_UI_CTX_OPT_MENU  ] = ctune_UI_Dialog.init();
    private.cache.dialogs[CTUNE_UI_CTX_SETOUTDIR ] = ctune_UI_Dialog.init();

    ctune_UI_ContextHelp_printScroll( CTUNE_UI_CTX_MAIN );
    ctune_UI_ContextHelp_printScroll( CTUNE_UI_CTX_FAV_TAB );
    ctune_UI_ContextHelp_printScroll( CTUNE_UI_CTX_SEARCH_TAB );
    ctune_UI_ContextHelp_printScroll( CTUNE_UI_CTX_BROWSE_TAB );
    ctune_UI_ContextHelp_printScroll( CTUNE_UI_CTX_RSFIND );
    ctune_UI_ContextHelp_printScroll( CTUNE_UI_CTX_RSINFO );
    ctune_UI_ContextHelp_printScroll( CTUNE_UI_CTX_RSEDIT );
    ctune_UI_ContextHelp_printScroll( CTUNE_UI_CTX_OPT_MENU );
    ctune_UI_ContextHelp_printScroll( CTUNE_UI_CTX_SETOUTDIR );

    private.init = true;

    return true;
}

/**
 * Switch mouse control UI on/off
 * @param mouse_ctrl_flag Flag to turn feature on/off
 */
static void ctune_UI_ContextHelp_setMouseCtrl( bool mouse_ctrl_flag ) {
    private.mouse_ctrl = mouse_ctrl_flag;
}

/**
 * Show the help dialog box for the given context
 * @param ctx Context ID enum
 */
static void ctune_UI_ContextHelp_show( ctune_UI_Context_e ctx ) {
    if( ctx >= CTUNE_UI_CTX_COUNT ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_UI_ContextHelp_show( %i )] Context ID out of bounds.", ctx );
        return;
    }

    private.cache.curr_ctx = ctx;

    ctune_UI_ContextHelp_createBorderWin( ctx );

    ctune_UI_Dialog.show( &private.cache.dialogs[ctx] );
    ctune_UI_ScrollWin.refreshView( &private.cache.dialogs[ctx].canvas );

    ctune_UI_Resizer.push( ctune_UI_ContextHelp.resize, NULL );

    doupdate();
}

/**
 * Resize current help context
 */
static void ctune_UI_ContextHelp_resize() {
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_ContextHelp_resize()] Resizing for context %i.", private.cache.curr_ctx );

    ctune_UI_ContextHelp_createBorderWin( private.cache.curr_ctx );

    ctune_UI_Dialog.show( &private.cache.dialogs[private.cache.curr_ctx] );
    ctune_UI_ScrollWin.refreshView( &private.cache.dialogs[private.cache.curr_ctx].canvas );

    doupdate();
}

/**
 * [PRIVATE] Handle mouse event
 * @param dialog Dialog
 * @param event  Mouse event mask
 * @param Exit state
 */
static bool ctune_UI_ContextHelp_handleMouseEvent( ctune_UI_Dialog_t * dialog, MEVENT * event ) {
    const ctune_UI_WinCtrlMask_m win_ctrl = ctune_UI_Dialog.isWinControl( dialog, event->y, event->x );
    const ctune_UI_ScrollMask_m  scroll   = ctune_UI_WinCtrlMask.scrollMask( win_ctrl );

    if( win_ctrl ) {
        if( scroll ) {
            if( event->bstate & BUTTON1_CLICKED ) {
                ctune_UI_Dialog.incrementalScroll( dialog, scroll );

            } else if( event->bstate & BUTTON1_DOUBLE_CLICKED ) {
                ctune_UI_Dialog.incrementalScroll( dialog, ctune_UI_ScrollMask.setScrollFactor( scroll, 2 ) );

            } else if( event->bstate & BUTTON1_TRIPLE_CLICKED ) {
                ctune_UI_Dialog.incrementalScroll( dialog, ctune_UI_ScrollMask.setScrollFactor( scroll, 3 ) );

            } else if( event->bstate & BUTTON3_CLICKED ) {
                ctune_UI_Dialog.edgeScroll( dialog, scroll );
            }

        } else if( win_ctrl & CTUNE_UI_WINCTRLMASK_CLOSE ) {
            if( event->bstate & BUTTON1_CLICKED ) {
                return true; //EARLY RETURN
            }
        }
    }

    return false;
}

/**
 * Pass keyboard input to the form
 */
static void ctune_UI_ContextHelp_captureInput() {
    const ctune_UI_Context_e ctx = private.cache.curr_ctx;

    keypad( private.cache.dialogs[ctx].canvas.pad, TRUE );
    bool   exit = false;
    int    ch;
    MEVENT mouse_event;

    while( !exit ) {
        ch = wgetch( private.cache.dialogs[ctx].canvas.pad );

        switch( ch ) {
            case ERR: {
                keypad( private.cache.dialogs[ctx].canvas.pad, TRUE );
            } break;

            case KEY_RESIZE: break;

            case KEY_UP: {
                if( ctune_UI_Dialog.isScrollableY( &private.cache.dialogs[ctx] ) )
                    ctune_UI_Dialog.incrementalScroll( &private.cache.dialogs[ctx], CTUNE_UI_SCROLL_UP );
                else
                    exit = true;
            } break;

            case KEY_DOWN: {
                if( ctune_UI_Dialog.isScrollableY( &private.cache.dialogs[ctx] ) )
                    ctune_UI_Dialog.incrementalScroll( &private.cache.dialogs[ctx], CTUNE_UI_SCROLL_DOWN );
                else
                    exit = true;
            } break;

            case KEY_LEFT: {
                if( ctune_UI_Dialog.isScrollableX( &private.cache.dialogs[ctx] ) )
                    ctune_UI_Dialog.incrementalScroll( &private.cache.dialogs[ctx], CTUNE_UI_SCROLL_LEFT );
                else
                    exit = true;
            } break;

            case KEY_RIGHT: {
                if( ctune_UI_Dialog.isScrollableX( &private.cache.dialogs[ctx] ) )
                    ctune_UI_Dialog.incrementalScroll( &private.cache.dialogs[ctx], CTUNE_UI_SCROLL_RIGHT );
                else
                    exit = true;
            } break;

            case KEY_HOME: {
                if( ctune_UI_Dialog.isScrollableY( &private.cache.dialogs[ctx] ) || ctune_UI_Dialog.isScrollableX( &private.cache.dialogs[ctx] ) )
                    ctune_UI_Dialog.edgeScroll( &private.cache.dialogs[ctx], CTUNE_UI_SCROLL_TO_HOME );
                else
                    exit = true;
            } break;

            case KEY_MOUSE: {
                if( getmouse( &mouse_event ) == OK ) {
                    exit = ctune_UI_ContextHelp_handleMouseEvent( &private.cache.dialogs[ctx], &mouse_event );
                }
            } break;

            default:
                exit = true;
                break;
        }

        ctune_UI_Dialog.refreshView( &private.cache.dialogs[ctx] );
    }

    //send to the bottom of the panel stack to hide it behind the tabs
    ctune_UI_Dialog.hide( &private.cache.dialogs[ctx] );
    doupdate();
    keypad( private.cache.dialogs[ctx].canvas.pad, FALSE );
    ctune_UI_Resizer.pop();
}

/**
 * De-allocates resources
 */
static void ctune_UI_ContextHelp_free( void ) {
    if( private.init ) {
        for( int i = 0; i < CTUNE_UI_CTX_COUNT; ++i ) {
            ctune_UI_Dialog.free( &private.cache.dialogs[ i ] );
        }

        private.init = false;
    }

    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_UI_ContextHelp_free()] ContextHelp freed." );
}


/**
 * Constructor
 */
const struct ctune_UI_ContextHelp_Instance ctune_UI_ContextHelp = {
    .init         = &ctune_UI_ContextHelp_init,
    .setMouseCtrl = &ctune_UI_ContextHelp_setMouseCtrl,
    .show         = &ctune_UI_ContextHelp_show,
    .resize       = &ctune_UI_ContextHelp_resize,
    .captureInput = &ctune_UI_ContextHelp_captureInput,
    .free         = &ctune_UI_ContextHelp_free,
};