#include "sandia.h"

#include <stdio.h>
#include <errno.h>

#define MAX_REQUEST_LEN (1024*16)
#define MAX_HEADER_COUNT (256)

sandia sandia_create(char* host, char* port) {
    sandia _sandia;
    _sandia.last_error = success;
    _sandia._is_valid = false;

    _sandia.last_error = _sandia_setup_socket(&_sandia, host, port);
    _sandia._is_valid = (_sandia.last_error == success);
    _sandia._sandia_socket.receive_buffer_size = 1024;

    _sandia._headers = (sandia_header*) calloc(MAX_HEADER_COUNT, sizeof (sandia_header) * MAX_HEADER_COUNT);
    _sandia._header_count = 0;
    
    return _sandia;
}

void sandia_close(sandia* s) {
    freeaddrinfo(s->_sandia_socket._host);
    freeaddrinfo(s->_sandia_socket._info);
    close(s->_sandia_socket._fd);
    
    free(s->body);
    s->body_length = 0;
    
    free(s->_headers);
    s->_header_count = 0;
    
    free(s->_request);
    s->_request_length = 0;
    
    free(s->_sandia_socket.host_address);
    free(s->_sandia_socket.port);
}

sandia_error _sandia_setup_socket(sandia* s, char* host, char* port) {
    sandia_error r = success;

    sandia_socket _socket;
    size_t host_length = strlen(host);
    _socket.host_address = (char*) calloc(host_length + 1, sizeof (char));
    strcpy(_socket.host_address, host);

    size_t port_length = strlen(port);
    _socket.port = (char*) calloc(port_length + 1, sizeof (char));
    strcpy(_socket.port, port);

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

sandia_error sandia_forge_request(sandia* s, request_mode mode, char* uri, char* content, size_t content_length) {
    if (!s->_is_valid) {
        return error_socket_not_ready;
    }

    int ret_sock = connect(s->_sandia_socket._fd, s->_sandia_socket._host->ai_addr, s->_sandia_socket._host->ai_addrlen);
    if (ret_sock < 0) {
        return error_connection;
    }

    char* url_uri = (strlen(uri) > 0 ? uri : "/");

    _sandia_build_request(s, mode);
    _sandia_append_request(s, url_uri);
    _sandia_append_request(s, " HTTP/1.1\r\nHost: ");
    _sandia_append_request(s, s->_sandia_socket.host_address);
    _sandia_append_request(s, "\r\n");
    
    sandia_add_header(s, "Connection", "close"); // avoids 'read' socket hang
    
    if(mode == POST) {
        char header_content_length[16];
        int l =sprintf(header_content_length, "%d", content_length);
        header_content_length[l] = 0;

        sandia_add_header(s, "Content-Length", header_content_length);
    }

    for (int i = 0; i < s->_header_count; i++) {
        sandia_header h = s->_headers[i];
        _sandia_append_request(s, h.key);
        _sandia_append_request(s, ": ");
        _sandia_append_request(s, h.value);
        _sandia_append_request(s, "\r\n");
    }

    _sandia_append_request(s, "\r\n");
    
    if(content_length > 0) {
         _sandia_append_request_size(s, content, content_length);
    }

    if(!_sandia_send_data(s, s->_request, s->_request_length)) {
        return error_send;
    }
   
    if(!_sandia_receive_data(s)) {
        return error_receive;
    }

    return success;
}

sandia_error sandia_get_request(sandia* s, char* uri) {
    return sandia_forge_request(s, GET, uri, "", 0);
}

sandia_error sandia_post_request(sandia* s, char* uri, char* content, size_t content_size) {
    return sandia_forge_request(s, POST, uri, content, content_size);
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
    ssize_t bytes_total = 0, bytes_received, buffer_size = s->_sandia_socket.receive_buffer_size;
    uint32_t num_reads = 1;
    
    char* r = (char*) calloc(buffer_size + 1, sizeof(char));
    if(!r)  {
        return false;
    }
    
    //char r[1024];
    while ((bytes_received = recv(s->_sandia_socket._fd, r + bytes_total, s->_sandia_socket.receive_buffer_size, 0)) > 0) {
        bytes_total += bytes_received;
        num_reads++;
        
        r = (char*) realloc(r, buffer_size * num_reads + 1);
        if(!r) { 
            return false;
        }
    }
    
    r[bytes_total] = 0;

    if (bytes_total > 0) {
        s->body_length = bytes_total;
        s->body = (char*) calloc(s->body_length + 1, sizeof (char));
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

const size_t _initial_request_length = 1024;

bool _sandia_build_request(sandia* s, request_mode mode) {
    s->_request_length = 0;
    s->_request = (char*) calloc(_initial_request_length, sizeof (char));

    char* m = (char*) calloc(16, sizeof (char));
    if (!m) {
        s->last_error = error_string;
        return false;
    }

    switch (mode) {
        case GET: m = "GET ";
            break;
        case POST: m = "POST ";
            break;
    }

    strcat(s->_request, m);
    s->_request_length += strlen(m);
    s->_request[s->_request_length] = 0;

    return true;
}

bool _sandia_append_request_size(sandia* s, char* str, size_t str_len) {
    if(str_len <= 0) {
         s->last_error = error_string;
         return false;
    }
    
    if (_initial_request_length < (s->_request_length + str_len)) {
        s->_request = (char*) realloc(s->_request, s->_request_length + str_len + 1);

        if (!s->_request) {
            s->last_error = error_string;
            return false;
        }
    }

    strcat(s->_request, str);
    s->_request_length += str_len;
    s->_request[s->_request_length] = 0;

    return true;
}

bool _sandia_append_request(sandia* s, char* str) {
    return _sandia_append_request_size(s, str, strlen(str));
}

sandia_error sandia_add_header(sandia* s, char* key, char* value) {
    if (s->_header_count >= MAX_HEADER_COUNT) {
        return error_header_limit;
    }

    s->_headers[s->_header_count].key = (char*) calloc(strlen(key) + 1, sizeof (char));
    s->_headers[s->_header_count].value = (char*) calloc(strlen(value) + 1, sizeof (char));

    strcpy(s->_headers[s->_header_count].key, key);    
    strcpy(s->_headers[s->_header_count].value, value);

    s->_header_count++;
    
    return success;
}

sandia_error sandia_add_headers(sandia* s, sandia_header* headers, uint32_t count) {
    if (s->_header_count >= MAX_HEADER_COUNT) {
        return error_header_limit;
    }

    for (int i = 0; i < count; i++) {
        sandia_header h = headers[i];
        s->_headers[s->_header_count].key = (char*) calloc(strlen(h.key) + 1, sizeof (char));
        s->_headers[s->_header_count].value = (char*) calloc(strlen(h.value) + 1, sizeof (char));
        
        strcpy(s->_headers[s->_header_count].key, h.key);        
        strcpy(s->_headers[s->_header_count].value, h.value);

        s->_header_count++;
    }
    
    return success;
}