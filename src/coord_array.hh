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
 * GNU General Public License for more details.#include "memory.hh"
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

//$Id$

#ifndef COORD_ARRAY_HH
#define COORD_ARRAY_HH

#include "coord.hh"
#include <cassert>

namespace mapnik
{

    template <typename T>
    class coord_array 
    {   
	typedef T coord_type;
	coord_type* pt_;
	const unsigned size_;
    public:	
	coord_array(unsigned size=0)
	    : pt_(static_cast<coord_type*>(size==0?0: ::operator new (sizeof(coord_type)*size))),
	      size_(size) {}
	
	coord_array(const coord_array& rhs)
	    : pt_(static_cast<coord_type*>(rhs.size_==0?0: ::operator new (sizeof(coord_type)*rhs.size_))),
	      size_(rhs.size_) {
	    memcpy(pt_,rhs.pt_,sizeof(coord_type)*rhs.size_);
	}
	
	~coord_array() 
	{
	    ::operator delete (pt_);
	}
	
	unsigned size() const 
	{
	    return size_;
	}
	
	void set(unsigned index,double x,double y)
	{
	    assert(index<size_);
	    pt_[index].x=x;
	    pt_[index].y=y;
	}
	
	const coord_type& at(unsigned index) const 
	{
	    assert(index<size_);
	    return pt_[index];
	}
	
	const coord_type& operator[] (unsigned index) const
	{
	    assert (index<size_);
	    return pt_[index];
	}
	
	coord_type& operator[] (unsigned index)
	{
	    assert (index<size_);
	    return pt_[index];
	}
	
    private:
	coord_array& operator=(const coord_array&);
    };
}


#endif //COORD_ARRAY_HH
