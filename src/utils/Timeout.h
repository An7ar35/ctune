#ifndef CTUNE_UTILS_TIMEOUT_H
#define CTUNE_UTILS_TIMEOUT_H

#include <stdbool.h>
#include <time.h>

struct ctune_Timeout {
    bool   timed_out;
    time_t timeout_val;
    time_t start;
    int    errno_on_timeout;
    void(* set_errno_cb)( int );
};

typedef struct ctune_Timeout ctune_Timeout_t;

extern const struct ctune_TimeoutClass {
    /**
     * Constructor
     * @param timeout timeout amount in seconds
     * @param timeout_errno `errorno` number to set on timeout
     * @param err_cb        Method to call with error number on timeout (can be NULL)
     * @return Instanciated Timeout
     */
    struct ctune_Timeout (* init)( time_t timeout, int timeout_errno, void(* err_cb)( int ) );

    /**
     * Check if timer has timed-out (if so the `ctune_set_err( int )` is called with the prearranged error number)
     * @param self Timeout instance
     * @return Timeout state (1: true, 0: false)
     */
    int  (* timedOut)( void * self );

    /**
     * Resets timeout
     * @param self Timeout instance
     */
    void (* reset)( struct ctune_Timeout * self );

    /**
     * Sets the error number to set `ctune_set_err(..)` with on timout
     * @param self          Timeout instance
     * @param timeout_errno cTune errno
     * @param err_cb        Method to call with error number on timeout (can be NULL)
     */
    void (* setFailErr)( struct ctune_Timeout * self, int timeout_errno, void(* err_cb)( int ) );

    /**
     * Gets the stored timeout errno
     * @param self Timeout instance
     */
    int  (* getErrNo)( struct ctune_Timeout * self );

} ctune_Timeout;


#endif //CTUNE_UTILS_TIMEOUT_H
