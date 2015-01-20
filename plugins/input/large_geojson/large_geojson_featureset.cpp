/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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
#include <mapnik/json/geometry_grammar_impl.hpp>
#include <mapnik/json/feature_grammar_impl.hpp>
// boost
#include <boost/spirit/include/support_multi_pass.hpp>
#include <boost/spirit/home/support/iterators/detail/functor_input_policy.hpp>
// stl
#include <string>
#include <vector>
#include <deque>

#include "large_geojson_featureset.hpp"

//namespace {
//using base_iterator_type = std::string::const_iterator;
//const mapnik::transcoder tr("utf8");
//const mapnik::json::feature_collection_grammar<base_iterator_type,mapnik::feature_impl> fc_grammar(tr);
//}

large_geojson_featureset::large_geojson_featureset(std::string const& filename,
                                                   array_type && index_array)
: file_(filename.c_str(), std::ios::binary),
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
#if BOOST_VERSION >= 105600
        large_geojson_datasource::item_type const& item = *index_itr_++;
        std::size_t file_offset = item.second.first;
        std::size_t size = item.second.second;
        //std::cerr << file_offset << " (" << size << ") " << item.first << std::endl;
#else
        std::size_t index = *index_itr_++;
#endif
        file_.seekg(file_offset);
        std::vector<char> json;
        json.resize(size);
        file_.read(json.data(), size);
        using chr_iterator_type = std::vector<char>::const_iterator;
        chr_iterator_type start = json.begin();
        chr_iterator_type end = json.end();

        static const mapnik::transcoder tr("utf8");
        static const mapnik::json::feature_grammar<chr_iterator_type,mapnik::feature_impl> grammar(tr);
        using namespace boost::spirit;
        standard_wide::space_type space;
        mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx_,1));
        if (!qi::phrase_parse(start, end, (grammar)(boost::phoenix::ref(*feature)), space))
        {
            throw std::runtime_error("Failed to parse geojson feature");
        }
        return feature;
    }
    return mapnik::feature_ptr();
}
