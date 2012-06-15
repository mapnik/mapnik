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

#include <mapnik/stroke.hpp>

namespace mapnik
{

static const char * line_cap_strings[] = {
    "butt",
    "square",
    "round",
    ""
};


IMPLEMENT_ENUM( line_cap_e, line_cap_strings )


static const char * line_join_strings[] = {
    "miter",
    "miter_revert",
    "round",
    "bevel",
    ""
};

IMPLEMENT_ENUM( line_join_e, line_join_strings )

stroke::stroke()
: c_(0,0,0),
    width_(1.0),
    opacity_(1.0),
    line_cap_(BUTT_CAP),
    line_join_(MITER_JOIN),
    gamma_(1.0),
    gamma_method_(GAMMA_POWER),
    dash_(),
    dash_offset_(0),
    miterlimit_(4.0) {}

stroke::stroke(color const& c, double width)
    : c_(c),
      width_(width),
      opacity_(1.0),
      line_cap_(BUTT_CAP),
      line_join_(MITER_JOIN),
      gamma_(1.0),
      gamma_method_(GAMMA_POWER),
      dash_(),
      dash_offset_(0.0),
      miterlimit_(4.0) {}

stroke::stroke(stroke const& other)
    : c_(other.c_),
      width_(other.width_),
      opacity_(other.opacity_),
      line_cap_(other.line_cap_),
      line_join_(other.line_join_),
      gamma_(other.gamma_),
      gamma_method_(other.gamma_method_),
      dash_(other.dash_),
      dash_offset_(other.dash_offset_),
      miterlimit_(other.miterlimit_) {}

stroke & stroke::operator=(const stroke& rhs)
{
    stroke tmp(rhs);
    swap(tmp);
    return *this;
}

void stroke::set_color(const color& c)
{
    c_=c;
}

color const& stroke::get_color() const
{
    return c_;
}

double stroke::get_width() const
{
    return width_;
}
void stroke::set_width(double w)
{
    width_=w;
}

void stroke::set_opacity(double opacity)
{
    if (opacity > 1.0) opacity_=1.0;
    else if (opacity < 0.0) opacity_=0.0;
    else opacity_=opacity;
}

double stroke::get_opacity() const
{
    return opacity_;
}

void stroke::set_line_cap(line_cap_e line_cap)
{
    line_cap_=line_cap;
}

line_cap_e stroke::get_line_cap() const
{
    return line_cap_;
}

void stroke::set_line_join(line_join_e line_join)
{
    line_join_=line_join;
}

line_join_e stroke::get_line_join() const
{
    return line_join_;
}

void stroke::set_gamma(double gamma)
{
    gamma_ = gamma;
}

double stroke::get_gamma() const
{
    return gamma_;
}

void stroke::set_gamma_method(gamma_method_e gamma_method)
{
    gamma_method_ = gamma_method;
}

gamma_method_e stroke::get_gamma_method() const
{
    return gamma_method_;
}

void stroke::add_dash(double dash, double gap)
{
    dash_.push_back(std::make_pair(dash,gap));
}

bool stroke::has_dash() const
{
    return ! dash_.empty();
}

void stroke::set_dash_offset(double offset)
{
    dash_offset_ = offset;
}

double stroke::dash_offset() const
{
    return dash_offset_;
}

dash_array const& stroke::get_dash_array() const
{
    return dash_;
}

void stroke::set_miterlimit(double val)
{
    miterlimit_ = val;
}

double stroke::get_miterlimit() const
{
    return miterlimit_;
}

void stroke::swap(const stroke& other) throw()
{
    c_ = other.c_;
    width_ = other.width_;
    opacity_ = other.opacity_;
    line_cap_ = other.line_cap_;
    line_join_ = other.line_join_;
    gamma_ = other.gamma_;
    gamma_method_ = other.gamma_method_;
    dash_ = other.dash_;
    dash_offset_ = other.dash_offset_;
    miterlimit_ = other.miterlimit_;
}
}
