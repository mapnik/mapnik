/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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
#ifndef MAPNIK_PIXEL_POSITION_HPP
#define MAPNIK_PIXEL_POSITION_HPP

// stl
#include <cmath>

namespace mapnik {

struct rotation
{
    rotation()
        : sin(0)
        , cos(1.)
    {}
    rotation(double sin_, double cos_)
        : sin(sin_)
        , cos(cos_)
    {}
    rotation(double angle)
        : sin(std::sin(angle))
        , cos(std::cos(angle))
    {}
    void reset()
    {
        sin = 0.;
        cos = 1.;
    }
    void init(double angle)
    {
        sin = std::sin(angle);
        cos = std::cos(angle);
    }
    double sin;
    double cos;
    rotation operator~() const { return rotation(sin, -cos); }
    rotation operator!() const { return rotation(-sin, cos); }

    double angle() const { return std::atan2(sin, cos); }
};

struct pixel_position
{
    double x;
    double y;
    pixel_position(double x_, double y_)
        : x(x_)
        , y(y_)
    {}
    pixel_position()
        : x(0)
        , y(0)
    {}
    pixel_position operator+(pixel_position const& other) const { return pixel_position(x + other.x, y + other.y); }

    pixel_position operator-(pixel_position const& other) const { return pixel_position(x - other.x, y - other.y); }

    pixel_position operator*(double other) const { return pixel_position(x * other, y * other); }

    void set(double x_, double y_)
    {
        x = x_;
        y = y_;
    }

    void clear()
    {
        x = 0;
        y = 0;
    }

    pixel_position rotate(rotation const& rot) const;

    pixel_position operator~() const { return pixel_position(x, -y); }

    double length() { return std::sqrt(x * x + y * y); }
};

inline pixel_position pixel_position::rotate(rotation const& rot) const
{
    return pixel_position(x * rot.cos - y * rot.sin, x * rot.sin + y * rot.cos);
}

inline pixel_position operator*(double factor, pixel_position const& pos)
{
    return pixel_position(factor * pos.x, factor * pos.y);
}

} // namespace mapnik

#endif // MAPNIK_PIXEL_POSITION_HPP
