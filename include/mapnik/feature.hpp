/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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

//$Id: feature.hpp 40 2005-04-13 20:20:46Z pavlenko $

#ifndef FEATURE_HPP
#define FEATURE_HPP
// mapnik
#include <mapnik/value.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/raster.hpp>

// boost
#include <boost/version.hpp>
#if BOOST_VERSION >= 104000
#include <boost/property_map/property_map.hpp>
#else
#include <boost/property_map.hpp>
#endif

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
// stl
#include <map>

namespace mapnik {
typedef boost::shared_ptr<raster> raster_ptr;    
typedef boost::associative_property_map<
std::map<std::string,value
         > > properties;
   
template <typename T1,typename T2>
struct feature : public properties,
                 private boost::noncopyable
{
public:
    typedef T1 geometry_type;
    typedef T2 raster_type;
    typedef std::map<std::string,value>::value_type value_type;
    typedef std::map<std::string,value>::size_type size_type;
    typedef std::map<std::string,value>::difference_type difference_type;
       
private:
    int id_;
    boost::ptr_vector<geometry_type> geom_cont_;
    raster_type   raster_;
    std::map<std::string,value> props_;
public:
    typedef std::map<std::string,value>::iterator iterator;
    explicit feature(int id)
        : properties(props_),
          id_(id),
          geom_cont_(),
          raster_() {}
       
    int id() const 
    {
        return id_;
    }

    void set_id(int id)
    {
        id_ = id;
    }
    
    boost::ptr_vector<geometry_type> & paths() 
    {
        return geom_cont_;
    }
    
    
    void add_geometry(geometry_type * geom)
    {
       geom_cont_.push_back(geom);
    }
       
    unsigned num_geometries() const
    {
        return geom_cont_.size();
    }
    
    geometry_type const& get_geometry(unsigned index) const
    {
        return geom_cont_[index];
    }
       
    geometry_type& get_geometry(unsigned index)
    {
        return geom_cont_[index];
    }
       
    box2d<double> envelope() const
    {
        box2d<double> result;
        for (unsigned i=0;i<num_geometries();++i)
        {
            geometry_type const& geom = get_geometry(i);
            if (i==0)
            {
                box2d<double> box = geom.envelope();
                result.init(box.minx(),box.miny(),box.maxx(),box.maxy());
            }
            else
            {
                result.expand_to_include(geom.envelope());
            }
        }
        return result;
    }
       
    const raster_type& get_raster() const
    {
        return raster_;
    }
       
    void set_raster(raster_type const& raster)
    {
        raster_=raster;
    }
       
    std::map<std::string,value> const& props() const 
    {
        return props_;
    }
       
    std::map<std::string,value>& props() 
    {
        return props_;
    }
    
    iterator begin()
    {
        return props_.begin();
    }
       
    iterator end()
    {
        return props_.end();
    }
       
    std::string to_string() const
    {
        std::stringstream ss;
        ss << "feature (" << std::endl;
        ss << "  id:" << id_ << std::endl;
        for (std::map<std::string,value>::const_iterator itr=props_.begin();
             itr != props_.end();++itr)
        {
            ss << "  " << itr->first  << ":" <<  itr->second << std::endl;
        }
        ss << ")" << std::endl;
        return ss.str();
    }
};
   
typedef feature<geometry_type,raster_ptr> Feature;
   
inline std::ostream& operator<< (std::ostream & out,Feature const& f)
{
    out << f.to_string();
    return out;
}
}

#endif //FEATURE_HPP
