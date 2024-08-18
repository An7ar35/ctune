#ifndef CTUNE_UI_RESIZER_H
#define CTUNE_UI_RESIZER_H

#include <stdbool.h>

/**
 * Resizer
 * -
 * Note: the first pushed callback should update all internally tracked window sizes as it will be called first
 */
extern const struct ctune_UI_Resizer_Instance {
    /**
     * Initialises the Resizer
     */
    void (* init)( void );

    /**
     * Adds a new resize callback method to the list
     * @param resize_fn Callback for resizing
     * @param data      Pointer to pass to callback
     * @return Success
     */
    bool (* push)( void(* resize_fn)( void * ), void * data );

    /**
     * Removes the last resize callback from the list
     */
    void (* pop)( void );

    /**
     * Calls all callbacks stored in the list in FIFO order
     */
    void (* resize)( void );

    /**
     * Gets the resizing request flag and resets it
     * @return Request flag
     */
    bool (* resizingRequested)( void );

    /**
     * Sets the resize request flag up
     */
    void (* requestResizing)( void );

    /**
     * De-allocates internal variables and resets everything back to an initialised state
     */
    void (* free)( void );

} ctune_UI_Resizer;

#endif //CTUNE_UI_RESIZER_H
