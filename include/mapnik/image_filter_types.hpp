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

#ifndef MAPNIK_IMAGE_FILTER_TYPES_HPP
#define MAPNIK_IMAGE_FILTER_TYPES_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/color.hpp>
#include <mapnik/util/variant.hpp>

// stl
#include <vector>
#include <ostream>
#include <iterator>  // for std::back_insert_iterator
#include <functional> // std::ref
#include <exception>

namespace mapnik { namespace filter {

struct image_filter_base
{
    inline bool operator==(image_filter_base const& ) const
    {
        return true;
    }
};

struct blur : image_filter_base {};
struct emboss : image_filter_base {};
struct sharpen : image_filter_base {};
struct edge_detect : image_filter_base {};
struct sobel : image_filter_base {};
struct gray : image_filter_base {};
struct x_gradient : image_filter_base {};
struct y_gradient : image_filter_base {};
struct invert : image_filter_base {};

// http://vision.psychol.cam.ac.uk/jdmollon/papers/colourmaps.pdf
struct color_blind_protanope : image_filter_base 
{
    const double x = 0.7465;
    const double y = 0.2535;
    const double m = 1.273463;
    const double yint = -0.073894;
};

struct color_blind_deuteranope : image_filter_base
{
    const double x = 1.4;
    const double y = -0.4;
    const double m = 0.968437;
    const double yint = 0.003331;
};

struct color_blind_tritanope : image_filter_base
{
    const double x = 0.1748;
    const double y = 0.0;
    const double m = 0.062921;
    const double yint = 0.292119;
};

struct agg_stack_blur : image_filter_base
{
    agg_stack_blur(unsigned rx_, unsigned ry_)
        : rx(rx_),ry(ry_) {}
    inline bool operator==(agg_stack_blur const& rhs) const
    {
        return rx == rhs.rx && ry == rhs.ry;
    }
    unsigned rx;
    unsigned ry;
};

struct color_to_alpha : image_filter_base
{
    color_to_alpha(mapnik::color const& c)
        : color(c) {}
    inline bool operator==(color_to_alpha const& rhs) const
    {
        return color == rhs.color;
    }
    mapnik::color color;
};

struct scale_hsla : image_filter_base
{
    scale_hsla(double _h0, double _h1,
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
      a1(_a1) { }

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

    inline bool operator==(scale_hsla const& rhs) const
    {
        return h0 == rhs.h0 &&
            h1 == rhs.h1 &&
            s0 == rhs.s0 &&
            s1 == rhs.s1 &&
            l0 == rhs.l0 &&
            l1 == rhs.l1 &&
            a0 == rhs.a0 &&
            a1 == rhs.a1;
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

struct color_stop
{
    color_stop()
        : color(),
          offset(0.0) {}
    color_stop(mapnik::color const& c, double val = 0.0)
        : color(c),
          offset(val) {}
    bool operator==(color_stop const& rhs) const { return color == rhs.color && offset == rhs.offset;}
    mapnik::color color;
    double offset;
};

struct colorize_alpha : std::vector<color_stop>
{
    colorize_alpha() {}
};

using filter_type =  util::variant<filter::blur,
                                   filter::gray,
                                   filter::agg_stack_blur,
                                   filter::emboss,
                                   filter::sharpen,
                                   filter::edge_detect,
                                   filter::sobel,
                                   filter::x_gradient,
                                   filter::y_gradient,
                                   filter::invert,
                                   filter::scale_hsla,
                                   filter::colorize_alpha,
                                   filter::color_to_alpha,
                                   filter::color_blind_protanope,
                                   filter::color_blind_deuteranope,
                                   filter::color_blind_tritanope>;

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

inline std::ostream& operator<< (std::ostream& os, color_to_alpha const& filter)
{
    os << "color-to-alpha(" << filter.color << ')';
    return os;
}

inline std::ostream& operator<< (std::ostream& os, scale_hsla const& filter)
{
    os << "scale-hsla("
                  << filter.h0 << ',' << filter.h1 << ','
                  << filter.s0 << ',' << filter.s1 << ','
                  << filter.l0 << ',' << filter.l1 << ','
                  << filter.a0 << ',' << filter.a1 << ')';
    return os;
}

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

inline std::ostream& operator<< (std::ostream& os, color_blind_protanope)
{
    os << "color-blind-protanope";
    return os;
}

inline std::ostream& operator<< (std::ostream& os, color_blind_deuteranope)
{
    os << "color-blind-deuteranope";
    return os;
}

inline std::ostream& operator<< (std::ostream& os, color_blind_tritanope)
{
    os << "color-blind-tritanope";
    return os;
}

inline std::ostream& operator<< (std::ostream& os, colorize_alpha const& filter)
{
    os << "colorize-alpha(";
    bool first = true;
    for ( mapnik::filter::color_stop const& stop : filter)
    {
        if (!first) os << ",";
        else first = false;
        os << stop.color;
        if (stop.offset > 0)
        {
            os << " " << stop.offset;
        }
    }
    os << ')';
    return os;
}


template <typename Out>
struct to_string_visitor
{
    to_string_visitor(Out & out)
    : out_(out) {}

    template <typename T>
    void operator () (T const& filter_tag)
    {
        out_ << filter_tag;
    }

    Out & out_;
};

inline std::ostream& operator<< (std::ostream& os, filter_type const& filter)
{
    to_string_visitor<std::ostream> visitor(os);
    util::apply_visitor(std::ref(visitor), filter);
    return os;
}

MAPNIK_DECL bool generate_image_filters(std::back_insert_iterator<std::string> & sink, std::vector<filter_type> const& v);

MAPNIK_DECL bool parse_image_filters(std::string const& filters, std::vector<filter_type>& image_filters);

}}

#endif // MAPNIK_IMAGE_FILTER_TYPES_HPP
