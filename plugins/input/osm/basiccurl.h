#ifndef BASICCURL_H
#define BASICCURL_H

#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
	char *data;
	int nbytes;
} CURL_LOAD_DATA;

CURL_LOAD_DATA *grab_http_response(const char *url);
CURL_LOAD_DATA *do_grab(CURL *curl,const char *url);
size_t response_callback(void *ptr,size_t size,size_t nmemb, void *data);

#endif
