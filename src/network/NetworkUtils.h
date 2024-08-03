#ifndef CTUNE_NETWORK_NETWORKUTILS_H
#define CTUNE_NETWORK_NETWORKUTILS_H

#include <netdb.h>

#include "../datastructure/StrList.h"
#include "../datastructure/ServerList.h"
#include "../datastructure/String.h"

typedef struct {
    /**
     * NS lookup on a hostname
     * @param hostname Hostname
     * @param list     Storage for results
     * @return Success
     */
    bool (* nslookup)( const char * hostname, const char * service, struct ctune_ServerList * server_list );

    /**
     * Curl fetch over HTTPS
     * @param host    Host information
     * @param path    Path
     * @param timeout Socket timeout value to use (seconds)
     * @param answer  String container for the data fetched
     * @return HTTP code
     */
    long (* curlSecureFetch)( const ServerListNode * host, const char * path, long timeout, struct String * answer );

    /**
     * Validates a URL
     * @param url URL string
     * @return Validation state
     */
    bool (* validateURL)( const char * url );

} ctune_NetworkUtils_Namespace;

extern ctune_NetworkUtils_Namespace const ctune_NetworkUtils;

#endif //CTUNE_NETWORK_NETWORKUTILS_H