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
#include <mapnik/util/file_io.hpp>
#include <mapnik/util/utf_conv_win.hpp>

#if defined(MAPNIK_MEMORY_MAPPED_FILE)
#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>
#include <boost/spirit/include/qi.hpp>
#pragma GCC diagnostic pop
#include <mapnik/mapped_memory_cache.hpp>
#endif

#include <mapnik/json/positions_grammar.hpp>
#include <mapnik/json/extract_bounding_box_grammar_impl.hpp>
#include <mapnik/json/feature_collection_grammar_impl.hpp>

namespace {
struct feature_validate_callback
{
    feature_validate_callback(mapnik::box2d<double> const& box)
        : box_(box) {}

    void operator() (mapnik::feature_ptr const& f) const
    {
        if (box_ != f->envelope())
        {
            throw std::runtime_error("Bounding boxes mismatch validation feature");
        }
    }
    mapnik::box2d<double> const& box_;
};

using base_iterator_type = char const*;
const mapnik::json::extract_bounding_box_grammar<base_iterator_type> geojson_datasource_static_bbox_grammar;
const mapnik::transcoder tr("utf8");
const mapnik::json::feature_grammar_callback<base_iterator_type, mapnik::feature_impl, feature_validate_callback> fc_grammar(tr);
}

namespace mapnik { namespace detail {

template <typename T>
std::pair<bool,box2d<double>> process_geojson_file(T & boxes, std::string const& filename, bool validate_features)
{
    mapnik::box2d<double> extent;
#if defined(MAPNIK_MEMORY_MAPPED_FILE)
    mapnik::mapped_region_ptr mapped_region;
    boost::optional<mapnik::mapped_region_ptr> memory =
        mapnik::mapped_memory_cache::instance().find(filename, true);
    if (!memory)
    {
        std::clog << "Error : cannot memory map " << filename << std::endl;
        return std::make_pair(false, extent);
    }
    else
    {
        mapped_region = *memory;
    }
    char const* start = reinterpret_cast<char const*>(mapped_region->get_address());
    char const* end = start + mapped_region->get_size();
#else
    mapnik::util::file file(filename);
    if (!file.open())
    {
        std::clog << "Error : cannot open " << filename << std::endl;
        return std::make_pair(false, extent);
    }
    std::string file_buffer;
    file_buffer.resize(file.size());
    std::fread(&file_buffer[0], file.size(), 1, file.get());
    char const* start = file_buffer.c_str();
    char const* end = start + file_buffer.length();
#endif

    boost::spirit::standard::space_type space;
    auto const* itr = start;
    try
    {
        if (!boost::spirit::qi::phrase_parse(itr, end, (geojson_datasource_static_bbox_grammar)(boost::phoenix::ref(boxes)) , space))
        {
            std::clog << "mapnik-index (GeoJSON) : could not extract bounding boxes from : '" <<  filename <<  "'" << std::endl;
            return std::make_pair(false, extent);
        }
    }
    catch (std::exception const& ex)
    {
        std::clog << "mapnik-index (GeoJSON): " << ex.what() << std::endl;
    }
    mapnik::context_ptr ctx = std::make_shared<mapnik::context_type>();
    std::size_t start_id = 1;
    for (auto const& item : boxes)
    {
        if (item.first.valid())
        {
            if (!extent.valid()) extent = item.first;
            else extent.expand_to_include(item.first);

            if (validate_features)
            {
                base_iterator_type feat_itr = start + item.second.first;
                base_iterator_type feat_end = feat_itr + item.second.second;
                feature_validate_callback callback(item.first);
                bool result = boost::spirit::qi::phrase_parse(feat_itr, feat_end, (fc_grammar)
                                                              (boost::phoenix::ref(ctx), boost::phoenix::ref(start_id), boost::phoenix::ref(callback)),
                                                              space);
                if (!result || feat_itr != feat_end)
                {
                    return std::make_pair(false, extent);
                }
            }
        }
    }
    return std::make_pair(true, extent);
}

using box_type = mapnik::box2d<double>;
using item_type = std::pair<box_type, std::pair<std::size_t, std::size_t>>;
using boxes_type = std::vector<item_type>;
template std::pair<bool,box2d<double>> process_geojson_file(boxes_type&, std::string const&, bool);

}}
