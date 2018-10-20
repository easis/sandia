#ifndef SANDIA_H
#define SANDIA_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef enum sandia_error_t {
        success = 1,
        error_string = -1,
        error_create_socket
    } sandia_error;

    typedef struct sandia_socket_t {
        char* host_address;
        uint16_t port;
        int fd;
        struct sockaddr_in host;
    } sandia_socket;

    typedef struct sandia_t {
        /* attributes*/
        sandia_error last_error;
        sandia_socket sandia_socket;

        char* user_agent;
        size_t user_agent_length;
    } sandia;

    sandia sandia_create(char*, uint16_t);
    sandia_error sandia_set_user_agent(sandia*, char*);

#ifdef __cplusplus
}
#endif

#endif /* SANDIA_H */

