/* This file is part of Mapnik (c++ mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko
 *
 * Mapnik is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

//$Id: feature.hpp 40 2005-04-13 20:20:46Z pavlenko $

#ifndef FEATURE_HPP
#define FEATURE_HPP

#include "geometry.hpp"
#include "raster.hpp"
#include "value.hpp"
#include <vector>

namespace mapnik
{
    typedef boost::shared_ptr<raster> raster_ptr;    
    typedef std::vector<value> properties;
    
    template <typename T1,typename T2>
    struct feature
    {
    public:
	typedef T1 geometry_type;
	typedef T2 raster_type;
    private:
	int id_;
	geometry_type geom_;
	raster_type   raster_;
	properties props_;
    public:
	explicit feature(int id)
	    : id_(id),
	      geom_(),
	      raster_() {}

	feature(int id,const geometry_type& geom)
	    : id_(id),
	      geom_(geom),
	      raster_() {}

	feature(const feature<T1,T2>& rhs)
	    : id_(rhs.id_),
	      geom_(rhs.geom_),
	      raster_(rhs.raster_) {}

	feature<T1,T2>& operator=(const feature<T1,T2>& rhs) 
	{
	    feature<T1,T2> tmp;
	    swap(tmp);
	    return *this;
	}
	
	~feature() {}

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

        void reserve_props(unsigned n)
	{
	    props_.reserve(n);
	}

	void add_property(int v)
	{
	    return props_.push_back(value(v));
	}
	
	void add_property(double v)
	{
	    return props_.push_back(value(v));
	}
	
	void add_property(std::string const& v)
	{
	    return props_.push_back(value(v));
	}

	value get_property(size_t index) const
	{
	    if (index < props_.size())
		return props_[index]; 
	    else
		return value("");
	}
	
	const properties& get_properties() const 
	{
	    return props_;
	}
   
    private:
	void swap(const feature<T1,T2>& rhs) throw()
	{
	    std::swap(id_,rhs.id_);
	    std::swap(geom_,rhs.geom_);
	    std::swap(raster_,rhs.raster_);
	    std::swap(props_,rhs.props_);
	}
    };

    typedef feature<geometry_ptr,raster_ptr> Feature;
    
}
#endif                                            //FEATURE_HPP
