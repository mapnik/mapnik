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

#ifndef MAPNIK_POINT_SYMBOLIZER_HPP
#define MAPNIK_POINT_SYMBOLIZER_HPP

// mapnik
#include <mapnik/symbolizer.hpp>
#include <mapnik/enumeration.hpp>

namespace mapnik
{

enum point_placement_enum {
    CENTROID_POINT_PLACEMENT,
    INTERIOR_POINT_PLACEMENT,
    point_placement_enum_MAX
};

DEFINE_ENUM( point_placement_e, point_placement_enum );

struct MAPNIK_DECL point_symbolizer :
        public symbolizer_with_image, public symbolizer_base
{
    point_symbolizer();
    point_symbolizer(path_expression_ptr file);
    point_symbolizer(point_symbolizer const& rhs);
    void set_allow_overlap(bool overlap);
    bool get_allow_overlap() const;
    void set_point_placement(point_placement_e point_p);
    point_placement_e get_point_placement() const;
    void set_ignore_placement(bool ignore_placement);
    bool get_ignore_placement() const;

private:
    bool overlap_;
    point_placement_e point_p_;
    bool ignore_placement_;
};
}

#endif // MAPNIK_POINT_SYMBOLIZER_HPP
