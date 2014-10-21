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

#if defined(GRID_RENDERER)

#include <mapnik/config.hpp>

// boost
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#include <boost/python.hpp>
#pragma GCC diagnostic pop

// mapnik
#include <mapnik/map.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/grid/grid_renderer.hpp>
#include <mapnik/grid/grid.hpp>
#include <mapnik/grid/grid_util.hpp>
#include <mapnik/grid/grid_view.hpp>
#include <mapnik/value_error.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_kv_iterator.hpp>
#include "python_grid_utils.hpp"

// stl
#include <stdexcept>

namespace mapnik {


template <typename T>
void grid2utf(T const& grid_type,
                     boost::python::list& l,
                     std::vector<typename T::lookup_type>& key_order)
{
    using keys_type = std::map< typename T::lookup_type, typename T::value_type>;
    using keys_iterator = typename keys_type::iterator;

    typename T::data_type const& data = grid_type.data();
    typename T::feature_key_type const& feature_keys = grid_type.get_feature_keys();
    typename T::feature_key_type::const_iterator feature_pos;

    keys_type keys;
    // start counting at utf8 codepoint 32, aka space character
    std::uint16_t codepoint = 32;

    unsigned array_size = data.width();
    for (unsigned y = 0; y < data.height(); ++y)
    {
        std::uint16_t idx = 0;
        const std::unique_ptr<Py_UNICODE[]> line(new Py_UNICODE[array_size]);
        typename T::value_type const* row = data.getRow(y);
        for (unsigned x = 0; x < data.width(); ++x)
        {
            typename T::value_type feature_id = row[x];
            feature_pos = feature_keys.find(feature_id);
            if (feature_pos != feature_keys.end())
            {
                mapnik::grid::lookup_type val = feature_pos->second;
                keys_iterator key_pos = keys.find(val);
                if (key_pos == keys.end())
                {
                    // Create a new entry for this key. Skip the codepoints that
                    // can't be encoded directly in JSON.
                    if (codepoint == 34) ++codepoint;      // Skip "
                    else if (codepoint == 92) ++codepoint; // Skip backslash
                    if (feature_id == mapnik::grid::base_mask)
                    {
                        keys[""] = codepoint;
                        key_order.push_back("");
                    }
                    else
                    {
                        keys[val] = codepoint;
                        key_order.push_back(val);
                    }
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
void grid2utf(T const& grid_type,
                     boost::python::list& l,
                     std::vector<typename T::lookup_type>& key_order,
                     unsigned int resolution)
{
    using keys_type = std::map< typename T::lookup_type, typename T::value_type>;
    using keys_iterator = typename keys_type::iterator;

    typename T::feature_key_type const& feature_keys = grid_type.get_feature_keys();
    typename T::feature_key_type::const_iterator feature_pos;

    keys_type keys;
    // start counting at utf8 codepoint 32, aka space character
    std::uint16_t codepoint = 32;

    unsigned array_size = std::ceil(grid_type.width()/static_cast<float>(resolution));
    for (unsigned y = 0; y < grid_type.height(); y=y+resolution)
    {
        std::uint16_t idx = 0;
        const std::unique_ptr<Py_UNICODE[]> line(new Py_UNICODE[array_size]);
        mapnik::grid::value_type const* row = grid_type.getRow(y);
        for (unsigned x = 0; x < grid_type.width(); x=x+resolution)
        {
            typename T::value_type feature_id = row[x];
            feature_pos = feature_keys.find(feature_id);
            if (feature_pos != feature_keys.end())
            {
                mapnik::grid::lookup_type val = feature_pos->second;
                keys_iterator key_pos = keys.find(val);
                if (key_pos == keys.end())
                {
                    // Create a new entry for this key. Skip the codepoints that
                    // can't be encoded directly in JSON.
                    if (codepoint == 34) ++codepoint;      // Skip "
                    else if (codepoint == 92) ++codepoint; // Skip backslash
                    if (feature_id == mapnik::grid::base_mask)
                    {
                        keys[""] = codepoint;
                        key_order.push_back("");
                    }
                    else
                    {
                        keys[val] = codepoint;
                        key_order.push_back(val);
                    }
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
void grid2utf2(T const& grid_type,
                      boost::python::list& l,
                      std::vector<typename T::lookup_type>& key_order,
                      unsigned int resolution)
{
    using keys_type = std::map< typename T::lookup_type, typename T::value_type>;
    using keys_iterator = typename keys_type::iterator;

    typename T::data_type const& data = grid_type.data();
    typename T::feature_key_type const& feature_keys = grid_type.get_feature_keys();
    typename T::feature_key_type::const_iterator feature_pos;

    keys_type keys;
    // start counting at utf8 codepoint 32, aka space character
    uint16_t codepoint = 32;

    mapnik::grid::data_type target(data.width()/resolution,data.height()/resolution);
    mapnik::scale_grid(target,grid_type.data(),0.0,0.0);

    unsigned array_size = target.width();
    for (unsigned y = 0; y < target.height(); ++y)
    {
        uint16_t idx = 0;
        const std::unique_ptr<Py_UNICODE[]> line(new Py_UNICODE[array_size]);
        mapnik::grid::value_type * row = target.getRow(y);
        unsigned x;
        for (x = 0; x < target.width(); ++x)
        {
            feature_pos = feature_keys.find(row[x]);
            if (feature_pos != feature_keys.end())
            {
                mapnik::grid::lookup_type val = feature_pos->second;
                keys_iterator key_pos = keys.find(val);
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
void write_features(T const& grid_type,
                           boost::python::dict& feature_data,
                           std::vector<typename T::lookup_type> const& key_order)
{
    typename T::feature_type const& g_features = grid_type.get_grid_features();
    if (g_features.size() <= 0)
    {
        return;
    }

    std::set<std::string> const& attributes = grid_type.property_names();
    typename T::feature_type::const_iterator feat_end = g_features.end();
    for ( std::string const& key_item :key_order )
    {
        if (key_item.empty())
        {
            continue;
        }

        typename T::feature_type::const_iterator feat_itr = g_features.find(key_item);
        if (feat_itr == feat_end)
        {
            continue;
        }

        bool found = false;
        boost::python::dict feat;
        mapnik::feature_ptr feature = feat_itr->second;
        for ( std::string const& attr : attributes )
        {
            if (attr == "__id__")
            {
                feat[attr.c_str()] = feature->id();
            }
            else if (feature->has_key(attr))
            {
                found = true;
                feat[attr.c_str()] = feature->get(attr);
            }
        }

        if (found)
        {
            feature_data[feat_itr->first] = feat;
        }
    }
}

template <typename T>
void grid_encode_utf(T const& grid_type,
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
    for ( typename T::lookup_type const& key_id : key_order )
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
boost::python::dict grid_encode( T const& grid, std::string const& format, bool add_features, unsigned int resolution)
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

template boost::python::dict grid_encode( mapnik::grid const& grid, std::string const& format, bool add_features, unsigned int resolution);
template boost::python::dict grid_encode( mapnik::grid_view const& grid, std::string const& format, bool add_features, unsigned int resolution);

void render_layer_for_grid(mapnik::Map const& map,
                                  mapnik::grid & grid,
                                  unsigned layer_idx,
                                  boost::python::list const& fields,
                                  double scale_factor,
                                  unsigned offset_x,
                                  unsigned offset_y)
{
    std::vector<mapnik::layer> const& layers = map.layers();
    std::size_t layer_num = layers.size();
    if (layer_idx >= layer_num) {
        std::ostringstream s;
        s << "Zero-based layer index '" << layer_idx << "' not valid, only '"
          << layer_num << "' layers are in map\n";
        throw std::runtime_error(s.str());
    }

    // convert python list to std::set
    boost::python::ssize_t num_fields = boost::python::len(fields);
    for(boost::python::ssize_t i=0; i<num_fields; i++) {
        boost::python::extract<std::string> name(fields[i]);
        if (name.check())
        {
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
    // todo - make this a static constant
    std::string known_id_key = "__id__";
    if (attributes.find(known_id_key) != attributes.end())
    {
        attributes.erase(known_id_key);
    }

    std::string join_field = grid.get_key();
    if (known_id_key != join_field &&
        attributes.find(join_field) == attributes.end())
    {
        attributes.insert(join_field);
    }

    mapnik::grid_renderer<mapnik::grid> ren(map,grid,scale_factor,offset_x,offset_y);
    mapnik::layer const& layer = layers[layer_idx];
    ren.apply(layer,attributes);
}

}

#endif
