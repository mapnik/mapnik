/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

// mapnik
#include "geojson_index_featureset.hpp"
#include <mapnik/feature.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/json/feature_grammar.hpp>
#include <mapnik/util/utf_conv_win.hpp>
#include <mapnik/util/spatial_index.hpp>
#include <mapnik/geometry_is_empty.hpp>
// stl
#include <string>
#include <vector>
#include <fstream>

geojson_index_featureset::geojson_index_featureset(std::string const& filename, mapnik::filter_in_box const& filter)
    :
#if defined(MAPNIK_MEMORY_MAPPED_FILE)
    //
#elif defined _WINDOWS
    file_(_wfopen(mapnik::utf8_to_utf16(filename).c_str(), L"rb"), std::fclose),
#else
    file_(std::fopen(filename.c_str(),"rb"), std::fclose),
#endif
    ctx_(std::make_shared<mapnik::context_type>())
{

#if defined (MAPNIK_MEMORY_MAPPED_FILE)
    boost::optional<mapnik::mapped_region_ptr> memory =
        mapnik::mapped_memory_cache::instance().find(filename, true);
    if (memory)
    {
        mapped_region_ = *memory;
    }
    else
    {
        throw std::runtime_error("could not create file mapping for " + filename);
    }
#else
    if (!file_) throw std::runtime_error("Can't open " + filename);
#endif
    std::string indexname = filename + ".index";
    std::ifstream index(indexname.c_str(), std::ios::binary);
    if (!index) throw mapnik::datasource_exception("GeoJSON Plugin: can't open index file " + indexname);
    mapnik::util::spatial_index<value_type,
                                mapnik::filter_in_box,
                                std::ifstream>::query(filter, index, positions_);

    std::sort(positions_.begin(), positions_.end(),
              [](value_type const& lhs, value_type const& rhs) { return lhs.first < rhs.first;});
    itr_ = positions_.begin();
}

geojson_index_featureset::~geojson_index_featureset() {}

mapnik::feature_ptr geojson_index_featureset::next()
{
    while( itr_ != positions_.end())
    {
        auto pos = *itr_++;
#if defined(MAPNIK_MEMORY_MAPPED_FILE)
        char const* start = (char const*)mapped_region_->get_address() + pos.first;
        char const*  end = start + pos.second;
#else
        std::fseek(file_.get(), pos.first, SEEK_SET);
        std::vector<char> record;
        record.resize(pos.second);
        std::fread(record.data(), pos.second, 1, file_.get());
        auto const* start = record.data();
        auto const*  end = start + record.size();
#endif
        static const mapnik::transcoder tr("utf8");
        static const mapnik::json::feature_grammar<char const*, mapnik::feature_impl> grammar(tr);
        using namespace boost::spirit;
        standard::space_type space;
        mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx_, feature_id_++));
        if (!qi::phrase_parse(start, end, (grammar)(boost::phoenix::ref(*feature)), space) || start != end)
        {
            throw std::runtime_error("Failed to parse GeoJSON feature");
        }
        // skip empty geometries
        if (mapnik::geometry::is_empty(feature->get_geometry()))
            continue;
        return feature;
    }
    return mapnik::feature_ptr();
}
