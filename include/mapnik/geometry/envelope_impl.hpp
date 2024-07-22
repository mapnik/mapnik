/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
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

#include <mapnik/geometry/envelope.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geometry/box2d.hpp>

namespace mapnik {
namespace geometry {

namespace detail {

template<typename T>
struct geometry_envelope
{
    using coordinate_type = T;
    using bbox_type = box2d<coordinate_type>;
    bbox_type& bbox;

    explicit geometry_envelope(bbox_type& bbox_)
        : bbox(bbox_)
    {}

    template<typename U>
    void operator()(U const& geom) const
    {
        return mapnik::util::apply_visitor(*this, geom);
    }

    void operator()(mapnik::geometry::geometry_empty const&) const {}

    void operator()(mapnik::geometry::point<T> const& pt) const
    {
        if (!bbox.valid())
        {
            bbox.init(pt.x, pt.y, pt.x, pt.y);
        }
        bbox.expand_to_include(pt.x, pt.y);
    }

    void operator()(mapnik::geometry::line_string<T> const& line) const { _envelope_impl(line, bbox); }

    void operator()(mapnik::geometry::linear_ring<T> const& ring) const { _envelope_impl(ring, bbox); }

    void operator()(mapnik::geometry::polygon<T> const& poly) const
    {
        if (!poly.empty())
            _envelope_impl(poly[0], bbox);
    }

    void operator()(mapnik::geometry::multi_point<T> const& multi_point) const { _envelope_impl(multi_point, bbox); }

    void operator()(mapnik::geometry::multi_line_string<T> const& multi_line) const
    {
        for (auto const& line : multi_line)
        {
            (*this)(line);
        }
    }

    void operator()(mapnik::geometry::multi_polygon<T> const& multi_poly) const
    {
        for (auto const& poly : multi_poly)
        {
            (*this)(poly);
        }
    }

    void operator()(mapnik::geometry::geometry_collection<T> const& collection) const
    {
        for (auto const& geom : collection)
        {
            (*this)(geom);
        }
    }

  private:
    template<typename Points>
    void _envelope_impl(Points const& points, bbox_type& b) const
    {
        bool first = true;
        for (auto const& pt : points)
        {
            if (first && !b.valid())
            {
                b.init(pt.x, pt.y, pt.x, pt.y);
                first = false;
            }
            else
            {
                b.expand_to_include(pt.x, pt.y);
            }
        }
    }
};

} // namespace detail

template<typename T>
auto envelope(T const& geom) -> box2d<typename T::coordinate_type>
{
    using coordinate_type = typename T::coordinate_type;
    box2d<coordinate_type> bbox;
    detail::geometry_envelope<coordinate_type> op(bbox);
    op(geom);
    return bbox;
}

} // namespace geometry
} // namespace mapnik
