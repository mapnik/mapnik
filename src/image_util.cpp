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

//$Id: image_util.cpp 36 2005-04-05 14:32:18Z pavlenko $

extern "C"
{
#include <png.h>
}

// mapnik
#include <mapnik/image_util.hpp>
#include <mapnik/png_io.hpp>
#include <mapnik/jpeg_io.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/memory.hpp>
#include <mapnik/image_view.hpp>

// stl
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

namespace mapnik
{    
    template <typename T>
    std::string save_to_string(T const& image,
                               std::string const& type)
    {
        std::ostringstream ss(std::ios::out|std::ios::binary);
        //all this should go into image_writer factory
        if (type=="png")  save_as_png(ss,image);
        else if (type == "png256") save_as_png256(ss,image);
        else if (boost::algorithm::istarts_with(type,std::string("jpeg")))
        {
            int quality = 85;
            try 
            {
                if(type.substr(4).length() != 0)
                {
                    quality = boost::lexical_cast<int>(type.substr(4));
                    if(quality<1 || quality>100)
                        throw ImageWriterException("invalid jpeg quality: " + type.substr(4));
                }
                save_as_jpeg(ss,quality,image); 
            } 
            catch(boost::bad_lexical_cast &)
            {
                throw ImageWriterException("invalid jpeg quality: " + type.substr(4));
            }
        }
        else throw ImageWriterException("unknown file type: " + type);
        return ss.str();
    }

    template <typename T>
    void save_to_file(T const& image,
                      std::string const& filename,
                      std::string const& type)
    {
        std::ofstream file (filename.c_str(), std::ios::out| std::ios::trunc|std::ios::binary);
        if (file)
        {
            //all this should go into image_writer factory
            if (type=="png")  save_as_png(file,image);
            else if (type == "png256") save_as_png256(file,image);
            else if (boost::algorithm::istarts_with(type,std::string("jpeg")))
            {
                int quality = 85;
                try 
                {
                    if(type.substr(4).length() != 0)
                    {
                        quality = boost::lexical_cast<int>(type.substr(4));
                        if(quality<0 || quality>100)
                            throw ImageWriterException("invalid jpeg quality: " + type.substr(4) + " out of bounds");
                    }
                    save_as_jpeg(file,quality,image); 
                } 
                catch(boost::bad_lexical_cast &)
                {
                    throw ImageWriterException("invalid jpeg quality: " + type.substr(4) + " not a number");
                }
            }
            else throw ImageWriterException("unknown file type: " + type);
        } 
    }
	
    template <typename T>
    void save_to_file(T const& image,std::string const& filename)
    {
        std::string type = type_from_filename(filename);
        save_to_file<T>(image,filename,type);
    }
     

    template void save_to_file<ImageData32>(ImageData32 const&,
                                            std::string const&,
                                            std::string const&);

    template void save_to_file<ImageData32>(ImageData32 const&,
                                            std::string const&);

    template std::string save_to_string<ImageData32>(ImageData32 const&,
                                                     std::string const&);

    template void save_to_file<image_view<ImageData32> > (image_view<ImageData32> const&,
                                                          std::string const&,
                                                          std::string const&);
   
    template void save_to_file<image_view<ImageData32> > (image_view<ImageData32> const&,
                                                          std::string const&);
   
    template std::string save_to_string<image_view<ImageData32> > (image_view<ImageData32> const&,
                                                                   std::string const&);

}
