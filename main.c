#include <stdio.h>
#include <stdlib.h>

#include "sandia.h"


int main(int argc, char** argv) {
    sandia s = sandia_create("127.0.0.1", 80);
    
    if(s.last_error == error_create_socket) {
        printf("error creating socket\n");
        return -1;
    }
    
    printf("s.sandia_socket host:port = %s:%d\n", s.sandia_socket.host_address, s.sandia_socket.port);
    
    sandia_error se = sandia_set_user_agent(&s, "Mozilla");
    if(se == success) {
        printf("s.user_agent = %s (error %d)\n", s.user_agent, s.last_error);
    } else {
        printf("error setting user agent\n");
    }
    
    return (EXIT_SUCCESS);
}

