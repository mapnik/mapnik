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

namespace helper
{
    template <std::size_t... Ts>
    struct index {};
 
    template <std::size_t N, std::size_t... Ts>
    struct gen_seq : gen_seq<N - 1, N - 1, Ts...> {};
 
    template <std::size_t... Ts>
    struct gen_seq<0, Ts...> : index<Ts...> {};
}

template <typename... Strategies>
struct strategy_group
{
    strategy_group(Strategies const& ... ops)
        : ops_(ops ...) {}

    template <typename P1, typename P2>
    inline bool apply(P1 const& p1, P2 & p2) const
    {
        bool status = true;
        p2 = execute_start<P1,P2>(p1, status, ops_);
        return status;
    }

    template <typename P1, typename P2, typename... Args, std::size_t... Is>
    inline P2 execute_start(P1 const & p1, bool & status, std::tuple<Args const&...> const& tup, helper::index<Is...>) const
    {
        return execute_first<P1,P2, Args...>(p1, status, std::get<Is>(tup)...);
    }

    template <typename P1, typename P2>
    inline P2 execute_start(P1 const& p, bool & status, std::tuple<Strategies const& ...> const& tup) const
    {
        return execute_start<P1,P2, Strategies...>(p, status, tup, helper::gen_seq<sizeof...(Strategies)> {} );
    }

    template <typename P1, typename P2, typename T, typename ...Args>
    inline P2 execute_first(P1 const& p, bool & status, T const& strat, Args const& ... args) const
    { 
        return execute_next(strat.template execute<P1,P2>(p, status), status, args...);
    }

    template <typename P2, typename T, typename ...Args>
    inline P2 execute_next(P2 const& p, bool & status, T const& strat, Args const& ... args) const
    {
        return execute_next(strat.template execute<P2,P2>(p, status), status, args...);
    }

    template <typename P2>
    inline P2 execute_next(P2 const& p, bool & status) const
    {
        return p;
    }
    
private:
    std::tuple<Strategies const& ...> ops_;

};

} // end geometry ns
} // end mapnik ns

#endif //MAPNIK_GEOMETRY_STRATEGY_HPP
