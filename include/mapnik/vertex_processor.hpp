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

#ifndef MAPNIK_VERTEX_PROCESSOR_HPP
#define MAPNIK_VERTEX_PROCESSOR_HPP

#include <mapnik/vertex_adapters.hpp>

namespace mapnik { namespace geometry {

template <typename T>
struct vertex_processor
{
    using processor_type = T;
    vertex_processor(processor_type& proc)
        : proc_(proc) {}

    template <typename Geometry>
    void operator() (Geometry const& geom)
    {
        util::apply_visitor(*this, geom);
    }
    void operator() (geometry_empty const&)
    {
        // no-op
    }
    
    template <typename T1>
    void operator() (point<T1> const& pt)
    {
        point_vertex_adapter<T1> va(pt);
        proc_(va);
    }

    template <typename T1>
    void operator() (line_string<T1> const& line)
    {
        line_string_vertex_adapter<T1> va(line);
        proc_(va);
    }

    template <typename T1>
    void operator() (polygon<T1> const& poly)
    {
        polygon_vertex_adapter<T1> va(poly);
        proc_(va);
    }

    template <typename T1>
    void operator() (multi_point<T1> const& multi_pt)
    {
        for (auto const& pt : multi_pt)
        {
            point_vertex_adapter<T1> va(pt);
            proc_(va);
        }
    }

    template <typename T1>
    void operator() (multi_line_string<T1> const& multi_line)
    {
        for (auto const& line : multi_line)
        {
            line_string_vertex_adapter<T1> va(line);
            proc_(va);
        }
    }

    template <typename T1>
    void operator() (multi_polygon<T1> const& multi_poly)
    {
        for ( auto const& poly : multi_poly)
        {
            polygon_vertex_adapter<T1> va(poly);
            proc_(va);
        }
    }

    template <typename T1>
    void operator() (geometry_collection<T1> const& collection)
    {
        for (auto const& geom : collection)
        {
            operator()(geom);
        }
    }
    processor_type & proc_;
};

}}

#endif // MAPNIK_VERTEX_PROCESSOR_HPP
