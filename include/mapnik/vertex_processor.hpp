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

#include <mapnik/geometry_impl.hpp>

namespace mapnik { namespace new_geometry {

template <typename T>
struct vertex_processor
{
    using processor_type = T;
    vertex_processor(processor_type const& proc)
        : proc_(proc) {}

    auto operator() (point const& pt) const
        -> typename std::result_of<processor_type(point_vertex_adapter const&)>::type
    {
        point_vertex_adapter va(pt);
        return proc_(va);
    }

    auto operator() (line_string const& line)
        -> typename std::result_of<processor_type(line_string_vertex_adapter const&)>::type
    {
        line_string_vertex_adapter va(line);
        return proc_(va);
    }

    auto operator() (polygon3 const& poly) const
        -> typename std::result_of<processor_type(polygon_vertex_adapter_3 const&)>::type
    {
        polygon_vertex_adapter_3 va(poly);
        return proc_(va);
    }

    processor_type const& proc_;
};

}}

#endif // MAPNIK_VERTEX_PROCESSOR_HPP
