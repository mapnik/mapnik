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

#ifndef MAPNIK_IMAGE_READER_HPP
#define MAPNIK_IMAGE_READER_HPP

// mapnik
#include <mapnik/image_data.hpp>
#include <mapnik/config.hpp>
#include <mapnik/noncopyable.hpp>

// stl
#include <stdexcept>
#include <string>

namespace mapnik
{
class image_reader_exception : public std::exception
{
private:
    std::string message_;
public:
    image_reader_exception(std::string const& message)
        : message_(message) {}

    ~image_reader_exception() throw() {}

    virtual const char* what() const throw()
    {
        return message_.c_str();
    }
};

struct MAPNIK_DECL image_reader : private mapnik::noncopyable
{
    virtual unsigned width() const=0;
    virtual unsigned height() const=0;
    virtual bool premultiplied_alpha() const=0;
    virtual void read(unsigned x,unsigned y,image_data_32& image)=0;
    virtual ~image_reader() {}
};

bool register_image_reader(std::string const& type,image_reader* (*)(std::string const&));
MAPNIK_DECL image_reader* get_image_reader(std::string const& file,std::string const& type);
MAPNIK_DECL image_reader* get_image_reader(std::string const& file);

}

#endif // MAPNIK_IMAGE_READER_HPP
