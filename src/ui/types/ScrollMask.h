#ifndef CTUNE_UI_TYPE_SCROLLMASK_H
#define CTUNE_UI_TYPE_SCROLLMASK_H

#define CTUNE_UI_SCROLLMASK_SIZE 8
#define CTUNE_UI_SCROLLMASK      0b11111111
#define CTUNE_UI_SCROLL_UP       0b00000011
#define CTUNE_UI_SCROLL_DOWN     0b00001100
#define CTUNE_UI_SCROLL_LEFT     0b00110000
#define CTUNE_UI_SCROLL_RIGHT    0b11000000
#define CTUNE_UI_SCROLL_TO_HOME  ( CTUNE_UI_SCROLL_UP   | CTUNE_UI_SCROLL_LEFT  )
#define CTUNE_UI_SCROLL_TO_END   ( CTUNE_UI_SCROLL_DOWN | CTUNE_UI_SCROLL_RIGHT )

/**
 * Scroll direction and magnitude mask
 * @details
 * <b>Masks Available</b>\n
 * CTUNE_UI_SCROLL_UP (3)
 * CTUNE_UI_SCROLL_DOWN (12)
 * CTUNE_UI_SCROLL_LEFT (48)
 * CTUNE_UI_SCROLL_RIGHT (192)
 * CTUNE_UI_SCROLL_UP_LEFT (51)
 * CTUNE_UI_SCROLL_DOWN_RIGHT (204)
 * CTUNE_UI_SCROLL (255)
 */
typedef int ctune_UI_ScrollMask_m;

/**
 * ScrollMask helper methods
 */
extern const struct ctune_UI_ScrollMask_Namespace {
    /**
     * Creates a scrolling mask
     * @param vertical   Vertical scroll value (up: -3 to +3 :down)
     * @param horizontal Horizontal scroll value (left: -3 to +3 :right)
     * @return Scroll mask
     */
    ctune_UI_ScrollMask_m (* createMask)( int vertical, int horizontal );

    /**
     * Modifies the scrolling factor of a scrolling mask
     * @param mask   Scrolling mask
     * @param factor Increase scroll factor (0-3)
     * @return Modified mask
     */
    ctune_UI_ScrollMask_m (* setScrollFactor)( ctune_UI_ScrollMask_m mask, int factor );

    /**
     * Gets the horizontal scroll factor from a mask
     * @param mask Scrolling bit mask
     * @return Scroll factor
     */
    int (* horizontalScrollFactor)( ctune_UI_ScrollMask_m mask );

    /**
     * Gets the horizontal scroll factor from a mask
     * @param mask Scrolling bit mask
     * @return Scroll factor
     */
    int (* verticalScrollFactor)( ctune_UI_ScrollMask_m mask );

} ctune_UI_ScrollMask;

#endif //CTUNE_UI_TYPE_SCROLLMASK_H
