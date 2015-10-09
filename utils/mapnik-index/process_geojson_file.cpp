/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

#include "process_geojson_file.hpp"
#include <mapnik/geometry.hpp>
#include <mapnik/geometry_envelope.hpp>
#include <mapnik/geometry_adapters.hpp>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>
#include <boost/spirit/include/qi.hpp>
#pragma GCC diagnostic pop
#include <mapnik/mapped_memory_cache.hpp>
#include <mapnik/json/positions_grammar.hpp>
#include <mapnik/json/extract_bounding_box_grammar_impl.hpp>

namespace {
using base_iterator_type = char const*;
const mapnik::json::extract_bounding_box_grammar<base_iterator_type> geojson_datasource_static_bbox_grammar;
}

namespace mapnik { namespace detail {

template <typename T>
std::pair<bool,box2d<double>> process_geojson_file(T & boxes, std::string const& filename)
{
    mapnik::box2d<double> extent;
    mapnik::mapped_region_ptr mapped_region;
    boost::optional<mapnik::mapped_region_ptr> memory =
        mapnik::mapped_memory_cache::instance().find(filename, true);
    if (!memory)
    {
        std::clog << "Error : cannot mmap " << filename << std::endl;
        return std::make_pair(false, extent);
    }
    else
    {
        mapped_region = *memory;
    }
    char const* start = reinterpret_cast<char const*>(mapped_region->get_address());
    char const* end = start + mapped_region->get_size();
    boost::spirit::standard::space_type space;
    try
    {
        if (!boost::spirit::qi::phrase_parse(start, end, (geojson_datasource_static_bbox_grammar)(boost::phoenix::ref(boxes)) , space))
        {
            std::clog << "mapnik-index (GeoJSON) : could not parse: '" <<  filename <<  "'";
            return std::make_pair(false, extent);
        }
    }
    catch (std::exception const& ex)
    {
        std::clog << "mapnik-index:" << ex.what() << std::endl;
    }
    for (auto const& item : boxes)
    {
        if (!extent.valid()) extent = item.first;
        else extent.expand_to_include(item.first);
    }
    return std::make_pair(true, extent);
}

using box_type = mapnik::box2d<double>;
using item_type = std::pair<box_type, std::pair<std::size_t, std::size_t>>;
using boxes_type = std::vector<item_type>;
template std::pair<bool,box2d<double>> process_geojson_file(boxes_type&, std::string const&);

}}
