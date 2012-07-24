/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

#include "basiccurl.h"

#include <iostream>

CURL_LOAD_DATA* grab_http_response(const char* url)
{
    CURL_LOAD_DATA* data;

    CURL* curl = curl_easy_init();

    if(curl)
    {
        data = do_grab(curl, url);
        curl_easy_cleanup(curl);
        return data;
    }
    return NULL;
}

CURL_LOAD_DATA* do_grab(CURL* curl,const char* url)
{
    CURL_LOAD_DATA* data = (CURL_LOAD_DATA*)malloc(sizeof(CURL_LOAD_DATA));
    data->data = NULL;
    data->nbytes = 0;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, response_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);

    CURLcode res = curl_easy_perform(curl);
    if (res !=0) {
        std::clog << "error grabbing data\n";
    }

    return data;
}

size_t response_callback(void* ptr, size_t size, size_t nmemb, void* d)
{
    size_t rsize = size * nmemb;
    CURL_LOAD_DATA* data = (CURL_LOAD_DATA*)d;

    // fprintf(stderr,"rsize is %d\n", rsize);

    data->data = (char*)realloc(data->data, (data->nbytes + rsize) * sizeof(char));
    memcpy(&(data->data[data->nbytes]), ptr, rsize);
    data->nbytes += rsize;

    // fprintf(stderr,"data->nbytes is %d\n", data->nbytes);

    return rsize;
}
