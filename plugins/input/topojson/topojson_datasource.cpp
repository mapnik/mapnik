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
#include <boost/variant.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/support_multi_pass.hpp>

#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/extensions/index/rtree/rtree.hpp>

// mapnik
#include <mapnik/unicode.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/json/topojson_grammar.hpp>
#include <mapnik/json/topojson_utils.hpp>

using mapnik::datasource;
using mapnik::parameters;

DATASOURCE_PLUGIN(topojson_datasource)

struct attr_value_converter : public boost::static_visitor<mapnik::eAttributeType>
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

struct geometry_type_visitor : public boost::static_visitor<int>
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

struct collect_attributes_visitor : public boost::static_visitor<void>
{
    mapnik::layer_descriptor & desc_;
    collect_attributes_visitor(mapnik::layer_descriptor & desc):
      desc_(desc) {}

    void operator() (mapnik::topojson::invalid const& g) {}

    template <typename GeomType>
    void operator() (GeomType const& g)
    {
        if (g.props)
        {
            for (auto const& p : *g.props)
            {
                desc_.add_descriptor(mapnik::attribute_descriptor(std::get<0>(p),
                    boost::apply_visitor(attr_value_converter(),std::get<1>(p))));
            }
        }
    }
};

topojson_datasource::topojson_datasource(parameters const& params)
  : datasource(params),
    type_(datasource::Vector),
    desc_(*params.get<std::string>("type"),
          *params.get<std::string>("encoding","utf-8")),
    file_(*params.get<std::string>("file","")),
    extent_(),
    tr_(new mapnik::transcoder(*params.get<std::string>("encoding","utf-8"))),
    tree_(16,1)
{
    if (file_.empty()) throw mapnik::datasource_exception("TopoJSON Plugin: missing <file> parameter");

    boost::optional<std::string> base = params.get<std::string>("base");
    if (base)
    {
        file_ = *base + "/" + file_;
    }

    typedef std::istreambuf_iterator<char> base_iterator_type;

#if defined (_WINDOWS)
    std::ifstream is(mapnik::utf8_to_utf16(file_),std::ios_base::in | std::ios_base::binary);
#else
    std::ifstream is(file_.c_str(),std::ios_base::in | std::ios_base::binary);
#endif
    if (!is.is_open())
    {
        throw mapnik::datasource_exception("TopoJSON Plugin: could not open: '" + file_ + "'");
    }

    boost::spirit::multi_pass<base_iterator_type> begin =
        boost::spirit::make_default_multi_pass(base_iterator_type(is));

    boost::spirit::multi_pass<base_iterator_type> end =
        boost::spirit::make_default_multi_pass(base_iterator_type());

    mapnik::topojson::topojson_grammar<boost::spirit::multi_pass<base_iterator_type> > g;
    boost::spirit::standard_wide::space_type space;
    bool result = boost::spirit::qi::phrase_parse(begin, end, g, space, topo_);
    if (!result)
    {
        throw mapnik::datasource_exception("topojson_datasource: Failed parse TopoJSON file '" + file_ + "'");
    }

    std::size_t count = 0;
    for (auto const& geom : topo_.geometries)
    {
        mapnik::box2d<double> bbox = boost::apply_visitor(mapnik::topojson::bounding_box_visitor(topo_), geom);
        if (bbox.valid())
        {
            if (count == 0)
            {
                extent_ = bbox;
                collect_attributes_visitor assessor(desc_);
                boost::apply_visitor(assessor,geom);
            }
            else
            {
                extent_.expand_to_include(bbox);
            }
            tree_.insert(box_type(point_type(bbox.minx(),bbox.miny()),point_type(bbox.maxx(),bbox.maxy())), count);
        }
        ++count;
    }
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
        int type = boost::apply_visitor(geometry_type_visitor(),geom);
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
        return std::make_shared<topojson_featureset>(topo_, *tr_, tree_.find(box));
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
