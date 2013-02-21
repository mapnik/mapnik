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

// mapnik
#include <mapnik/request.hpp>

namespace mapnik
{


request::request(int width,int height)
    : width_(width),
      height_(height),
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

void request::set_width(unsigned width)
{
    if (width != width_ &&
        width >= MIN_MAPSIZE &&
        width <= MAX_MAPSIZE)
    {
        width_=width;
    }
}

void request::set_height(unsigned height)
{
    if (height != height_ &&
        height >= MIN_MAPSIZE &&
        height <= MAX_MAPSIZE)
    {
        height_=height;
    }
}

void request::resize(unsigned width,unsigned height)
{
    if (width != width_ &&
        height != height_ &&
        width >= MIN_MAPSIZE &&
        width <= MAX_MAPSIZE &&
        height >= MIN_MAPSIZE &&
        height <= MAX_MAPSIZE)
    {
        width_=width;
        height_=height;
    }
}

void request::set_buffer_size( int buffer_size)
{
    buffer_size_ = buffer_size;
}

int request::buffer_size() const
{
    return buffer_size_;
}

void request::set_maximum_extent(box2d<double> const& box)
{
    maximum_extent_.reset(box);
}

boost::optional<box2d<double> > const& request::maximum_extent() const
{
    return maximum_extent_;
}

void request::reset_maximum_extent()
{
    maximum_extent_.reset();
}

void request::zoom_to_box(const box2d<double> &box)
{
    current_extent_=box;
}

const box2d<double>& request::get_current_extent() const
{
    return current_extent_;
}

box2d<double> request::get_buffered_extent() const
{
    double extra = 2.0 * scale() * buffer_size_;
    box2d<double> ext(current_extent_);
    ext.width(current_extent_.width() + extra);
    ext.height(current_extent_.height() + extra);
    return ext;
}

double request::scale() const
{
    if (width_>0)
        return current_extent_.width()/width_;
    return current_extent_.width();
}

}
