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

#ifndef MAPNIK_PROJ_TRANSFORM_HPP
#define MAPNIK_PROJ_TRANSFORM_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/geometry/point.hpp>
#include <mapnik/projection.hpp>
// stl
#include <vector>

namespace mapnik {

template<typename T>
class box2d;

class MAPNIK_DECL proj_transform : private util::noncopyable
{
  public:
    proj_transform(projection const& source, projection const& dest);
    ~proj_transform();
    bool equal() const;
    bool is_known() const;
    bool forward(double& x, double& y, double& z) const;
    bool backward(double& x, double& y, double& z) const;
    bool forward(double* x, double* y, double* z, std::size_t point_count, std::size_t offset = 1) const;
    bool backward(double* x, double* y, double* z, std::size_t point_count, std::size_t offset = 1) const;
    bool forward(geometry::point<double>& p) const;
    bool backward(geometry::point<double>& p) const;
    unsigned int forward(std::vector<geometry::point<double>>& ls) const;
    unsigned int backward(std::vector<geometry::point<double>>& ls) const;
    bool forward(box2d<double>& box) const;
    bool backward(box2d<double>& box) const;
    bool forward(box2d<double>& box, std::size_t points) const;
    bool backward(box2d<double>& box, std::size_t points) const;
    std::string definition() const;

  private:
    PJ_CONTEXT* ctx_ = nullptr;
    PJ* transform_ = nullptr;
    bool is_source_longlat_;
    bool is_dest_longlat_;
    bool is_source_equal_dest_;
    bool wgs84_to_merc_;
    bool merc_to_wgs84_;
};

} // namespace mapnik

#endif // MAPNIK_PROJ_TRANSFORM_HPP
