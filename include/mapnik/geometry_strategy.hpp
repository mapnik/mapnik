/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

#ifndef MAPNIK_GEOMETRY_STRATEGY_HPP
#define MAPNIK_GEOMETRY_STRATEGY_HPP

namespace mapnik { 
namespace geometry {

template <typename Strategy, typename... Strategies>
struct strategy_group
{
    strategy_group(Strategy const& op, Strategies const& ... ops)
        : ops_(op, ops ...) {}

    template <typename P1, typename P2>
    inline bool apply(P1 const& p1, P2 & p2) const
    {
        bool status = true;
        p2 = execute_first<0, P1, P2, Strategy, ...Strategies>(p1, status);
        return status;
    }

    template <int c, typename P1, typename P2, typename T, typename ...Args>
    inline P2 execute_first(P1 const& p, bool & status) 
    { 
        return execute_next<c+1,P1,P2,...Args>(std::get<c>(ops_).template execute<P1,P2>(p, status), status);
    }

    template <int c, typename P2, typename T, typename ...Args>
    inline P2 execute_next(P2 const& p, bool & status)
    {
        return execute_next<c+1, P2, ...Args>(std::get<c>(ops_).template execute<P2,P2>(p, status), status);
    }

    template <int c, typename P2, typename T>
    inline P2 execute_next(P2 const& p, bool & status)
    {
        return std::get<c>(ops_).template execute<P2,P2>(p, status);
    }
    
private:
    std::tuple<Strategy const&, Strategies const& ...> ops_;

};

} // end geometry ns
} // end mapnik ns

#endif //MAPNIK_GEOMETRY_STRATEGY_HPP
