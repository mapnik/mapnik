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

#ifndef MAPNIK_POLYGON_PATTERN_SYMBOLIZER_HPP
#define MAPNIK_POLYGON_PATTERN_SYMBOLIZER_HPP

// mapnik
#include <mapnik/symbolizer.hpp>
#include <mapnik/enumeration.hpp>
#include <mapnik/gamma_method.hpp>

namespace mapnik
{

enum pattern_alignment_enum {
    LOCAL_ALIGNMENT,
    GLOBAL_ALIGNMENT,
    pattern_alignment_enum_MAX
};

DEFINE_ENUM( pattern_alignment_e, pattern_alignment_enum );

struct MAPNIK_DECL polygon_pattern_symbolizer :
        public symbolizer_with_image, public symbolizer_base
{
    polygon_pattern_symbolizer(path_expression_ptr file);
    polygon_pattern_symbolizer(polygon_pattern_symbolizer const& rhs);
    pattern_alignment_e get_alignment() const;
    void set_alignment(pattern_alignment_e align);
    void set_gamma(double gamma);
    double get_gamma() const;
    void set_gamma_method(gamma_method_e gamma_method);
    gamma_method_e get_gamma_method() const;
private:
    pattern_alignment_e alignment_;
    double gamma_;
    gamma_method_e gamma_method_;
};
}

#endif // MAPNIK_POLYGON_PATTERN_SYMBOLIZER_HPP
