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
//$Id: raster_info.cc 17 2005-03-08 23:58:43Z pavlenko $

#include "raster_info.hpp"

raster_info::raster_info(const std::string& file,const std::string& format,const mapnik::Envelope<double>& extent,int srid)
    :file_(file),
     format_(format),
     extent_(extent),
     srid_(srid) {}

raster_info::raster_info(const raster_info& rhs)
    :file_(rhs.file_),
     format_(rhs.format_),
     extent_(rhs.extent_),
     srid_(rhs.srid_) {}

void raster_info::swap(raster_info& other) throw()
{
    file_=other.file_;
    format_=other.format_;
    extent_=other.extent_;
    srid_=other.srid_;
}


raster_info& raster_info::operator=(const raster_info& rhs)
{
    raster_info tmp(rhs);
    swap(tmp);
    return *this;
}


const Envelope<double>& raster_info::envelope() const
{
    return extent_;
}


const std::string& raster_info::file() const
{
    return file_;
}

const std::string& raster_info::format() const
{
    return format_;
}

const int raster_info::srid() const
{
    return srid_;
}
