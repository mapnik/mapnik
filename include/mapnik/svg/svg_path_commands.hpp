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

#ifndef MAPNIK_SVG_PATH_COMMANDS_HPP
#define MAPNIK_SVG_PATH_COMMANDS_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/util/math.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/fusion/sequence/intrinsic/at.hpp>
MAPNIK_DISABLE_WARNING_POP

namespace mapnik {
namespace svg {

struct move_to
{
    using result_type = void;

    template<typename PathType, typename T0, typename T1>
    void operator()(PathType& path, T0 v, T1 rel) const
    {
        path.move_to(boost::fusion::at_c<0>(v), boost::fusion::at_c<1>(v), rel); // impl
    }
};

struct hline_to
{
    using result_type = void;
    template<typename PathType, typename T0, typename T1>
    void operator()(PathType& path, T0 const& x, T1 rel) const
    {
        path.hline_to(x, rel);
    }
};

struct vline_to
{
    using result_type = void;
    template<typename PathType, typename T0, typename T1>
    void operator()(PathType& path, T0 const& y, T1 rel) const
    {
        path.vline_to(y, rel);
    }
};

struct line_to
{
    using result_type = void;
    template<typename PathType, typename T0, typename T1>
    void operator()(PathType& path, T0 const& v, T1 rel) const
    {
        path.line_to(boost::fusion::at_c<0>(v), boost::fusion::at_c<1>(v), rel); // impl
    }
};

struct curve4
{
    using result_type = void;
    template<typename PathType, typename T0, typename T1, typename T2, typename T3>
    void operator()(PathType& path, T0 const& v0, T1 const& v1, T2 const& v2, T3 rel) const
    {
        path.curve4(boost::fusion::at_c<0>(v0),
                    boost::fusion::at_c<1>(v0),
                    boost::fusion::at_c<0>(v1),
                    boost::fusion::at_c<1>(v1),
                    boost::fusion::at_c<0>(v2),
                    boost::fusion::at_c<1>(v2),
                    rel); // impl
    }
};

struct curve4_smooth
{
    using result_type = void;
    template<typename PathType, typename T0, typename T1, typename T2>
    void operator()(PathType& path, T0 const& v0, T1 const& v1, T2 rel) const
    {
        path.curve4(boost::fusion::at_c<0>(v0),
                    boost::fusion::at_c<1>(v0),
                    boost::fusion::at_c<0>(v1),
                    boost::fusion::at_c<1>(v1),
                    rel); // impl
    }
};

struct curve3
{
    using result_type = void;
    template<typename PathType, typename T0, typename T1, typename T2>
    void operator()(PathType& path, T0 const& v0, T1 const& v1, T2 rel) const
    {
        path.curve3(boost::fusion::at_c<0>(v0),
                    boost::fusion::at_c<1>(v0),
                    boost::fusion::at_c<0>(v1),
                    boost::fusion::at_c<1>(v1),
                    rel); // impl
    }
};

struct curve3_smooth
{
    using result_type = void;
    template<typename PathType, typename T0, typename T1>
    void operator()(PathType& path, T0 const& v0, T1 rel) const
    {
        path.curve3(boost::fusion::at_c<0>(v0), boost::fusion::at_c<1>(v0),
                    rel); // impl
    }
};

struct arc_to
{
    using result_type = void;
    template<typename PathType, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
    void
      operator()(PathType& path, T0 const& rv, T1 const& angle, T2 large_arc_flag, T3 sweep_flag, T4 const& v, T5 rel)
        const
    {
        path.arc_to(boost::fusion::at_c<0>(rv),
                    boost::fusion::at_c<1>(rv),
                    util::radians(angle),
                    large_arc_flag,
                    sweep_flag,
                    boost::fusion::at_c<0>(v),
                    boost::fusion::at_c<1>(v),
                    rel);
    }
};

struct close
{
    using result_type = void;
    template<typename PathType>
    void operator()(PathType& path) const
    {
        path.close_subpath();
    }
};
} // namespace svg
} // namespace mapnik

#endif // MAPNIK_SVG_COMMANDS_HPP
