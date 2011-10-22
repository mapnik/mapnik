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
#include <mapnik/grid/grid_util.hpp>
#include <mapnik/grid/grid_view.hpp>
#include <mapnik/value_error.hpp>
#include "mapnik_value_converter.hpp"


namespace mapnik {


template <typename T>
static void grid2utf(T const& grid_type, 
    boost::python::list& l,
    std::vector<grid::lookup_type>& key_order)
{
    typename T::data_type const& data = grid_type.data();
    typename T::feature_key_type const& feature_keys = grid_type.get_feature_keys();
    typename T::key_type keys;
    typename T::key_type::const_iterator key_pos;
    typename T::feature_key_type::const_iterator feature_pos;
    // start counting at utf8 codepoint 32, aka space character
    boost::uint16_t codepoint = 32;
    
    unsigned array_size = data.width();
    for (unsigned y = 0; y < data.height(); ++y)
    {
	boost::uint16_t idx = 0;
        boost::scoped_array<Py_UNICODE> line(new Py_UNICODE[array_size]);
        typename T::value_type const* row = data.getRow(y);
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
                    if (codepoint == 34) ++codepoint;      // Skip "
                    else if (codepoint == 92) ++codepoint; // Skip backslash
                
                    keys[val] = codepoint;
                    key_order.push_back(val);
                    line[idx++] = static_cast<Py_UNICODE>(codepoint);
                    ++codepoint;
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
                        PyUnicode_FromUnicode(line.get(), array_size))));
    }
}


template <typename T>
static void grid2utf(T const& grid_type, 
    boost::python::list& l,
    std::vector<typename T::lookup_type>& key_order,
    unsigned int resolution)
{
    //typename T::data_type const& data = grid_type.data();
    typename T::feature_key_type const& feature_keys = grid_type.get_feature_keys();
    typename T::key_type keys;
    typename T::key_type::const_iterator key_pos;
    typename T::feature_key_type::const_iterator feature_pos;
    // start counting at utf8 codepoint 32, aka space character
    boost::uint16_t codepoint = 32;

    // TODO - use double?
    unsigned array_size = static_cast<unsigned int>(grid_type.width()/resolution);
    for (unsigned y = 0; y < grid_type.height(); y=y+resolution)
    {
	boost::uint16_t idx = 0;
        boost::scoped_array<Py_UNICODE> line(new Py_UNICODE[array_size]);
        mapnik::grid::value_type const* row = grid_type.getRow(y);
        for (unsigned x = 0; x < grid_type.width(); x=x+resolution)
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
                    if (codepoint == 34) ++codepoint;      // Skip "
                    else if (codepoint == 92) ++codepoint; // Skip backslash
                    keys[val] = codepoint;
                    key_order.push_back(val);
                    line[idx++] = static_cast<Py_UNICODE>(codepoint);
                    ++codepoint;
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
                        PyUnicode_FromUnicode(line.get(), array_size))));
    }
}


template <typename T>
static void grid2utf2(T const& grid_type, 
    boost::python::list& l,
    std::vector<typename T::lookup_type>& key_order,
    unsigned int resolution)
{
    typename T::data_type const& data = grid_type.data();
    typename T::feature_key_type const& feature_keys = grid_type.get_feature_keys();
    typename T::key_type keys;
    typename T::key_type::const_iterator key_pos;
    typename T::feature_key_type::const_iterator feature_pos;
    // start counting at utf8 codepoint 32, aka space character
    uint16_t codepoint = 32;

    mapnik::grid::data_type target(data.width()/resolution,data.height()/resolution);
    mapnik::scale_grid(target,grid_type.data(),0.0,0.0);

    unsigned array_size = target.width();
    for (unsigned y = 0; y < target.height(); ++y)
    {
        uint16_t idx = 0;
        boost::scoped_array<Py_UNICODE> line(new Py_UNICODE[array_size]);
        mapnik::grid::value_type * row = target.getRow(y);
        unsigned x;
        for (x = 0; x < target.width(); ++x)
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
                    if (codepoint == 34) ++codepoint;      // Skip "
                    else if (codepoint == 92) ++codepoint; // Skip backslash
                    keys[val] = codepoint;
                    key_order.push_back(val);
                    line[idx++] = static_cast<Py_UNICODE>(codepoint);
                    ++codepoint;
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
                        PyUnicode_FromUnicode(line.get(), array_size))));
    }
}


template <typename T>
static void write_features(T const& grid_type,
    boost::python::dict& feature_data,
    std::vector<typename T::lookup_type> const& key_order)
{
    std::string const& key = grid_type.get_key();
    std::set<std::string> const& attributes = grid_type.property_names();
    typename T::feature_type const& g_features = grid_type.get_grid_features();
    typename T::feature_type::const_iterator feat_itr = g_features.begin();
    typename T::feature_type::const_iterator feat_end = g_features.end();
    bool include_key = (attributes.find(key) != attributes.end());
    for (; feat_itr != feat_end; ++feat_itr)
    {
        std::map<std::string,mapnik::value> const& props = feat_itr->second;
        std::map<std::string,mapnik::value>::const_iterator const& itr = props.find(key);
        if (itr != props.end())
        {
            typename T::lookup_type const& join_value = itr->second.to_string();
    
            // only serialize features visible in the grid
            if(std::find(key_order.begin(), key_order.end(), join_value) != key_order.end()) {
                boost::python::dict feat;
                std::map<std::string,mapnik::value>::const_iterator it = props.begin();
                std::map<std::string,mapnik::value>::const_iterator end = props.end();
                bool found = false;
                for (; it != end; ++it)
                {
                    std::string const& key_name = it->first;
                    if (key_name == key) {
                        // drop key unless requested
                        if (include_key) {
                            found = true;
                            feat[it->first] = boost::python::object(
                                boost::python::handle<>(
                                    boost::apply_visitor(
                                        boost::python::value_converter(),
                                            it->second.base())));
                        }
                    }
                    else if ( (attributes.find(key_name) != attributes.end()) )
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
            std::clog << "should not get here: key '" << key << "' not found in grid feature properties\n";
        }
    }
}

template <typename T>
static void grid_encode_utf(T const& grid_type,
    boost::python::dict & json,
    bool add_features,
    unsigned int resolution)
{
    // convert buffer to utf and gather key order
    boost::python::list l;
    std::vector<typename T::lookup_type> key_order;
    
    if (resolution != 1) {
        // resample on the fly - faster, less accurate
        mapnik::grid2utf<T>(grid_type,l,key_order,resolution);

        // resample first - slower, more accurate
        //mapnik::grid2utf2<T>(grid_type,l,key_order,resolution);
    }
    else
    {
        mapnik::grid2utf<T>(grid_type,l,key_order);    
    }

    // convert key order to proper python list
    boost::python::list keys_a;
    BOOST_FOREACH ( typename T::lookup_type const& key_id, key_order )
    {
        keys_a.append(key_id);
    }

    // gather feature data
    boost::python::dict feature_data;
    if (add_features) {
        mapnik::write_features<T>(grid_type,feature_data,key_order);
    }

    json["grid"] = l;
    json["keys"] = keys_a;
    json["data"] = feature_data;

}

template <typename T>
static boost::python::dict grid_encode( T const& grid, std::string format, bool add_features, unsigned int resolution)
{
    if (format == "utf") {
        boost::python::dict json;
        grid_encode_utf<T>(grid,json,add_features,resolution);
        return json;
    }
    else
    {
        std::stringstream s;
        s << "'utf' is currently the only supported encoding format.";
        throw mapnik::value_error(s.str());
    }
}

/* new approach: key comes from grid object
 * grid size should be same as the map
 * encoding, resizing handled as method on grid object
 * whether features are dumped is determined by argument not 'fields'
 */
static void render_layer_for_grid(const mapnik::Map& map,
    mapnik::grid& grid,
    unsigned layer_idx, // TODO - layer by name or index
    boost::python::list const& fields)
{
    std::vector<mapnik::layer> const& layers = map.layers();
    std::size_t layer_num = layers.size();
    if (layer_idx >= layer_num) {
        std::ostringstream s;
        s << "Zero-based layer index '" << layer_idx << "' not valid, only '"
          << layer_num << "' layers are in map\n";
        throw std::runtime_error(s.str());
    }

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
    std::string const& key = grid.get_key();

    // if key is special __id__ keyword
    if (key == grid.id_name_) 
    {
        // TODO - should feature.id() be a first class attribute?
        
        // if __id__ is requested to be dumped out
        // remove it so that datasource queries will not break
        if (attributes.find(key) != attributes.end())
        {
            attributes.erase(key);
        }
    }
    // if key is not the special __id__ keyword
    else if (attributes.find(key) == attributes.end())
    {
        // them make sure the datasource query includes this field
        attributes.insert(key);
    }
    
    mapnik::grid_renderer<mapnik::grid> ren(map,grid,1.0,0,0);
    mapnik::layer const& layer = layers[layer_idx];
    ren.apply(layer,attributes);
}

/* old, original impl - to be removed after further testing
 * grid object is created on the fly at potentially reduced size
 */
static boost::python::dict render_grid(const mapnik::Map& map,
    unsigned layer_idx, // layer
    std::string const& key, // key_name
    unsigned int step, // resolution
    boost::python::list const& fields)
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

    // TODO - no need to pass step here
    mapnik::grid grid(grid_width,grid_height,key,step);
    
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
    
    // if key is special __id__ keyword
    if (key == grid.id_name_) 
    {
        // TODO - should feature.id() be a first class attribute?
        
        // if __id__ is requested to be dumped out
        // remove it so that datasource queries will not break
        if (attributes.find(key) != attributes.end())
        {
            attributes.erase(key);
        }
    }
    // if key is not the special __id__ keyword
    else if (attributes.find(key) == attributes.end())
    {
        // them make sure the datasource query includes this field
        attributes.insert(key);
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
    
    bool add_features = false;
    if (num_fields > 0)
        add_features = true;
    // build dictionary and return to python
    boost::python::dict json;
    grid_encode_utf(grid,json,add_features,1);
    return json;
}

}

#endif // MAPNIK_PYTHON_BINDING_GRID_UTILS_INCLUDED
