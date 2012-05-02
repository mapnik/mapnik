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

#ifndef MAPNIK_STROKE_HPP
#define MAPNIK_STROKE_HPP

// mapnik
#include <mapnik/color.hpp>
#include <mapnik/gamma_method.hpp>
#include <mapnik/enumeration.hpp>

// stl
#include <vector>

namespace mapnik
{
using std::pair;
using std::vector;
typedef vector<pair<double,double> > dash_array;

// if you add new tokens, don't forget to add them to the corresponding
// string array in the cpp file.
enum line_cap_enum
{
    BUTT_CAP,
    SQUARE_CAP,
    ROUND_CAP,
    line_cap_enum_MAX
};

DEFINE_ENUM( line_cap_e, line_cap_enum );

// if you add new tokens, don't forget to add them to the corresponding
// string array in the cpp file.
enum line_join_enum
{
    MITER_JOIN,
    MITER_REVERT_JOIN,
    ROUND_JOIN,
    BEVEL_JOIN,
    line_join_enum_MAX
};

DEFINE_ENUM( line_join_e, line_join_enum );

class MAPNIK_DECL stroke
{
    color c_;
    double width_;
    double opacity_; // 0.0 - 1.0
    line_cap_e  line_cap_;
    line_join_e line_join_;
    double gamma_;
    gamma_method_e gamma_method_;
    dash_array dash_;
    double dash_offset_;
    double miterlimit_;
public:
    stroke();
    explicit stroke(color const& c, double width=1.0);
    stroke(stroke const& other);
    stroke& operator=(const stroke& rhs);

    void set_color(const color& c);
    color const& get_color() const;

    double get_width() const;
    void set_width(double w);

    void set_opacity(double opacity);
    double get_opacity() const;

    void set_line_cap(line_cap_e line_cap);
    line_cap_e get_line_cap() const;

    void set_line_join(line_join_e line_join);
    line_join_e get_line_join() const;

    void set_gamma(double gamma);
    double get_gamma() const;

    void set_gamma_method(gamma_method_e gamma_method);
    gamma_method_e get_gamma_method() const;

    void add_dash(double dash,double gap);
    bool has_dash() const;

    void set_dash_offset(double offset);
    double dash_offset() const;

    dash_array const& get_dash_array() const;
    
    void set_miterlimit(double val);
    double get_miterlimit() const;
    
private:
    void swap(const stroke& other) throw();
};
}

#endif // MAPNIK_STROKE_HPP
