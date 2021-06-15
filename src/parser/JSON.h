#ifndef CTUNE_PARSER_JSON_H
#define CTUNE_PARSER_JSON_H

#include "../enum/StationSrc.h"
#include "../dto/ServerStats.h"
#include "../dto/ServerConfig.h"
#include "../dto/ClickCounter.h"
#include "../dto/RadioStationVote.h"
#include "../dto/NewRadioStation.h"
#include "../datastructure/String.h"
#include "../datastructure/Vector.h"

extern const struct ctune_parser_JSON_Namespace {
    /**
     * Parse a raw JSON formatted string into a ServerStats DTO struct
     * @param raw_str Raw JSON string
     * @param stats   ServerStats DTO instance
     * @return Success
     */
    bool (* parseToServerStats)( const struct String * raw_str, struct ctune_ServerStats * stats );

    /**
     * Parse a raw JSON formatted string into a ServerConfig DTO struct
     * @param raw_str Raw JSON string
     * @param cfg     ServerConfig DTO instance
     * @return Success
     */
    bool (* parseToServerConfig)( const struct String * raw_str, struct ctune_ServerConfig * cfg );

    /**
     * Parse a raw JSON formatted string into a collection of RadioStationInfo
     * @param raw_str        Raw JSON string
     * @param radio_stations RadioStationInfo collection instance
     * @return Success
     */
    bool (* parseToRadioStationList)( const struct String * raw_str, struct Vector * radio_stations );

    /**
     * Parse a raw JSON formatted string into a collection of RadioStationInfo from a specified station source
     * @param raw_str        Raw JSON string
     * @param src            Radio station source to specify in each RSI objects
     * @param radio_stations RadioStationInfo collection instance
     * @return Success
     */
    bool (* parseToRadioStationListFrom)( const struct String * raw_str, ctune_StationSrc_e src, struct Vector * radio_stations );

    /**
     * Parse a raw JSON formatted string into a collection of CategoryItems
     * @param raw_str        Raw JSON string
     * @param category_items CategoryItems collection instance
     * @return Success
     */
    bool (* parseToCategoryItemList)( const struct String * raw_str, struct Vector * category_items );

    /**
     * Parse a raw JSON formatted string into a ClickCounter DTO
     * @param raw_str     Raw JSON string
     * @param clk_counter ClickCounter DTO
     * @return Success
     */
    bool (* parseToClickCounter)( const struct String * raw_str, struct ctune_ClickCounter * clk_counter );

    /**
     * Parse a raw JSON formatted string into a RadioStationVote DTO
     * @param raw_str    Raw JSON string
     * @param vote_state RadioStationVote DTO
     * @return Success
     */
    bool (* parseToRadioStationVote)( const struct String * raw_str, struct ctune_RadioStationVote * vote_state );

    /**
     * Parse a raw JSON formatted string into a NewRadioStation.received DTO
     * @param raw_str     Raw JSON string
     * @param new_station NewRadioStation DTO
     * @return Success
     */
    bool (* parseToNewRadioStationRcv)( const struct String * raw_str, struct ctune_NewRadioStation * new_station );

    /**
     * JSON-ify a list of station object into a JSON string
     * @param stations Collection of RadioStationInfo_t objects
     * @param json_str Container string to store the json string into
     * @return Success
     */
    bool (* parseRadioStationListToJSON)( const struct Vector * stations, struct String * json_str );

} ctune_parser_JSON;

#endif //CTUNE_PARSER_JSON_H
