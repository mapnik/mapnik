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

#ifndef MAPNIK_VIEW_STRATEGY_HPP
#define MAPNIK_VIEW_STRATEGY_HPP

// mapnik
#include <mapnik/view_transform.hpp>
#include <mapnik/safe_cast.hpp>
// boost
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/access.hpp>

namespace mapnik {

struct view_strategy
{
    view_strategy(view_transform const& tr)
        : tr_(tr)
    {}

    template<typename P1, typename P2>
    inline bool apply(P1 const& p1, P2& p2) const
    {
        using coordinate_type = typename boost::geometry::coordinate_type<P2>::type;
        double x = boost::geometry::get<0>(p1);
        double y = boost::geometry::get<1>(p1);
        tr_.forward(&x, &y);
        boost::geometry::set<0>(p2, safe_cast<coordinate_type>(x));
        boost::geometry::set<1>(p2, safe_cast<coordinate_type>(y));
        return true;
    }

    template<typename P1, typename P2>
    inline P2 execute(P1 const& p1, bool& status) const
    {
        P2 p2;
        status = apply(p1, p2);
        return p2;
    }

    view_transform const& tr_;
};

struct unview_strategy
{
    unview_strategy(view_transform const& tr)
        : tr_(tr)
    {}

    template<typename P1, typename P2>
    inline bool apply(P1 const& p1, P2& p2) const
    {
        using coordinate_type = typename boost::geometry::coordinate_type<P2>::type;
        double x = boost::geometry::get<0>(p1);
        double y = boost::geometry::get<1>(p1);
        tr_.backward(&x, &y);
        boost::geometry::set<0>(p2, static_cast<coordinate_type>(x));
        boost::geometry::set<1>(p2, static_cast<coordinate_type>(y));
        return true;
    }

    template<typename P1, typename P2>
    inline P2 execute(P1 const& p1, bool& status) const
    {
        P2 p2;
        status = apply(p1, p2);
        return p2;
    }

    view_transform const& tr_;
};

} // namespace mapnik

#endif // MAPNIK_VIEW_STRATEGY_HPP
