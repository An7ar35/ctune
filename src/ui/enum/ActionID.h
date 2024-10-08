#ifndef CTUNE_UI_ENUM_ACTIONID_H
#define CTUNE_UI_ENUM_ACTIONID_H

typedef enum ctune_UI_ActionID {
    CTUNE_UI_ACTION_ERR  = -1,
    CTUNE_UI_ACTION_NONE =  0,

    CTUNE_UI_ACTION_ESC,
    CTUNE_UI_ACTION_GO_BACK,
    CTUNE_UI_ACTION_HELP,
    CTUNE_UI_ACTION_QUIT,
    CTUNE_UI_ACTION_TRIGGER,
    CTUNE_UI_ACTION_TAB1,
    CTUNE_UI_ACTION_TAB2,
    CTUNE_UI_ACTION_TAB3,
    CTUNE_UI_ACTION_RSI_QUEUED,
    CTUNE_UI_ACTION_RSI_SELECTED,

    CTUNE_UI_ACTION_FIND,
    CTUNE_UI_ACTION_NEW,
    CTUNE_UI_ACTION_EDIT,
    CTUNE_UI_ACTION_OPTIONS,
    CTUNE_UI_ACTION_FAV,
    CTUNE_UI_ACTION_PLAY,
    CTUNE_UI_ACTION_STOP,
    CTUNE_UI_ACTION_RESUME,
    CTUNE_UI_ACTION_TOGGLE_PLAYBACK,
    CTUNE_UI_ACTION_RECORD,
    CTUNE_UI_ACTION_VOLUP_5,
    CTUNE_UI_ACTION_VOLUP_10,
    CTUNE_UI_ACTION_VOLDOWN_5,
    CTUNE_UI_ACTION_VOLDOWN_10,

    CTUNE_UI_ACTION_PAGE_UP,
    CTUNE_UI_ACTION_PAGE_DOWN,
    CTUNE_UI_ACTION_SELECT_FIRST,
    CTUNE_UI_ACTION_SELECT_LAST,
    CTUNE_UI_ACTION_SELECT_PREV,
    CTUNE_UI_ACTION_SELECT_NEXT,
    CTUNE_UI_ACTION_SCROLL_HOME,
    CTUNE_UI_ACTION_SCROLL_UP,
    CTUNE_UI_ACTION_SCROLL_DOWN,
    CTUNE_UI_ACTION_SCROLL_LEFT,

    CTUNE_UI_ACTION_SCROLL_RIGHT,
    CTUNE_UI_ACTION_FOCUS_RIGHT,
    CTUNE_UI_ACTION_FOCUS_LEFT,
    CTUNE_UI_ACTION_TOGGLE,
    CTUNE_UI_ACTION_TOGGLE_ALT,
    CTUNE_UI_ACTION_FIELD_BEGIN,
    CTUNE_UI_ACTION_FIELD_END,
    CTUNE_UI_ACTION_FIELD_FIRST,
    CTUNE_UI_ACTION_FIELD_LAST,
    CTUNE_UI_ACTION_FIELD_PREV,

    CTUNE_UI_ACTION_FIELD_NEXT,
    CTUNE_UI_ACTION_DEL_PREV,
    CTUNE_UI_ACTION_DEL_NEXT,
    CTUNE_UI_ACTION_GO_LEFT,
    CTUNE_UI_ACTION_GO_RIGHT,
    CTUNE_UI_ACTION_CLEAR_SELECTED,
    CTUNE_UI_ACTION_CLEAR_ALL,

    CTUNE_UI_ACTION_MOUSE_EVENT,
    CTUNE_UI_ACTION_RESIZE,

    CTUNE_UI_ACTION_COUNT,
} ctune_UI_ActionID_e;

#endif //CTUNE_UI_ENUM_ACTIONID_H
