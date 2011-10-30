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

#ifndef OSMPARSER_H
#define OSMPARSER_H

#include <libxml/xmlreader.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include "osm.h"
#include <map>

class osmparser
{
public:
    static void processNode(xmlTextReaderPtr reader);
    static void startElement(xmlTextReaderPtr reader, const xmlChar* name);
    static void endElement(const xmlChar* name);
    static bool parse(osm_dataset* ds, const char* filename);
    static bool parse(osm_dataset* ds, char* data, int nbytes);

private:
    static osm_item *cur_item;
    static long curID;
    static bool in_node, in_way;
    static osm_dataset* components;
    static std::string error;
    static std::map<long, osm_node*> tmp_node_store;

    static int do_parse(xmlTextReaderPtr);
};

#endif // OSMPARSER_H
