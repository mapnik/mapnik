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
#include <boost/variant.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/karma.hpp>
// stl
#include <iostream>

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

template <typename Out>
struct to_string_visitor : boost::static_visitor<void>
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
    boost::apply_visitor(visitor, filter);
    return os;
}

template <typename OutputIterator, typename Container>
bool generate_image_filters(OutputIterator& sink, Container const& v)
{
    using boost::spirit::karma::stream;
    using boost::spirit::karma::generate;
    bool r = generate(sink, stream % ' ', v);
    return r;
}

}}

#endif // MAPNIK_IMAGE_FILTER_TYPES_HPP
