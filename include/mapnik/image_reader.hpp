/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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

//$Id: image_reader.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef IMAGE_READER_HPP
#define IMAGE_READER_HPP
// mapnik
#include <mapnik/image_data.hpp>
#include <mapnik/config.hpp>
// stl
#include <stdexcept>
#include <string>

namespace mapnik 
{
    class ImageReaderException : public std::exception
    {
    private:
        std::string message_;
    public:
        ImageReaderException(const std::string& message) 
            : message_(message) {}

        ~ImageReaderException() throw() {}

        virtual const char* what() const throw()
        {
            return message_.c_str();
        }
    };

    struct MAPNIK_DECL ImageReader
    {
        virtual unsigned width() const=0;
        virtual unsigned height() const=0;
        virtual void read(unsigned x,unsigned y,ImageData32& image)=0;
        virtual ~ImageReader() {}
    };

   bool register_image_reader(const std::string& type,ImageReader* (*)(const std::string&));
   MAPNIK_DECL ImageReader* get_image_reader(const std::string& file,const std::string& type);
   MAPNIK_DECL ImageReader* get_image_reader(const std::string& file);
   
}

#endif //IMAGE_READER_HPP
