#include "sandia.h"

#include <stdio.h>
#include <errno.h>
#define MAX_REQUEST_LEN (1024*16)

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
    signal(SIGPIPE, SIG_IGN);

    sandia _sandia;
    _sandia.last_error = success;
    _sandia._is_valid = false;

    _sandia.last_error = _sandia_setup_socket(&_sandia, host, port);
    _sandia._is_valid = (_sandia.last_error == success);

    return _sandia;
}


void sandia_close(sandia* s) {
    freeaddrinfo(s->_sandia_socket._host);
    freeaddrinfo(s->_sandia_socket._info);
    close(s->_sandia_socket._fd);
    free(s->body);
    s->body_length = 0;
}

sandia_error _sandia_setup_socket(sandia* s, char* host, char* port) {
    sandia_error r = success;

    sandia_socket _socket;
    size_t host_length = strlen(host);
    _socket.host_address = (char*) malloc(host_length + 1);
    strcpy(_socket.host_address, host);
    _socket.host_address[host_length] = 0;

    size_t port_length = strlen(port);
    _socket.port = (char*) malloc(port_length + 1);
    strcpy(_socket.port, port);
     _socket.port[port_length] = 0;

    _socket._fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket._fd == -1) {
        return error_create_socket;
    }

    _socket._host = (void*) malloc(sizeof (struct addrinfo));
    _socket._info = (void*) malloc(sizeof (struct addrinfo));
    memset(_socket._info, 0, sizeof (struct addrinfo));
    
    _socket._info->ai_family = AF_UNSPEC;
    _socket._info->ai_socktype = SOCK_STREAM;
    _socket._info->ai_protocol = IPPROTO_TCP;

    int ret_addr = getaddrinfo(_socket.host_address, _socket.port, _socket._info, &_socket._host);
    if (ret_addr != 0) {
        //printf("getaddrinfo failed with error %d (%s)\n", ret_addr, gai_strerror(ret_addr));
        return error_get_host;
    }

    s->_sandia_socket = _socket;

    return r;
}

sandia_error sandia_get_request(sandia* s, char* uri) {
    if (!s->_is_valid) {
        return error_socket_not_ready;
    }

    int ret_sock = connect(s->_sandia_socket._fd, s->_sandia_socket._host->ai_addr, s->_sandia_socket._host->ai_addrlen);
    if (ret_sock < 0) {
        return error_connection;
    }

    char* url_uri = (strlen(uri) > 0 ? uri : "/");

    char request[MAX_REQUEST_LEN];
    char request_template[] = "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n";

    int request_len = snprintf(request, MAX_REQUEST_LEN, request_template, url_uri, s->_sandia_socket.host_address);

    _sandia_send_data(s, request, request_len);
    _sandia_receive_data(s);

    return success;
}

bool _sandia_send_data(sandia* s, char* data, size_t data_length) {
    ssize_t bytes_total = 0, bytes_sent;
    while (bytes_total < data_length) {
        bytes_sent = send(s->_sandia_socket._fd, data + bytes_total, data_length - bytes_total, 0);
        if (bytes_sent == -1) {
            break;
        }
        bytes_total += bytes_sent;
    }

    return (bytes_total == data_length);
}

bool _sandia_receive_data(sandia* s) {
    ssize_t bytes_total = 0, bytes_received;
    char r[1024]; 
    while ((bytes_received = recv(s->_sandia_socket._fd, r , sizeof(r), 0)) > 0) {
            bytes_total += bytes_received;
            break;
    }

    if (bytes_total > 0) {
        s->body_length = bytes_total;
        s->body = (char*) malloc(s->body_length + 1);
        strncpy(s->body, r, s->body_length);
        s->body[s->body_length] = 0;
        
    }

    return true;
}

bool sandia_is_connected(sandia* s) {
    int error_code;
    int error_code_size = sizeof (error_code);
    getsockopt(s->_sandia_socket._fd, SOL_SOCKET, SO_ERROR, &error_code, &error_code_size);

    return (error_code == 0);
}