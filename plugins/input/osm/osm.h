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

#ifndef OSM_H
#define OSM_H

#include <mapnik/value_types.hpp>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <utility>

struct bounds
{
    double w, s, e, n;
    bounds() { w = -180; s = -90; e = 180; n = 90; }
    bounds(double w_, double s_, double e_, double n_)
    { 
        this->w = w_;
        this->s = s_;
        this->e = e_;
        this->n = n_;
    }
};

class polygon_types
{
public:
    std::vector<std::pair<std::string, std::string> > ptypes;

    polygon_types()
    {
        ptypes.push_back(std::pair<std::string, std::string>("water", ""));
        ptypes.push_back(std::pair<std::string, std::string>("aeroway", ""));
        ptypes.push_back(std::pair<std::string, std::string>("building", ""));
        ptypes.push_back(std::pair<std::string, std::string>("natural", "wood"));
        ptypes.push_back(std::pair<std::string, std::string>("natural", "water"));
        ptypes.push_back(std::pair<std::string, std::string>("natural", "heath"));
        ptypes.push_back(std::pair<std::string, std::string>("natural", "marsh"));
        ptypes.push_back(std::pair<std::string, std::string>("military", "danger_area"));
        ptypes.push_back(std::pair<std::string, std::string>("landuse", "forest"));
        ptypes.push_back(std::pair<std::string, std::string>("landuse", "industrial"));
        ptypes.push_back(std::pair<std::string, std::string>("leisure", "park"));
        ptypes.push_back(std::pair<std::string, std::string>("area", "yes"));
    }
};

struct osm_item
{
    mapnik::value_integer id;
    std::map<std::string, std::string> keyvals;
    virtual std::string to_string();
    virtual ~osm_item() {}
};

struct osm_node : public osm_item
{
    double lat, lon;
    std::string to_string();
};

struct osm_way : public osm_item
{
    std::vector<osm_node*> nodes; 
    std::string to_string();
    bounds get_bounds();
    bool is_polygon();
    static polygon_types ptypes;
};

class osm_dataset
{
public:
    osm_dataset()
    {
        node_i = nodes.begin();
        way_i = ways.begin();
        next_item_mode = Node;
    }

    osm_dataset(const char* name)
    {
        node_i = nodes.begin();
        way_i = ways.begin();
        next_item_mode = Node;
        load(name);
    }

    ~osm_dataset();

    bool load(const char* name, std::string const& parser = "libxml2");
    void clear();
    void add_node(osm_node* n) { nodes.push_back(n); }
    void add_way(osm_way* w) { ways.push_back(w); }
    std::string to_string();
    bounds get_bounds();
    std::set<std::string> get_keys();
    void rewind_nodes() { node_i = nodes.begin(); }
    void rewind_ways() { way_i = ways.begin(); }
    void rewind() { rewind_nodes(); rewind_ways(); next_item_mode = Node; }
    osm_node * next_node();
    osm_way * next_way();
    osm_item * next_item();
    bool current_item_is_node() { return next_item_mode == Node; }
    bool current_item_is_way() { return next_item_mode == Way; }

private:
    int next_item_mode;
    enum { Node, Way };
    std::vector<osm_node*>::iterator node_i;
    std::vector<osm_way*>::iterator way_i;
    std::vector<osm_node*> nodes;
    std::vector<osm_way*> ways;
};

#endif // OSM_H
