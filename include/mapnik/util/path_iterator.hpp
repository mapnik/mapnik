/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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
#include <mapnik/path.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/iterator/iterator_facade.hpp>
MAPNIK_DISABLE_WARNING_POP

// stl
#include <tuple>

namespace mapnik {
namespace util {

template<typename T>
class path_iterator : public boost::iterator_facade<path_iterator<T>,
                                                    std::tuple<unsigned, double, double> const,
                                                    boost::forward_traversal_tag>
{
  public:
    using path_type = T;
    using value_type = typename std::tuple<unsigned, double, double>;

    path_iterator()
        : v_(mapnik::SEG_END, 0, 0)
        , vertices_()
    {}

    explicit path_iterator(path_type const& vertices)
        : vertices_(&vertices)
    {
        vertices_->rewind(0);
        increment();
    }

  private:
    friend class boost::iterator_core_access;

    void increment() { std::get<0>(v_) = vertices_->vertex(&std::get<1>(v_), &std::get<2>(v_)); }

    bool equal(path_iterator const& other) const { return std::get<0>(v_) == std::get<0>(other.v_); }

    value_type const& dereference() const { return v_; }

    value_type v_;
    const path_type* vertices_;
};

// specialization for mapnik::path_type - vertex interface has been removed
template<>
class path_iterator<path_type> : public boost::iterator_facade<path_iterator<path_type>,
                                                               std::tuple<unsigned, double, double> const,
                                                               boost::forward_traversal_tag>
{
  public:
    using path_type = mapnik::path_type;
    using value_type = std::tuple<unsigned, double, double>;
    using size_type = path_type::size_type;
    path_iterator()
        : v_(mapnik::SEG_END, 0, 0)
        , vertices_()
        , pos_(0)
    {}

    explicit path_iterator(path_type const& vertices)
        : vertices_(&vertices)
        , pos_(0)
    {
        increment();
    }

  private:
    friend class boost::iterator_core_access;

    void increment()
    {
        std::get<0>(v_) =
          vertices_->cont_.get_vertex(static_cast<unsigned>(pos_++), &std::get<1>(v_), &std::get<2>(v_));
    }

    bool equal(path_iterator const& other) const { return std::get<0>(v_) == std::get<0>(other.v_); }

    value_type const& dereference() const { return v_; }

    value_type v_;
    const path_type* vertices_;
    size_type pos_;
};

} // namespace util
} // namespace mapnik

#endif // MAPNIK_VERTEX_ITERATOR_HPP
