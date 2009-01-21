/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2009 Artem Pavlenko
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

// mapnik
#include <mapnik/color.hpp>
#include <mapnik/color_factory.hpp>
// boost
#include <boost/format.hpp>
// stl
#include <sstream>

namespace mapnik {

   color::color( std::string const& css_string)
   {
      color_factory::init_from_string(*this,css_string.c_str());   
   }

   std::string color::to_string() const
   {
       std::stringstream ss;
       if (alpha() == 255) {
           ss << "rgb("
           << red()   << ","
           << green() << ","
           << blue()  << ")";
       } else {
           ss << "rgba("
           << red()   << ","
           << green() << ","
           << blue()  << ","
           << alpha()/255.0 << ")";
       }
       return ss.str();
   }
   
   std::string color::to_hex_string() const
   {
      return (boost::format("#%1$02x%2$02x%3$02x") 
              % red() 
              % green() 
              % blue() ).str();
   }
}

