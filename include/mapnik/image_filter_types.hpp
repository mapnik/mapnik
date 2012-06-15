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

#include <boost/variant.hpp>

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
    // an attempt to support older boost spirit (< 1.46)
    agg_stack_blur()
        : rx(1),ry(1) {}
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

}}

#endif // MAPNIK_IMAGE_FILTER_TYPES_HPP
