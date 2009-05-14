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
//$Id$

#include <mapnik/symbolizer.hpp>

#include <mapnik/image_reader.hpp>

#include <iostream>

namespace mapnik {

   symbolizer_with_image::symbolizer_with_image(boost::shared_ptr<ImageData32> img) :
      image_( img ) {}

   symbolizer_with_image::symbolizer_with_image(std::string const& file,
                                                std::string const& type, unsigned width,unsigned height)
      : image_(new ImageData32(width,height)),
        image_filename_( file )
   {
      std::auto_ptr<ImageReader> reader(get_image_reader(file,type));
      if (reader.get())
         reader->read(0,0,*image_);		
   }
   
   symbolizer_with_image::symbolizer_with_image( symbolizer_with_image const& rhs)
      : image_(rhs.image_), image_filename_(rhs.image_filename_) {}
   
   
   boost::shared_ptr<ImageData32> symbolizer_with_image::get_image() const
   {
      return image_;
   }
   void symbolizer_with_image::set_image(boost::shared_ptr<ImageData32> image) 
   {
      image_ = image;
   }
   
   std::string const& symbolizer_with_image::get_filename() const
   {
      return image_filename_;
   }

   void symbolizer_with_image::set_filename(std::string const& image_filename) 
   {
      image_filename_ = image_filename;
   }
      
} // end of namespace mapnik



