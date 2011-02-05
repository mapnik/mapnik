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
#include <boost/tuple/tuple.hpp>
#include <boost/shared_ptr.hpp>

namespace mapnik {

template <typename Value, typename Container=geometry_type>
class geometry_iterator
  : public boost::iterator_adaptor<geometry_iterator<Value, Container>,
                                   Value*,
                                   boost::use_default,
                                   boost::forward_traversal_tag>
{
public:
    typedef Value value_type;
    typedef Container container_type;

    geometry_iterator(Container const& geometry) 
      : geometry_iterator::iterator_adaptor_(0),
        geometry_(geometry),
        first_value_(new Value(0,0,0,0,0))
    {}

    explicit geometry_iterator(Value* p, Container const& geometry) 
      : geometry_iterator::iterator_adaptor_(p),
        geometry_(geometry),
        first_value_(new Value(0,0,0,0,0))
    {
        this->increment();
    }

    struct enabler {};

    template <typename OtherValue>
    geometry_iterator(geometry_iterator<OtherValue> const& other,
                      typename boost::enable_if<boost::is_convertible<OtherValue*, Value*>,
		      enabler>::type = enabler())   
      : geometry_iterator::iterator_adaptor_(other.base()) {}

private:
    friend class boost::iterator_core_access;

    void increment()
    {
        geometry_type::value_type x;
        geometry_type::value_type y;
        unsigned cmd = geometry_.vertex(&x, &y);

        if(cmd == SEG_END)
	{
            this->base_reference() = 0;
        }
	else if(this->base_reference() == 0)
	{
            *first_value_ = Value(cmd, x, y, x, y);
	    this->base_reference() = first_value_.get();
        }
        else
	{
            *(this->base_reference()) = Value(cmd, x, y, x, y);
        }
    }

    template <typename OtherValue>
    bool equal(geometry_iterator<OtherValue, Container> const& other) const
    {
        return this->base_reference() == other.base();
    }

    Container const& geometry_;
    boost::shared_ptr<Value> first_value_;
};

typedef geometry_iterator<boost::tuple<unsigned, geometry_type::value_type, geometry_type::value_type, geometry_type::value_type, geometry_type::value_type>, geometry_type> geometry_iterator_type;
}

#endif //GEOMETRY_ITERATOR_HPP
