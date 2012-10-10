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

#ifndef MAPNIK_BUILDING_SYMBOLIZER_HPP
#define MAPNIK_BUILDING_SYMBOLIZER_HPP

// mapnik
#include <mapnik/color.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/expression.hpp>

namespace mapnik
{

struct MAPNIK_DECL building_symbolizer : public symbolizer_base
{
    building_symbolizer();
    building_symbolizer(color const& fill, expression_ptr const& height);
    color const& get_fill() const;
    void set_fill(color const& fill);
    expression_ptr const& height() const;
    void set_height(expression_ptr const& height);
    void set_opacity(double opacity);
    double get_opacity() const;

private:
    color fill_;
    expression_ptr height_;
    double opacity_;
};

}

#endif // MAPNIK_BUILDING_SYMBOLIZER_HPP
