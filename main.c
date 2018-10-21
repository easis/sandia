#include <stdio.h>
#include <stdlib.h>

#include "sandia.h"


int main(int argc, char** argv) {
    sandia s = sandia_create("google.com", "80");
    
    if(s.last_error == error_create_socket) {
        printf("error creating socket\n");
        return -1;
    }
    
    printf("s.sandia_socket host:port = %s:%s (ip: %s)\n", s._sandia_socket.host_address, s._sandia_socket.port, s._sandia_socket.ip_address);
    
    if(sandia_set_user_agent(&s, "Mozilla")) {
        printf("s.user_agent = %s (error %d)\n", s.user_agent, s.last_error);
    } else {
        printf("error setting user agent\n");
    }
    
    printf("sandia_get_request == %d\n", sandia_get_request(&s, ""));
    
    return (EXIT_SUCCESS);
}

