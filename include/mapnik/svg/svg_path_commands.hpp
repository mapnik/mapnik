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

#ifndef MAPNIK_SVG_PATH_COMMANDS_HPP
#define MAPNIK_SVG_PATH_COMMANDS_HPP

// mapnik
#include <mapnik/global.hpp>

// boost
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#pragma GCC diagnostic pop

namespace mapnik { namespace svg {

using namespace boost::fusion;

inline double deg2rad(double deg)
{
    return (M_PI * deg)/180.0;
}

template <typename PathType>
struct move_to
{

    template <typename T0>
    struct result
    {
        using type = void;
    };

    explicit move_to(PathType & path)
        : path_(path) {}

    template  <typename T0, typename T1>
    void operator() (T0 v, T1 rel) const
    {
        path_.move_to(at_c<0>(v),at_c<1>(v),rel); // impl
    }

    PathType & path_;
};

template <typename PathType>
struct hline_to
{
    template <typename T0>
    struct result
    {
        using type = void;
    };

    explicit hline_to(PathType & path)
        : path_(path) {}

    template  <typename T0, typename T1>
    void operator() (T0 const& x, T1 rel) const
    {
        path_.hline_to(x,rel);
    }

    PathType & path_;
};


template <typename PathType>
struct vline_to
{
    template <typename T0>
    struct result
    {
        using type = void;
    };

    explicit vline_to(PathType & path)
        : path_(path) {}

    template  <typename T0, typename T1>
    void operator() (T0 const& y, T1 rel) const
    {
        path_.vline_to(y,rel);
    }

    PathType & path_;
};

template <typename PathType>
struct line_to
{
    template <typename T0>
    struct result
    {
        using type = void;
    };

    explicit line_to(PathType & path)
        : path_(path) {}

    template  <typename T0, typename T1>
    void operator() (T0 const& v, T1 rel) const
    {
        path_.line_to(at_c<0>(v),at_c<1>(v),rel); // impl
    }

    PathType & path_;
};


template <typename PathType>
struct curve4
{
    template <typename T0>
    struct result
    {
        using type = void;
    };

    explicit curve4(PathType & path)
        : path_(path) {}

    template  <typename T0, typename T1,typename T2, typename T3>
    void operator() (T0 const& v0, T1 const& v1, T2 const& v2, T3 rel) const
    {
        path_.curve4(at_c<0>(v0),at_c<1>(v0),
                     at_c<0>(v1),at_c<1>(v1),
                     at_c<0>(v2),at_c<1>(v2),
                     rel); // impl
    }

    PathType & path_;
};


template <typename PathType>
struct curve4_smooth
{
    template <typename T0>
    struct result
    {
        using type = void;
    };

    explicit curve4_smooth(PathType & path)
        : path_(path) {}

    template  <typename T0, typename T1,typename T2>
    void operator() (T0 const& v0, T1 const& v1, T2 rel) const
    {
        path_.curve4(at_c<0>(v0),at_c<1>(v0),
                     at_c<0>(v1),at_c<1>(v1),
                     rel); // impl
    }
    PathType & path_;
};

template <typename PathType>
struct curve3
{
    template <typename T0>
    struct result
    {
        using type = void;
    };

    explicit curve3(PathType & path)
        : path_(path) {}

    template  <typename T0, typename T1,typename T2>
    void operator() (T0 const& v0, T1 const& v1, T2 rel) const
    {
        path_.curve3(at_c<0>(v0),at_c<1>(v0),
                     at_c<0>(v1),at_c<1>(v1),
                     rel); // impl
    }

    PathType & path_;
};

template <typename PathType>
struct curve3_smooth
{
    template <typename T0>
    struct result
    {
        using type = void;
    };

    explicit curve3_smooth(PathType & path)
        : path_(path) {}

    template  <typename T0, typename T1>
    void operator() (T0 const& v0, T1 rel) const
    {
        path_.curve3(at_c<0>(v0),at_c<1>(v0),
                     rel); // impl
    }

    PathType & path_;
};

template <typename PathType>
struct arc_to
{
    template <typename T0>
    struct result
    {
        using type = void;
    };

    explicit arc_to(PathType & path)
        : path_(path) {}

    template  <typename T0, typename T1,typename T2, typename T3, typename T4, typename T5>
    void operator() (T0 const& rv, T1 const& angle, T2 large_arc_flag, T3 sweep_flag, T4 const& v, T5 rel) const
    {
        path_.arc_to(at_c<0>(rv),at_c<1>(rv),
                     deg2rad(angle),large_arc_flag,sweep_flag,
                     at_c<0>(v),at_c<1>(v),
                     rel);
    }

    PathType & path_;
};

template <typename PathType>
struct close
{
    using result_type = void;

    explicit close(PathType & path)
        : path_(path) {}

    void operator()() const
    {
        path_.close_subpath();
    }

    PathType & path_;
};

}}


#endif // MAPNIK_SVG_COMMANDS_HPP
