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

#include "osm.h"
#include "osmparser.h"
#include "basiccurl.h"

#include <mapnik/debug.hpp>

#include <libxml/parser.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

polygon_types osm_way::ptypes;

bool osm_dataset::load(const char* filename,std::string const& parser)
{
    if (parser == "libxml2")
    {
        return osmparser::parse(this, filename);
    }
    return false;
}

bool osm_dataset::load_from_url(std::string const& url,
                                std::string const& bbox,
                                std::string const& parser)
{
    if (parser == "libxml2")
    {
        MAPNIK_LOG_DEBUG(osm) << "osm_dataset: load_from_url url=" << url << ",bbox=" << bbox;

        std::ostringstream str;
        // use curl to grab the data
        // fetch all the data we want - probably from osmxpai

        str << url << "?bbox=" << bbox;

        MAPNIK_LOG_DEBUG(osm) << "osm_dataset: Full url=" << str.str();

        CURL_LOAD_DATA* resp = grab_http_response(str.str().c_str());

        if (resp != NULL)
        {
            char *blx = new char[resp->nbytes + 1];
            memcpy(blx, resp->data, resp->nbytes);
            blx[resp->nbytes] = '\0';

            MAPNIK_LOG_DEBUG(osm) << "osm_dataset: CURL Response=" << blx;

            delete[] blx;
            bool success = osmparser::parse(this, resp->data, resp->nbytes);
            return success;
        }
    }
    return false;
}

osm_dataset::~osm_dataset()
{
    clear();
}

void osm_dataset::clear()
{
    MAPNIK_LOG_DEBUG(osm) << "osm_dataset: Clear";

    MAPNIK_LOG_DEBUG(osm) << "osm_dataset: -- Deleting ways";
    for (unsigned int count = 0; count < ways.size(); ++count)
    {
        delete ways[count];
        ways[count] = NULL;
    }
    ways.clear();

    MAPNIK_LOG_DEBUG(osm) << "osm_dataset: -- Deleting nodes";
    for (unsigned int count = 0; count < nodes.size(); ++count)
    {
        delete nodes[count];
        nodes[count] = NULL;
    }
    nodes.clear();

    MAPNIK_LOG_DEBUG(osm) << "osm_dataset: Clear done";
}

std::string osm_dataset::to_string()
{
    std::string result;

    for (unsigned int count = 0; count < nodes.size(); ++count)
    {
        result += nodes[count]->to_string();
    }

    for (unsigned int count = 0; count < ways.size(); ++count)
    {
        result += ways[count]->to_string();
    }

    return result;
}

bounds osm_dataset::get_bounds()
{
    bounds b (-180, -90, 180, 90);
    for (unsigned int count = 0; count < nodes.size(); ++count)
    {
        if(nodes[count]->lon > b.w) b.w = nodes[count]->lon;
        if(nodes[count]->lon < b.e) b.e = nodes[count]->lon;
        if(nodes[count]->lat > b.s) b.s = nodes[count]->lat;
        if(nodes[count]->lat < b.n) b.n = nodes[count]->lat;
    }
    return b;
}

osm_node* osm_dataset::next_node()
{
    if (node_i != nodes.end())
    {
        return *(node_i++);
    }
    return NULL;
}

osm_way* osm_dataset::next_way()
{
    if (way_i != ways.end())
    {
        return *(way_i++);
    }
    return NULL;
}

osm_item* osm_dataset::next_item()
{
    osm_item* item = NULL;
    if (next_item_mode == Node)
    {
        item = next_node();
        if (item == NULL)
        {
            next_item_mode = Way;
            rewind_ways();
            item = next_way();
        }
    }
    else
    {
        item = next_way();
    }
    return item;
}

std::set<std::string> osm_dataset::get_keys()
{
    std::set<std::string> keys;
    for (unsigned int count = 0; count < nodes.size(); ++count)
    {
        for (std::map<std::string, std::string>::iterator i = nodes[count]->keyvals.begin();
             i != nodes[count]->keyvals.end(); i++)
        {
            keys.insert(i->first);
        }
    }

    for (unsigned int count = 0; count < ways.size(); ++count)
    {
        for (std::map<std::string, std::string>::iterator i = ways[count]->keyvals.begin();
             i != ways[count]->keyvals.end(); i++)
        {
            keys.insert(i->first);
        }
    }
    return keys;
}


std::string osm_item::to_string()
{
    std::ostringstream strm;
    strm << "id=" << id << std::endl << "Keyvals: " << std::endl;

    for (std::map<std::string, std::string>::iterator i = keyvals.begin();
         i != keyvals.end(); i++)
    {
        strm << "Key " << i->first << " Value " << i->second << std::endl;
    }

    return strm.str();
}

std::string osm_node::to_string()
{
    std::ostringstream strm;
    strm << "Node: " << osm_item::to_string() << " lat=" << lat <<" lon="  <<lon << std::endl;
    return strm.str();
}

std::string osm_way::to_string()
{
    std::ostringstream strm;
    strm << "Way: " << osm_item::to_string() << "Nodes in way:";

    for (unsigned int count = 0; count < nodes.size(); ++count)
    {
        if (nodes[count] != NULL)
        {
            strm << nodes[count]->id << " ";
        }
    }

    strm << std::endl;
    return strm.str();
}

bounds osm_way::get_bounds()
{
    bounds b (-180, -90, 180, 90);

    for (unsigned int count = 0; count < nodes.size(); ++count)
    {
        if(nodes[count]->lon > b.w) b.w = nodes[count]->lon;
        if(nodes[count]->lon < b.e) b.e = nodes[count]->lon;
        if(nodes[count]->lat > b.s) b.s = nodes[count]->lat;
        if(nodes[count]->lat < b.n) b.n = nodes[count]->lat;
    }
    return b;
}

bool osm_way::is_polygon()
{
    if (keyvals.find("building") != keyvals.end())
    {
        return true;
    }

    for (unsigned int count = 0; count < ptypes.ptypes.size(); ++count)
    {
        if (keyvals.find(ptypes.ptypes[count].first) != keyvals.end() &&
            keyvals[ptypes.ptypes[count].first] == ptypes.ptypes[count].second)
        {
            return true;
        }
    }

    return false;
}
