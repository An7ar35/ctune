#ifndef CTUNE_DTO_COLOURTHEME_H
#define CTUNE_DTO_COLOURTHEME_H

/**
 * Container for theme colour settings
 */
struct ctune_ColourTheme {
    short background;
    short foreground;

    struct {
        short background;
        short foreground;
        short selected_focused_bg;
        short selected_focused_fg;
        short selected_unfocused_bg;
        short selected_unfocused_fg;
        short favourite_local_fg;
        short favourite_remote_fg;
        short broken_fg;
    } rows;

    struct {
        short playback_on;
        short playback_off;
        short queued_station;
    } icons;

    struct {
        short invalid_fg;
    } field;

    struct {
        short background;
        short foreground;
        short invalid_fg;
        short validated_fg;
    } button;
};

/**
 * ColourTheme namespace
 */
extern const struct ctune_ColourTheme_Namespace {
    /**
     * Initiate a colour theme object with default values
     * @return Initiated ColourTheme
     */
    struct ctune_ColourTheme (* init)( void );

    /**
     * Gets the string representation of a colour code
     * @param colour Colour code (0-8)
     * @return String
     */
    const char * (* str)( short colour );

} ctune_ColourTheme;

#endif //CTUNE_DTO_COLOURTHEME_H
