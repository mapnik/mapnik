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

#ifndef MAPNIK_VERTEX_ITERATOR_HPP
#define MAPNIK_VERTEX_ITERATOR_HPP

// mapnik
#include <mapnik/global.hpp>
#include <mapnik/geometry.hpp>

// boost
#include <boost/tuple/tuple.hpp>
#include <boost/iterator/iterator_facade.hpp>

namespace mapnik { namespace util {

typedef boost::tuple<unsigned,double,double> vertex_type;

class vertex_iterator
    : public boost::iterator_facade<vertex_iterator,
                                    vertex_type const,                                   
                                    boost::forward_traversal_tag,
                                    vertex_type const&
                                    >
{

public:
    
    vertex_iterator()
        : geom_(0),
          v_(SEG_END,0,0) {}
    
    explicit vertex_iterator(geometry_type const& geom)
        : geom_(&geom),
          v_(SEG_END,0,0) 
    {
        increment();
    }
    
private:
    friend class boost::iterator_core_access;

    void increment() 
    {
        v_.get<0>() = geom_->vertex(&v_.get<1>(),&v_.get<2>());
    }
    
    bool equal( vertex_iterator const& other) const
    {
        return v_.get<0>() == other.v_.get<0>();
    }
    
    vertex_type const& dereference() const
    {
        return v_; 
    }
    
    geometry_type const *geom_;
    vertex_type v_;
};
                                     
}}


#endif // MAPNIK_VERTEX_ITERATOR_HPP
