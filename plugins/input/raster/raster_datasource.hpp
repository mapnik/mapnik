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
//$Id: raster_datasource.hh 44 2005-04-22 18:53:54Z pavlenko $

#ifndef RASTER_DATASOURCE_HPP
#define RASTER_DATASOURCE_HPP

// mapnik
#include <mapnik/box2d.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/datasource.hpp>

class raster_datasource : public mapnik::datasource
{
    private:
       mapnik::layer_descriptor     desc_;
       std::string                  filename_;
       std::string                  format_;
       mapnik::box2d<double>        extent_;
       bool                         extent_initialized_;
       bool                         multi_tiles_;
       unsigned                     tile_size_;
       unsigned                     tile_stride_;
       mutable unsigned             width_;
       mutable unsigned             height_;
    public:
       raster_datasource(const mapnik::parameters& params, bool bind=true);
       virtual ~raster_datasource();
       int type() const;
       static std::string name();
       mapnik::featureset_ptr features(const mapnik::query& q) const;
       mapnik::featureset_ptr features_at_point(mapnik::coord2d const& pt) const;
       mapnik::box2d<double> envelope() const;
       mapnik::layer_descriptor get_descriptor() const;
       void bind() const;
    private:
       //no copying
       raster_datasource(const raster_datasource&);
       raster_datasource& operator=(const raster_datasource&);
};

#endif //RASTER_DATASOURCE_HPP
