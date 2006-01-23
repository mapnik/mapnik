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

//$Id: geometry.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include "vertex_vector.hpp"
#include "vertex_transform.hpp"
#include "ctrans.hpp"
#include "geom_util.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

namespace mapnik
{
    enum {
    	Point = 1,
    	LineString = 2,
    	Polygon = 3,
    };
             
    template <typename T>
    class geometry : private boost::noncopyable
    {   
    public:
	typedef T vertex_type;
	typedef typename vertex_type::type value_type;
    private:
	int srid_;
    public:
	geometry (int srid=-1)
	    : srid_(srid) {}	

	int srid() const
	{
	    return srid_;
	}
	
	virtual int type() const=0;
	virtual bool hit_test(value_type x,value_type y) const=0;
	virtual void move_to(value_type x,value_type y)=0;
	virtual void line_to(value_type x,value_type y)=0;
	virtual void transform(const mapnik::CoordTransform& t)=0;
	virtual unsigned num_points() const = 0;
	virtual unsigned vertex(double* x, double* y)=0;
	virtual void rewind(unsigned )=0;
	virtual ~geometry() {}
    };
    
    template <typename T>
    class point : public geometry<T>
    {
	typedef geometry<T> geometry_base;
	typedef typename geometry<T>::vertex_type vertex_type;
	typedef typename geometry<T>::value_type value_type;
    private:
	vertex_type pt_;
    public:
	point(int srid)
	    : geometry<T>(srid)
	{}
	 
	int type() const 
	{
	    return Point;
	}
	
	void move_to(value_type x,value_type y)
	{
	    pt_.x = x;
	    pt_.y = y;
	}
	
	void line_to(value_type ,value_type ) {}
	
	void transform(const mapnik::CoordTransform& t)
	{
	    t.forward_x(&pt_.x);
	    t.forward_y(&pt_.y);
	}
	
	unsigned num_points() const
	{
	    return 1;
	}
	
	unsigned vertex(double* x, double* y)
	{
	    *x = pt_.x;
	    *y = pt_.y;
	    return SEG_LINETO;
	}
	
	void rewind(unsigned ) {}
	
	bool hit_test(value_type x,value_type y) const
	{
	    return false;
	}
	virtual ~point() {}
    };

    template <typename T, template <typename> class Container=vertex_vector>
    class polygon : public geometry<T>
    {
	typedef geometry<T> geometry_base;
	typedef typename geometry<T>::vertex_type vertex_type;
	typedef typename geometry_base::value_type value_type;
	typedef Container<vertex_type> container_type;
    private:
	container_type cont_;
	mutable unsigned itr_;
    public:
	polygon(int srid)
	    : geometry_base(srid),
	      itr_(0)
	{}
        
	int type() const 
	{
	    return Polygon;
	}
	
	void line_to(value_type x,value_type y)
	{
	    cont_.push_back(x,y,SEG_LINETO);
	}

	void move_to(value_type x,value_type y)
	{
	    cont_.push_back(x,y,SEG_MOVETO);
	}
	
	void transform(mapnik::CoordTransform const& t)
	{
	    unsigned size = cont_.size();
	    for (unsigned pos=0; pos < size; ++pos)
	    {	
		cont_.transform_at(pos,t);
	    }
	}
	
        unsigned num_points() const
	{
	    return cont_.size();
	}
	
	unsigned vertex(double* x, double* y)
	{
	    return cont_.get_vertex(itr_++,x,y);
	}
	
	void rewind(unsigned )
	{
	    itr_=0;
	}
	
	bool hit_test(value_type x,value_type y) const
	{	    
	    return false;
	} 
	virtual ~polygon() {}
    };
    
    template <typename T, template <typename> class Container=vertex_vector>
    class line_string : public geometry<T>
    {
	typedef geometry<T> geometry_base;
	typedef typename geometry_base::value_type value_type;
	typedef typename geometry<T>::vertex_type vertex_type;
	typedef Container<vertex_type> container_type;
    private:
	container_type cont_;
	mutable unsigned itr_;
    public:
	line_string(int srid)
	    : geometry_base(srid),
	      itr_(0)
	{}
        
	int type() const 
	{
	    return LineString;
	}
	
	void line_to(value_type x,value_type y)
	{
	    cont_.push_back(x,y,SEG_LINETO);
	}

	void move_to(value_type x,value_type y)
	{
	    cont_.push_back(x,y,SEG_MOVETO);
	}
	
	void transform(mapnik::CoordTransform const& t)
	{
	    unsigned size = cont_.size();
	    for (unsigned pos=0; pos < size; ++pos)
	    {	
		cont_.transform_at(pos,t);
	    }
	}
	
        unsigned num_points() const
	{
	    return cont_.size();
	}
	
	unsigned vertex(double* x, double* y)
	{
	    return cont_.get_vertex(itr_++,x,y);
	}
	
	void rewind(unsigned )
	{
	    itr_=0;
	}
	
	bool hit_test(value_type x,value_type y) const
	{	    
	    return false;
	} 
	virtual ~line_string() {}
    };

    typedef point<vertex2d> point_impl;
    typedef line_string<vertex2d,vertex_vector> line_string_impl;
    typedef polygon<vertex2d,vertex_vector> polygon_impl;
    
    typedef geometry<vertex2d> geometry_type;
    typedef boost::shared_ptr<geometry_type> geometry_ptr;
}

#endif //GEOMETRY_HPP
