#include "sandia.h"

sandia_error sandia_set_user_agent(sandia* s, char* user_agent) {
    s->user_agent_length = strlen(user_agent);
    if (s->user_agent_length < 0) {
        return error_string;
    }

    s->user_agent = (char*) malloc(s->user_agent_length + 1);
    strncpy(s->user_agent, user_agent, s->user_agent_length);
    s->last_error = (strncmp(s->user_agent, user_agent, s->user_agent_length) == 0) ? success : error_string;
    return (s->last_error);
}

sandia sandia_create(char* host, char* port) {
    sandia _sandia;
    _sandia.last_error = success;
    _sandia._is_valid = false;

    _sandia.last_error = _sandia_setup_socket(&_sandia, host, port);
    _sandia._is_valid = (_sandia.last_error == success);

    /*if (_sandia._is_valid) {
        size_t ipaddr_length = strlen(host);
        _sandia._sandia_socket.ip_address = (char*) malloc(ipaddr_length + 1);
        strncpy(_sandia._sandia_socket.ip_address, _sandia._sandia_socket._host., ipaddr_length);
    }*/

    return _sandia;
}
#include <stdio.h>

sandia_error _sandia_setup_socket(sandia* s, char* host, char* port) {
    sandia_error r = success;

    sandia_socket _socket;
    size_t host_length = strlen(host);
    _socket.host_address = (char*) malloc(host_length + 1);
    strncpy(_socket.host_address, host, host_length);

    size_t port_length = strlen(port);
    _socket.port = (char*) malloc(port_length + 1);
    strncpy(_socket.port, port, port_length);

    _socket._fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket._fd == -1) {
        return error_create_socket;
    }

    _socket._host = (void*) malloc(sizeof (struct addrinfo));

    _socket._info = (void*) malloc(sizeof (struct addrinfo));
    memset(_socket._info, 0, sizeof (_socket._info));

    _socket._info->ai_family = AF_INET;
    _socket._info->ai_socktype = SOCK_STREAM;
    _socket._info->ai_protocol = IPPROTO_TCP;

    int ret_addr = getaddrinfo(_socket.host_address, "80", _socket._info, &_socket._host);
    if (ret_addr != 0) {
        return error_get_host;
    }

    s->_sandia_socket = _socket;

    return r;
}

#include <errno.h>
#define MAX_REQUEST_LEN (1024)

sandia_error sandia_get_request(sandia* s, char* uri) {
    if (!s->_is_valid) {
        return error_socket_not_ready;
    }

    int ret_sock = connect(s->_sandia_socket._fd, s->_sandia_socket._host, sizeof (s->_sandia_socket._host));
    if (ret_sock < 0) {
        return error_connection;
    }

    char buffer[MAX_REQUEST_LEN * 16];
    char request[MAX_REQUEST_LEN];
    char request_template[] = "GET / HTTP/1.1\r\nHost: google.com\r\n\r\n";
    int request_len = snprintf(request, MAX_REQUEST_LEN, request_template);

    ssize_t nbytes_total = 0, nbytes_last;
    while (nbytes_total < request_len) {
        nbytes_last = write(s->_sandia_socket._fd, request + nbytes_total, request_len - nbytes_total);
        if (nbytes_last == -1) {
            printf("socket.write error: %s\n", strerror(nbytes_last));
            exit(EXIT_FAILURE);
        }
        nbytes_total += nbytes_last;
    }

    fprintf(stderr, "debug: before first read\n");
    while ((nbytes_total = read(s->_sandia_socket._fd, buffer, BUFSIZ)) > 0) {
        fprintf(stderr, "debug: after a read\n");
        write(STDOUT_FILENO, buffer, nbytes_total);
    }
    fprintf(stderr, "debug: after last read\n");
    if (nbytes_total == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    return success;
}