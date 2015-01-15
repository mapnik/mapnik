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

#ifndef MAPNIK_IMAGE_HPP
#define MAPNIK_IMAGE_HPP

#include <mapnik/image_data_any.hpp>

namespace mapnik {

class MAPNIK_DECL image
{
public:
    image() = default;
    inline std::size_t width() const { return data_.width(); }
    inline std::size_t height() const { return data_.height(); }
    void set_data(image_data_any && data);
    inline image_data_any const& data() const { return data_;}
    static image read_from_file(std::string const& filename);
    void save_to_file(std::string const& filename, std::string const& format);
private:
    image(image && other) = default;
    image(image_data_any && data) noexcept;
    image_data_any data_;
};

}

#endif // MAPNIK_IMAGE_HPP
