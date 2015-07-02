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

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/json/geometry_grammar.hpp>
#include <mapnik/json/feature_grammar.hpp>
#include <mapnik/util/utf_conv_win.hpp>
// stl
#include <string>
#include <vector>
#include <deque>

#include "large_geojson_featureset.hpp"

large_geojson_featureset::large_geojson_featureset(std::string const& filename,
                                                   array_type && index_array)
:
#ifdef _WINDOWS
    file_(_wfopen(mapnik::utf8_to_utf16(filename).c_str(), L"rb"), std::fclose),
#else
    file_(std::fopen(filename.c_str(),"rb"), std::fclose),
#endif
    index_array_(std::move(index_array)),
    index_itr_(index_array_.begin()),
    index_end_(index_array_.end()),
    ctx_(std::make_shared<mapnik::context_type>())
{
    if (!file_) throw std::runtime_error("Can't open " + filename);
}

large_geojson_featureset::~large_geojson_featureset() {}

mapnik::feature_ptr large_geojson_featureset::next()
{
    if (index_itr_ != index_end_)
    {
        geojson_datasource::item_type const& item = *index_itr_++;
        std::size_t file_offset = item.second.first;
        std::size_t size = item.second.second;
        std::fseek(file_.get(), file_offset, SEEK_SET);
        std::vector<char> json;
        json.resize(size);
        std::fread(json.data(), size, 1, file_.get());

        using chr_iterator_type = char const*;
        chr_iterator_type start = json.data();
        chr_iterator_type end = start + json.size();

        static const mapnik::transcoder tr("utf8");
        static const mapnik::json::feature_grammar<chr_iterator_type,mapnik::feature_impl> grammar(tr);
        using namespace boost::spirit;
        standard::space_type space;
        mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx_,1));
        if (!qi::phrase_parse(start, end, (grammar)(boost::phoenix::ref(*feature)), space))
        {
            throw std::runtime_error("Failed to parse geojson feature");
        }
        return feature;
    }
    return mapnik::feature_ptr();
}
