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

#ifndef COMPARE_IMAGES_HPP
#define COMPARE_IMAGES_HPP

// stl
#include <memory>

// mapnik
#include <mapnik/image_util.hpp>
#include <mapnik/image_reader.hpp>

namespace visual_tests
{

template <typename Image>
unsigned compare_images(Image const & actual, std::string const & reference)
{
    std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(reference, "png"));
    if (!reader.get())
    {
        throw mapnik::image_reader_exception("Failed to load: " + reference);
    }

    mapnik::image_any ref_image_any = reader->read(0, 0, reader->width(), reader->height());
    Image const & reference_image = mapnik::util::get<Image>(ref_image_any);

    return mapnik::compare(actual, reference_image, 0, true);
}

}

#endif
