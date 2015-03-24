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

namespace mapnik { namespace new_geometry {

namespace detail {

struct geometry_envelope
{
    using bbox_type = box2d<double>;

    bbox_type operator() (mapnik::new_geometry::geometry const& geom) const
    {
        return mapnik::util::apply_visitor(*this, geom);
    }

    bbox_type operator() (mapnik::new_geometry::geometry_empty const&) const
    {
        return mapnik::box2d<double>();
    }

    bbox_type operator() (mapnik::new_geometry::point const& pt) const
    {
        return mapnik::box2d<double>(pt.x, pt.y, pt.x, pt.y);
    }

    bbox_type operator() (mapnik::new_geometry::line_string const& line) const
    {
        bbox_type bbox;
        for (auto const& pt : line)
        {
            if (!bbox.valid()) bbox.init(pt.x, pt.y, pt.x, pt.y);
            else bbox.expand_to_include(pt.x, pt.y);
        }
        return bbox;
    }

    bbox_type operator() (mapnik::new_geometry::polygon const& poly) const
    {
        return (*this) (static_cast<mapnik::new_geometry::line_string>(poly.exterior_ring));
    }

    bbox_type operator() (mapnik::new_geometry::multi_point const& multi_point) const
    {
        return (*this) (static_cast<mapnik::new_geometry::line_string>(multi_point));
    }

    bbox_type operator() (mapnik::new_geometry::multi_line_string const& multi_line) const
    {
        bbox_type bbox;
        for (auto const& line : multi_line)
        {
            if (!bbox.valid()) bbox = (*this)(line);
            else bbox.expand_to_include((*this)(line));
        }
        return bbox;
    }

    bbox_type operator() (mapnik::new_geometry::multi_polygon const& multi_poly) const
    {
        bbox_type bbox;
        for (auto const& poly : multi_poly)
        {
            if (!bbox.valid()) bbox = (*this)(poly);
            else bbox.expand_to_include((*this)(poly));
        }
        return bbox;
    }

    bbox_type operator() (mapnik::new_geometry::geometry_collection const& collection) const
    {
        bbox_type bbox;
        for (auto const& geom : collection)
        {
            if (!bbox.valid()) bbox = (*this)(geom);
            else bbox.expand_to_include((*this)(geom));
        }
        return bbox;
    }
};

}

inline mapnik::box2d<double> envelope(mapnik::new_geometry::geometry const& geom)
{
    return detail::geometry_envelope() (geom);
}

}}

#endif // MAPNIK_GEOMETRY_ENVELOPE_HPP
