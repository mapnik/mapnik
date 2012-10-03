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

#include "geojson_datasource.hpp"
#include "geojson_featureset.hpp"

#include <fstream>
#include <iostream>
#include <algorithm>

// boost
#include <boost/variant.hpp>
#include <boost/make_shared.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/support_multi_pass.hpp>
#include <boost/foreach.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/extensions/index/rtree/rtree.hpp>
// mapnik
#include <mapnik/box2d.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/util/geometry_to_ds_type.hpp>
#include <mapnik/json/feature_collection_parser.hpp>

using mapnik::datasource;
using mapnik::parameters;

DATASOURCE_PLUGIN(geojson_datasource)

geojson_datasource::geojson_datasource(parameters const& params, bool bind)
: datasource(params),
    type_(datasource::Vector),
    desc_(*params_.get<std::string>("type"),
          *params_.get<std::string>("encoding","utf-8")),
    file_(*params_.get<std::string>("file","")),
    extent_(),
    tr_(new mapnik::transcoder(*params_.get<std::string>("encoding","utf-8"))),
    features_(),
    tree_(16,1)
{
    if (file_.empty()) throw mapnik::datasource_exception("GeoJSON Plugin: missing <file> parameter");
    if (bind)
    {
        this->bind();
    }
}

struct attr_value_converter : public boost::static_visitor<mapnik::eAttributeType>
{
    mapnik::eAttributeType operator() (int /*val*/) const
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

    mapnik::eAttributeType operator() (UnicodeString const& /*val*/) const
    {
        return mapnik::String;
    }

    mapnik::eAttributeType operator() (mapnik::value_null const& /*val*/) const
    {
        return mapnik::String;
    }
};

void geojson_datasource::bind() const
{
    if (is_bound_) return;

    typedef std::istreambuf_iterator<char> base_iterator_type;    
    
    std::ifstream is(file_.c_str());
    boost::spirit::multi_pass<base_iterator_type> begin = 
        boost::spirit::make_default_multi_pass(base_iterator_type(is));

    boost::spirit::multi_pass<base_iterator_type> end = 
        boost::spirit::make_default_multi_pass(base_iterator_type());
    
    mapnik::context_ptr ctx = boost::make_shared<mapnik::context_type>();
    mapnik::json::feature_collection_parser<boost::spirit::multi_pass<base_iterator_type> > p(ctx,*tr_);
    bool result = p.parse(begin,end, features_);
    if (!result) 
    {
        throw mapnik::datasource_exception("geojson_datasource: Failed parse GeoJSON file '" + file_ + "'");
    }
    
    bool first = true;
    std::size_t count=0;
    BOOST_FOREACH (mapnik::feature_ptr f, features_)
    {
        mapnik::box2d<double> const& box = f->envelope();
        if (first) 
        {
            extent_ = box;
            first = false;
            mapnik::feature_kv_iterator itr = f->begin();
            mapnik::feature_kv_iterator end = f->end();
            for ( ;itr!=end; ++itr)
            {
                desc_.add_descriptor(mapnik::attribute_descriptor(boost::get<0>(*itr),
                    boost::apply_visitor(attr_value_converter(),boost::get<1>(*itr).base())));
            }
        }
        else
        {
            extent_.expand_to_include(box);
        }       
        tree_.insert(box_type(point_type(box.minx(),box.miny()),point_type(box.maxx(),box.maxy())), count++);
    }
    is_bound_ = true;
}

geojson_datasource::~geojson_datasource() { }

const char * geojson_datasource::name()
{
    return "geojson";
}

boost::optional<mapnik::datasource::geometry_t> geojson_datasource::get_geometry_type() const 
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

mapnik::datasource::datasource_t geojson_datasource::type() const 
{
    return type_;
}

mapnik::box2d<double> geojson_datasource::envelope() const
{
    if (!is_bound_) bind();
    return extent_;
}

mapnik::layer_descriptor geojson_datasource::get_descriptor() const
{
    if (!is_bound_) bind();

    return desc_;
}

mapnik::featureset_ptr geojson_datasource::features(mapnik::query const& q) const
{
    if (!is_bound_) bind();
    
    // if the query box intersects our world extent then query for features
    mapnik::box2d<double> const& b = q.get_bbox();
    if (extent_.intersects(b))
    {
        box_type box(point_type(b.minx(),b.miny()),point_type(b.maxx(),b.maxy()));
        index_array_ = tree_.find(box);
        return boost::make_shared<geojson_featureset>(features_, index_array_.begin(), index_array_.end());
    }    
    // otherwise return an empty featureset pointer
    return mapnik::featureset_ptr();
}

// FIXME
mapnik::featureset_ptr geojson_datasource::features_at_point(mapnik::coord2d const& pt, double tol) const
{
    if (!is_bound_) bind();
    throw mapnik::datasource_exception("GeoJSON Plugin: features_at_point is not supported yet");
    return mapnik::featureset_ptr();
}
