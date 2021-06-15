#ifndef CTUNE_FS_PLAYBACKLOG_H
#define CTUNE_FS_PLAYBACKLOG_H

#include <stdbool.h>

#include "../datastructure/String.h"

typedef struct ctune_PlaybackLog_Instance {
    /**
     * Opens the log file for writing
     * @param file_path      File path
     * @param overwrite_flag Flag for overwriting the file (true=overwrite, false=append)
     * @return Success
     */
    bool (* open)( const char * file_path, bool overwrite_flag );

    /**
     * Gets the underlining file descriptor for the log
     * @return Internal FILE pointer
     */
    FILE * (* output)( void );

    /**
     * Closes log file descriptor
     */
    void (* close)( void );

    /**
     * Writes string to log
     * @param str String to write
     */
    void (* write)( const char * str );

    /**
     * Write string and append newline after it
     * @param str String to write
     */
    void (* writeln)( const char * str );

    /**
     * Write String_t to log
     * @param string String_t to write
     */
    void (* writeString)( const String_t * string );

} ctune_PlaybackLog_t;

extern const struct ctune_PlaybackLog_Instance ctune_PlaybackLog;

#endif //CTUNE_FS_PLAYBACKLOG_H
