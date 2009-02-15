#include "basiccurl.h"

CURL_LOAD_DATA *grab_http_response(const char *url)
{
	CURL_LOAD_DATA *data;

	CURL *curl =  curl_easy_init(); 

	if(curl)
	{
		data = do_grab(curl,url);
		curl_easy_cleanup(curl);
		return data;
	}
	return NULL;
}

CURL_LOAD_DATA *do_grab(CURL *curl,const char *url)
{
	CURLcode res;
	CURL_LOAD_DATA *data = (CURL_LOAD_DATA *)malloc(sizeof(CURL_LOAD_DATA));
	data->data = NULL;
	data->nbytes = 0;
	
	curl_easy_setopt(curl,CURLOPT_URL,url);
	curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,response_callback);
	curl_easy_setopt(curl,CURLOPT_WRITEDATA,data);

	res=curl_easy_perform(curl);

	return data;
}

size_t response_callback(void *ptr,size_t size,size_t nmemb, void *d)
{
	size_t rsize=size*nmemb;
	CURL_LOAD_DATA *data=(CURL_LOAD_DATA *)d;
//	fprintf(stderr,"rsize is %d\n", rsize);
	data->data=(char *)realloc(data->data,(data->nbytes+rsize)
										*sizeof(char));
	memcpy(&(data->data[data->nbytes]),ptr,rsize);
	data->nbytes += rsize;
//	fprintf(stderr,"data->nbytes is %d\n", data->nbytes);
	return rsize;
}
