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

#include "raster_info.hpp"

raster_info::raster_info(std::string const& file,
                         std::string const& format,
                         mapnik::box2d<double> const& extent,
                         unsigned width,
                         unsigned height)
    : file_(file),
      format_(format),
      extent_(extent),
      width_(width),
      height_(height)
{
}

raster_info::raster_info(const raster_info& rhs)
    : file_(rhs.file_),
      format_(rhs.format_),
      extent_(rhs.extent_),
      width_(rhs.width_),
      height_(rhs.height_)
{
}

void raster_info::swap(raster_info& other) //throw()
{
    std::swap(file_,other.file_);
    std::swap(format_,other.format_);
    std::swap(extent_ ,other.extent_);
    std::swap(width_,other.width_);
    std::swap(height_, other.height_);
}


raster_info& raster_info::operator=(const raster_info& rhs)
{
    raster_info tmp(rhs);
    swap(tmp);
    return *this;
}
