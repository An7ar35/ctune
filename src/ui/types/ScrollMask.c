#include "ScrollMask.h"

#include <stdlib.h>

/**
 * Creates a scrolling mask
 * @param vertical   Vertical scroll value (up: -3 to +3 :down)
 * @param horizontal Horizontal scroll value (left: -3 to +3 :right)
 * @return Scroll mask
 */
static ctune_UI_ScrollMask_m ctune_UI_ScrollMask_createMask( int vertical, int horizontal ) {
    ctune_UI_ScrollMask_m mask = 0;

    if( vertical >= 0 ) { //DOWN
        mask |= ( vertical << 2 );
    } else { //UP
        mask |= abs( vertical );
    }

    if( horizontal >= 0 ) { //RIGHT
        mask |= ( horizontal << 6 );
    } else { //LEFT
        mask |= ( abs( horizontal ) << 4 );
    }

    return mask;
}

/**
 * Modifies the scrolling factor of a scrolling mask
 * @param mask   Scrolling mask
 * @param factor Increase scroll factor (0-3)
 * @return Modified mask
 */
static ctune_UI_ScrollMask_m ctune_UI_ScrollMask_setScrollFactor( ctune_UI_ScrollMask_m mask, int factor ) {
    if( factor > 3 ) { //normalise
        factor = 3;
    } else if( factor < 0 ) {
        factor = 0;
    }

    ctune_UI_ScrollMask_m new_mask = 0b00000000;

    if( mask & CTUNE_UI_SCROLL_UP ) {
        new_mask |= factor;
    }

    if( mask & CTUNE_UI_SCROLL_DOWN ) {
        new_mask |= ( factor << 2 );
    }

    if( mask & CTUNE_UI_SCROLL_LEFT ) {
        new_mask |= ( factor << 4 );
    }

    if( mask & CTUNE_UI_SCROLL_RIGHT ) {
        new_mask |= ( factor << 6 );
    }

    return new_mask;
}

/**
 * Gets the horizontal scroll factor from a mask
 * @param mask Scrolling bit mask
 * @return Scroll factor
 */
static int ctune_UI_ScrollMask_horizontalScrollFactor( ctune_UI_ScrollMask_m mask ) {
    return ( ( mask & CTUNE_UI_SCROLL_RIGHT ) >> 6 ) - ( ( mask & CTUNE_UI_SCROLL_LEFT ) >> 4 );
}

/**
 * Gets the horizontal scroll factor from a mask
 * @param mask Scrolling bit mask
 * @return Scroll factor
 */
static int ctune_UI_ScrollMask_verticalScrollFactor( ctune_UI_ScrollMask_m mask ) {
    return ( ( mask & CTUNE_UI_SCROLL_DOWN ) >> 2 ) - ( mask & CTUNE_UI_SCROLL_UP );
}

/**
 * Namespace constructor
 */
const struct ctune_UI_ScrollMask_Namespace ctune_UI_ScrollMask = {
    .createMask             = &ctune_UI_ScrollMask_createMask,
    .setScrollFactor        = &ctune_UI_ScrollMask_setScrollFactor,
    .horizontalScrollFactor = &ctune_UI_ScrollMask_horizontalScrollFactor,
    .verticalScrollFactor   = &ctune_UI_ScrollMask_verticalScrollFactor,
};