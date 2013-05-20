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

typedef factory<image_reader,std::string,
                image_reader* (*)(char const*, std::size_t)>  MemImageReaderFactory;


inline boost::optional<std::string> type_from_bytes(char const* data, size_t size)
{
    typedef boost::optional<std::string> result_type;
    if (size >= 4)
    {
        unsigned int magic = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
        if (magic == 0x89504E47U)
        {
            return result_type("png");
        }
        else if (magic == 0x49492A00 || magic == 0x4D4D002A)
        {
            return result_type("tiff");
        }
    }
    if (size>=2)
    {
        unsigned int magic = ((data[0] << 8) | data[1]) & 0xffff;
        if (magic == 0xffd8)
        {
            return result_type("jpeg");
        }
    }

    return result_type();
}

bool register_image_reader(std::string const& type,image_reader* (* fun)(std::string const&))
{
    return ImageReaderFactory::instance().register_product(type,fun);
}

bool register_image_reader(std::string const& type,image_reader* (* fun)(char const*, std::size_t))
{
    return MemImageReaderFactory::instance().register_product(type,fun);
}

image_reader* get_image_reader(char const* data, size_t size)
{
    boost::optional<std::string> type = type_from_bytes(data,size);
    if (type)
        return MemImageReaderFactory::instance().create_object(*type, data,size);
    return 0;
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
