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

#ifndef MAPNIK_POLYGON_SYMBOLIZER_HPP
#define MAPNIK_POLYGON_SYMBOLIZER_HPP

// mapnik
#include <mapnik/color.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/filter_factory.hpp>
#include <mapnik/enumeration.hpp>

// stl
#include <string>

namespace mapnik 
{

enum polygon_gamma_method_enum {
    POLYGON_GAMMA_POWER, //agg::gamma_power
    POLYGON_GAMMA_LINEAR, //agg::gamma_linear
    POLYGON_GAMMA_NONE, //agg::gamma_none
    POLYGON_GAMMA_THRESHOLD, //agg::gamma_threshold
    POLYGON_GAMMA_MULTIPLY, //agg::gamma_multiply
    polygon_gamma_method_enum_MAX
};

DEFINE_ENUM( polygon_gamma_method_e, polygon_gamma_method_enum );

struct MAPNIK_DECL polygon_symbolizer : public symbolizer_base
{
    explicit polygon_symbolizer() 
        : symbolizer_base(),
        fill_(color(128,128,128)),
        opacity_(1.0),
        gamma_(1.0),
        gamma_method_(POLYGON_GAMMA_POWER) {}

    polygon_symbolizer(color const& fill)
        : symbolizer_base(),
        fill_(fill),
        opacity_(1.0),
        gamma_(1.0),
        gamma_method_(POLYGON_GAMMA_POWER) {}
        
    color const& get_fill() const
    {
        return fill_;
    }
    void set_fill(color const& fill)
    {
        fill_ = fill;
    }
    void set_opacity(double opacity)
    {
        opacity_ = opacity;
    }
    double get_opacity() const
    {
        return opacity_;
    }
    void set_gamma(double gamma)
    {
        gamma_ = gamma;
    }
    double get_gamma() const
    {
        return gamma_;
    }
    void set_gamma_method(polygon_gamma_method_e gamma_method)
    {
        gamma_method_ = gamma_method;
    }
    polygon_gamma_method_e get_gamma_method() const
    {
        return gamma_method_;
    }

private:
    color fill_;
    double opacity_;
    double gamma_;
    polygon_gamma_method_e gamma_method_;
}; 
   
struct MAPNIK_DECL building_symbolizer : public symbolizer_base
{
    explicit building_symbolizer() 
        : symbolizer_base(),
        fill_(color(128,128,128)),
        opacity_(1.0)
        {}
       
    building_symbolizer(color const& fill, expression_ptr height)
        : symbolizer_base(),
        fill_(fill),
        height_(height),
        opacity_(1.0) {}
       
    color const& get_fill() const
    {
        return fill_;
    }
    void set_fill(color const& fill)
    {
        fill_ = fill;
    }
    expression_ptr height() const
    {
        return height_;
    }
    void set_height(expression_ptr height)
    {
        height_=height;
    }
    void set_opacity(double opacity)
    {
        opacity_ = opacity;
    }
    double get_opacity() const
    {
        return opacity_;
    }
private:
    color fill_;
    expression_ptr height_;
    double opacity_;
};  
}

#endif // MAPNIK_POLYGON_SYMBOLIZER_HPP
