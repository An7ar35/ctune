#ifndef CTUNE_UI_TYPES_WINCTRLMASK_H
#define CTUNE_UI_TYPES_WINCTRLMASK_H

#include "ScrollMask.h"

#define CTUNE_UI_WINCTRL_MASK_SIZE 4
#define CTUNE_UI_WINCTRL_MASK      ( 0b1111 << CTUNE_UI_SCROLLMASK_SIZE )
#define CTUNE_UI_WINCTRLMASK_CLOSE ( 0b0001 << CTUNE_UI_SCROLLMASK_SIZE )

typedef int ctune_UI_WinCtrlMask_m;

/**
 * WinCtrlMask helper methods
 */
extern const struct ctune_UI_WinCtrlMask_Namespace {
    /**
     * Extracts the scroll mask
     * @param win_ctrl_mask Window control mask
     * @return Scroll mask
     */
    ctune_UI_ScrollMask_m (* scrollMask)( ctune_UI_WinCtrlMask_m win_ctrl_mask );

    /**
     * Combine a scroll mask to a window control mask
     * @param win_ctrl_mask Window control mask
     * @param scroll_mask   Scroll mask
     * @return Window control mask
     */
    ctune_UI_WinCtrlMask_m (* combine)( ctune_UI_WinCtrlMask_m win_ctrl_mask, ctune_UI_ScrollMask_m scroll_mask );

} ctune_UI_WinCtrlMask;

#endif //CTUNE_UI_TYPES_WINCTRLMASK_H
