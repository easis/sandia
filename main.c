#include <stdio.h>
#include <stdlib.h>

#include "sandia.h"

int main(int argc, char** argv) {
    if(0){
        printf("obtain ip address by amazon service\n");
        sandia s = sandia_create("checkip.amazonaws.com", "80");
        
        sandia_get_request(&s, "/");

        printf("%s\n", s.body);
        sandia_close(&s);
    }
    
    if(1){
         printf("echo bin get request\n");
        sandia s = sandia_create("httpbin.org", "80");

        // add headers indivually
        sandia_add_header(&s, "Referer", "https://github.com/blau72/sandia");
        
        // or as an array
        sandia_header headers[2] = {
            { "User-Agent", "Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:47.0) Gecko/20100101 Firefox/47.0"},
            {"Origin", "https://github.com/blau72"}
        };
        sandia_add_headers(&s, headers, sizeof(headers) / sizeof(sandia_header));
        
        sandia_get_request(&s, "/get?foo=bar");

        printf("%s\n", s.body);
        sandia_close(&s);
    }
    
    if(0){
         printf("echo bin post request\n");
        sandia s = sandia_create("httpbin.org", "80");
        
        char content[] = "hello=world&happy=christmas";
        sandia_post_request(&s, "/post?foo=bar", content, strlen(content));

        printf("%s\n", s.body);
        sandia_close(&s);
    }
    
    return (EXIT_SUCCESS);
}

