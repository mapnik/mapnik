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

#include <mapnik/geometry_envelope.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/box2d.hpp>

namespace mapnik { namespace geometry {

namespace detail {

template <typename T>
struct geometry_envelope
{
    using coord_type = T;
    using bbox_type = box2d<coord_type>;
    bbox_type & bbox;

    geometry_envelope(bbox_type & bbox_)
        : bbox(bbox_) {}

    template <typename U>
    void operator() (U const& geom) const
    {
        return mapnik::util::apply_visitor(*this, geom);
    }

    void operator() (mapnik::geometry::geometry_empty const&) const {}

    void operator() (mapnik::geometry::point<T> const& pt) const
    {
        if (!bbox.valid())
        {
            bbox.init(pt.x, pt.y, pt.x, pt.y);
        }
        bbox.expand_to_include(pt.x, pt.y);
    }

    void operator() (mapnik::geometry::line_string<T> const& line) const
    {
        bool first = true;
        for (auto const& pt : line)
        {
            if (first && !bbox.valid())
            {
                bbox.init(pt.x, pt.y, pt.x, pt.y);
                first = false;
            }
            else
            {
                bbox.expand_to_include(pt.x, pt.y);
            }
        }
    }

    void operator() (mapnik::geometry::linear_ring<T> const& ring) const
    {
        (*this)(static_cast<mapnik::geometry::line_string<T> const&>(ring));
    }

    void operator() (mapnik::geometry::polygon<T> const& poly) const
    {
        bool first = true;
        for (auto const& pt : poly.exterior_ring)
        {
            if (first && !bbox.valid())
            {
                bbox.init(pt.x, pt.y, pt.x, pt.y);
                first = false;
            }
            else
            {
                bbox.expand_to_include(pt.x, pt.y);
            }
        }
    }

    void operator() (mapnik::geometry::multi_point<T> const& multi_point) const
    {
        bool first = true;
        for (auto const& pt : multi_point)
        {
            if (first && !bbox.valid())
            {
                bbox.init(pt.x, pt.y, pt.x, pt.y);
                first = false;
            }
            else
            {
                bbox.expand_to_include(pt.x, pt.y);
            }
        }
    }

    void operator() (mapnik::geometry::multi_line_string<T> const& multi_line) const
    {
        for (auto const& line : multi_line)
        {
            (*this)(line);
        }
    }

    void operator() (mapnik::geometry::multi_polygon<T> const& multi_poly) const
    {
        for (auto const& poly : multi_poly)
        {
            (*this)(poly);
        }
    }

    void operator() (mapnik::geometry::geometry_collection<T> const& collection) const
    {
        for (auto const& geom : collection)
        {
            (*this)(geom);
        }
    }
};

} // end ns detail

template <typename T>
auto envelope(T const& geom) -> box2d<typename T::coord_type>
{
    using coord_type = typename T::coord_type;
    box2d<coord_type> bbox;
    detail::geometry_envelope<coord_type> op(bbox);
    op(geom);
    return bbox;
}

} // end ns geometry
} // end ns mapnik
