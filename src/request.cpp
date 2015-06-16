/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

// mapnik
#include <mapnik/request.hpp>

namespace mapnik
{

request::request(unsigned width,
                 unsigned height,
                 box2d<double> const& extent)
    : width_(width),
      height_(height),
      extent_(extent),
      buffer_size_(0) {}

request::~request() {}

unsigned request::width() const
{
    return width_;
}

unsigned request::height() const
{
    return height_;
}

void request::set_buffer_size(int buffer_size)
{
    buffer_size_ = buffer_size;
}

int request::buffer_size() const
{
    return buffer_size_;
}

void request::set_extent(box2d<double> const& box)
{
    extent_ = box;
}

box2d<double> const& request::extent() const
{
    return extent_;
}

box2d<double> request::get_buffered_extent() const
{
    double extra = 2.0 * scale() * buffer_size_;
    box2d<double> ext(extent_);
    ext.width(extent_.width() + extra);
    ext.height(extent_.height() + extra);
    return ext;
}

double request::scale() const
{
    if (width_>0)
        return extent_.width()/width_;
    return extent_.width();
}

}
