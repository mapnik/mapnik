/* This file is part of Mapnik (c++ mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko
 *
 * Mapnik is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

//$Id: raster_info.cc 17 2005-03-08 23:58:43Z pavlenko $

#include "raster_info.hh"

RasterInfo::RasterInfo(const std::string& file,const std::string& format,const mapnik::Envelope<double>& extent,int srid)
    :file_(file),
     format_(format),
     extent_(extent),
     srid_(srid) {}

RasterInfo::RasterInfo(const RasterInfo& rhs)
    :file_(rhs.file_),
     format_(rhs.format_),
     extent_(rhs.extent_),
     srid_(rhs.srid_) {}

void RasterInfo::swap(RasterInfo& other) throw()
{
    file_=other.file_;
    format_=other.format_;
    extent_=other.extent_;
    srid_=other.srid_;
}


RasterInfo& RasterInfo::operator=(const RasterInfo& rhs)
{
    RasterInfo tmp(rhs);
    swap(tmp);
    return *this;
}


const Envelope<double>& RasterInfo::envelope() const
{
    return extent_;
}


const std::string& RasterInfo::file() const
{
    return file_;
}

const std::string& RasterInfo::format() const
{
    return format_;
}

const int RasterInfo::srid() const
{
    return srid_;
}
