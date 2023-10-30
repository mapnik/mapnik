/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

#ifndef MAPNIK_MARKER_HPP
#define MAPNIK_MARKER_HPP

// mapnik
#include <mapnik/image.hpp>
#include <mapnik/svg/svg_storage.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/util/variant.hpp>

// stl
#include <deque>
#include <memory>

namespace mapnik {

struct image_any;

using svg::svg_path_adapter;
using svg_attribute_type = std::deque<svg::path_attributes>;
using svg_storage_type = svg::svg_storage<svg::svg_path_storage, svg_attribute_type>;
using svg_path_ptr = std::shared_ptr<svg_storage_type>;
using image_ptr = std::shared_ptr<image_any>;

struct marker_rgba8
{
  public:
    marker_rgba8()
        : bitmap_data_(4, 4, true, true)
    {
        // create default OGC 4x4 black pixel
        bitmap_data_.set(0xff000000);
    }

    explicit marker_rgba8(image_rgba8 const& data)
        : bitmap_data_(data)
    {}

    explicit marker_rgba8(image_rgba8&& data) noexcept
        : bitmap_data_(std::move(data))
    {}

    box2d<double> bounding_box() const
    {
        std::size_t _width = bitmap_data_.width();
        std::size_t _height = bitmap_data_.height();
        return box2d<double>(static_cast<double>(0),
                             static_cast<double>(0),
                             static_cast<double>(_width),
                             static_cast<double>(_height));
    }

    inline double width() const { return static_cast<double>(bitmap_data_.width()); }

    inline double height() const { return static_cast<double>(bitmap_data_.height()); }

    image_rgba8 const& get_data() const { return bitmap_data_; }

  private:
    image_rgba8 bitmap_data_;
};

struct marker_svg
{
  public:
    marker_svg() = default;

    explicit marker_svg(mapnik::svg_path_ptr data) noexcept
        : vector_data_(data)
    {}

    inline box2d<double> bounding_box() const { return vector_data_->bounding_box(); }

    inline double width() const { return vector_data_->bounding_box().width(); }
    inline double height() const { return vector_data_->bounding_box().height(); }

    inline mapnik::svg_path_ptr get_data() const { return vector_data_; }

    inline std::tuple<double, double> dimensions() const
    {
        return std::make_tuple(vector_data_->width(), vector_data_->height());
    }

  private:
    mapnik::svg_path_ptr vector_data_;
};

struct marker_null
{
  public:
    inline box2d<double> bounding_box() const { return box2d<double>(); }
    inline double width() const { return 0; }
    inline double height() const { return 0; }
};

using marker_base = util::variant<marker_null, marker_rgba8, marker_svg>;
namespace detail {

struct get_marker_bbox_visitor
{
    template<typename T>
    box2d<double> operator()(T& data) const
    {
        return data.bounding_box();
    }
};

struct get_marker_width_visitor
{
    template<typename T>
    double operator()(T const& data) const
    {
        return data.width();
    }
};

struct get_marker_height_visitor
{
    template<typename T>
    double operator()(T const& data) const
    {
        return data.height();
    }
};

} // namespace detail

struct marker : marker_base
{
    marker() = default;

    template<typename T>
    marker(T&& _data) noexcept(std::is_nothrow_constructible<marker_base, T&&>::value)
        : marker_base(std::forward<T>(_data))
    {}

    double width() const { return util::apply_visitor(detail::get_marker_width_visitor(), *this); }

    double height() const { return util::apply_visitor(detail::get_marker_height_visitor(), *this); }

    box2d<double> bounding_box() const { return util::apply_visitor(detail::get_marker_bbox_visitor(), *this); }
};

} // namespace mapnik

#endif // MAPNIK_MARKER_HPP
