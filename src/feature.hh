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

//$Id$

#ifndef FEATURE_HH
#define FEATURE_HH

#include "geometry.hh"
#include "raster.hh"
#include "attribute.hh"

namespace mapnik
{
    typedef ref_ptr<raster> raster_ptr;
      
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
	attributes attr_;
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
	
	geometry_type& get_geometry()
	{
	    return geom_;
	}
        
	const raster_type& get_raster() const
	{
	    return raster_;
	}
        
	template <typename T>
	void add_attribute(const std::string& name,const T& value)
	{
	    attr_.insert(std::make_pair(name,attribute(value)));
	}
	
	attribute attribute_by_name(const std::string& name) const
	{
	    typename attributes::const_iterator pos=attr_.find(name);
	    if (pos!=attr_.end())
		return pos->second;
	    return attribute();
	}
	
	const attributes& get_attributes() const 
	{
	    return attr_;
	}
   
    private:
	void swap(const feature<T1,T2>& rhs) throw()
	{
	    std::swap(id_,rhs.id_);
	    std::swap(geom_,rhs.geom_);
	    std::swap(raster_,rhs.raster_);
	    std::swap(attr_,rhs.attr_);
	}
    };

    typedef feature<geometry_ptr,raster_ptr> Feature;
    
}
#endif                                            //FEATURE_HH
