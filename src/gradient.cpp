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

#include <mapnik/gradient.hpp>
#include <mapnik/enumeration.hpp>

namespace mapnik {

gradient::gradient()
    : transform_()
    , x1_(0)
    , y1_(0)
    , x2_(0)
    , y2_(0)
    , r_(0)
    , stops_()
    , units_(OBJECT_BOUNDING_BOX)
    , gradient_type_(NO_GRADIENT)
{}

gradient::gradient(gradient const& other)
    : transform_(other.transform_)
    , x1_(other.x1_)
    , y1_(other.y1_)
    , x2_(other.x2_)
    , y2_(other.y2_)
    , r_(other.r_)
    , stops_(other.stops_)
    , units_(other.units_)
    , gradient_type_(other.gradient_type_)
{}

gradient::gradient(gradient&& other)
    : transform_(std::move(other.transform_))
    , x1_(std::move(other.x1_))
    , y1_(std::move(other.y1_))
    , x2_(std::move(other.x2_))
    , y2_(std::move(other.y2_))
    , r_(std::move(other.r_))
    , stops_(std::move(other.stops_))
    , units_(std::move(other.units_))
    , gradient_type_(std::move(other.gradient_type_))
{}

gradient& gradient::operator=(gradient rhs)
{
    swap(rhs);
    return *this;
}

bool gradient::operator==(gradient const& other) const
{
    return transform_ == other.transform_ && x1_ == other.x1_ && y1_ == other.y1_ && x2_ == other.x2_ &&
           y2_ == other.y2_ && r_ == other.r_ && std::equal(stops_.begin(), stops_.end(), other.stops_.begin()) &&
           units_ == other.units_ && gradient_type_ == other.gradient_type_;
}

void gradient::set_gradient_type(gradient_e grad)
{
    gradient_type_ = grad;
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

void gradient::add_stop(double offset, mapnik::color const& c)
{
    stops_.push_back(mapnik::stop_pair(offset, c));
}

bool gradient::has_stop() const
{
    return !stops_.empty();
}

stop_array const& gradient::get_stop_array() const
{
    return stops_;
}

void gradient::swap(gradient& other) noexcept
{
    std::swap(gradient_type_, other.gradient_type_);
    std::swap(stops_, other.stops_);
    std::swap(units_, other.units_);
    std::swap(transform_, other.transform_);
    std::swap(x1_, other.x1_);
    std::swap(y1_, other.y1_);
    std::swap(x2_, other.x2_);
    std::swap(y2_, other.y2_);
    std::swap(r_, other.r_);
}

void gradient::set_control_points(double x1, double y1, double x2, double y2, double r)
{
    x1_ = x1;
    y1_ = y1;
    x2_ = x2;
    y2_ = y2;
    r_ = r;
}
void gradient::get_control_points(double& x1, double& y1, double& x2, double& y2, double& r) const
{
    get_control_points(x1, y1, x2, y2);
    r = r_;
}
void gradient::get_control_points(double& x1, double& y1, double& x2, double& y2) const
{
    x1 = x1_;
    y1 = y1_;
    x2 = x2_;
    y2 = y2_;
}

} // namespace mapnik
