/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

#include <mapnik/gradient.hpp>
#include <mapnik/enumeration.hpp>

namespace mapnik
{

static const char * gradient_strings[] = {
    "no-gradient",
    "linear",
    "radial",
    ""
};

IMPLEMENT_ENUM( gradient_e, gradient_strings )

static const char * gradient_unit_strings[] = {
    "user-space-on-use",
    "user-space-on-use-bounding-box",
    "object-bounding-box",
    ""
};

IMPLEMENT_ENUM( gradient_unit_e, gradient_unit_strings )

gradient::gradient()
: transform_(),
  x1_(0),
  y1_(0),
  x2_(0),
  y2_(0),
  r_(0),
  stops_(),
  units_(OBJECT_BOUNDING_BOX),
  gradient_type_(NO_GRADIENT) {}

gradient::gradient(gradient const& other)
    : transform_(other.transform_),
      x1_(other.x1_),
      y1_(other.y1_),
      x2_(other.x2_),
      y2_(other.y2_),
      r_(other.r_),
      stops_(other.stops_),
      units_(other.units_),
      gradient_type_(other.gradient_type_) {}

gradient & gradient::operator=(const gradient& rhs)
{
    gradient tmp(rhs);
    swap(tmp);
    return *this;
}

void gradient::set_gradient_type(gradient_e grad)
{
    gradient_type_=grad;
}

gradient_e gradient::get_gradient_type() const
{
    return gradient_type_;
}

void gradient::set_transform(agg::trans_affine const& transform)
{
    transform_ = transform;
}
agg::trans_affine const& gradient::get_transform() const
{
    return transform_;
}

void gradient::set_units(gradient_unit_e units)
{
    units_ = units;
}
gradient_unit_e gradient::get_units() const
{
    return units_;
}

void gradient::add_stop(double offset,mapnik::color const& c)
{
    stops_.push_back(mapnik::stop_pair(offset,c));
}

bool gradient::has_stop() const
{
    return ! stops_.empty();
}

stop_array const& gradient::get_stop_array() const
{
    return stops_;
}

void gradient::swap(const gradient& other) throw()
{
    gradient_type_=other.gradient_type_;
    stops_=other.stops_;
    units_=other.units_;
    transform_=other.transform_;
    other.get_control_points(x1_,y1_,x2_,y2_,r_);
}

void gradient::set_control_points(double x1, double y1, double x2, double y2, double r)
{
    x1_ = x1;
    y1_ = y1;
    x2_ = x2;
    y2_ = y2;
    r_ = r;
}
void gradient::get_control_points(double &x1, double &y1, double &x2, double &y2, double &r) const
{
    get_control_points(x1,y1,x2,y2);
    r=r_;
}
void gradient::get_control_points(double &x1, double &y1, double &x2, double &y2) const
{
    x1 = x1_;
    y1 = y1_;
    x2 = x2_;
    y2 = y2_;
}

}
