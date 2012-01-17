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
#include <mapnik/feature_factory.hpp>
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
    // TODO - collect attribute descriptors?
    //desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::Integer));
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
    
size_t memory_datasource::size() const
{
    return features_.size();
}

void memory_datasource::clear()
{
    features_.clear();
}

// point_datasource

void point_datasource::add_point(double x, double y, const char* key, const char* value)
{
        feature_ptr feature(feature_factory::create(feature_id_));
        ++feature_id_;
        geometry_type * pt = new geometry_type(mapnik::Point);
        pt->move_to(x,y);
        feature->add_geometry(pt);
        transcoder tr("utf-8");
        (*feature)[key] = tr.transcode(value);
        this->push(feature);
}

}
