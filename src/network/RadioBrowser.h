#ifndef CTUNE_NETWORK_RADIOBROWSER_H
#define CTUNE_NETWORK_RADIOBROWSER_H

#include <stdbool.h>

#include "../datastructure/ServerList.h"
#include "../datastructure/StrList.h"
#include "../datastructure/Vector.h"
#include "../dto/RadioBrowserFilter.h"
#include "../dto/RadioStationInfo.h"
#include "../dto/NewRadioStation.h"
#include "../dto/ServerStats.h"
#include "../dto/ServerConfig.h"
#include "../dto/CategoryItem.h"
#include "../dto/ClickCounter.h"
#include "../dto/RadioStationVote.h"
#include "../enum/ByCategory.h"
#include "../enum/ListCategory.h"

typedef struct {
    /**
     * Download RadioBrowser server stats of the first valid server in the server address list
     * @param addr_list List of available API servers for querying
     * @param timeout   Socket timeout value to use (seconds)
     * @param stats     ServerStats DTO
     * @return Success
     */
    bool (* downloadServerStats)( ctune_ServerList_t * addr_list, int timeout, ctune_ServerStats_t * stats );

    /**
     * Download RadioBrowser server configuration of the first valid server in the server address list
     * @param addr_list List of available API servers for querying
     * @param timeout   Socket timeout value to use (seconds)
     * @param srv_cfg   ServerConfig DTO
     * @return Success
     */
    bool (* downloadServerConfig)( ctune_ServerList_t * addr_list, int timeout, ctune_ServerConfig_t * srv_cfg );

    /**
     * Download radio stations information
     * @param addr_list      List of available source servers
     * @param timeout        Socket timeout value to use (seconds)
     * @param filter         Search filter
     * @param radio_stations Data-structure to store the RadioStationInfo DTOs into
     * @return Success
     */
    bool (* downloadStations)( ctune_ServerList_t * addr_list, int timeout, const ctune_RadioBrowserFilter_t * filter, Vector_t * radio_stations );

    /**
     * Download items within a specified category
     * @param addr_list      List of available source servers
     * @param timeout        Socket timeout value to use (seconds)
     * @param category       Category for which to download items from
     * @param filter         Search filter
     * @param category_items Data-structure to store the category items (`ctune_CategoryItem`) into
     * @return Success
     */
    bool (* downloadCategoryItems)( ctune_ServerList_t * addr_list, int timeout, const ctune_ListCategory_e category, const  ctune_RadioBrowserFilter_t * filter, Vector_t * categories );

    /**
     * Increases the click count of a station by 1
     * @param addr_list     List of available source servers
     * @param timeout       Socket timeout value to use (seconds)
     * @param station_uuid  Station UUID
     * @param click_counter StationClickCounter DTO to store returned message into
     * @return Success
     */
    bool (* stationClickCounter)( ctune_ServerList_t * addr_list, int timeout, const char * station_uuid,  ctune_ClickCounter_t * click_counter );

    /**
     * Download radio station information from a category
     * @param addr_list      List of available source servers
     * @param timeout        Socket timeout value to use (seconds)
     * @param category       Category type
     * @param search_term    Search term (optional)
     * @param radio_stations Data-structure to store the RadioStationInfo DTOs into
     * @return Success
     */
    bool (* downloadStationsBy)( ctune_ServerList_t * addr_list, int timeout, const ctune_ByCategory_e category, const char * search_term, Vector_t * radio_stations );

    /**
     * Cast a vote for a radio station
     * @param addr_list    List of available source servers
     * @param timeout      Socket timeout value to use (seconds)
     * @param station_uuid Station UUID
     * @param vote_state   RadioStationVote DTO (for returned voting state)
     * @return Success of operation (see in the DTO for voting success)
     */
    bool (* voteForStation)( ctune_ServerList_t * addr_list, int timeout, const char * station_uuid,  ctune_RadioStationVote_t * vote_state );

    /**
     * Add a new radio station to RadioBrowser API
     * @param addr_list   List of available source servers
     * @param timeout     Socket timeout value to use (seconds)
     * @param new_station NewRadioStation DTO with the required `.send` fields filled
     * @return Success of operation (see in the DTO for adding success in `.received`)
     */
    bool (* addNewStation)( ctune_ServerList_t * addr_list, int timeout, ctune_NewRadioStation_t * new_station );

} ctune_RadioBrowser_Namespace;

extern ctune_RadioBrowser_Namespace const ctune_RadioBrowser;

#endif //CTUNE_NETWORK_RADIOBROWSER_H
