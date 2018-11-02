#ifndef SANDIA_H
#define SANDIA_H

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib")

#else
#define _POSIX_C_SOURCE 200112L
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#endif

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef enum sandia_error_t {
        success = 1,
        error_string = -100,
        error_create_socket,
        error_get_host,
        error_connect,
        error_socket_not_ready,
        error_connection,
        error_not_connected,
        error_header_limit,
        error_receive,
        error_send,
        error_host_name
    } sandia_error;

    typedef enum request_mode_t {
        GET,
        POST
    } request_mode;

    typedef enum http_version_t {
        HTTP_09, HTTP_10, HTTP_11, HTTP_20, UNKNOWN
    } http_version;

    typedef struct sandia_socket_t {
#ifdef _WIN32    
        WSADATA wsa;
#endif
        char* host_address;
        uint32_t port;
        char* ip;
        
        int _fd;
        struct sockaddr_in _address;
        struct hostent * _host;
        struct in_addr **_addresses;

        uint32_t receive_buffer_size;
    } sandia_socket;

    typedef struct sandia_header_t {
        char* key;
        char* value;
    } sandia_header;

    typedef struct sandia_t {
        sandia_error last_error;
        sandia_socket _sandia_socket;
        bool _is_valid;

        char* body;
        size_t body_length;

        char* _request;
        size_t _request_length;

        sandia_header* _headers;
        uint32_t _header_count;

        http_version version;
    } sandia;



    sandia sandia_create(char*, uint32_t);
    void sandia_close(sandia*);
    sandia_error sandia_setup_socket(sandia*, char*, uint32_t);

    sandia_error sandia_forge_request(sandia*, request_mode, char*, char*, size_t);
    sandia_error sandia_get_request(sandia*, char*);
    sandia_error sandia_post_request(sandia*, char*, char*, size_t);

    bool sandia_send_data(sandia*, char*, size_t);
    bool sandia_receive_data(sandia*);

    bool sandia_is_connected(sandia*);

    bool sandia_build_request(sandia*, request_mode);
    bool sandia_append_request_size(sandia*, char*, size_t);
    bool sandia_append_request(sandia* s, char*);

    char* sandia_version_to_string(http_version);
    http_version sandia_string_to_version(char*);

    sandia_error sandia_add_header(sandia*, char*, char*);
    sandia_error sandia_add_headers(sandia*, sandia_header*, uint32_t);

#ifdef __cplusplus
}
#endif

#endif /* SANDIA_H */

