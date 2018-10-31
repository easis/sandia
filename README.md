# Overview
Sandia is a simple http only library made in C99, compiled with GCC 8.2 and tested under Antergos.

# Features
  - GET and POST requests
  - Add custom headers
  - Uses sockets, no third party libraries required
  
# TODO
  - Better string handling
  - Response parsing (status codes, headers, content)

# Examples

## Simple GET request:
```C
#include "sandia.h"
#include <stdio.h> // printf

sandia s = sandia_create("checkip.amazonaws.com" /* server host */, 80 /* server port */);
sandia_get_request(&s, "/" /* or "" */);
printf("%s\n", s.body);
```

Output:
```
HTTP/1.1 200 OK
Date: Wed, 31 Oct 2018 11:33:12 GMT
Server: lighttpd/1.4.41
Content-Length: 15
Connection: Close

XXX.XXX.XXX.XXX

```

## GET request with headers:
```C
#include "sandia.h"
#include <stdio.h> // printf

sandia s = sandia_create("httpbin.org", 80);

// add headers indivually
sandia_add_header(&s, "Referer", "https://github.com/blau72/sandia");

// or as an array
sandia_header headers[2] = {
  { "User-Agent", "Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:47.0) Gecko/20100101 Firefox/47.0" },
  { "Origin",     "https://github.com/blau72" }
};

sandia_add_headers(&s, headers, sizeof (headers) / sizeof (sandia_header));
        
sandia_get_request(&s, "/get?foo=bar");
printf("%s\n", s.body);
```

Output:
```
HTTP/1.1 200 OK
Connection: close
Server: gunicorn/19.9.0
Date: Wed, 31 Oct 2018 11:36:53 GMT
Content-Type: application/json
Content-Length: 383
Access-Control-Allow-Origin: https://github.com/blau72
Access-Control-Allow-Credentials: true
Via: 1.1 vegur

{
  "args": {
    "foo": "bar"
  }, 
  "headers": {
    "Connection": "close", 
    "Host": "httpbin.org", 
    "Origin": "https://github.com/blau72", 
    "Referer": "https://github.com/blau72/sandia", 
    "User-Agent": "Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:47.0) Gecko/20100101 Firefox/47.0"
  }, 
  "origin": "XXX.XXX.XXX.XXX", 
  "url": "http://httpbin.org/get?foo=bar"
}

```

## POST request:
```C
#include "sandia.h"
#include <stdio.h> // printf

sandia s = sandia_create("httpbin.org", 80);

char content[] = "hello=world&happy=christmas";
sandia_post_request(&s, "/post?foo=bar", content, strlen(content));

printf("%s\n", s.body);
```

Output:
```
HTTP/1.1 200 OK
Connection: close
Server: gunicorn/19.9.0
Date: Wed, 31 Oct 2018 11:39:18 GMT
Content-Type: application/json
Content-Length: 307
Access-Control-Allow-Origin: *
Access-Control-Allow-Credentials: true
Via: 1.1 vegur

{
  "args": {
    "foo": "bar"
  }, 
  "data": "hello=world&happy=christmas", 
  "files": {}, 
  "form": {}, 
  "headers": {
    "Connection": "close", 
    "Content-Length": "27", 
    "Host": "httpbin.org"
  }, 
  "json": null, 
  "origin": "XXX.XXX.XXX.XXX", 
  "url": "http://httpbin.org/post?foo=bar"
}

```
