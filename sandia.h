#ifndef SANDIA_H
#define SANDIA_H

#ifdef _WIN32

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
#include <signal.h>

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
        error_not_connected
    } sandia_error;

    typedef struct sandia_socket_t {
        char* host_address;
        char ip_address[128];
        char* port;
        int _fd;
        struct addrinfo* _host;
        struct addrinfo* _info;
    } sandia_socket;

    typedef struct sandia_t {
        /* attributes*/
        sandia_error last_error;
        sandia_socket _sandia_socket;
        bool _is_valid;

        char* user_agent;
        size_t user_agent_length;

        char* body;
        size_t body_length;
    } sandia;

    sandia sandia_create(char*, char*);
    void sandia_close(sandia*);
    sandia_error _sandia_setup_socket(sandia*, char*, char*);
    sandia_error sandia_set_user_agent(sandia*, char*);
    sandia_error sandia_get_request(sandia*, char*);
    bool _sandia_send_data(sandia*, char*, size_t);
    bool _sandia_receive_data(sandia*);
    bool sandia_is_connected(sandia*);

#ifdef __cplusplus
}
#endif

#endif /* SANDIA_H */

