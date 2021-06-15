#ifndef CTUNE_UI_DATASTRUCTURE_WINDOWPROPERTY_H
#define CTUNE_UI_DATASTRUCTURE_WINDOWPROPERTY_H

/**
 * Struct to hold the size and initial position of a NCurses Window
 * @param rows  Vertical size in number of rows
 * @param cols  Horizontal size in number of columns
 * @param pos_y Starting row position
 * @param pos_x Starting column position
 */
typedef struct ctune_UI_WindowProperty {
    int rows;
    int cols;
    int pos_y;
    int pos_x;

} WindowProperty_t;

#endif //CTUNE_UI_DATASTRUCTURE_WINDOWPROPERTY_H
