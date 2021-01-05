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

#ifndef MAPNIK_PATTERN_ALIGNMENT_HPP
#define MAPNIK_PATTERN_ALIGNMENT_HPP

#include <mapnik/coord.hpp>

namespace mapnik {

struct symbolizer_base;
class feature_impl;
class proj_transform;
struct renderer_common;

coord<double, 2> pattern_offset(
    symbolizer_base const & sym,
    feature_impl const & feature,
    proj_transform const & prj_trans,
    renderer_common const & common,
    unsigned pattern_width,
    unsigned pattern_height);

}

#endif // MAPNIK_PATTERN_ALIGNMENT_HPP
