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

#ifndef GEOMETRY_ITERATOR_HPP
#define GEOMETRY_ITERATOR_HPP

// mapnik
#include <mapnik/geometry.hpp>

// boost
#include <boost/iterator/iterator_adaptor.hpp>

template <typename Value, typename Container=mapnik::geometry_type>
class geometry_iterator
  : public boost::iterator_adaptor<geometry_iterator<Value, Container>,
                                   Value*,
                                   boost::use_default,
                                   boost::forward_traversal_tag>
{
public:
    geometry_iterator(Container const& geometry) 
      : geometry_iterator::iterator_adaptor_(0),
        geometry_(geometry)
    {}

    explicit geometry_iterator(Value* p, Container const& geometry) 
      : geometry_iterator::iterator_adaptor_(p),
        geometry_(geometry)
    {}

private:
    friend class boost::iterator_core_access;

    void increment()
    {
      //Container::value_type* x;
        double* x;
      //Container::value_type* y;        
        double* y;
        *(this->base_reference()) = Value(geometry_.vertex(x, y), *x, *y);
    }

    Container const& geometry_;
};

#endif //GEOMETRY_ITERATOR_HPP
