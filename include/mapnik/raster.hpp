/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#ifndef MAPNIK_RASTER_HPP
#define MAPNIK_RASTER_HPP

// mapnik
#include <mapnik/box2d.hpp>
#include <mapnik/image_data.hpp>
#include <mapnik/noncopyable.hpp>

 // boost
#include <boost/optional.hpp>

namespace mapnik {
class raster : private mapnik::noncopyable
{
public:
    box2d<double> ext_;
    image_data_32 data_;
    double filter_factor_;
    bool premultiplied_alpha_;
    boost::optional<double> nodata_;
    raster(box2d<double> const& ext,
           unsigned width,
           unsigned height,
           double filter_factor,
           bool premultiplied_alpha = false)
        : ext_(ext),
          data_(width,height),
          filter_factor_(filter_factor),
          premultiplied_alpha_(premultiplied_alpha) {}

    void set_nodata(double nodata)
    {
        nodata_ = nodata;
    }

    boost::optional<double> const& nodata() const
    {
        return nodata_;
    }

    double get_filter_factor() const
    {
        return filter_factor_;
    }

    void set_filter_factor(double factor)
    {
        filter_factor_ = factor;
    }

};
}

#endif // MAPNIK_RASTER_HPP
