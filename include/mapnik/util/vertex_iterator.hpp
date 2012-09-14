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

#ifndef MAPNIK_VERTEX_ITERATOR_HPP
#define MAPNIK_VERTEX_ITERATOR_HPP

// mapnik
#include <mapnik/global.hpp>
#include <mapnik/vertex_vector.hpp>

// boost
#include <boost/tuple/tuple.hpp>
#include <boost/iterator/iterator_facade.hpp>

namespace mapnik { namespace util {

    template <typename T>
    class vertex_iterator
        : public boost::iterator_facade< vertex_iterator<T>,
                                         typename boost::tuple<unsigned,T,T> const,
                                         boost::forward_traversal_tag
                                         >
    {

    public:
        typedef typename boost::tuple<unsigned, T, T> value_type;
        typedef vertex_vector<T> container_type;

        vertex_iterator()
            : v_(SEG_END,0,0),
              vertices_(),
              pos_(0)
        {}

        explicit vertex_iterator(container_type const& vertices)
            : vertices_(&vertices),
              pos_(0)
        {
            increment();
        }

    private:
        friend class boost::iterator_core_access;

        void increment()
        {
            boost::get<0>(v_) = vertices_->get_vertex(pos_++, &boost::get<1>(v_), &boost::get<2>(v_));
        }

        bool equal( vertex_iterator const& other) const
        {
            return boost::get<0>(v_) == boost::get<0>(other.v_);
        }

        value_type const& dereference() const
        {
            return v_;
        }

        value_type v_;
        container_type const *vertices_;
        unsigned pos_;
    };

    }}


#endif // MAPNIK_VERTEX_ITERATOR_HPP
