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
// stl
#include <map>
// boost
#include <boost/property_map.hpp>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
// mapnik
#include <mapnik/value.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/raster.hpp>

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
    private:
        int id_;
        geometry_type geom_;
        raster_type   raster_;
        std::map<std::string,value> props_;
    public:
        explicit feature(int id)
            : properties(props_),
              id_(id),
              geom_(),
              raster_() {}

        feature(int id,const geometry_type& geom)
            : properties(props_),
              id_(id),
              geom_(geom),
              raster_() {}

        int id() const 
        {
            return id_;
        }
	
        void set_geometry(geometry_type& geom)
        {
            geom_=geom;
        }
	
        geometry_type const& get_geometry() const
        {
            return geom_;
        }
        
        const raster_type& get_raster() const
        {
            return raster_;
        }
        void set_raster(raster_type const& raster)
        {
            raster_=raster;
        }
	
        const properties& get_properties() const 
        {
            return props_;
        }
	
        std::string to_string() const
        {
            std::stringstream ss;
            ss << "feature (" << std::endl;
            for (std::map<std::string,value>::const_iterator itr=props_.begin();
                 itr != props_.end();++itr)
            {
                ss << "  " << itr->first  << ":" <<  itr->second << std::endl;
            }
            ss << ")" << std::endl;
            return ss.str();
        }
    };

    typedef feature<geometry_ptr,raster_ptr> Feature;
    
    inline std::ostream& operator<< (std::ostream & out,Feature const& f)
    {
        out << f.to_string();
    	return out;
    }
}

#endif //FEATURE_HPP
