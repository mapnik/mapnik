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

#ifndef MAPNIK_GEOMETRY_POLYGON_VERTEX_PROCESSOR_HPP
#define MAPNIK_GEOMETRY_POLYGON_VERTEX_PROCESSOR_HPP

// geometry
#include <mapnik/geometry.hpp>

namespace mapnik { namespace geometry {

template <typename T>
struct polygon_vertex_processor
{
    template <typename Path>
    void add_path(Path & path)
    {
        point<T> p;
        unsigned cmd;
        linear_ring<T> ring;
        bool exterior = true;
        while ((cmd = path.vertex(&p.x, &p.y)) != SEG_END)
        {
            switch (cmd)
            {
                case SEG_MOVETO:
                case SEG_LINETO:
                    ring.emplace_back(p);
                    break;
                case SEG_CLOSE:
                    if (!ring.empty())
                    {
                        ring.emplace_back(ring.front());
                    }
                    if (exterior)
                    {
                        polygon_.exterior_ring = std::move(ring);
                        exterior = false;
                    }
                    else
                    {
                        polygon_.interior_rings.emplace_back(std::move(ring));
                    }
                    ring = linear_ring<T>();
                    break;
            }
        }
    }

    polygon<T> polygon_;
};

} }

#endif // MAPNIK_GEOMETRY_POLYGON_VERTEX_PROCESSOR_HPP
