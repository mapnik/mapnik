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

namespace mapnik
{
    enum {
    	Point = 1,
    	LineString = 2,
    	Polygon = 3,
    };
             
    template <typename T,template <typename> class Container=vertex_vector>
    class geometry
    {   
    public:
	typedef T vertex_type;
	typedef typename vertex_type::type value_type;
	typedef Container<vertex_type> container_type;
    private:
	int srid_;
	mutable unsigned itr_;
    protected:
	container_type cont_;
    public:
	geometry (int srid=-1)
	    : srid_(srid),
	      itr_(0),
	      cont_() {}	
	
	virtual int type() const=0;
	
	virtual bool hit_test(value_type x,value_type y) const=0;
	
	int srid() const
	{
	    return srid_;
	}
	
	void move_to(value_type x,value_type y)
	{
	    cont_.push_back(x,y,SEG_MOVETO);
	}

	void line_to(value_type x,value_type y)
	{
	    cont_.push_back(x,y,SEG_LINETO);
	}
	
	template <typename Transform>
	class path_iterator
	{
	    typedef vertex<typename Transform::return_type,2> vertex_type;
	    const container_type* cont_;
	    unsigned pos_;
	    unsigned cmd_;
	    vertex_type vertex_; 
	    
	private:

	    void advance () 
	    {
		if (pos_ < cont_->size())
		{
		    value_type x,y;
		    vertex_.cmd=cont_->get_vertex(pos_,&x,&y);
		    vertex_.x=Transform::apply(x);
		    vertex_.y=Transform::apply(y);
		}
		else 
		{
		    vertex_.cmd=SEG_END;
		    vertex_.x=0;
		    vertex_.y=0;
		}
		++pos_;
	    } 

	public:

	    path_iterator()
		: cont_(0),
		  pos_(0),
		  cmd_(SEG_END),
		  vertex_(0,0,cmd_) {}

	    explicit path_iterator(const container_type& cont)
		: cont_(&cont),
		  pos_(0),
		  cmd_(SEG_MOVETO),
		  vertex_(0,0,cmd_)
	    {
		advance();
	    }

	    path_iterator& operator++()
	    {
		advance();
		return *this;
	    }
	    
	    const vertex_type& operator*() const
	    {
		return vertex_;
	    }
	    
	    const vertex_type* operator->() const
	    {
		return &vertex_;
	    }
	    
	    bool operator !=(const path_iterator& itr)
	    {
		return vertex_.cmd !=itr.vertex_.cmd;
	    }
	};
	
	template <typename Transform>
	path_iterator<Transform> begin() const 
	{
	    return path_iterator<Transform>(cont_);
	}
	
	template <typename Transform>
	path_iterator<Transform> end() const 
	{
	    return path_iterator<Transform>();
	}

	void transform(const mapnik::CoordTransform& t)
	{
	    for (unsigned pos=0;pos<cont_.size();++pos)
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
	
	virtual ~geometry() {}
    private:
	geometry(const geometry&);
	geometry& operator=(const geometry&);
    };

    template <typename T, template <typename> class Container=vertex_vector>
    class point : public geometry<T,Container>
    {
	typedef geometry<T,Container> geometry_base;
	typedef typename   geometry<T,Container>::value_type value_type;
	using geometry<T,Container>::cont_;
    public:
	point(int srid)
	    : geometry<T,Container>(srid)
	{}
	 
	int type() const 
	{
	    return Point;
	}
	bool hit_test(value_type x,value_type y) const
	{
	    typedef typename geometry_base::template path_iterator<NO_SHIFT> path_iterator;
	    path_iterator start = geometry_base::template begin<NO_SHIFT>();
	    path_iterator end = geometry_base::template end<NO_SHIFT>();
	    return point_on_points(x,y,start,end);
	}
    };

    template <typename T, template <typename> class Container=vertex_vector>
    class polygon : public geometry<T,Container>
    {
	typedef geometry<T,Container> geometry_base;
	typedef typename geometry_base::value_type value_type;
	
    public:
	polygon(int srid)
	    : geometry_base(srid)
	{}

	int type() const 
	{
	    return Polygon;
	}
	
	bool hit_test(value_type x,value_type y) const
	{	    
	    typedef typename geometry_base::template path_iterator<NO_SHIFT> path_iterator;
	    path_iterator start = geometry_base::template begin<NO_SHIFT>();
	    path_iterator end = geometry_base::template end<NO_SHIFT>();
	    return point_inside_path(x,y, start,end);
	} 
    };
    
    template <typename T, template <typename> class Container=vertex_vector>
    class line_string : public geometry<T,Container>
    {
	typedef geometry<T,Container> geometry_base;
	typedef typename   geometry<T,Container>::value_type value_type; 
    public:
	line_string(int srid)
	    : geometry<T,Container>(srid)
	{}
	
	int type() const 
	{
	    return LineString;
	}
	
	bool hit_test(value_type x,value_type y) const
	{
	    typedef typename geometry_base::template path_iterator<NO_SHIFT> path_iterator;
	    path_iterator start = geometry_base::template begin<NO_SHIFT>();
	    path_iterator end = geometry_base::template end<NO_SHIFT>();
	    return point_on_path(x,y,start,end);
	}
    };

    typedef point<vertex2d> point_impl;
    typedef line_string<vertex2d> line_string_impl;
    typedef polygon<vertex2d> polygon_impl;
    
    typedef geometry<vertex2d> geometry_type;
    typedef boost::shared_ptr<geometry_type> geometry_ptr;
}

#endif //GEOMETRY_HPP
