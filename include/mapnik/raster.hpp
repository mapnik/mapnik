/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/image_any.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/util/variant.hpp>

#include <optional>

namespace mapnik {

class raster : private util::noncopyable
{
  public:
    box2d<double> ext_;
    box2d<double> query_ext_;
    image_any data_;
    double filter_factor_;
    std::optional<double> nodata_;

    template<typename ImageData>
    raster(box2d<double> const& ext, box2d<double> const& query_ext, ImageData&& data, double filter_factor)
        : ext_(ext)
        , query_ext_(query_ext)
        , data_(std::move(data))
        , filter_factor_(filter_factor)
    {}

    template<typename ImageData>
    raster(box2d<double> const& ext, ImageData&& data, double filter_factor)
        : ext_(ext)
        , query_ext_(ext)
        , data_(std::move(data))
        , filter_factor_(filter_factor)
    {}

    void set_nodata(double _nodata) { nodata_ = _nodata; }

    std::optional<double> const& nodata() const { return nodata_; }

    double get_filter_factor() const { return filter_factor_; }

    void set_filter_factor(double factor) { filter_factor_ = factor; }
};
} // namespace mapnik

#endif // MAPNIK_RASTER_HPP
