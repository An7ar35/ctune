#include "RadioBrowser.h"

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "project_version.h"
#include "../logger/Logger.h"
#include "NetworkUtils.h"
#include "../parser/JSON.h"
#include "../ctune_err.h"

#define CTUNE_RADIOBROWSER_DNS_ADDRESS  "all.api.radio-browser.info"
#define CTUNE_RADIOBROWSER_SERVICE_PORT "443" //https

/**
 * [PRIVATE] Shuffles a server list
 * @param addr_list Address StrList to randomize
 * @return Success
 */
static int ctune_RadioBrowser_randomizeServerList( ctune_ServerList_t * addr_list ) {
    srand( time(NULL ) );

    size_t item_count = ctune_ServerList.size( addr_list );

    for( size_t k = item_count; k > 0; --k ) {
        int pick = rand() % k;

        ServerListNode * node = ctune_ServerList.at( addr_list, pick );

        if( node == NULL ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[RadioInfo.randomize_server_list( ServerList * )] "
                       "Randomised index pick returns NULL (i=%i, k=%i)",
                       pick, k
            );

            return false;
        }

        node = ctune_ServerList.extract_node( addr_list, node );
        ctune_ServerList.emplace_back( addr_list, node );
    }

    return true;
}

/**
 * [PRIVATE] Download relevant data pertaining to a query
 * @param addr_list  List of available API servers for querying
 * @param timeout    Socket timeout value to use (seconds)
 * @param path       File path to get the data from
 * @param param      File path parameters
 * @param rcv_buffer String to store the response to the query into
 * @return Success
 */
static bool ctune_RadioBrowser_downloadRadioBrowserData( ctune_ServerList_t * addr_list, int timeout, const char * path, String_t * rcv_buffer ) {

    CTUNE_LOG( CTUNE_LOG_TRACE,
               "[ctune_RadioBrowser_downloadRadioBrowserData( %p, \"%s\", %p )] Attempting to download data...",
               addr_list, path, rcv_buffer
    );

    if( addr_list == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_RadioBrowser_downloadRadioBrowserData( %p, \"%s\", %p )] ServerList parameter is NULL.",
                   addr_list, path, rcv_buffer
        );

        return false;
    }

    if( ctune_ServerList.size( addr_list ) == 0 ) {
        if( !ctune_NetworkUtils.nslookup( CTUNE_RADIOBROWSER_DNS_ADDRESS, CTUNE_RADIOBROWSER_SERVICE_PORT, addr_list ) )
            return false;

        ctune_RadioBrowser_randomizeServerList( addr_list );
    }

    const char * query_format = "GET %s\r\n"
                                "User-Agent: %s\r\n"
                                "Host: https://%s\r\n" //root endpoint
                                "Content-type: application/json; charset=utf-8\r\n\r\n";

    const size_t query_buff_length = 1024;
    char         query_buff[query_buff_length];

    int cx = snprintf( query_buff, query_buff_length, query_format, path, CTUNE_USERAGENT, addr_list->_front->hostname );

    #ifndef NDEBUG
        CTUNE_LOG( CTUNE_LOG_TRACE, "\n[HTTP request]\n==============\n%s\n==============\n", query_buff );
    #endif

    if( cx < 0 || (size_t) cx >= query_buff_length ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_RadioBrowser_downloadRadioBrowserData( %p, \"%s\", %p )] "
                   "Failed to construct the query (buffer too small, req. length: %i)",
                   addr_list, path, rcv_buffer, cx
        );

        ctune_err.set( CTUNE_ERR_BUFF_ALLOC );
        return false;
    }

    ServerListNode * curr_srv      = addr_list->_front;
    bool             fetch_success = true;

    while( curr_srv && !( fetch_success = ctune_NetworkUtils.sfetch( curr_srv, timeout, query_buff, rcv_buffer ) ) )
        curr_srv = ctune_ServerList.remove( addr_list, curr_srv );

    if( !fetch_success ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_RadioBrowser_downloadRadioBrowserData( %p, \"%s\", %p )] Failed fetching.",
                   addr_list, path, rcv_buffer
        );

        return false;
    }

    if( String.empty( rcv_buffer ) ) {
        CTUNE_LOG( CTUNE_LOG_WARNING,
                   "[ctune_RadioBrowser_downloadRadioBrowserData( %p, \"%s\", %p )] No data returned.",
                   addr_list, path, rcv_buffer, rcv_buffer->_raw );

        return false;
    }

    CTUNE_LOG( CTUNE_LOG_TRACE,
               "[ctune_RadioBrowser_downloadRadioBrowserData( %p, \"%s\", %p )] Download successful.",
               addr_list, path, rcv_buffer
    );

    return true;
}

/**
 * Download RadioBrowser server stats of the first valid server in the server address list
 * @param addr_list List of available API servers for querying
 * @param timeout   Socket timeout value to use (seconds)
 * @param stats     ServerStats DTO
 * @return Success
 */
static bool ctune_RadioBrowser_downloadServerStats( ctune_ServerList_t * addr_list, int timeout, ctune_ServerStats_t * stats ) {
    static const char * path = "/json/stats";

    struct String rcv_buff = String.init();
    bool          dwl_ok   = ctune_RadioBrowser_downloadRadioBrowserData( addr_list, timeout, path, &rcv_buff );
    bool          parse_ok = true;

    if( dwl_ok ) {
        if( !( parse_ok = ctune_parser_JSON.parseToServerStats( &rcv_buff, stats ) ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_RadioBrowser_downloadServerStats( %p, %p )] Error downloading data (uri=\"%s\").",
                       addr_list, stats, path
            );
            CTUNE_LOG( CTUNE_LOG_TRACE,
                       "[ctune_RadioBrowser_downloadServerStats( %p, %p )] rcv_buff:\n%s",
                       addr_list, stats, ( String.empty( &rcv_buff ) ? "\"\"" : rcv_buff._raw )
            );
        }
    } else {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_RadioBrowser_downloadServerStats( %p, %p )] Error parsing data (uri=\"%s\").",
                   addr_list, stats, path
        );
    }

    String.free( &rcv_buff );  //free buffer String

    return ( dwl_ok && parse_ok );
}

/**
 * Download RadioBrowser server configuration of the first valid server in the server address list
 * @param addr_list List of available API servers for querying
 * @param timeout   Socket timeout value to use (seconds)
 * @param srv_cfg   ServerConfig DTO
 * @return Success
 */
static bool ctune_RadioBrowser_downloadServerConfig( ctune_ServerList_t * addr_list, int timeout, ctune_ServerConfig_t * srv_cfg ) {
    static const char * path = "/json/config";

    struct String rcv_buff = String.init();
    bool          dwl_ok   = ctune_RadioBrowser_downloadRadioBrowserData( addr_list, timeout, path, &rcv_buff );
    bool          parse_ok = true;

    if( dwl_ok ) {
        if( !( parse_ok = ctune_parser_JSON.parseToServerConfig( &rcv_buff, srv_cfg ) ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_RadioBrowser_downloadServerConfig( %p, %p )] Error parsing data (uri=\"%s\").",
                       addr_list, srv_cfg, path
            );
            CTUNE_LOG( CTUNE_LOG_TRACE,
                       "[ctune_RadioBrowser_downloadServerConfig( %p, %p )] rcv_buff:\n%s",
                       addr_list, srv_cfg, ( String.empty( &rcv_buff ) ? "\"\"" : rcv_buff._raw )
            );
        }

    } else {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_RadioBrowser_downloadServerConfig( %p, %p )] Error downloading data (uri=\"%s\").",
                   addr_list, srv_cfg, path
        );
    }

    String.free( &rcv_buff );  //free buffer String

    return ( dwl_ok && parse_ok );
}

/**
 * Download radio stations information
 * @param addr_list      List of available source servers
 * @param timeout        Socket timeout value to use (seconds)
 * @param hide_broken    Flag to hide broken station (default=true)
 * @param filter         Search filter
 * @param radio_stations Data-structure to store the RadioStationInfo DTOs into
 * @return Success
 */
static bool ctune_RadioBrowser_downloadStations(
    ctune_ServerList_t               * addr_list,
    int                                timeout,
    const ctune_RadioBrowserFilter_t * filter,
    Vector_t                         * radio_stations )
{
    static const char * base_path = "/json/stations/search";
    struct String       final_uri = String.init();
    struct String       rcv_buff  = String.init();

    {
        struct String filter_str = String.init();

        ctune_RadioBrowserFilter.parameteriseFields( filter, &filter_str );
        String.append_back( &final_uri, base_path );
        String.append_back( &final_uri, filter_str._raw );

        String.free( &filter_str );
    }

    bool dwl_ok   = ctune_RadioBrowser_downloadRadioBrowserData( addr_list, timeout, final_uri._raw, &rcv_buff );
    bool parse_ok = true;

    if( dwl_ok ) {
        if( !( parse_ok = ctune_parser_JSON.parseToRadioStationListFrom( &rcv_buff, CTUNE_STATIONSRC_RADIOBROWSER, radio_stations ) ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_RadioBrowser_downloadStations( %p, %p, %p )] Error downloading data (uri=\"%s\").",
                       addr_list, filter, radio_stations, final_uri._raw
            );
            CTUNE_LOG( CTUNE_LOG_TRACE,
                       "[ctune_RadioBrowser_downloadStations( %p, %p, %p )] rcv_buff:\n%s",
                       addr_list, filter, radio_stations, ( String.empty( &rcv_buff ) ? "\"\"" : rcv_buff._raw )
            );
        }
    } else {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_RadioBrowser_downloadStations( %p, %p, %p )] Error parsing data (uri=\"%s\").",
                   addr_list, filter, radio_stations, final_uri._raw
        );
    }

    String.free( &rcv_buff );  //free buffer String
    String.free( &final_uri );

    return ( dwl_ok && parse_ok );
}

/**
 * Download items within a specified category
 * @param addr_list      List of available source servers
 * @param timeout        Socket timeout value to use (seconds)
 * @param category       Category for which to download items from
 * @param filter         Search filter
 * @param category_items Data-structure to store the category items (`ctune_CategoryItem`) into
 * @return Success
 */
static bool ctune_RadioBrowser_downloadCategoryItems(
    ctune_ServerList_t               * addr_list,
    int                                timeout,
    const ctune_ListCategory_e         category,
    const ctune_RadioBrowserFilter_t * filter,
    Vector_t                         * category_items )
{
    //all cats return { name, stationcount } except the 'states' cat that also has { .., country }
    static const char * base_path = "/json/";
    struct String       final_uri = String.init();
    struct String       rcv_buff  = String.init();

    {
        struct String filter_str = String.init();

        ctune_RadioBrowserFilter.parameteriseFields( filter, &filter_str );

        String.append_back( &final_uri, base_path );
        String.append_back( &final_uri, ctune_ListCategory.str( category ) );
        String.append_back( &final_uri, filter_str._raw );

        String.free( &filter_str );
    }

    bool dwl_ok   = ctune_RadioBrowser_downloadRadioBrowserData( addr_list, timeout, final_uri._raw, &rcv_buff );
    bool parse_ok = true;

    if( dwl_ok ) {
        if( !( parse_ok = ctune_parser_JSON.parseToCategoryItemList( &rcv_buff, category_items ) ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_RadioBrowser_downloadCategoryies( %p, %i, %p, %p )] Error parsing data (uri=\"%s\").",
                       addr_list, category, filter, category_items, final_uri._raw
            );
            CTUNE_LOG( CTUNE_LOG_TRACE,
                       "[ctune_RadioBrowser_downloadCategoryies( %p, %i, %p, %p )] rcv_buff:\n%s",
                       addr_list, category, filter, category_items, ( String.empty( &rcv_buff ) ? "\"\"" : rcv_buff._raw )
            );
        }
    } else {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_RadioBrowser_downloadCategoryies( %p, %i, %p, %p )] Error downloading data (uri=\"%s\").",
                   addr_list, category, filter, category_items, final_uri._raw
        );
    }

    String.free( &rcv_buff );  //free buffer String
    String.free( &final_uri ); //free constructed URI String

    return ( dwl_ok && parse_ok );
}

/**
 * Increases the click count of a station by 1
 * @param addr_list     List of available source servers
 * @param timeout       Socket timeout value to use (seconds)
 * @param station_uuid  Station UUID
 * @param click_counter StationClickCounter DTO to store returned message into
 * @return Success
 */
static bool ctune_RadioBrowser_increaseClickCounter( ctune_ServerList_t * addr_list, int timeout, const char * station_uuid, ctune_ClickCounter_t * click_counter ) {
    static const char * base_path = "/json/url/";

    struct String final_uri = String.init();
    String.append_back( &final_uri, base_path );
    String.append_back( &final_uri, station_uuid );

    struct String rcv_buff = String.init();
    bool          dwl_ok   = ctune_RadioBrowser_downloadRadioBrowserData( addr_list, timeout, final_uri._raw, &rcv_buff );
    bool          parse_ok = true;

    if( dwl_ok ) {
        if( !( parse_ok = ctune_parser_JSON.parseToClickCounter( &rcv_buff, click_counter ) ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_RadioBrowser_increaseClickCounter( %p, \"%s\", %p )] Error parsing data (uri=\"%s\").",
                       addr_list, station_uuid, click_counter, final_uri._raw
            );
            CTUNE_LOG( CTUNE_LOG_TRACE,
                       "[ctune_RadioBrowser_increaseClickCounter( %p, \"%s\", %p )] rcv_buff:\n%s",
                       addr_list, station_uuid, click_counter, ( String.empty( &rcv_buff ) ? "\"\"" : rcv_buff._raw )
            );
        }
    } else {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_RadioBrowser_increaseClickCounter( %p, \"%s\", %p )] Error downloading data (uri=\"%s\").",
                   addr_list, station_uuid, click_counter, final_uri._raw
        );
    }

    String.free( &rcv_buff );  //free buffer String
    String.free( &final_uri ); //free constructed URI String

    return ( dwl_ok && parse_ok );
}

/**
 * Download radio station information from a category
 * @param addr_list      List of available source servers
 * @param timeout        Socket timeout value to use (seconds)
 * @param category       Category type
 * @param search_term    Search term (optional)
 * @param radio_stations Data-structure to store the RadioStationInfo DTOs into
 * @return Success
 */
static bool ctune_RadioBrowser_downloadStationsBy(
    ctune_ServerList_t       * addr_list,
    int                        timeout,
    const ctune_ByCategory_e   category,
    const char               * search_term,
    Vector_t                 * radio_stations )
{
    static const char * base_path = "/json/stations/";

    struct String final_uri = String.init();
    String.append_back( &final_uri, base_path );
    String.append_back( &final_uri, ctune_ByCategory.str( category ) );

    if( search_term != NULL ) {
        String_t encoded = String.init();
        ctune_encodeURI( search_term, &encoded );
        String.append_back( &final_uri, "/" );
        String.append_back( &final_uri, encoded._raw );
        String.free( &encoded );
    }

    struct String rcv_buff = String.init();
    bool          dwl_ok   = ctune_RadioBrowser_downloadRadioBrowserData( addr_list, timeout, final_uri._raw, &rcv_buff );
    bool          parse_ok = true;

    if( dwl_ok ) {
        if( !( parse_ok = ctune_parser_JSON.parseToRadioStationListFrom( &rcv_buff, CTUNE_STATIONSRC_RADIOBROWSER, radio_stations ) ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_RadioBrowser_downloadStationsBy( %p, %i, %i, %p, %p )] Error parsing data (uri=\"%s\").\n",
                       addr_list, timeout, category, search_term, radio_stations, final_uri._raw
            );
            CTUNE_LOG( CTUNE_LOG_TRACE,
                       "[ctune_RadioBrowser_downloadStationsBy( %p, %i, %i, %p, %p )] rcv_buff:\n%s",
                       addr_list, timeout, category, search_term, radio_stations, ( String.empty( &rcv_buff ) ? "\"\"" : rcv_buff._raw )
            );
        }
    } else {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_RadioBrowser_downloadStationsBy( %p, %i, %i, %p, %p )] Error downloading data (uri=\"%s\").",
                   addr_list, timeout, category, search_term, radio_stations, final_uri._raw
        );

    }

    String.free( &rcv_buff );  //free buffer String
    String.free( &final_uri ); //free constructed URI String

    return ( dwl_ok && parse_ok );
}

/**
 * Cast a vote for a radio station
 * @param addr_list    List of available source servers
 * @param timeout      Socket timeout value to use (seconds)
 * @param station_uuid Station UUID
 * @param vote_state   RadioStationVote DTO (for returned voting state)
 * @return Success of operation (see in the DTO for voting success)
 */
static bool ctune_RadioBrowser_voteForStation( ctune_ServerList_t * addr_list, int timeout, const char * station_uuid, ctune_RadioStationVote_t * vote_state ) {
    static const char * base_path = "/json/vote/";

    struct String final_uri = String.init();
    String.append_back( &final_uri, base_path );
    String.append_back( &final_uri, station_uuid );

    struct String rcv_buff = String.init();
    bool          dwl_ok   = ctune_RadioBrowser_downloadRadioBrowserData( addr_list, timeout, final_uri._raw, &rcv_buff );
    bool          parse_ok = true;

    if( dwl_ok ) {
        if( !( parse_ok = ctune_parser_JSON.parseToRadioStationVote( &rcv_buff, vote_state ) ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_RadioBrowser_voteForStation( %p, \"%s\", %p )] Error parsing data (uri=\"%s\").",
                       addr_list, station_uuid, vote_state, final_uri._raw
            );
            CTUNE_LOG( CTUNE_LOG_TRACE,
                       "[ctune_RadioBrowser_voteForStation( %p, \"%s\", %p )] rcv_buff:\n%s",
                       addr_list, station_uuid, vote_state, ( String.empty( &rcv_buff ) ? "\"\"" : rcv_buff._raw )
            );
        }
    } else {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_RadioBrowser_voteForStation( %p, \"%s\", %p )] Error downloading data (uri=\"%s\").",
                   addr_list, station_uuid, vote_state, final_uri._raw
        );
    }

    String.free( &rcv_buff );  //free buffer String
    String.free( &final_uri ); //free constructed URI String

    return ( dwl_ok && parse_ok );
}

/**
 * Add a new radio station to RadioBrowser API
 * @param addr_list   List of available source servers
 * @param timeout     Socket timeout value to use (seconds)
 * @param new_station NewRadioStation DTO with the required `.send` fields filled
 * @return Success of operation (see in the DTO for adding success in `.received`)
 */
static bool ctune_RadioBrowser_addNewStation( ctune_ServerList_t * addr_list, int timeout, ctune_NewRadioStation_t * new_station ) {
    static const char * base_path = "/json/add";
    struct String       final_uri = String.init();
    struct String       rcv_buff  = String.init();

    {
        struct String station_str = String.init();

        ctune_NewRadioStation.parameteriseSendFields( new_station, &station_str );

        String.append_back( &final_uri, base_path );
        String.append_back( &final_uri, station_str._raw );

        String.free( &station_str );
    }

    bool dwl_ok   = ctune_RadioBrowser_downloadRadioBrowserData( addr_list, timeout, final_uri._raw, &rcv_buff );
    bool parse_ok = true;

    if( dwl_ok ) {
        if( !( parse_ok = ctune_parser_JSON.parseToNewRadioStationRcv( &rcv_buff, new_station ) ) ) {
            CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_RadioBrowser_addNewStation( %p, %p )] Error parsing data (uri=\"%s\").",
                       addr_list, new_station, final_uri._raw
            );
            CTUNE_LOG( CTUNE_LOG_TRACE,
                       "[ctune_RadioBrowser_addNewStation( %p, %p )] rcv_buff:\n%s",
                       addr_list, new_station, rcv_buff._raw
            );
        }
    } else {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_RadioBrowser_addNewStation( %p, %p )] Error downloading data (uri=\"%s\").",
                   addr_list, new_station, final_uri._raw
        );
    }

    String.free( &rcv_buff );  //free buffer String
    String.free( &final_uri ); //free constructed URI String

    return ( dwl_ok && parse_ok );
}


ctune_RadioBrowser_Namespace const ctune_RadioBrowser = {
    .downloadServerStats   = &ctune_RadioBrowser_downloadServerStats,
    .downloadServerConfig  = &ctune_RadioBrowser_downloadServerConfig,
    .downloadStations      = &ctune_RadioBrowser_downloadStations,
    .downloadCategoryItems = &ctune_RadioBrowser_downloadCategoryItems,
    .stationClickCounter   = &ctune_RadioBrowser_increaseClickCounter,
    .downloadStationsBy    = &ctune_RadioBrowser_downloadStationsBy,
    .voteForStation        = &ctune_RadioBrowser_voteForStation,
    .addNewStation         = &ctune_RadioBrowser_addNewStation
};