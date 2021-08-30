#ifndef CTUNE_UTILS_LOGGER_H
#define CTUNE_UTILS_LOGGER_H

#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

#ifndef NDEBUG
    #include <libgen.h>
    #define CTUNE_LOG( lvl, fmt, args... ) ctune_Logger.logDBG( lvl, basename(__FILE__), __LINE__, fmt, ##args );
#else
    #define CTUNE_LOG( lvl, fmt, args... ) ctune_Logger.log( lvl, fmt, ##args );
#endif

enum ctune_LogLevel {
    CTUNE_LOG_FATAL   = 1, //RFC.5424 Syslog
    CTUNE_LOG_ERROR   = 3, //RFC.5424 Syslog
    CTUNE_LOG_WARNING = 4, //RFC.5424 Syslog
    CTUNE_LOG_MSG     = 5, //RFC.5424 Syslog
    CTUNE_LOG_DEBUG   = 7, //RFC.5424 Syslog
    CTUNE_LOG_TRACE   = 8, //not syslog compliant
};

typedef enum ctune_LogLevel ctune_LogLevel_e;

extern const struct ctune_Logger_Singleton {
    /**
     * Initialises the logger
     * @param log_filepath Output file path
     * @param file_mode    File mode (see `fopen(..)` docs) for the output file
     * @param level        Minimal log level to process
     * @return Init success (if false all messages will be piped to syslog)
     */
    bool (* init)( const char *, const char *, enum ctune_LogLevel );

    /**
     * Terminate logger and closes output file
     */
    void (* close)();

    /**
     * Sends a message to the log
     * @param lvl      Message log level
     * @param format   String format (similar to `printf`)
     * @param ...      Arguments (similar to `printf`)
     */
    void (* log)( enum ctune_LogLevel lvl, const char * format, ... );

    /**
     * Sends a message to the log
     * @param lvl      Message log level
     * @param filename Source file name
     * @param line_num Line number in source file
     * @param format   String format (similar to `printf`)
     * @param ...      Arguments (similar to `printf`)
     */
    void (* logDBG)( enum ctune_LogLevel lvl, char * filename, int line_number, const char * format, ... );

} ctune_Logger;

#endif //CTUNE_UTILS_LOGGER_H