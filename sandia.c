#include "sandia.h"

sandia_error sandia_set_user_agent(sandia* s, char* user_agent) {
    s->user_agent_length = strlen(user_agent);
    if(s->user_agent_length < 0) {
        return error_string;
    }
    
    s->user_agent = (char*) malloc(s->user_agent_length + 1);
    strncpy(s->user_agent , user_agent, s->user_agent_length);
    s->last_error = (strncmp(s->user_agent, user_agent, s->user_agent_length) == 0) ? success : error_string;
    return (s->last_error);
}

sandia sandia_create(char* host, uint16_t port) {
    sandia _sandia;
    _sandia.last_error = success;
    
    sandia_socket _socket;
    size_t host_length = strlen(host);
    _socket.host_address = (char*) malloc(host_length + 1);
    strncpy(_socket.host_address, host, host_length);
    _socket.port = port;
    
    _socket.fd = socket(AF_INET , SOCK_STREAM , 0);
    if(_socket.fd == -1) {
        _sandia.last_error = error_create_socket;
    }
    
    _sandia.sandia_socket = _socket;

    return _sandia;
}