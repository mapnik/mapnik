/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
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

#ifndef MAPNIK_REQUEST_HPP
#define MAPNIK_REQUEST_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/geometry/box2d.hpp>

namespace mapnik {

class MAPNIK_DECL request
{
  public:
    request(unsigned width, unsigned height, box2d<double> const& extent);
    unsigned width() const;
    unsigned height() const;
    void set_buffer_size(int buffer_size);
    int buffer_size() const;
    box2d<double> const& extent() const;
    void set_extent(box2d<double> const& box);
    box2d<double> get_buffered_extent() const;
    double scale() const;
    ~request();

  private:
    unsigned width_;
    unsigned height_;
    box2d<double> extent_;
    int buffer_size_;
};

} // namespace mapnik

#endif // MAPNIK_REQUEST_HPP
