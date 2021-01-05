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

#ifndef MAPNIK_IMAGE_UTIL_TIFF_HPP
#define MAPNIK_IMAGE_UTIL_TIFF_HPP

// stl
#include <string>
#include <iostream>

namespace mapnik {

struct tiff_saver
{
    tiff_saver(std::ostream &, std::string const&);
    template <typename T>
    void operator() (T const&) const;
  private:
    std::ostream & stream_;
    std::string const& t_;
};

} // end ns

#endif // MAPNIK_IMAGE_UTIL_TIFF_HPP
