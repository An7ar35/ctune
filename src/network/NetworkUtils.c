#include "NetworkUtils.h"

#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <curl/curl.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "../logger/Logger.h"
#include "../ctune_err.h"

#define RCV_BUFFER_SIZE        4096
#define PROTO_STR_BUFF_LENGTH   100

/**
 * [PRIVATE] Container for easy printing of connection properties (used internally only)
 */
struct HostInfo {
    char _ip[INET6_ADDRSTRLEN];
    long _port;
    char _prot_name[PROTO_STR_BUFF_LENGTH];
};

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

    memset( &hints, 0, sizeof hints );   //reset struct to 0
    hints.ai_family    = AF_INET;           //IPv4
    hints.ai_socktype  = SOCK_STREAM;       //TCP socket
    hints.ai_protocol  = IPPROTO_TCP;       //TCP

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
 * [PRIVATE] Pack into a struct an more easily printable definition of the socket address and port from `sockaddr_storage`
 * @param family   IP family type
 * @param sa       `sockaddr` container
 * @param protocol Protocol number
 * @param host_ip  IP information container
 * @return Success
 */
static bool ctune_NetworkUtils_packHostInfo( const int family, const struct sockaddr_storage * sa, const int protocol, struct HostInfo * host_ip ) {
    switch( family ) {
        case AF_INET: {
            struct sockaddr_in *addr_in = (struct sockaddr_in *) sa;
            inet_ntop( AF_INET, &(addr_in->sin_addr ), host_ip->_ip, INET_ADDRSTRLEN );
            host_ip->_port = ntohs( addr_in->sin_port );
            break;
        }

        case AF_INET6: {
            struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *) sa;
            inet_ntop( AF_INET6, &(addr_in6->sin6_addr), host_ip->_ip, INET6_ADDRSTRLEN );
            host_ip->_port = ntohs( addr_in6->sin6_port );
            break;
        }

        default:
            CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_NetworkUtils_getHostIP(..)] Family type unknown (%i).", family );
            ctune_err.set( CTUNE_ERR_NETWORK_IO );
            return false;
    }

    struct protoent * proto = getprotobynumber( protocol );

    if( strlen( proto->p_name ) < PROTO_STR_BUFF_LENGTH ) {
        strcpy( host_ip->_prot_name, proto->p_name );
    } else {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_NetworkUtils_getHostIP(..)] Protocol (%i) string length exceed buffer length.", protocol );
        ctune_err.set( CTUNE_ERR_BUFF_OVERFLOW );
    }

    return true;
}

/**
 * [PRIVATE] Creates a connected socket
 * @param host      Host to connect to
 * @param host_info Host information container
 * @param timeout   Socket timout value in seconds
 * @return Connected socket file descriptor (-1 if error)
 */
static int ctune_NetworkUtils_createSocket( const ServerListNode * host, const struct HostInfo * host_info, int timeout ) {
    CTUNE_LOG( CTUNE_LOG_DEBUG,
               "[ctune_NetworkUtils_createSocket(..)] Trying to connect to \"%s\" (%s %li '%s').",
               host->hostname, host_info->_ip, host_info->_port, host_info->_prot_name
    );

    int sock_fd = socket( host->ai_family, host->ai_socktype, host->ai_protocol );

    //set socket options (https://www.freebsd.org/cgi/man.cgi?query=setsockopt&sektion=2)
    struct timeval timeout_val;
    timeout_val.tv_sec  = timeout;
    timeout_val.tv_usec = 0;

    if( setsockopt( sock_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout_val, sizeof( timeout_val ) ) < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_NetworkUtils_createSocket(..)] "
                   "Error setting input timeout socket option (\"%s\").",
                   host->hostname
        );

        ctune_err.set( CTUNE_ERR_SOCK_SETUP );
        return -1;
    }

    if( setsockopt( sock_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout_val, sizeof( timeout_val ) ) < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_NetworkUtils_createSocket(..)] Error setting output timeout socket option (\"%s\").",
                   host->hostname
        );

        ctune_err.set( CTUNE_ERR_SOCK_SETUP );
        return -1;
    }

    if( sock_fd == -1 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_NetworkUtils_createSocket(..)] Could not create socket (\"%s\").",
                   host->hostname
        );

        ctune_err.set( CTUNE_ERR_SOCK_SETUP );
        return -1;
    }

    //connect socket
    if( connect( sock_fd, (struct sockaddr *)host->ai_addr, host->ai_addrlen ) < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_NetworkUtils_createSocket(..)] Could not connect to \"%s\".",
                   host->hostname
        );

        ctune_err.set( CTUNE_ERR_SOCK_CONNECT );
        return -1;
    }

    CTUNE_LOG( CTUNE_LOG_MSG,
               "[ctune_NetworkUtils_createSocket(..)] Successfully connected to \"%s\" (%s:%li).",
               host->hostname, host_info->_ip, host_info->_port
    );

    return sock_fd;
}

/**
 * [PRIVATE] Call all the cleaning methods for SSL
 * @param ssl_ctx     SSL context
 * @param ssl         SSL connection
 * @param certificate X509 certificate
 */
static void ctune_NetworkUtils_shutdownSSL( SSL_CTX * ssl_ctx, SSL * ssl, X509 * certificate ) {
    ERR_free_strings();
    EVP_cleanup();

    if( ssl ) {
        SSL_shutdown( ssl );
        SSL_free( ssl );
    }

    if( ssl_ctx )
        SSL_CTX_free( ssl_ctx );

    if( certificate )
        X509_free( certificate );
}

/**
 * Fetch over HTTP
 * @param host    Host information
 * @param timeout Socket timeout value to use (seconds)
 * @param msg     Query message
 * @param answer  String container for the data fetched
 * @return Success
 */
static bool ctune_NetworkUtils_fetch( const ServerListNode * host, int timeout, const char * msg, struct String * answer ) {
    struct HostInfo host_info; //easy to print-out info (ip+port)
    bool            error_state = false;

    if( !ctune_NetworkUtils_packHostInfo( host->ai_family, host->ai_addr, host->ai_protocol, &host_info ) ) {
        return false;
    }

    int sock_fd = ctune_NetworkUtils_createSocket( host, &host_info, timeout );

    if( sock_fd == -1 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_NetworkUtils.fetch(..)] Could not create socket (\"%s\").", host->hostname );
        error_state = true;
        goto end;
    }

    //Send query
    int bytes_sent = write( sock_fd, msg, strlen( msg ) );

    if( bytes_sent < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_NetworkUtils.fetch(..)] Could not write query to \"%s\" (%s:%li): %s.",
                   host->hostname, host_info._ip, host_info._port, strerror( errno )
        );

        ctune_err.set( CTUNE_ERR_SOCK_WRITE );
        error_state = true;
        goto end;
    }

    //Receive answer to query
    while( 1 ) {
        char buffer[RCV_BUFFER_SIZE + 2];

        int bytes_rcv = read( sock_fd, buffer, RCV_BUFFER_SIZE );

        if( bytes_rcv == 0 )
            break;

        if( bytes_rcv < 0 ) {
            CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_NetworkUtils.fetch(..)] Failed to receive data from \"%s\".", host->hostname );
            ctune_err.set( CTUNE_ERR_SOCK_READ );
            error_state = true;
            goto end;
        }

        buffer[bytes_rcv] = '\0';
        String.append_back( answer, buffer );
    }

    end:
        if( sock_fd != -1 )
            close( sock_fd );

        return !(error_state);
}

/**
 * Secure fetch over HTTPS
 * @param host    Host information
 * @param timeout Socket timeout value to use (seconds)
 * @param msg     Query message
 * @param answer  String container for the data fetched
 * @return Success
 */
static bool ctune_NetworkUtils_sfetch( const ServerListNode * host, int timeout, const char * msg, struct String * answer ) {
    SSL_CTX * ssl_ctx     = NULL;
    SSL     * ssl         = NULL;
    X509    * certificate = NULL;
    int       sock_fd     = -1;
    bool      error_state = false;

    //SSL - init
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
    SSL_load_error_strings();

    if( SSL_library_init() < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_NetworkUtils.sfetch(..)] Could not init SSLlib." );
        ctune_err.set( CTUNE_ERR_SOCK_SSL_SETUP );
        error_state = true;
        goto end;
    }

    const SSL_METHOD * ssl_method = SSLv23_client_method();

    if ( ( ssl_ctx = SSL_CTX_new( ssl_method ) ) == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_NetworkUtils.sfetch(..)] Unable to create a new SSL context structure." );
        ctune_err.set( CTUNE_ERR_SOCK_SSL_SETUP );
        error_state = true;
        goto end;
    }

    SSL_CTX_set_options( ssl_ctx, SSL_OP_NO_SSLv2 );
    ssl = SSL_new( ssl_ctx );

    //setup socket
    struct HostInfo host_info; //easy to print-out info (ip+port)

    if( !ctune_NetworkUtils_packHostInfo( host->ai_family, host->ai_addr, host->ai_protocol, &host_info ) ) {
        error_state = true;
        goto end;
    }

    sock_fd = ctune_NetworkUtils_createSocket( host, &host_info, timeout );

    if( sock_fd == -1 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_NetworkUtils.sfetch(..)] Could not create socket (\"%s\").", host->hostname );
        error_state = true;
        goto end;
    }

    SSL_set_fd( ssl, sock_fd );       //attach SSL session to socket
    int ret_val = SSL_connect( ssl ); //initiate SSL connection

    switch( ret_val  ) {
        case 1:
            CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_NetworkUtils.sfetch(..)] Successfully enabled SSL/TLS session to \"%s\".", host->hostname );
            break;

        case 0:
            CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_NetworkUtils.sfetch(..)] Unsuccessful SSL session handshake to \"%s\": %s",
                       host->hostname, ERR_reason_error_string( SSL_get_error( ssl, ret_val ) )
            );
            ctune_err.set( CTUNE_ERR_SOCK_SCONNECT );
            error_state = true;
            goto end;

        default:
            CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_NetworkUtils.sfetch(..)] Could not build a SSL session to \"%s\": %s",
                       host->hostname, ERR_reason_error_string( SSL_get_error( ssl, ret_val ) )
            );
            ctune_err.set( CTUNE_ERR_SOCK_SCONNECT );
            error_state = true;
            goto end;
    }

    certificate = SSL_get_peer_certificate( ssl ); //get the remote certificate

    if( certificate == NULL ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_NetworkUtils.sfetch(..)] Could not get a certificate from \"%s\".", host->hostname );
        ctune_err.set( CTUNE_ERR_SOCK_SSL_CERT );
        error_state = true;
        goto end;

    } else {
        CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_NetworkUtils.sfetch(..)] Retrieved server certificate from: \"%s\".", host->hostname );
    }

    //Send query
    int bytes_sent = SSL_write( ssl, msg, strlen( msg ) );
    if( bytes_sent < 0 ) {
        CTUNE_LOG( CTUNE_LOG_ERROR,
                   "[ctune_NetworkUtils.sfetch(..)] Could not write query to \"%s\" (%s:%li): %s.\n",
                   host->hostname, host_info._ip, host_info._port, ERR_reason_error_string( SSL_get_error( ssl, bytes_sent ) )
        );

        ctune_err.set( CTUNE_ERR_SOCK_SREAD );
        error_state = true;
        goto end;
    }

    //Receive answer to query
    while( 1 ) {
        char buffer[RCV_BUFFER_SIZE + 2];

        int bytes_rcv = SSL_read( ssl, buffer, RCV_BUFFER_SIZE );

        if( bytes_rcv == 0 )
            break;

        if( bytes_rcv < 0 ) {
            CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_NetworkUtils.sfetch(..)] Failed to receive data from \"%s\": %s\n",
                       host->hostname, ERR_lib_error_string( SSL_get_error( ssl, bytes_rcv ) )
            );

            ctune_err.set( CTUNE_ERR_SOCK_SREAD );
            error_state = true;
            goto end;
        }

        buffer[bytes_rcv] = '\0';
        String.append_back( answer, buffer );
    }

    end:
        //SSL - destroy & cleanup
        ctune_NetworkUtils_shutdownSSL( ssl_ctx, ssl, certificate );

        if( sock_fd != -1 )
            close( sock_fd );

        return !(error_state);
}

/**
 * Validates a URL
 * @param url URL string
 * @return Validation state
 */
static bool ctune_NetworkUtils_validateURL( const char * url ) {
    bool    is_valid = false;
    CURLU * curlu    = curl_url();
    int     ret      = curl_url_set( curlu, CURLUPART_URL, url, 0 );

    if( ret == CURLE_OK ) {
        is_valid = true;
        CTUNE_LOG( CTUNE_LOG_DEBUG, "[ctune_NetworkUtils_validateURL( \"%s\" )] URL is valid.", url );

    } else if( ret == CURLUE_MALFORMED_INPUT ) {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_NetworkUtils_validateURL( \"%s\" )] Curl: malformed input.", url );

    } else {
        CTUNE_LOG( CTUNE_LOG_ERROR, "[ctune_NetworkUtils_validateURL( \"%s\" )] Curl: error (%i).", url, ret );
    }

    curl_url_cleanup( curlu );

    if( !is_valid )
        ctune_err.set( CTUNE_ERR_INVALID_URL );

    return is_valid;
}

ctune_NetworkUtils_Namespace const ctune_NetworkUtils = {
    .nslookup    = &ctune_NetworkUtils_nslookup,
    .fetch       = &ctune_NetworkUtils_fetch,
    .sfetch      = &ctune_NetworkUtils_sfetch,
    .validateURL = &ctune_NetworkUtils_validateURL,
};