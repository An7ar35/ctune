#ifndef CTUNE_DTO_NEWRADIOSTATION_H
#define CTUNE_DTO_NEWRADIOSTATION_H

#include <stdlib.h>
#include <stdio.h>

#include "../datastructure/String.h"
#include "../datastructure/StrList.h"


/* TODO THIS IS NOT FULLY IMPLEMENTED IN CTUNE FOR THE MOMENT AND HAS NO USER-FACING INTERFACE
 *      This will need a lot of validation to stop mistakes and incomplete entries being sent to pollute
 *      the RadioBrowser.info database further. This is why I've opted to not implement that functionality
 *      for the moment. Maybe at a later stage if/when there is a possibility to edit/correct mistakes and
 *      duplicate entries upstream. Here's some old commented testing code to help future implementation...
 */

//{ //ADD A RADIO STATION
//    struct ctune_NewRadioStation nrs;
//    ctune_NewRadioStation_init( &nrs );
//
//    fprintf( stdout, "Sending...\n" );
//    ctune_NewRadioStation_printSend( stdout, &nrs );
//    fprintf( stdout, "\n\n" );
//
//    if( ctune_RadioBrowser.addNewStation( &ctune_vars.radio_browser_servers, ctune_Settings.getNetworkTimeoutVal(), &nrs ) ) {
//        fprintf( stdout, "Received...\n" );
//        ctune_NewRadioStation_printRcv( stdout, &nrs );
//        fprintf( stdout, "\n" );
//
//    } else {
//        fprintf( stderr, "Error adding radio stations.\n" );
//    }
//
//    StrList.free_strlist( &nrs.send.tags );
//}


/**
 * New radio station object for the RadioBrowser API
 */
typedef struct ctune_NewRadioStation {
    struct { //Sent values
        char *         name;        //MANDATORY - name of the radio station (400 chars max)
        char *         url;         //MANDATORY - URL of the station
        char *         homepage;    //homepage URL of the station
        char *         favicon;     //URL of an image file (jpg or png)
        char *         country;     //name of the country where the radio station is located
        char *         countrycode; //2 letter countrycode of the country where the radio station is located
        char *         state;       //name of the part of the country where the station is located
        char *         language;    //main language used in spoken text parts of the radio station
        struct StrList tags;        //list of tags separated by commas to describe the station

    } send;

    struct { //Returned values
        char * ok;
        char * message;
        char * uuid;

    } received;

} ctune_NewRadioStation_t;


extern const struct ctune_NewRadioStation_Namespace {
    /**
     * Initialise fields in the struct
     * @param stats NewRadioStation DTO pointer
     */
    void (* init)( struct ctune_NewRadioStation * nrs );

    /**
     * Frees the content of a NewRadioStation DTO
     * @param cat_item NewRadioStation DTO
     */
    void (* freeContent)( struct ctune_NewRadioStation * nrs );

    /**
     * Prints a NewRadioStation.send struct
     * @param out Output
     * @param nrs NewRadioStation instance
     */
    void (* printSend)( FILE * out, const struct ctune_NewRadioStation * nrs );

    /**
     * Prints a NewRadioStation.received struct
     * @param out Output
     * @param nrs NewRadioStation instance
     */
    void (* printRcv)( FILE * out, const struct ctune_NewRadioStation * nrs );

    /**
     * Converts a the fields in the NewRadioStation.send struct into a fully formed query parameter string ready to be appended to a path
     * @param nrs NewRadioStation struct
     * @param str String to append the formed filter query to (if any)
     * @return Number of filter parsed
     */
    void (* parameteriseSendFields)( const struct ctune_NewRadioStation * nrs, struct String * str );

    /**
     * Validates `.send` fields
     * @param nrs NewRadioStation struct
     * @return Valid state
     */
    bool (* validateSendFields)( const struct ctune_NewRadioStation * nrs );

} ctune_NewRadioStation;


#endif //CTUNE_DTO_NEWRADIOSTATION_H
