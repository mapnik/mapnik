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
#ifndef MAPNIK_PYTHON_BINDING_GRID_UTILS_INCLUDED
#define MAPNIK_PYTHON_BINDING_GRID_UTILS_INCLUDED

// boost
#include <boost/python.hpp>
#include <boost/scoped_array.hpp>
#include <boost/foreach.hpp>

// mapnik
#include <mapnik/grid/grid_renderer.hpp>
#include <mapnik/grid/grid.hpp>
#include <mapnik/value_error.hpp>
#include "mapnik_value_converter.hpp"

namespace mapnik {


void grid2utf(mapnik::grid const& grid, 
    boost::python::list& l,
    std::vector<mapnik::grid::lookup_type>& key_order)
{
    mapnik::grid::data_type const& data = grid.data();
    mapnik::grid::feature_key_type const& feature_keys = grid.get_feature_keys();
    mapnik::grid::key_type keys;
    mapnik::grid::key_type::const_iterator key_pos;
    mapnik::grid::feature_key_type::const_iterator feature_pos;
    uint16_t codepoint = 31;

    for (unsigned y = 0; y < data.height(); ++y)
    {
        uint16_t idx = 0;
        boost::scoped_array<Py_UNICODE> line(new Py_UNICODE[data.width()]);
        mapnik::grid::value_type const* row = data.getRow(y);
        for (unsigned x = 0; x < data.width(); ++x)
        {
            feature_pos = feature_keys.find(row[x]);
            if (feature_pos != feature_keys.end())
            {
                mapnik::grid::lookup_type val = feature_pos->second;
                key_pos = keys.find(val);
                if (key_pos == keys.end())
                {
                    // Create a new entry for this key. Skip the codepoints that
                    // can't be encoded directly in JSON.
                    ++codepoint;
                    if (codepoint == 34) ++codepoint;      // Skip "
                    else if (codepoint == 92) ++codepoint; // Skip backslash
                
                    keys[val] = codepoint;
                    key_order.push_back(val);
                    line[idx++] = static_cast<Py_UNICODE>(codepoint);
                }
                else
                {
                    line[idx++] = static_cast<Py_UNICODE>(key_pos->second);
                }
            }
            // else, shouldn't get here...
        }
        l.append(boost::python::object(
                    boost::python::handle<>(
                        PyUnicode_FromUnicode(line.get(), data.width()))));
    }
}


void write_features(mapnik::grid::feature_type const& g_features,
    boost::python::dict& feature_data,
    std::vector<mapnik::grid::lookup_type> const& key_order,
    std::string const& join_field,
    std::set<std::string> const& attributes)
{
    mapnik::grid::feature_type::const_iterator feat_itr = g_features.begin();
    mapnik::grid::feature_type::const_iterator feat_end = g_features.end();
    bool include_join_field = (attributes.find(join_field) != attributes.end());
    for (; feat_itr != feat_end; ++feat_itr)
    {
        std::map<std::string,mapnik::value> const& props = feat_itr->second;
        std::map<std::string,mapnik::value>::const_iterator const& itr = props.find(join_field);
        if (itr != props.end())
        {
            mapnik::grid::lookup_type const& join_value = itr->second.to_string();
    
            // only serialize features visible in the grid
            if(std::find(key_order.begin(), key_order.end(), join_value) != key_order.end()) {
                boost::python::dict feat;
                std::map<std::string,mapnik::value>::const_iterator it = props.begin();
                std::map<std::string,mapnik::value>::const_iterator end = props.end();
                bool found = false;
                for (; it != end; ++it)
                {
                    std::string const& key = it->first;
                    if (key == join_field) {
                        // drop join_field unless requested
                        if (include_join_field) {
                            found = true;
                            feat[it->first] = boost::python::object(
                                boost::python::handle<>(
                                    boost::apply_visitor(
                                        boost::python::value_converter(),
                                            it->second.base())));
                        }
                    }
                    else if ( (attributes.find(key) != attributes.end()) )
                    {
                        found = true;
                        feat[it->first] = boost::python::object(
                            boost::python::handle<>(
                                boost::apply_visitor(
                                    boost::python::value_converter(),
                                        it->second.base())));
                    }
                }
                if (found)
                {
                    feature_data[feat_itr->first] = feat;
                }
            }
        }
        else
        {
            std::clog << "should not get here: join_field '" << join_field << "' not found in grid feature properties\n";
        }
    }
}

boost::python::dict render_grid(const mapnik::Map& map,
    unsigned layer_idx, // layer
    std::string const& join_field, // key_name
    unsigned int step, // resolution
    boost::python::list fields)
{
    std::vector<mapnik::layer> const& layers = map.layers();
    std::size_t layer_num = layers.size();
    if (layer_idx >= layer_num) {
        std::ostringstream s;
        s << "Zero-based layer index '" << layer_idx << "' not valid, only '"
          << layer_num << "' layers are in map\n";
        throw std::runtime_error(s.str());
    }

    unsigned int grid_width = map.width()/step;
    unsigned int grid_height = map.height()/step;

    mapnik::grid grid(grid_width,grid_height,join_field,step);

    // convert python list to std::vector
    boost::python::ssize_t num_fields = boost::python::len(fields);
    for(boost::python::ssize_t i=0; i<num_fields; i++) {
        boost::python::extract<std::string> name(fields[i]);
        if (name.check()) {
            grid.add_property_name(name());
        }
        else
        {
          std::stringstream s;
          s << "list of field names must be strings";
          throw mapnik::value_error(s.str());    
        }
    }

    // copy property names
    std::set<std::string> attributes = grid.property_names();
    
    if (join_field == grid.id_name_) 
    {
        // TODO - should feature.id() be a first class attribute?
        if (attributes.find(join_field) != attributes.end())
        {
            attributes.erase(join_field);
        }
    }
    else if (attributes.find(join_field) == attributes.end())
    {
        attributes.insert(join_field);
    }
    
    try
    {
        mapnik::grid_renderer<mapnik::grid> ren(map,grid,1.0,0,0);
        mapnik::layer const& layer = layers[layer_idx];
        ren.apply(layer,attributes);
    }
    catch (...)
    {
        throw;
    }

    // convert buffer to utf and gather key order
    boost::python::list l;
    std::vector<mapnik::grid::lookup_type> key_order;
    mapnik::grid2utf(grid,l,key_order);

    // convert key order to proper python list
    boost::python::list keys_a;
    BOOST_FOREACH ( mapnik::grid::lookup_type const& key_id, key_order )
    {
        keys_a.append(key_id);
    }

    // gather feature data
    boost::python::dict feature_data;
    if (num_fields > 0) {
        mapnik::grid::feature_type const& g_features = grid.get_grid_features();
        mapnik::write_features(g_features,feature_data,key_order,join_field,grid.property_names());
    }

    // build dictionary and return to python
    boost::python::dict json;
    json["grid"] = l;
    json["keys"] = keys_a;
    json["data"] = feature_data;
    return json;
}

}

#endif // MAPNIK_PYTHON_BINDING_GRID_UTILS_INCLUDED
