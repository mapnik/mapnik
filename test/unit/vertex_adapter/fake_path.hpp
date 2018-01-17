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

// mapnik
#include <mapnik/vertex.hpp>

// stl
#include <vector>
#include <tuple>

namespace detail
{

template <typename T>
struct fake_path
{
    using coord_type = std::tuple<T, T, unsigned>;
    using cont_type = std::vector<coord_type>;
    cont_type vertices_;
    typename cont_type::iterator itr_;

    fake_path(std::initializer_list<T> l)
        : fake_path(l.begin(), l.size())
    {
    }

    fake_path(std::vector<T> const &v, bool make_invalid = false)
        : fake_path(v.begin(), v.size(), make_invalid)
    {
    }

    template <typename Itr>
    fake_path(Itr itr, size_t sz, bool make_invalid = false)
    {
        size_t num_coords = sz >> 1;
        vertices_.reserve(num_coords + (make_invalid ? 1 : 0));
        if (make_invalid)
        {
            vertices_.push_back(std::make_tuple(0,0,mapnik::SEG_END));
        }

        for (size_t i = 0; i < num_coords; ++i)
        {
            T x = *itr++;
            T y = *itr++;
            unsigned cmd = (i == 0) ? mapnik::SEG_MOVETO : mapnik::SEG_LINETO;
            vertices_.push_back(std::make_tuple(x, y, cmd));
        }
        itr_ = vertices_.begin();
    }

    unsigned vertex(T *x, T *y)
    {
        if (itr_ == vertices_.end())
        {
            return mapnik::SEG_END;
        }
        *x = std::get<0>(*itr_);
        *y = std::get<1>(*itr_);
        unsigned cmd = std::get<2>(*itr_);
        ++itr_;
        return cmd;
    }

    void rewind(unsigned)
    {
        itr_ = vertices_.begin();
    }
};

}

using fake_path = detail::fake_path<double>;

