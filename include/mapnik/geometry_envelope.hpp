/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

#ifndef MAPNIK_GEOMETRY_ENVELOPE_HPP
#define MAPNIK_GEOMETRY_ENVELOPE_HPP

#include <mapnik/geometry.hpp>
#include <mapnik/box2d.hpp>

namespace mapnik { namespace geometry {

namespace detail {

struct geometry_envelope
{
    using bbox_type = box2d<double>;
    bbox_type & bbox;

    geometry_envelope(bbox_type & bbox_)
        : bbox(bbox_) {}

    template <typename T>
    void operator() (T const& geom) const
    {
        return mapnik::util::apply_visitor(*this, geom);
    }
    
    void operator() (mapnik::geometry::geometry_empty const&) const {}

    void operator() (mapnik::geometry::point const& pt) const
    {
        if (!bbox.valid())
        {
            bbox.init(pt.x, pt.y, pt.x, pt.y);
        }
        bbox.expand_to_include(pt.x, pt.y);
    }

    void operator() (mapnik::geometry::line_string const& line) const
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

    void operator() (mapnik::geometry::polygon const& poly) const
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

    void operator() (mapnik::geometry::multi_point const& multi_point) const
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

    void operator() (mapnik::geometry::multi_line_string const& multi_line) const
    {
        for (auto const& line : multi_line)
        {
            (*this)(line);
        }
    }

    void operator() (mapnik::geometry::multi_polygon const& multi_poly) const
    {
        for (auto const& poly : multi_poly)
        {
            (*this)(poly);
        }
    }

    void operator() (mapnik::geometry::geometry_collection const& collection) const
    {
        for (auto const& geom : collection)
        {
            (*this)(geom);
        }
    }
};

}

template <typename T>
inline mapnik::box2d<double> envelope(T const& geom)
{   
    box2d<double> bbox;
    detail::geometry_envelope op(bbox);
    op(geom);
    return bbox;
}

}}

#endif // MAPNIK_GEOMETRY_ENVELOPE_HPP
