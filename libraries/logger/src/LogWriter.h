#ifndef CTUNE_LOGGER_WRITER_H
#define CTUNE_LOGGER_WRITER_H

#include "LogQueue.h"

extern const struct ctune_LogWriter_Instance {
    /**
     * Starts the log writer
     * @param out_filepath Log file output path
     * @param file_mode    Mode to use when opening the log file
     * @param queue        Pointer to an instanciated LogQueue
     * @return Success
     */
    bool (* start)( const char * out_filepath, const char * file_mode, LogQueue_t * queue );

    /**
     * [THREAD SAFE] Stops the log writer
     * @return 0 for success or negative cTune error number on failure
     */
    int  (* stop)( void );

    /**
     * [THREAD SAFE] Wake-up the log writer
     */
    void (* resume)( void );

} ctune_LogWriter;

#endif //CTUNE_LOGGER_WRITER_H
