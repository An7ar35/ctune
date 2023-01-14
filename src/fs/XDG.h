#ifndef CTUNE_FS_XDG_H
#define CTUNE_FS_XDG_H

#include "../datastructure/String.h"

extern const struct ctune_XDG_Instance {
    /**
     * Resolves the application configuration directory path and append given filename to it
     * @param file_name     File name to resolve on the application's configuration directory
     * @param resolved_path Container for the resolved file path to be stored in
     */
    void (* resolveCfgFilePath)( const char * file_name, String_t * resolved_path );

    /**
     * Resolves the application data directory path and append given filename to it
     * @param file_name     File name to resolve on the application's configuration directory
     * @param resolved_path Container for the resolved file path to be stored in
     */
    void (* resolveDataFilePath)( const char * file_name, String_t * resolved_path );

    /**
     * De-allocates anything stored on the heap
     */
    void (* free)( void );

} ctune_XDG;

#endif //CTUNE_FS_XDG_H
