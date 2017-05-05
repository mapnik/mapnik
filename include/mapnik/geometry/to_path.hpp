/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

#ifndef MAPNIK_GEOMETRY_TO_PATH_HPP
#define MAPNIK_GEOMETRY_TO_PATH_HPP

#include <mapnik/geometry.hpp>
#include <mapnik/path.hpp>

namespace mapnik { namespace geometry { namespace detail {

template <typename T>
struct geometry_to_path
{
    geometry_to_path(path_type & p)
        : p_(p) {}

    void operator() (geometry<T> const& geom) const
    {
        mapnik::util::apply_visitor(*this, geom);
    }

    void operator() (geometry_empty const&) const
    {
        // no-op
    }
    // point
    void operator() (point<T> const& pt) const
    {
        //point pt_new;
        //Transformer::apply(pt, pt_new);
        p_.move_to(pt.x, pt.y);
    }

    // line_string
    void operator() (line_string<T> const& line) const
    {
        bool first = true;
        for (auto const& pt : line)
        {
            //point pt_new;
            //Transformer::apply(pt, pt_new);
            if (first) { p_.move_to(pt.x, pt.y); first=false;}
            else p_.line_to(pt.x, pt.y);
        }
    }

    // polygon
    void operator() (polygon<T> const& poly) const
    {
        // rings: exterior *interior
        for (auto const& ring : poly)
        {
            bool first = true;
            for (auto const& pt : ring)
            {
                if (first)
                {
                    p_.move_to(pt.x, pt.y);
                    first=false;
                }
                else
                {
                    p_.line_to(pt.x, pt.y);
                }
            }
            if (!first)
            {
                p_.close_path();
            }
        }
    }

    // multi point
    void operator() (multi_point<T> const& multi_pt) const
    {
        for (auto const& pt : multi_pt)
        {
            (*this)(pt);
        }
    }
    // multi_line_string
    void operator() (multi_line_string<T> const& multi_line) const
    {
        for (auto const& line : multi_line)
        {
            (*this)(line);
        }
    }

    // multi_polygon
    void operator() (multi_polygon<T> const& multi_poly) const
    {
        for (auto const& poly : multi_poly)
        {
            (*this)(poly);
        }
    }
    // geometry_collection
    void operator() (geometry_collection<T> const& collection) const
    {
        for (auto const& geom :  collection)
        {
            (*this)(geom);
        }
    }

    path_type & p_;

};
} // ns detail

template <typename T>
void to_path(T const& geom, path_type & p)
{
    using coordinate_type = typename T::coordinate_type;
    detail::geometry_to_path<coordinate_type> func(p);
    func(geom);
}

}}

#endif // MAPNIK_GEOMETRY_TO_PATH_HPP
