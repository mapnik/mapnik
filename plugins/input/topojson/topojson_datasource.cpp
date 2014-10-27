/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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

#include "topojson_datasource.hpp"
#include "topojson_featureset.hpp"

#include <fstream>
#include <algorithm>

// boost
#include <boost/algorithm/string.hpp>

// mapnik
#include <mapnik/unicode.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/json/topojson_grammar.hpp>
#include <mapnik/json/topojson_utils.hpp>
#include <mapnik/util/variant.hpp>
#include <mapnik/util/file_io.hpp>
#include <mapnik/make_unique.hpp>

using mapnik::datasource;
using mapnik::parameters;

DATASOURCE_PLUGIN(topojson_datasource)

struct attr_value_converter : public mapnik::util::static_visitor<mapnik::eAttributeType>
{
    mapnik::eAttributeType operator() (mapnik::value_integer /*val*/) const
    {
        return mapnik::Integer;
    }

    mapnik::eAttributeType operator() (double /*val*/) const
    {
        return mapnik::Double;
    }

    mapnik::eAttributeType operator() (float /*val*/) const
    {
        return mapnik::Double;
    }

    mapnik::eAttributeType operator() (bool /*val*/) const
    {
        return mapnik::Boolean;
    }

    mapnik::eAttributeType operator() (std::string const& /*val*/) const
    {
        return mapnik::String;
    }

    mapnik::eAttributeType operator() (mapnik::value_unicode_string const& /*val*/) const
    {
        return mapnik::String;
    }

    mapnik::eAttributeType operator() (mapnik::value_null const& /*val*/) const
    {
        return mapnik::String;
    }
};

struct geometry_type_visitor : public mapnik::util::static_visitor<int>
{
    int operator() (mapnik::topojson::point const&) const
    {
        return static_cast<int>(mapnik::datasource::Point);
    }
    int operator() (mapnik::topojson::multi_point const&) const
    {
        return static_cast<int>(mapnik::datasource::Point);
    }
    int operator() (mapnik::topojson::linestring const&) const
    {
        return static_cast<int>(mapnik::datasource::LineString);
    }
    int operator() (mapnik::topojson::multi_linestring const&) const
    {
        return static_cast<int>(mapnik::datasource::LineString);
    }
    int operator() (mapnik::topojson::polygon const&) const
    {
        return static_cast<int>(mapnik::datasource::Polygon);
    }
    int operator() (mapnik::topojson::multi_polygon const&) const
    {
        return static_cast<int>(mapnik::datasource::Polygon);
    }
    int operator() (mapnik::topojson::invalid const&) const
    {
        return -1;
    }
};

struct collect_attributes_visitor : public mapnik::util::static_visitor<void>
{
    mapnik::layer_descriptor & desc_;
    collect_attributes_visitor(mapnik::layer_descriptor & desc):
      desc_(desc) {}

    void operator() (mapnik::topojson::invalid const&) {}

    template <typename GeomType>
    void operator() (GeomType const& g)
    {
        if (g.props)
        {
            for (auto const& p : *g.props)
            {
                desc_.add_descriptor(mapnik::attribute_descriptor(std::get<0>(p),
                                                                  mapnik::util::apply_visitor(attr_value_converter(),
                                                                                              std::get<1>(p))));
            }
        }
    }
};

topojson_datasource::topojson_datasource(parameters const& params)
  : datasource(params),
    type_(datasource::Vector),
    desc_(topojson_datasource::name(),
          *params.get<std::string>("encoding","utf-8")),
    filename_(),
    inline_string_(),
    extent_(),
    tr_(new mapnik::transcoder(*params.get<std::string>("encoding","utf-8"))),
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
        if (!file) throw mapnik::datasource_exception("TopoJSON Plugin: missing <file> parameter");

        boost::optional<std::string> base = params.get<std::string>("base");
        if (base)
            filename_ = *base + "/" + *file;
        else
            filename_ = *file;
    }
    if (!inline_string_.empty())
    {
        parse_topojson(inline_string_);
    }
    else
    {
        mapnik::util::file file(filename_);
        if (!file.open())
        {
            throw mapnik::datasource_exception("TopoJSON Plugin: could not open: '" + filename_ + "'");
        }
        std::string file_buffer;
        file_buffer.resize(file.size());
        std::fread(&file_buffer[0], file.size(), 1, file.get());
        parse_topojson(file_buffer);
    }
}

namespace {
using base_iterator_type = std::string::const_iterator;
const mapnik::topojson::topojson_grammar<base_iterator_type> g;
}

template <typename T>
void topojson_datasource::parse_topojson(T const& buffer)
{
    boost::spirit::standard_wide::space_type space;
    bool result = boost::spirit::qi::phrase_parse(buffer.begin(), buffer.end(), g, space, topo_);
    if (!result)
    {
        throw mapnik::datasource_exception("topojson_datasource: Failed parse TopoJSON file '" + filename_ + "'");
    }

#if BOOST_VERSION >= 105600
    using values_container = std::vector< std::pair<box_type, std::size_t> >;
    values_container values;
    values.reserve(topo_.geometries.size());
#else
    tree_ = std::make_unique<spatial_index_type>(16, 4);
#endif

    std::size_t geometry_index = 0;

    for (auto const& geom : topo_.geometries)
    {
        mapnik::box2d<double> box = mapnik::util::apply_visitor(mapnik::topojson::bounding_box_visitor(topo_), geom);
        if (box.valid())
        {
            if (geometry_index == 0)
            {
                extent_ = box;
                collect_attributes_visitor assessor(desc_);
                mapnik::util::apply_visitor( std::ref(assessor), geom);
            }
            else
            {
                extent_.expand_to_include(box);
            }
        }
#if BOOST_VERSION >= 105600
        values.emplace_back(box_type(point_type(box.minx(),box.miny()),point_type(box.maxx(),box.maxy())), geometry_index);
#else
        tree_->insert(box_type(point_type(box.minx(),box.miny()),point_type(box.maxx(),box.maxy())),geometry_index);
#endif
        ++geometry_index;
    }

#if BOOST_VERSION >= 105600
    // packing algorithm
    tree_ = std::make_unique<spatial_index_type>(values);
#endif
}

topojson_datasource::~topojson_datasource() { }

const char * topojson_datasource::name()
{
    return "topojson";
}

boost::optional<mapnik::datasource::geometry_t> topojson_datasource::get_geometry_type() const
{
    boost::optional<mapnik::datasource::geometry_t> result;
    int multi_type = 0;
    std::size_t num_features = topo_.geometries.size();
    for (std::size_t i = 0; i < num_features && i < 5; ++i)
    {
        mapnik::topojson::geometry const& geom = topo_.geometries[i];
        int type = mapnik::util::apply_visitor(geometry_type_visitor(),geom);
        if (type > 0)
        {
            if (multi_type > 0 && multi_type != type)
            {
                result.reset(mapnik::datasource::Collection);
                return result;
            }
            else
            {
                result.reset(static_cast<mapnik::datasource::geometry_t>(type));
            }
            multi_type = type;
        }
    }
    return result;
}

mapnik::datasource::datasource_t topojson_datasource::type() const
{
    return type_;
}

mapnik::box2d<double> topojson_datasource::envelope() const
{
    return extent_;
}

mapnik::layer_descriptor topojson_datasource::get_descriptor() const
{
    return desc_;
}

mapnik::featureset_ptr topojson_datasource::features(mapnik::query const& q) const
{
    // if the query box intersects our world extent then query for features
    mapnik::box2d<double> const& b = q.get_bbox();
    if (extent_.intersects(b))
    {
        box_type box(point_type(b.minx(),b.miny()),point_type(b.maxx(),b.maxy()));
#if BOOST_VERSION >= 105600
        topojson_featureset::array_type index_array;
        if (tree_)
        {
            tree_->query(boost::geometry::index::intersects(box),std::back_inserter(index_array));
            return std::make_shared<topojson_featureset>(topo_, *tr_, std::move(index_array));
        }
#else
        if (tree_)
        {
            return std::make_shared<topojson_featureset>(topo_, *tr_, tree_->find(box));
        }
#endif
    }
    // otherwise return an empty featureset pointer
    return mapnik::featureset_ptr();
}

mapnik::featureset_ptr topojson_datasource::features_at_point(mapnik::coord2d const& pt, double tol) const
{
    mapnik::box2d<double> query_bbox(pt, pt);
    query_bbox.pad(tol);
    mapnik::query q(query_bbox);
    for (auto const& attr_info : desc_.get_descriptors())
    {
        q.add_property_name(attr_info.get_name());
    }
    return features(q);
}
