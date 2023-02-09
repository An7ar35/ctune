#include "XDG.h"

#include <stdlib.h>
#include <unistd.h>

#include "fs.h"
#include "logger/src/Logger.h"

static struct ctune_Settings_XDG {
    //https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html
    const char * ctune_directory_name;
    const char * fallback_data_path;
    const char * fallback_cfg_path;
    String_t     resolved_data_path;
    String_t     resolved_cfg_path;
} xdg = {
    .ctune_directory_name = "ctune/",
    .fallback_data_path   = ".local/share/",
    .fallback_cfg_path    = ".config/",
    .resolved_data_path   = { ._raw = NULL, ._length = 0 }, //i.e. String.init()
    .resolved_cfg_path    = { ._raw = NULL, ._length = 0 }  //i.e. String.init()
};

/**
 * [PRIVATE] Gets the XDG or fallback configuration directory path for the application
 * @param path_str String container
 * @return Success
 */
static bool ctune_XDG_getCfgBaseDir( String_t * path_str ) {
    const char * xdg_cfg_home = getenv( "XDG_CONFIG_HOME" );
    bool         error_state  = false;

    if( xdg_cfg_home != NULL && strlen( xdg_cfg_home ) > 0 ) {
        String.append_back( path_str, xdg_cfg_home );
        String.append_back( path_str, "/" );

    } else {
        CTUNE_LOG( CTUNE_LOG_WARNING,
                   "[ctune_XDG_getCfgBaseDir( String_t * )] "
                   "Env. variable 'XDG_CONFIG_HOME' not set, using default ('${HOME}/%s%s').",
                   xdg.fallback_cfg_path, xdg.ctune_directory_name
        );

        char * home_dir = getenv( "HOME" );

        if( home_dir == NULL || strlen( home_dir ) <= 0 ) { //place in running directory as fallback
            CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_XDG_getCfgBaseDir( String_t * )] Env. variable 'HOME' not found.\"" )
            error_state = true;

        } else {
            String.append_back( path_str, home_dir );
            String.append_back( path_str, "/" );
            String.append_back( path_str, xdg.fallback_cfg_path );
        }
    }

    String.append_back( path_str, xdg.ctune_directory_name );
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_XDG_getCfgBaseDir( String_t * )] Base config dir set as: %s", path_str->_raw );

    return !( error_state );
}

/**
 * [PRIVATE] Gets the XDG or fallback data directory path for the application
 * @param path_str String container
 * @return Success
 */
static bool ctune_XDG_getDataBaseDir( String_t * path_str ) {
    const char * xdg_data_home = getenv( "XDG_DATA_HOME" );
    bool         error_state   = false;

    if( xdg_data_home != NULL && strlen( xdg_data_home ) > 0 ) {
        String.append_back( path_str, xdg_data_home );
        String.append_back( path_str, "/" );

    } else {
        CTUNE_LOG( CTUNE_LOG_WARNING,
                   "[ctune_XDG_getDataBaseDir( String_t * )] "
                   "Env. variable 'XDG_DATA_HOME' not set, using default (${HOME}/%s%s).",
                   xdg.fallback_data_path, xdg.ctune_directory_name
        );

        char * home_dir = getenv( "HOME" );

        if( home_dir == NULL || strlen( home_dir ) <= 0 ) { //place in running directory as fallback
            CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_XDG_getDataBaseDir( String_t * )] Env. variable 'HOME' not found.\"" )
            error_state = true;

        } else {
            String.append_back( path_str, home_dir );
            String.append_back( path_str, "/" );
            String.append_back( path_str, xdg.fallback_data_path );
        }
    }

    String.append_back( path_str, xdg.ctune_directory_name );
    CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_XDG_getDataBaseDir( String_t * )] Base data dir set as: %s", path_str->_raw );

    return !( error_state );
}

/**
 * Resolves the application configuration directory path and append given filename to it
 * @param file_name     File name to resolve on the application's configuration directory
 * @param resolved_path Container for the resolved file path to be stored in
 */
static void ctune_XDG_resolveCfgFilePath( const char * file_name, String_t * resolved_path ) {
    bool error_state = false;

    if( String.empty( &xdg.resolved_cfg_path ) ) {
        //cfg path never been resolved
        if( !ctune_XDG_getCfgBaseDir( &xdg.resolved_cfg_path ) ) {
            CTUNE_LOG( CTUNE_LOG_WARNING,
                       "[ctune_XDG_resolveCfgFilePath( \"%s\", %p )] "
                       "Failed to resolve config data path: using current directory as base.",
                       file_name, resolved_path
            );

            error_state = true;
        }
    }

    ctune_fs.createDirectory( &xdg.resolved_cfg_path );

    //copy resolved ${config dir path}/${file_name}
    String.set( resolved_path, xdg.resolved_cfg_path._raw );
    String.append_back( resolved_path, file_name );

    if( error_state ) {
        String.free( &xdg.resolved_cfg_path );
    }
}

/**
 * Resolves the application data directory path and append given filename to it
 * @param file_name     File name to resolve on the application's configuration directory
 * @param resolved_path Container for the resolved file path to be stored in
 */
static void ctune_XDG_resolveDataFilePath( const char * file_name, String_t * resolved_path ) {
    bool error_state = false;

    if( String.empty( &xdg.resolved_data_path ) ) {
        //data path never been resolved
        if( !ctune_XDG_getDataBaseDir( &xdg.resolved_data_path ) ) {
            CTUNE_LOG( CTUNE_LOG_WARNING,
                       "[ctune_XDG_resolveDataFilePath( \"%s\", %p )] "
                       "Failed to resolve config data path: using current directory as base.",
                       file_name, resolved_path
            );

            error_state = true;
        }
    }

    ctune_fs.createDirectory( &xdg.resolved_data_path );

    //copy resolved ${data dir path}/${file_name}
    String.set( resolved_path, xdg.resolved_data_path._raw );
    String.append_back( resolved_path, file_name );

    if( error_state ) {
        String.free( &xdg.resolved_data_path );
    }
}

/**
 * De-allocates anything stored on the heap
 */
static void ctune_XDG_free( void ) {
    String.free( &xdg.resolved_cfg_path );
    String.free( &xdg.resolved_data_path );
}


/**
 * Namespace constructor
 */
const struct ctune_XDG_Instance ctune_XDG = {
    .resolveCfgFilePath  = &ctune_XDG_resolveCfgFilePath,
    .resolveDataFilePath = &ctune_XDG_resolveDataFilePath,
    .free                = &ctune_XDG_free,
};