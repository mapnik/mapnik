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

#ifndef MAPNIK_TRANSFORM_PATH_ADAPTER_HPP
#define MAPNIK_TRANSFORM_PATH_ADAPTER_HPP

#include <mapnik/proj_transform.hpp>
#include <mapnik/vertex.hpp>
#include <mapnik/config.hpp>

#include <cstddef>

namespace mapnik {

template<typename Transform, typename Geometry>
struct transform_path_adapter
{
    // SFINAE value_type detector
    template<typename T>
    struct void_type
    {
        using type = void;
    };

    template<typename T, typename D, typename _ = void>
    struct select_value_type
    {
        using type = D;
    };

    template<typename T, typename D>
    struct select_value_type<T, D, typename void_type<typename T::value_type>::type>
    {
        using type = typename T::value_type;
    };

    using size_type = std::size_t;
    using value_type = typename select_value_type<Geometry, void>::type;

    transform_path_adapter(Transform const& _t, Geometry& _geom, proj_transform const& prj_trans)
        : t_(&_t),
          geom_(_geom),
          prj_trans_(&prj_trans)
    {}

    explicit transform_path_adapter(Geometry& _geom)
        : t_(0),
          geom_(_geom),
          prj_trans_(0)
    {}

    void set_proj_trans(proj_transform const& prj_trans) { prj_trans_ = &prj_trans; }

    void set_trans(Transform const& t) { t_ = &t; }

    unsigned vertex(double* x, double* y) const
    {
        unsigned command;
        bool ok = false;
        bool skipped_points = false;
        while (!ok)
        {
            command = geom_.vertex(x, y);
            if (command == SEG_END || command == SEG_CLOSE)
            {
                return command;
            }
            double z = 0;
            ok = prj_trans_->backward(*x, *y, z);
            if (!ok)
            {
                skipped_points = true;
            }
        }
        if (skipped_points && (command == SEG_LINETO))
        {
            command = SEG_MOVETO;
        }
        t_->forward(x, y);
        return command;
    }

    void rewind(unsigned pos) const { geom_.rewind(pos); }

    unsigned type() const { return static_cast<unsigned>(geom_.type()); }

    Geometry const& geom() const { return geom_; }

  private:
    Transform const* t_;
    Geometry& geom_;
    proj_transform const* prj_trans_;
};

} // namespace mapnik

#endif // MAPNIK_TRANSFORM_PATH_ADAPTER_HPP
