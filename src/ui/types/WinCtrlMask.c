#include "WinCtrlMask.h"

/**
 * Extracts the scroll mask
 * @param win_ctrl_mask Window control mask
 * @return Scroll mask
 */
static ctune_UI_ScrollMask_m ctune_UI_WinCtrlMask_scrollMask( ctune_UI_WinCtrlMask_m win_ctrl_mask ) {
    return ( win_ctrl_mask & CTUNE_UI_SCROLLMASK );
}

/**
 * Combine a scroll mask to a window control mask
 * @param win_ctrl_mask Window control mask
 * @param scroll_mask   Scroll mask
 * @return Window control mask
 */
static ctune_UI_WinCtrlMask_m ctune_UI_WinCtrlMask_combine( ctune_UI_WinCtrlMask_m win_ctrl_mask, ctune_UI_ScrollMask_m scroll_mask ) {
    return ( win_ctrl_mask | scroll_mask );
}

/**
 * Namespace constructor
 */
const struct ctune_UI_WinCtrlMask_Namespace ctune_UI_WinCtrlMask = {
    .scrollMask = &ctune_UI_WinCtrlMask_scrollMask,
    .combine    = &ctune_UI_WinCtrlMask_combine,
};