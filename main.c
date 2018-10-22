#include <stdio.h>
#include <stdlib.h>

#include "sandia.h"

int main(int argc, char** argv) {
    {
        printf("obtain ip address by amazon service\n");
        sandia s = sandia_create("checkip.amazonaws.com", "80");

        size_t response_length;
        char* response;
        sandia_get_request(&s, "/");

        printf("%s\n", s.body);
        sandia_close(&s);
    }
    
    {
         printf("echo bin get request\n");
        sandia s = sandia_create("httpbin.org", "80");

        sandia_get_request(&s, "/get?foo=bar");

        printf("%s\n", s.body);
        sandia_close(&s);
    }

    return (EXIT_SUCCESS);
}

