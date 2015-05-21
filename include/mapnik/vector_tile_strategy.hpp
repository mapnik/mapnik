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

#ifndef MAPNIK_VECTOR_TILE_STRATEGY_HPP
#define MAPNIK_VECTOR_TILE_STRATEGY_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/view_transform.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/numeric/conversion/cast.hpp>
#pragma GCC diagnostic pop


namespace mapnik {

namespace geometry {

struct vector_tile_strategy
{
    vector_tile_strategy(proj_transform const& prj_trans,
                         view_transform const& tr,
                         double scaling)
        : prj_trans_(prj_trans),
          tr_(tr),
          scaling_(scaling) {}

    template <typename P1, typename P2>
    inline bool apply(P1 const& p1, P2 & p2) const
    {
        using p2_type = typename boost::geometry::coordinate_type<P2>::type;
        double x = boost::geometry::get<0>(p1);
        double y = boost::geometry::get<1>(p1);
        double z = 0.0;
        if (!prj_trans_.backward(x, y, z)) return false;
        tr_.forward(&x,&y);
        x = x * scaling_;
        y = y * scaling_;
        x = std::floor(x + 0.5);
        y = std::floor(y + 0.5);
        boost::geometry::set<0>(p2, static_cast<p2_type>(x));
        boost::geometry::set<1>(p2, static_cast<p2_type>(y));
        return true;
    }
    
    template <typename P1, typename P2>
    inline P2 execute(P1 const& p1, bool & status) const
    {
        P2 p2;
        status = apply(p1, p2);
        return p2;
    }

    proj_transform const& prj_trans_;
    view_transform const& tr_;
    double const scaling_;
};

}
}

#endif // MAPNIK_VECTOR_TILE_STRATEGY_HPP
