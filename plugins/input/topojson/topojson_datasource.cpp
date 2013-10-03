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
#include <boost/make_shared.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/support_multi_pass.hpp>

#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/extensions/index/rtree/rtree.hpp>

// mapnik
#include <mapnik/unicode.hpp>
#include <mapnik/utils.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/json/topojson_grammar.hpp>
#include <mapnik/json/topojson_utils.hpp>

using mapnik::datasource;
using mapnik::parameters;

DATASOURCE_PLUGIN(topojson_datasource)

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
    bool result = boost::spirit::qi::phrase_parse(begin, end, g, boost::spirit::standard_wide::space, topo_);
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
            if (count == 0) extent_ = bbox;
            else extent_.expand_to_include(bbox);
            tree_.insert(box_type(point_type(bbox.minx(),bbox.miny()),point_type(bbox.maxx(),bbox.maxy())), count++);
        }
    }
    std::cerr << extent_ << std::endl;
}

topojson_datasource::~topojson_datasource() { }

const char * topojson_datasource::name()
{
    return "topojson";
}

boost::optional<mapnik::datasource::geometry_t> topojson_datasource::get_geometry_type() const
{
    boost::optional<mapnik::datasource::geometry_t> result;
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
        index_array_ = tree_.find(box);
        return std::make_shared<topojson_featureset>(topo_, index_array_.begin(), index_array_.end());
    }
    // otherwise return an empty featureset pointer
    return mapnik::featureset_ptr();
}

mapnik::featureset_ptr topojson_datasource::features_at_point(mapnik::coord2d const& pt, double tol) const
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
