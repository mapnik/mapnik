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
#include <mapnik/image_reader.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/factory.hpp>

namespace mapnik
{
typedef factory<image_reader,std::string,
                image_reader* (*)(std::string const&)>  ImageReaderFactory;


bool register_image_reader(std::string const& type,image_reader* (* fun)(std::string const&))
{
    return ImageReaderFactory::instance().register_product(type,fun);
}

image_reader* get_image_reader(std::string const& filename,std::string const& type)
{
    return ImageReaderFactory::instance().create_object(type,filename);
}

image_reader* get_image_reader(std::string const& filename)
{
    boost::optional<std::string> type = type_from_filename(filename);
    if (type)
    {
        return ImageReaderFactory::instance().create_object(*type,filename);
    }
    return 0;
}

}
