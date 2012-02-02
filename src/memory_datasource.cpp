/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

//$Id$
// mapnik
#include <mapnik/memory_datasource.hpp>
#include <mapnik/memory_featureset.hpp>
#include <mapnik/params.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/feature_kv_iterator.hpp>

#include <boost/make_shared.hpp>
#include <boost/math/distributions/normal.hpp>
// stl
#include <algorithm>

namespace mapnik {
    
struct accumulate_extent
{
    accumulate_extent(box2d<double> & ext)
        : ext_(ext),first_(true) {}
        
    void operator() (feature_ptr feat)
    {
        for (unsigned i=0;i<feat->num_geometries();++i)
        {
            geometry_type & geom = feat->get_geometry(i);
            if ( first_ ) 
            {
                first_ = false;
                ext_ = geom.envelope();
            }
            else
            {
                ext_.expand_to_include(geom.envelope());
            }
        }
    }
        
    box2d<double> & ext_;
    bool first_;
};
    
memory_datasource::memory_datasource()
    : datasource(parameters()),
      desc_("in-memory datasource","utf-8") {}

memory_datasource::~memory_datasource() {}
    
void memory_datasource::push(feature_ptr feature)
{
    mapnik::feature_kv_iterator::feature_kv_iterator it(*feature, true);
    mapnik::feature_kv_iterator::feature_kv_iterator end(*feature);
    for (; it != end; ++it) {
        try {
            accumulators_[boost::get<0>(*it)](boost::get<1>(*it).to_double());
        } catch(boost::bad_lexical_cast &) {
            // string values are not accumulated.
        }
    }
    features_.push_back(feature);
}
    
datasource::datasource_t memory_datasource::type() const
{
    return datasource::Vector;
}
    
featureset_ptr memory_datasource::features(const query& q) const
{
    return featureset_ptr(new memory_featureset(q.get_bbox(),*this));
}


featureset_ptr memory_datasource::features_at_point(coord2d const& pt) const
{
    box2d<double> box = box2d<double>(pt.x, pt.y, pt.x, pt.y);
#ifdef MAPNIK_DEBUG
    std::clog << "box=" << box << ", pt x=" << pt.x << ", y=" << pt.y << "\n";
#endif
    return featureset_ptr(new memory_featureset(box,*this));
}
    
box2d<double> memory_datasource::envelope() const
{
    box2d<double> ext;
    accumulate_extent func(ext);
    std::for_each(features_.begin(),features_.end(),func);
    return ext;      
}

boost::optional<datasource::geometry_t> memory_datasource::get_geometry_type() const
{
    // TODO - detect this?
    return datasource::Collection;
}
    
layer_descriptor memory_datasource::get_descriptor() const
{
    return desc_;
}

statistics_ptr memory_datasource::get_statistics() const
{
    std::map<std::string, mapnik::parameters> _stats;
    std::map<std::string, statistics_accumulator>::const_iterator it = accumulators_.begin();
    std::map<std::string, statistics_accumulator>::const_iterator end = accumulators_.end();
    for (; it != end; ++it) {
        mapnik::parameters p;
        p["mean"] = boost::accumulators::mean(it->second);
        p["median"] = boost::accumulators::median(it->second);
        p["min"] = boost::accumulators::min(it->second);
        p["stddev"] = sqrt(boost::accumulators::variance(it->second));
        p["max"] = boost::accumulators::max(it->second);
        _stats[it->first] = p;
    }
    return boost::make_shared<mapnik::statistics>(_stats);
}
    
size_t memory_datasource::size() const
{
    return features_.size();
}

void memory_datasource::clear()
{
    features_.clear();
}

}
