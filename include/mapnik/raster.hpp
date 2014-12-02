/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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
#include <mapnik/util/variant.hpp>
 // boost
#include <boost/optional.hpp>

namespace mapnik {

using image_data_base = util::variant<image_data_32, image_data_8, image_data_16, image_data_float32>;

namespace detail {

struct get_bytes_visitor : util::static_visitor<unsigned char*>
{
    template <typename T>
    unsigned char* operator()(T & data)
    {
        return data.getBytes();
    }
};

struct get_bytes_visitor_const : util::static_visitor<unsigned char const*>
{
    template <typename T>
    unsigned char const* operator()(T const& data) const
    {
        return data.getBytes();
    }
};

struct get_width_visitor : util::static_visitor<std::size_t>
{
    template <typename T>
    std::size_t operator()(T const& data) const
    {
        return data.width();
    }
};

struct get_height_visitor : util::static_visitor<std::size_t>
{
    template <typename T>
    std::size_t operator()(T const& data) const
    {
        return data.height();
    }
};


} // namespace detail

struct image_data_any : image_data_base
{
    template <typename T>
    image_data_any(T && data) noexcept
        : image_data_base(std::move(data)) {}

    unsigned char const* getBytes() const
    {
        return util::apply_visitor(detail::get_bytes_visitor_const(),*this);
    }

    unsigned char* getBytes()
    {
        return util::apply_visitor(detail::get_bytes_visitor(),*this);
    }

    std::size_t width() const
    {
        return util::apply_visitor(detail::get_width_visitor(),*this);
    }

    std::size_t height() const
    {
        return util::apply_visitor(detail::get_height_visitor(),*this);
    }
};


class raster : private mapnik::noncopyable
{
public:
    box2d<double> ext_;
    image_data_any data_;
    double filter_factor_;
    bool premultiplied_alpha_;
    boost::optional<double> nodata_;

    template <typename ImageData>
    raster(box2d<double> const& ext,
           ImageData && data,
           double filter_factor,
           bool premultiplied_alpha = false)
        : ext_(ext),
          data_(std::move(data)),
          filter_factor_(filter_factor),
          premultiplied_alpha_(premultiplied_alpha) {}

    raster(box2d<double> const& ext, image_data_32 && data,
           double filter_factor, bool premultiplied_alpha = false)
        : ext_(ext),
          data_(std::move(data)),
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
