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
//$Id: symbolizer.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef SYMBOLIZER_HPP
#define SYMBOLIZER_HPP

#include <mapnik/graphics.hpp> 
#include <boost/shared_ptr.hpp>

namespace mapnik 
{

    class MAPNIK_DECL symbolizer_with_image {
        public:
            boost::shared_ptr<ImageData32> get_image() const;
            const std::string & get_filename() const;
            void set_filename(std::string const& image_filename);
            void set_image( boost::shared_ptr<ImageData32> symbol);
            
        protected:
            symbolizer_with_image(boost::shared_ptr<ImageData32> img);
            symbolizer_with_image(std::string const& file,
                                  std::string const& type,
                                  unsigned width,unsigned height);
            
            symbolizer_with_image(symbolizer_with_image const& rhs);
        
            boost::shared_ptr<ImageData32> image_;
            std::string image_filename_;

    };
}

#endif //SYMBOLIZER_HPP
