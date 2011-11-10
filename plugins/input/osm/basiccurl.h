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

#ifndef BASICCURL_H
#define BASICCURL_H

#include <curl/curl.h>
#include <cstdlib>
#include <cstring>

typedef struct
{
  char *data;
  int nbytes;
} CURL_LOAD_DATA;

CURL_LOAD_DATA *grab_http_response(const char *url);
CURL_LOAD_DATA *do_grab(CURL *curl, const char *url);
size_t response_callback(void *ptr ,size_t size, size_t nmemb, void *data);

#endif // BASICCURL_H
