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

//$Id: polygon_symbolizer.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef POLYGON_SYMBOLIZER_HPP
#define POLYGON_SYMBOLIZER_HPP

#include <mapnik/color.hpp>

namespace mapnik 
{
    struct MAPNIK_DECL polygon_symbolizer
    {
        explicit polygon_symbolizer() 
            : fill_(color(128,128,128)),
              opacity_(1.0),
              gamma_(1.0) {}

        polygon_symbolizer(color const& fill)
            : fill_(fill),
              opacity_(1.0),
              gamma_(1.0) {}
        
        color const& get_fill() const
        {
            return fill_;
        }
        void set_fill(color const& fill)
        {
            fill_ = fill;
        }
        void set_opacity(float opacity)
        {
            opacity_ = opacity;
        }
        float get_opacity() const
        {
            return opacity_;
        }
        void set_gamma(float gamma)
        {
            gamma_ = gamma;
        }
        float get_gamma() const
        {
            return gamma_;
        }

    private:
        color fill_;
        float opacity_;
        float gamma_;
    }; 
   
   struct MAPNIK_DECL building_symbolizer
    {
       explicit building_symbolizer() 
          : fill_(color(128,128,128)),
          height_(0.0),
          opacity_(1.0)
          {}

       building_symbolizer(color const& fill,double height)
          : fill_(fill),
          height_(height),
          opacity_(1.0) {}
       
        color const& get_fill() const
        {
            return fill_;
        }
        void set_fill(color const& fill)
        {
            fill_ = fill;
        }
        double height() const
        {
           return height_;
        }
        void set_height(double height) 
        {
           height_=height;
        }
        void set_opacity(float opacity)
        {
            opacity_ = opacity;
        }
        float get_opacity() const
        {
            return opacity_;
        }
    private:
        color fill_;
        double height_;
        float opacity_;
    };  
}

#endif // POLYGON_SYMBOLIZER_HPP
