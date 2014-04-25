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

#ifndef MAPNIK_AGG_HELPERS_HPP
#define MAPNIK_AGG_HELPERS_HPP

// mapnik
#include <mapnik/gamma_method.hpp>
#include <mapnik/symbolizer.hpp>


// agg
#include "agg_gamma_functions.h"        // for gamma_power, gamma_linear, etc
#include "agg_math_stroke.h"            // for line_join_e::miter_join, etc
#include "agg_rasterizer_outline_aa.h"

namespace mapnik {

template <typename T>
void set_gamma_method(T & ras_ptr, double gamma, gamma_method_enum method)
{
    switch (method)
    {
    case GAMMA_POWER:
        ras_ptr->gamma(agg::gamma_power(gamma));
        break;
    case GAMMA_LINEAR:
        ras_ptr->gamma(agg::gamma_linear(0.0, gamma));
        break;
    case GAMMA_NONE:
        ras_ptr->gamma(agg::gamma_none());
        break;
    case GAMMA_THRESHOLD:
        ras_ptr->gamma(agg::gamma_threshold(gamma));
        break;
    case GAMMA_MULTIPLY:
        ras_ptr->gamma(agg::gamma_multiply(gamma));
        break;
    default:
        ras_ptr->gamma(agg::gamma_power(gamma));
    }
}

template <typename Symbolizer, typename PathType, typename Feature>
void set_join_caps(Symbolizer const& sym, PathType & stroke, Feature const& feature)
{
    line_join_enum join = get<line_join_enum>(sym, keys::stroke_linejoin, feature, MITER_JOIN);
    switch (join)
    {
    case MITER_JOIN:
        stroke.generator().line_join(agg::miter_join);
        break;
    case MITER_REVERT_JOIN:
        stroke.generator().line_join(agg::miter_join);
        break;
    case ROUND_JOIN:
        stroke.generator().line_join(agg::round_join);
        break;
    default:
        stroke.generator().line_join(agg::bevel_join);
    }

    line_cap_enum cap = get<line_cap_enum>(sym, keys::stroke_linecap, feature, BUTT_CAP);

    switch (cap)
    {
    case BUTT_CAP:
        stroke.generator().line_cap(agg::butt_cap);
        break;
    case SQUARE_CAP:
        stroke.generator().line_cap(agg::square_cap);
        break;
    default:
        stroke.generator().line_cap(agg::round_cap);
    }
}


template <typename Symbolizer, typename Rasterizer, typename Feature>
void set_join_caps_aa(Symbolizer const& sym, Rasterizer & ras, Feature & feature)
{
    line_join_enum join = get<line_join_enum>(sym, keys::stroke_linejoin, feature, MITER_JOIN);
    switch (join)
    {
    case MITER_JOIN:
        ras.line_join(agg::outline_miter_accurate_join);
        break;
    case MITER_REVERT_JOIN:
        ras.line_join(agg::outline_no_join);
        break;
    case ROUND_JOIN:
        ras.line_join(agg::outline_round_join);
        break;
    default:
        ras.line_join(agg::outline_no_join);
    }

    line_cap_enum cap = get<line_cap_enum>(sym, keys::stroke_linecap, feature, BUTT_CAP);

    switch (cap)
    {
    case BUTT_CAP:
        ras.round_cap(false);
        break;
    case SQUARE_CAP:
        ras.round_cap(false);
        break;
    default:
        ras.round_cap(true);
    }
}

}

#endif //MAPNIK_AGG_HELPERS_HPP
