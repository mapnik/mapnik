/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
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

#include <mapnik/geometry/boost_adapters.hpp>
#include <mapnik/util/rounding_cast.hpp>

namespace mapnik {
namespace geometry {

namespace helper {
template<std::size_t... Ts>
struct index
{};

template<std::size_t N, std::size_t... Ts>
struct gen_seq : gen_seq<N - 1, N - 1, Ts...>
{};

template<std::size_t... Ts>
struct gen_seq<0, Ts...> : index<Ts...>
{};
} // namespace helper

// Groups a set of strategies at runtime, the conversion from P1 to P2 will take place on the LAST strategy.
template<typename... Strategies>
struct strategy_group
{
    strategy_group(Strategies const&... ops)
        : ops_(ops...)
    {}

    template<typename P1, typename P2>
    inline bool apply(P1 const& p1, P2& p2) const
    {
        bool status = true;
        p2 = execute_start<P1, P2>(p1, status, ops_);
        return status;
    }

    template<typename P1, typename P2, typename... Args, std::size_t... Is>
    inline P2
      execute_start(P1 const& p1, bool& status, std::tuple<Args const&...> const& tup, helper::index<Is...>) const
    {
        return execute<P1, P2, Args...>(p1, status, std::get<Is>(tup)...);
    }

    template<typename P1, typename P2>
    inline P2 execute_start(P1 const& p, bool& status, std::tuple<Strategies const&...> const& tup) const
    {
        return execute_start<P1, P2, Strategies...>(p, status, tup, helper::gen_seq<sizeof...(Strategies)>{});
    }

    template<typename P1, typename P2, typename T, typename... Args>
    inline P2 execute(P1 const& p, bool& status, T const& strat, Args const&... args) const
    {
        return execute<P1, P2>(strat.template execute<P1, P1>(p, status), status, args...);
    }

    template<typename P1, typename P2, typename T>
    inline P2 execute(P1 const& p, bool& status, T const& strat) const
    {
        return strat.template execute<P1, P2>(p, status);
    }

  private:
    std::tuple<Strategies const&...> ops_;
};

// The difference between this strategy group and the previous is that the conversion from P1 to P2 happens
// in the first strategy rather then the last strategy.
template<typename... Strategies>
struct strategy_group_first
{
    strategy_group_first(Strategies const&... ops)
        : ops_(ops...)
    {}

    template<typename P1, typename P2>
    inline bool apply(P1 const& p1, P2& p2) const
    {
        bool status = true;
        p2 = execute_start<P1, P2>(p1, status, ops_);
        return status;
    }

    template<typename P1, typename P2, typename... Args, std::size_t... Is>
    inline P2
      execute_start(P1 const& p1, bool& status, std::tuple<Args const&...> const& tup, helper::index<Is...>) const
    {
        return execute_first<P1, P2, Args...>(p1, status, std::get<Is>(tup)...);
    }

    template<typename P1, typename P2>
    inline P2 execute_start(P1 const& p, bool& status, std::tuple<Strategies const&...> const& tup) const
    {
        return execute_start<P1, P2, Strategies...>(p, status, tup, helper::gen_seq<sizeof...(Strategies)>{});
    }

    template<typename P1, typename P2, typename T, typename... Args>
    inline P2 execute_first(P1 const& p, bool& status, T const& strat, Args const&... args) const
    {
        return execute<P2>(strat.template execute<P1, P2>(p, status), status, args...);
    }

    template<typename P2, typename T, typename... Args>
    inline P2 execute(P2 const& p, bool& status, T const& strat, Args const&... args) const
    {
        return execute<P2>(strat.template execute<P2, P2>(p, status), status, args...);
    }

    template<typename P2, typename T>
    inline P2 execute(P2 const& p, bool& status, T const& strat) const
    {
        return strat.template execute<P2, P2>(p, status);
    }

    template<typename P2>
    inline P2 execute(P2 const& p, bool& status) const
    {
        return p;
    }

  private:
    std::tuple<Strategies const&...> ops_;
};

struct scale_strategy
{
    scale_strategy(double scale, double offset = 0)
        : scale_(scale)
        , offset_(offset)
    {}

    template<typename P1, typename P2>
    inline bool apply(P1 const& p1, P2& p2) const
    {
        using p2_type = typename boost::geometry::coordinate_type<P2>::type;
        double x = (boost::geometry::get<0>(p1) * scale_) + offset_;
        double y = (boost::geometry::get<1>(p1) * scale_) + offset_;
        boost::geometry::set<0>(p2, static_cast<p2_type>(x));
        boost::geometry::set<1>(p2, static_cast<p2_type>(y));
        return true;
    }

    template<typename P1, typename P2>
    inline P2 execute(P1 const& p1, bool& status) const
    {
        P2 p2;
        status = apply(p1, p2);
        return p2;
    }

  private:
    double scale_;
    double offset_;
};

struct scale_rounding_strategy
{
    scale_rounding_strategy(double scale, double offset = 0)
        : scale_(scale)
        , offset_(offset)
    {}

    template<typename P1, typename P2>
    inline bool apply(P1 const& p1, P2& p2) const
    {
        using p2_type = typename boost::geometry::coordinate_type<P2>::type;
        double x = (boost::geometry::get<0>(p1) * scale_) + offset_;
        double y = (boost::geometry::get<1>(p1) * scale_) + offset_;
        boost::geometry::set<0>(p2, static_cast<p2_type>(std::round(x)));
        boost::geometry::set<1>(p2, static_cast<p2_type>(std::round(y)));
        return true;
    }

    template<typename P1, typename P2>
    inline P2 execute(P1 const& p1, bool& status) const
    {
        P2 p2;
        status = apply(p1, p2);
        return p2;
    }

  private:
    double scale_;
    double offset_;
};

} // namespace geometry
} // namespace mapnik

#endif // MAPNIK_GEOMETRY_STRATEGY_HPP
