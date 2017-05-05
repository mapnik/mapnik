/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

#ifndef MAPNIK_JSON_CREATE_FEATURE_HPP
#define MAPNIK_JSON_CREATE_FEATURE_HPP

#include <mapnik/json/json_grammar_config.hpp>
#include <mapnik/json/geojson_grammar_x3.hpp>
#include <mapnik/json/create_geometry.hpp>
#include <mapnik/util/conversions.hpp>

#include <mapnik/value.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/feature.hpp>

namespace mapnik { namespace json {

struct stringifier
{
    stringifier(keys_map const& keys)
        : keys_(keys) {}

    std::string operator()(std::string const& val) const
    {
        return "\"" + val + "\"";
    }

    std::string operator()(value_null) const
    {
        return "null";
    }

    std::string operator()(value_bool val) const
    {
        return val ? "true" : "false";
    }

    std::string operator()(value_integer val) const
    {
        std::string str;
        util::to_string(str, val);
        return str;
    }

    std::string operator()(value_double val) const
    {
        std::string str;
        util::to_string(str, val);
        return str;
    }

    std::string operator()(mapnik::json::geojson_array const& array) const
    {
        std::string str = "[";
        bool first = true;
        for (auto const& val : array)
        {
            if (first) first = false;
            else str += ",";
            str += mapnik::util::apply_visitor(*this, val);
        }
        str += "]";
        return str;
    }

    std::string operator()(mapnik::json::geojson_object const& object) const
    {
        std::string str = "{";
        bool first = true;
        for (auto const& kv : object)
        {
            auto itr = keys_.right.find(std::get<0>(kv));
            if (itr != keys_.right.end())
            {
                if (first) first = false;
                else str += ",";
                str +=  "\"" + itr->second + "\"";
                str += ":";
                str += mapnik::util::apply_visitor(*this, kv.second);
            }
        }
        str += "}";
        return str;
    }
    template <typename T>
    std::string operator()(T const&) const
    {
        return "";
    }

    keys_map const& keys_;
};

struct attribute_value_visitor
{
public:
    attribute_value_visitor(mapnik::transcoder const& tr, keys_map const& keys)
        : tr_(tr),
          keys_(keys) {}

    mapnik::value operator()(std::string const& val) const
    {
        return mapnik::value(tr_.transcode(val.c_str()));
    }

    mapnik::value operator()(mapnik::json::geojson_array const& array) const
    {
        std::string str = stringifier(keys_)(array);
        return mapnik::value(tr_.transcode(str.c_str()));
    }

    mapnik::value operator()(mapnik::json::geojson_object const& object) const
    {
        std::string str = stringifier(keys_)(object);
        return mapnik::value(tr_.transcode(str.c_str()));
    }

    mapnik::value operator() (mapnik::value_bool val) const
    {
        return mapnik::value(val);
    }

    mapnik::value operator() (mapnik::value_integer val) const
    {
        return mapnik::value(val);
    }

    mapnik::value operator() (mapnik::value_double val) const
    {
        return mapnik::value(val);
    }

    template <typename T>
    mapnik::value operator()(T const& val) const
    {
        return mapnik::value_null{};
    }

    mapnik::transcoder const& tr_;
    mapnik::json::keys_map const& keys_;
};

void create_feature(feature_impl & feature,
                    mapnik::json::geojson_value const& value,
                    mapnik::json::keys_map const& keys,
                    mapnik::transcoder const& tr)
{

    if (!value.is<mapnik::json::geojson_object>())
    {
        throw std::runtime_error("Expecting an GeoJSON object");
    }
    mapnik::json::geojson_object const& feature_value = mapnik::util::get<mapnik::json::geojson_object>(value);
    for (auto const& elem : feature_value)
    {
        auto const key = std::get<0>(elem);
        if (key == mapnik::json::well_known_names::geometry)
        {
            auto const& geom_value = std::get<1>(elem);
            if (!geom_value.is<mapnik::json::geojson_object>())
            {
                throw std::runtime_error("\"geometry\": xxx <-- expecting an JSON object here");
            }
            auto const& geometry = mapnik::util::get<mapnik::json::geojson_object>(geom_value);
            mapnik::geometry::geometry_types geom_type;
            mapnik::json::positions const* coordinates = nullptr;
            for (auto const& elem2 : geometry)
            {
                auto const key2 = std::get<0>(elem2);
                if (key2 == mapnik::json::well_known_names::type)
                {
                    auto const& geom_type_value = std::get<1>(elem2);
                    if (!geom_type_value.is<mapnik::geometry::geometry_types>())
                    {
                        throw std::runtime_error("\"type\": xxx <-- expecting an GeoJSON geometry type here");
                    }
                    geom_type = mapnik::util::get<mapnik::geometry::geometry_types>(geom_type_value);
                    if (geom_type == mapnik::geometry::geometry_types::GeometryCollection)
                    {
                        throw std::runtime_error("GeometryCollections are not allowed");
                    }
                }
                else if (key2 == mapnik::json::well_known_names::coordinates)
                {
                    auto const& coordinates_value = std::get<1>(elem2);
                    if (!coordinates_value.is<mapnik::json::positions>())
                    {
                        throw std::runtime_error("\"coordinates\": xxx <-- expecting an GeoJSON positions here");
                    }
                    coordinates = &mapnik::util::get<mapnik::json::positions>(coordinates_value);
                }
            }

            mapnik::geometry::geometry<double> geom;
            mapnik::json::create_geometry(geom, geom_type, *coordinates);
            feature.set_geometry(std::move(geom));
        }
        else if (key == mapnik::json::well_known_names::properties)
        {
            auto const& prop_value = std::get<1>(elem);
            if (!prop_value.is<mapnik::json::geojson_object>())
            {
                throw std::runtime_error("\"properties\": xxx <-- expecting an JSON object here");
            }
            auto const& properties = mapnik::util::get<mapnik::json::geojson_object>(prop_value);
            auto end = keys.right.end();
            for (auto const& kv : properties)
            {
                auto itr = keys.right.find(std::get<0>(kv));
                if (itr != end)
                {
                    feature.put_new(itr->second,
                                    mapnik::util::apply_visitor(mapnik::json::attribute_value_visitor(tr, keys),
                                                                std::get<1>(kv)));
                }
            }
        }
    }
}

}}

#endif //MAPNIK_JSON_CREATE_FEATURE_HPP
