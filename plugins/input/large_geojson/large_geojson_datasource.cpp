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

#include "large_geojson_datasource.hpp"
#include "large_geojson_featureset.hpp"

#include <fstream>
#include <algorithm>
#include <functional>

// boost

#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_multi_pass.hpp>
#include <boost/spirit/home/support/iterators/detail/functor_input_policy.hpp>

// mapnik
#include <mapnik/unicode.hpp>
#include <mapnik/utils.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_kv_iterator.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/util/geometry_to_ds_type.hpp>
#include <mapnik/util/variant.hpp>
#include <mapnik/util/file_io.hpp>
#include <mapnik/make_unique.hpp>
#include <mapnik/json/feature_collection_grammar.hpp>
#include <mapnik/json/extract_bounding_box_grammar_impl.hpp>
#include <mapnik/polygon_clipper.hpp> // boost::geometry - register box2d<double>

using mapnik::datasource;
using mapnik::parameters;

DATASOURCE_PLUGIN(large_geojson_datasource)

struct attr_value_converter
{
    mapnik::eAttributeType operator() (mapnik::value_integer) const
    {
        return mapnik::Integer;
    }

    mapnik::eAttributeType operator() (double) const
    {
        return mapnik::Double;
    }

    mapnik::eAttributeType operator() (float) const
    {
        return mapnik::Double;
    }

    mapnik::eAttributeType operator() (bool) const
    {
        return mapnik::Boolean;
    }

    mapnik::eAttributeType operator() (std::string const& ) const
    {
        return mapnik::String;
    }

    mapnik::eAttributeType operator() (mapnik::value_unicode_string const&) const
    {
        return mapnik::String;
    }

    mapnik::eAttributeType operator() (mapnik::value_null const& ) const
    {
        return mapnik::String;
    }
};

large_geojson_datasource::large_geojson_datasource(parameters const& params)
  : datasource(params),
    type_(datasource::Vector),
    desc_(large_geojson_datasource::name(),
          *params.get<std::string>("encoding","utf-8")),
    filename_(),
    inline_string_(),
    extent_(),
    features_(),
    tree_(nullptr)
{
    boost::optional<std::string> inline_string = params.get<std::string>("inline");
    if (inline_string)
    {
        inline_string_ = *inline_string;
    }
    else
    {
        boost::optional<std::string> file = params.get<std::string>("file");
        if (!file) throw mapnik::datasource_exception("GeoJSON Plugin: missing <file> parameter");

        boost::optional<std::string> base = params.get<std::string>("base");
        if (base)
            filename_ = *base + "/" + *file;
        else
            filename_ = *file;
    }
    if (!inline_string_.empty())
    {
        //parse_geojson(inline_string_);
    }
    else
    {
        mapnik::util::file file(filename_);
        if (!file.open())
        {
            throw mapnik::datasource_exception("GeoJSON Plugin: could not open: '" + filename_ + "'");
        }
        std::vector<char> json;
        json.resize(file.size());
        std::fread(json.data(), file.size(), 1, file.get());
        initialise_index(json.begin(), json.end());
    }
}

template <typename Iterator>
void large_geojson_datasource::initialise_index(Iterator start, Iterator end)
{
    mapnik::json::boxes boxes;
    mapnik::json::extract_bounding_box_grammar<Iterator> g;
    boost::spirit::standard_wide::space_type space;
    if (!boost::spirit::qi::phrase_parse(start, end, (g)(boost::phoenix::ref(boxes)) , space))
    {
        throw mapnik::datasource_exception("GeoJSON Plugin: could not parse: '" + filename_ + "'");
    }
    std::cerr << "OK size=" << boxes.size() << std::endl;
    std::cerr << "Populate index" << std::endl;
    tree_ = std::make_unique<spatial_index_type>(boxes);
    std::cerr << "Calculate total extent" << std::endl;

    for (auto const& item : boxes)
    {
        auto const& box = std::get<0>(item);
        if (!extent_.valid())
        {
            extent_ = box;
        }
        else
        {
            extent_.expand_to_include(box);
        }
    }
    std::cerr << "Extent: " << extent_ << std::endl;
}

large_geojson_datasource::~large_geojson_datasource() {}

const char * large_geojson_datasource::name()
{
    return "large-geojson";
}

boost::optional<mapnik::datasource::geometry_t> large_geojson_datasource::get_geometry_type() const
{
    boost::optional<mapnik::datasource::geometry_t> result;
    int multi_type = 0;
    unsigned num_features = features_.size();
    for (unsigned i = 0; i < num_features && i < 5; ++i)
    {
        mapnik::util::to_ds_type(features_[i]->paths(),result);
        if (result)
        {
            int type = static_cast<int>(*result);
            if (multi_type > 0 && multi_type != type)
            {
                result.reset(mapnik::datasource::Collection);
                return result;
            }
            multi_type = type;
        }
    }
    return result;
}

mapnik::datasource::datasource_t large_geojson_datasource::type() const
{
    return type_;
}

mapnik::box2d<double> large_geojson_datasource::envelope() const
{
    return extent_;
}

mapnik::layer_descriptor large_geojson_datasource::get_descriptor() const
{
    return desc_;
}

mapnik::featureset_ptr large_geojson_datasource::features(mapnik::query const& q) const
{
    // if the query box intersects our world extent then query for features
    mapnik::box2d<double> const& box = q.get_bbox();
    if (extent_.intersects(box))
    {
#if BOOST_VERSION >= 105600
        large_geojson_featureset::array_type index_array;
        using item_type = large_geojson_featureset::array_type::value_type;
        if (tree_)
        {
            tree_->query(boost::geometry::index::intersects(box),std::back_inserter(index_array));
            std::cerr << "Query size=" << index_array.size() << std::endl;
            std::cerr << "Sort index_array by offsets" << std::endl;
            std::sort(index_array.begin(),index_array.end(), [](item_type const& item0, item_type const& item1) {return item0.second.first < item1.second.first;});
            std::cerr << "Done" << std::endl;
            return std::make_shared<large_geojson_featureset>(filename_, std::move(index_array));
        }
#else
        if (tree_)
        {
            return std::make_shared<large_geojson_featureset>(features_, tree_->find(box));
        }
#endif
    }
    // otherwise return an empty featureset pointer
    return mapnik::featureset_ptr();
}

mapnik::featureset_ptr large_geojson_datasource::features_at_point(mapnik::coord2d const& pt, double tol) const
{
    mapnik::box2d<double> query_bbox(pt, pt);
    query_bbox.pad(tol);
    mapnik::query q(query_bbox);
    std::vector<mapnik::attribute_descriptor> const& desc = desc_.get_descriptors();
    std::vector<mapnik::attribute_descriptor>::const_iterator itr = desc.begin();
    std::vector<mapnik::attribute_descriptor>::const_iterator end = desc.end();
    for ( ;itr!=end;++itr)
    {
        q.add_property_name(itr->get_name());
    }
    return features(q);
}
