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
//$Id: raster_info.hh 17 2005-03-08 23:58:43Z pavlenko $

#ifndef RASTER_INFO_HPP
#define RASTER_INFO_HPP

#include "raster_datasource.hpp"
#include <string>

using mapnik::Envelope;

class raster_info
{
   std::string file_;
   std::string format_;
   Envelope<double> extent_;
   unsigned width_;
   unsigned height_;
public:
   raster_info(const std::string& file,const std::string& format, const Envelope<double>& extent, unsigned width, unsigned height);
   raster_info(const raster_info& rhs);
   raster_info& operator=(const raster_info& rhs);
   inline Envelope<double> const& envelope() const {return extent_;}
   inline std::string const& file() const { return file_;}
   inline std::string const& format() const {return format_;}
   inline unsigned width() const { return width_;}
   inline unsigned height() const { return height_;}
   
private:
   void swap(raster_info& other) throw();
};

#endif  //RASTER_INFO_HPP
