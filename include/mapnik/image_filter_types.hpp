/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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

#ifndef MAPNIK_IMAGE_FILTER_TYPES_HPP
#define MAPNIK_IMAGE_FILTER_TYPES_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/color.hpp>
// boost
#include <boost/variant.hpp>
#include <boost/variant/variant_fwd.hpp>
// stl
#include <vector>
#include <ostream>
#include <iterator>  // for std::back_insert_iterator

namespace mapnik { namespace filter {

struct blur {};
struct emboss {};
struct sharpen {};
struct edge_detect {};
struct sobel {};
struct gray {};
struct x_gradient {};
struct y_gradient {};
struct invert {};

struct agg_stack_blur
{
    agg_stack_blur(unsigned rx_, unsigned ry_)
        : rx(rx_),ry(ry_) {}
    unsigned rx;
    unsigned ry;
};

/*
struct hsla
{
    hsla(double _h0, double _h1,
         double _s0, double _s1,
         double _l0, double _l1,
         double _a0, double _a1) :
      h0(_h0),
      h1(_h1),
      s0(_s0),
      s1(_s1),
      l0(_l0),
      l1(_l1),
      a0(_a0),
      a1(_a1) {}
    inline bool is_identity() const {
        return (h0 == 0 &&
                h1 == 1 &&
                s0 == 0 &&
                s1 == 1 &&
                l0 == 0 &&
                l1 == 1);
    }
    inline bool is_alpha_identity() const {
        return (a0 == 0 &&
                a1 == 1);
    }
    std::string to_string() const {
        std::ostringstream s;
        s << h0 << "x" << h1 << ";"
          << s0 << "x" << s1 << ";"
          << l0 << "x" << l1 << ";"
          << a0 << "x" << a1;
        return s.str();
    }
    double h0;
    double h1;
    double s0;
    double s1;
    double l0;
    double l1;
    double a0;
    double a1;
};
*/

struct color_stop
{
    color_stop() {}
    color_stop(mapnik::color const& c, double val = 0.0)
        : color(c),offset(val) {}
    mapnik::color color;
    double offset;
};

struct colorize_alpha : std::vector<color_stop>
{
    colorize_alpha() {}
};

typedef boost::variant<filter::blur,
                       filter::gray,
                       filter::agg_stack_blur,
                       filter::emboss,
                       filter::sharpen,
                       filter::edge_detect,
                       filter::sobel,
                       filter::x_gradient,
                       filter::y_gradient,
                       filter::invert,
                       //filter::hsla,
                       filter::colorize_alpha> filter_type;

inline std::ostream& operator<< (std::ostream& os, blur)
{
    os << "blur";
    return os;
}

inline std::ostream& operator<< (std::ostream& os, gray)
{
    os << "gray";
    return os;
}

inline std::ostream& operator<< (std::ostream& os, agg_stack_blur const& filter)
{
    os << "agg-stack-blur(" << filter.rx << ',' << filter.ry << ')';
    return os;
}

/*
inline std::ostream& operator<< (std::ostream& os, hsla const& filter)
{
    os << "hsla(" << filter.h0 << 'x' << filter.h1 << ':'
                  << filter.s0 << 'x' << filter.s1 << ':'
                  << filter.l0 << 'x' << filter.l1 << ':'
                  << filter.a0 << 'x' << filter.a1 << ')';
    return os;
}
*/

inline std::ostream& operator<< (std::ostream& os, emboss)
{
    os << "emboss";
    return os;
}

inline std::ostream& operator<< (std::ostream& os, sharpen)
{
    os << "sharpen";
    return os;
}

inline std::ostream& operator<< (std::ostream& os, edge_detect)
{
    os << "edge-detect";
    return os;
}

inline std::ostream& operator<< (std::ostream& os, sobel)
{
    os << "sobel";
    return os;
}

inline std::ostream& operator<< (std::ostream& os, x_gradient)
{
    os << "x-gradient";
    return os;
}

inline std::ostream& operator<< (std::ostream& os, y_gradient)
{
    os << "y-gradient";
    return os;
}

inline std::ostream& operator<< (std::ostream& os, invert)
{
    os << "invert";
    return os;
}

inline std::ostream& operator<< (std::ostream& os, filter_type const& filter);

MAPNIK_DECL bool generate_image_filters(std::back_insert_iterator<std::string> & sink, std::vector<filter_type> const& v);

MAPNIK_DECL bool parse_image_filters(std::string const& filters, std::vector<filter_type>& image_filters);

}}

#endif // MAPNIK_IMAGE_FILTER_TYPES_HPP
