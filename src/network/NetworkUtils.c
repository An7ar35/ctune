#include "NetworkUtils.h"

#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <curl/curl.h>

#include "logger/src/Logger.h"
#include "../ctune_err.h"
#include "project_version.h"

/**
 * NS lookup on a hostname
 * @param hostname Hostname
 * @param list     Storage for results
 * @return Success
 */
static bool ctune_NetworkUtils_nslookup( const char * hostname, const char * service, struct ctune_ServerList * server_list ) {
    struct addrinfo   hints;
    struct addrinfo * results     = NULL;
    bool              error_state = false;

    memset( &hints, 0, sizeof hints ); //reset struct to 0
    hints.ai_family    = AF_INET;             //IPv4
    hints.ai_socktype  = SOCK_STREAM;         //TCP socket
    hints.ai_protocol  = IPPROTO_TCP;         //TCP

    { //DNS lookup - API docs at https://linux.die.net/man/3/getaddrinfo
        int err_code = getaddrinfo( hostname, service, &hints, &results );

        if( err_code ) {
            CTUNE_LOG( CTUNE_LOG_ERROR,
                       "[ctune_NetworkUtils.nslookup( \"%s\", \"%s\", %p )] getaddrinfo error (%i): %s",
                       hostname, service, server_list, err_code, gai_strerror( err_code )
            );

            ctune_err.set( CTUNE_ERR_NSLOOKUP_ADDR );
            error_state = true;
            goto end;
        }
    }

    { //Inverse lookup - API docs at https://linux.die.net/man/3/getnameinfo
        struct addrinfo *curr = results;

        while( curr ) {
            char hostname_buff[NI_MAXHOST] = { 0 };
            int err_code = getnameinfo( curr->ai_addr, curr->ai_addrlen, hostname_buff, NI_MAXHOST, NULL, 0, 0 );

            if( err_code ) {
                CTUNE_LOG( CTUNE_LOG_ERROR,
                           "[ctune_NetworkUtils.nslookup( \"%s\", \"%s\", %p )] getnameinfo error (%i): %s",
                           hostname, service, server_list, err_code, gai_strerror( err_code )
                );

                ctune_err.set( CTUNE_ERR_NSLOOKUP_NAME );

            } else {
                ctune_ServerList.insert_back( server_list, &hostname_buff[0], curr );
            }

            curr = curr->ai_next;
        }
    }

    if( ctune_ServerList.size( server_list ) == 0 ) {
        error_state = true;
    }

    end:
        freeaddrinfo( results ); //cleanup
        return !(error_state);
}

/**
 * [PRIVATE] Curl write callback function
 * @param contents Content from curl action (not null terminated)
 * @param size     Size of content
 * @param nmemb    Size of content byte blocks
 * @param userdata Userdata pointer
 * @return Number of bytes written
 */
static size_t ctune_NetworkUtils_curlWrite_cb( char * contents, size_t size, size_t nmemb, void * userdata ) {
    size_t     real_size      = size * nmemb;
    String_t * output_string  = (String_t *) userdata;
    char     * new_string_raw = realloc( output_string->_raw, output_string->_length + real_size + 1 );

    if( new_string_raw == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_NetworkUtils_curlWrite_cb( %p, %lu, %lu, %p )] "
                   "Error reallocating output_string buffer: not enough memory.",
                   new_string_raw, size, nmemb, userdata
        );
        return 0;
    }

    output_string->_raw = new_string_raw;
    memcpy( &(output_string->_raw[output_string->_length]), contents, real_size );
    output_string->_length += real_size;
    output_string->_raw[output_string->_length] = '\0';

    return real_size;
}

/**
 * Curl fetch over HTTPS
 * @param host    Host information
 * @param path    Path
 * @param timeout Socket timeout value to use (seconds)
 * @param answer  String container for the data fetched
 * @return HTTP code
 */
static long ctune_NetworkUtils_curlSecureFetch( const ServerListNode * host, const char * path, long timeout, struct String * answer ) {
    CURL              * curl      = curl_easy_init();
    struct curl_slist * list      = NULL;
    CURLcode            curl_code = CURLE_OK;
    long                http_code = 0;

    if( curl ) {
        String_t url = String.init();

        String.append_back( &url, "https://" );
        String.append_back( &url, host->hostname );
        String.append_back( &url, path );

        curl_easy_setopt( curl, CURLOPT_URL, url._raw );
        curl_easy_setopt( curl, CURLOPT_TIMEOUT, timeout );
        curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, ctune_NetworkUtils_curlWrite_cb );
        curl_easy_setopt( curl, CURLOPT_WRITEDATA, answer );
        curl_easy_setopt( curl, CURLOPT_USERAGENT, CTUNE_USERAGENT );

        list = curl_slist_append(list, "Content-type: application/json; charset=utf-8");

        curl_code = curl_easy_perform ( curl );
        curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &http_code );

        String.free( &url );
        curl_easy_cleanup( curl );
        curl_slist_free_all( list );

    } else {
        CTUNE_LOG( CTUNE_LOG_FATAL,
                   "[ctune_NetworkUtils_curlSecureFetch( %p, \"%s\", %d, %p )] Failed to fetch: could not initialize curl",
                   host, path, timeout, answer
        );

        ctune_err.set( CTUNE_ERR_CURL_INIT );
    }

    if( curl_code == CURLE_ABORTED_BY_CALLBACK ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_NetworkUtils_curlSecureFetch( %p, \"%s\", %d, %p )] Failed to fetch: aborted by callback print function",
                   host, path, timeout, answer
        );

        ctune_err.set( CTUNE_ERR_CURL_WRITE_CALLBACK );

    } else if( curl_code != CURLE_OK || http_code != 200 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_NetworkUtils_curlSecureFetch( %p, \"%s\", %d, %p )] Failed to fetch: Curl %s / HTTP code %ld",
                   host, path, timeout, answer, ( curl_code == CURLE_OK ? "OK" : "KO" ), http_code
        );

        ctune_err.set( CTUNE_ERR_HTTP_GET );
        CTUNE_LOG( CTUNE_LOG_TRACE, "%s", answer->_raw) ;
    }

    return http_code;
}

/**
 * Validates a URL
 * @param url URL string
 * @return Validation state
 */
static bool ctune_NetworkUtils_validateURL( const char * url ) {
    bool    is_valid = false;
    CURLU * curl_URL = curl_url();
    int     ret      = curl_url_set( curl_URL, CURLUPART_URL, url, 0 );

    if( ret == CURLE_OK ) {
        is_valid = true;
        CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_NetworkUtils_validateURL( \"%s\" )] URL is valid.", url );

    } else if( ret == CURLUE_MALFORMED_INPUT ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_NetworkUtils_validateURL( \"%s\" )] Curl: malformed input.", url );

    } else {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_NetworkUtils_validateURL( \"%s\" )] Curl: error (%i).", url, ret );
    }

    curl_url_cleanup( curl_URL );

    if( !is_valid ) {
        ctune_err.set( CTUNE_ERR_INVALID_URL );
    }

    return is_valid;
}

ctune_NetworkUtils_Namespace const ctune_NetworkUtils = {
    .nslookup        = &ctune_NetworkUtils_nslookup,
    .curlSecureFetch = &ctune_NetworkUtils_curlSecureFetch,
    .validateURL     = &ctune_NetworkUtils_validateURL,
};