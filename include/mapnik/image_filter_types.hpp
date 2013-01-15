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

// boost
#include <boost/variant/variant_fwd.hpp>

// stl
#include <iostream>
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

typedef boost::variant<filter::blur,
                       filter::gray,
                       filter::agg_stack_blur,
                       filter::emboss,
                       filter::sharpen,
                       filter::edge_detect,
                       filter::sobel,
                       filter::x_gradient,
                       filter::y_gradient,
                       filter::invert> filter_type;

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
    os << "agg-stack-blur:" << filter.rx << ',' << filter.ry;
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

inline std::ostream& operator<< (std::ostream& os, filter_type const& filter);

bool generate_image_filters(std::back_insert_iterator<std::string> & sink, std::vector<filter_type> const& v);

}}

#endif // MAPNIK_IMAGE_FILTER_TYPES_HPP
