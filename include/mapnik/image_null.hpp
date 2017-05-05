/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

#ifndef MAPNIK_IMAGE_NULL_HPP
#define MAPNIK_IMAGE_NULL_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/image.hpp>
#include <mapnik/pixel_types.hpp>

//stl
#include <stdexcept>

namespace mapnik
{

template <>
class MAPNIK_DECL image<null_t>
{
public:
    using pixel_type = null_t::type;
    static const image_dtype dtype = null_t::id;
private:
public:
    image() {}
    image(int /*width*/,
          int /*height*/,
          bool /*initialize*/ = true,
          bool /*premultiplied*/ = false,
          bool /*painted*/ = false) {}
    bool operator==(image<null_t> const&) const { return true; }
    bool operator<(image<null_t> const&) const { return false; }

    std::size_t width() const { return 0; }
    std::size_t height() const { return 0; }
    std::size_t size() const { return 0; }
    std::size_t row_size() const { return 0; }
    void set(pixel_type const&) { throw std::runtime_error("Can not set values for null image"); }
    pixel_type& operator() (std::size_t, std::size_t) { throw std::runtime_error("Can not get or set values for null image"); }
    pixel_type const& operator() (std::size_t, std::size_t) const { throw std::runtime_error("Can not get or set values for null image"); }
    unsigned const char* bytes() const { return nullptr; }
    unsigned char* bytes() {return nullptr; }
    double get_offset() const { return 0.0; }
    void set_offset(double) {}
    double get_scaling() const { return 1.0; }
    void set_scaling(double) {}
    bool get_premultiplied() const { return false; }
    void set_premultiplied(bool) {}
    void painted(bool) {}
    bool painted() const { return false; }
    image_dtype get_dtype() const { return dtype; }
};

} // end ns mapnik

#endif // MAPNIK_IMAGE_NULL_HPP
