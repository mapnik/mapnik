/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

#ifndef MAPNIK_GRID_VIEW_HPP
#define MAPNIK_GRID_VIEW_HPP

#include <mapnik/image.hpp>
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/global.hpp>
#include <mapnik/value.hpp>
#include <mapnik/feature.hpp>

// stl
#include <cstdint>
#include <map>
#include <set>
#include <cmath>
#include <string>
#include <vector>

namespace mapnik {

template <typename T>
class hit_grid_view
{
public:
    using data_type = T;
    using value_type = typename T::pixel_type;
    using pixel_type = typename T::pixel_type;
    using lookup_type = std::string;
    using feature_key_type = std::map<value_type, lookup_type>;
    using feature_type = std::map<std::string, mapnik::feature_ptr>;

    hit_grid_view(unsigned x, unsigned y,
                  unsigned width, unsigned height,
                  T const& _data,
                  std::string const& key,
                  std::string const& id_name,
                  std::set<std::string> const& names,
                  feature_key_type const& f_keys,
                  feature_type const& features
        )
        : x_(x),
          y_(y),
          width_(width),
          height_(height),
          data_(_data),
          key_(key),
          id_name_(id_name),
          names_(names),
          f_keys_(f_keys),
          features_(features)

    {
        if (x_ >= data_.width()) x_=data_.width()-1;
        if (y_ >= data_.height()) y_=data_.height()-1;
        if (x_ + width_ > data_.width()) width_= data_.width() - x_;
        if (y_ + height_ > data_.height()) height_= data_.height() - y_;
    }

    ~hit_grid_view() {}

    hit_grid_view(hit_grid_view<T> const& rhs)
        : x_(rhs.x_),
          y_(rhs.y_),
          width_(rhs.width_),
          height_(rhs.height_),
          data_(rhs.data_),
          key_(rhs.key_),
          id_name_(rhs.id_name_),
          names_(rhs.names_),
          f_keys_(rhs.f_keys_),
          features_(rhs.features_)
    {}

    hit_grid_view<T> & operator=(hit_grid_view<T> const& rhs)
    {
        if (&rhs==this) return *this;
        x_ = rhs.x_;
        y_ = rhs.y_;
        width_ = rhs.width_;
        height_ = rhs.height_;
        data_ = rhs.data_;
        key_ = rhs.key_;
        id_name_ = rhs.id_name_;
        names_ = rhs.names_;
        f_keys_ = rhs.f_keys_;
        features_ = rhs.features_;
        return *this;
    }

    inline unsigned x() const
    {
        return x_;
    }

    inline unsigned y() const
    {
        return y_;
    }

    inline unsigned width() const
    {
        return width_;
    }

    inline unsigned height() const
    {
        return height_;
    }

    inline std::string const& key_name() const
    {
        return id_name_;
    }

    inline value_type const * get_row(unsigned row) const
    {
        return data_.get_row(row + y_) + x_;
    }

    inline T& data()
    {
        return data_;
    }

    inline T const& data() const
    {
        return data_;
    }

    inline const unsigned char* raw_data() const
    {
        return data_.bytes();
    }

    inline std::set<std::string> const& get_fields() const
    {
        return names_;
    }

    inline feature_type const& get_grid_features() const
    {
        return features_;
    }

    inline feature_key_type const& get_feature_keys() const
    {
        return f_keys_;
    }

    inline lookup_type const& get_key() const
    {
        return key_;
    }

private:
    unsigned x_;
    unsigned y_;
    unsigned width_;
    unsigned height_;
    T const& data_;
    std::string const& key_;
    std::string const& id_name_;
    std::set<std::string> const& names_;
    feature_key_type const& f_keys_;
    feature_type const& features_;
};

using grid_view = hit_grid_view<mapnik::image<mapnik::value_integer_pixel> >;

}

#endif // MAPNIK_GRID_VIEW_HPP
