#ifndef CTUNE_FS_FS_H
#define CTUNE_FS_FS_H

#include <stdbool.h>
#include <sys/types.h>

#include "../datastructure/String.h"
#include "../enum/FileState.h"

/**
 * Filesystem utility functions
 */
extern const struct ctune_fs_Namespace {
    /**
     * Get file state
     * @param filename File path/name
     * @param size Pointer to container to set the found file size (only on CTUNE_FILE_FOUND)
     * @return CTUNE_FILE_*
     */
    ctune_FileState_e (* getFileState)( const char * filename, ssize_t * size );

    /**
     * [PRIVATE] Checks and creates a directory path if absent
     * @param dir_path Directory path to create
     * @return Success/Path exists
     */
    bool (* createDirectory)( String_t * dir_path );

    /**
     * Create a copy of a file's content
     * @param src Source filepath
     * @param target Target filepath
     * @param buff_size Size of buffer to use
     * @return Success
     */
    bool (* duplicateFile)( const char * src, const char * target, size_t buff_size );

    /**
     * Reads a file into a String_t object
     * @param file_path File path
     * @param content   String_t object to dump file content into (assumed to be pre-initialised)
     * @return Success
     */
    bool (* readFile)( const char * file_path, String_t * content );

} ctune_fs;

#endif //CTUNE_FS_FS_H
