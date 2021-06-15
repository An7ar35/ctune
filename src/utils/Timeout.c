#include "Timeout.h"

#include <assert.h>
#include "../logger/Logger.h"

/**
 * Check if timer has timed-out (if so the `ctune_set_err( int )` is called with the prearranged error number)
 * @param self Timeout instance
 * @return Timeout state (1: true, 0: false)
 */
static int ctune_Timeout_timedOut( void * self ) {
    ctune_Timeout_t * timer = (struct ctune_Timeout *) self;
    if( ( time(0) - timer->start ) >= timer->timeout_val ) {
        if( timer->set_errno_cb != NULL )
            timer->set_errno_cb( timer->errno_on_timeout );
        CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_Timeout_timedOut( %p )] Timed-out (%ds).", self, timer->timeout_val )
        return 1;
    }
    return 0;
}

/**
 * Resets timeout
 * @param self Timeout instance
 */
static void ctune_Timeout_reset( struct ctune_Timeout * self ) {
    self->timed_out   = false;
    self->start       = time(0);
}

/**
 * Sets the error number to set `ctune_set_err(..)` with on timout
 * @param self          Timeout instance
 * @param timeout_errno cTune errno
 * @param err_cb        Method to call with error number on timeout (can be NULL)
 */
static void ctune_Timeout_setFailErr( struct ctune_Timeout * self, int timeout_errno, void(* err_cb)( int ) ) {
    self->errno_on_timeout = timeout_errno;
    self->set_errno_cb     = err_cb;
}

/**
 * Gets the stored timeout errno
 * @param self Timeout instance
 */
static int ctune_Timeout_getErrNo( struct ctune_Timeout * self ) {
    return self->errno_on_timeout;
}

/**
 * Constructor
 * @param timeout timeout amount in seconds
 * @param timeout_errno `errorno` number to set on timeout
 * @param err_cb        Method to call with error number on timeout (can be NULL)
 * @return Instanciated Timeout
 */
static struct ctune_Timeout ctune_Timeout_init( time_t timeout, int timeout_errno, void(* err_cb)( int ) ) {
    assert( timeout > 0 );

    return (struct ctune_Timeout) {
        .timed_out        = false,
        .timeout_val      = timeout,
        .start            = time( 0 ),
        .errno_on_timeout = timeout_errno,
        .set_errno_cb     = err_cb
    };
}

/**
 * Constructor
 */
const struct ctune_TimeoutClass ctune_Timeout = {
    .init       = &ctune_Timeout_init,
    .timedOut   = &ctune_Timeout_timedOut,
    .reset      = &ctune_Timeout_reset,
    .setFailErr = &ctune_Timeout_setFailErr,
    .getErrNo   = &ctune_Timeout_getErrNo
};