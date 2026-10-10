/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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
#include <mapnik/symbolizer_enumerations.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_gamma_functions.h"
MAPNIK_DISABLE_WARNING_POP

// stl
#include <cmath>

namespace mapnik {

// Feed a dash array and dash offset (both unscaled) into an agg::conv_dash.
template<typename Dash, typename DashArray>
void apply_dash_array(Dash& dash, DashArray const& dashes, double dash_offset, double scale_factor)
{
    double dash_length = 0.0;
    for (auto const& d : dashes)
    {
        dash.add_dash(d.first * scale_factor, d.second * scale_factor);
        dash_length += d.first + d.second;
    }
    if (dash_offset != 0.0 && dash_length > 0.0 && std::isfinite(dash_offset))
    {
        // AGG treats negative dash_start values as a request to continue
        // the pattern across subpaths. Normalize to a positive phase
        // instead, and avoid iterating over whole pattern repetitions.
        dash_offset = std::fmod(dash_offset, dash_length);
        if (dash_offset < 0.0)
            dash_offset += dash_length;
        dash.dash_start(dash_offset * scale_factor);
    }
}

template<typename T>
void set_gamma_method(T& ras_ptr, double gamma, gamma_method_enum method)
{
    switch (method)
    {
        case gamma_method_enum::GAMMA_POWER:
            ras_ptr->gamma(agg::gamma_power(gamma));
            break;
        case gamma_method_enum::GAMMA_LINEAR:
            ras_ptr->gamma(agg::gamma_linear(0.0, gamma));
            break;
        case gamma_method_enum::GAMMA_NONE:
            ras_ptr->gamma(agg::gamma_none());
            break;
        case gamma_method_enum::GAMMA_THRESHOLD:
            ras_ptr->gamma(agg::gamma_threshold(gamma));
            break;
        case gamma_method_enum::GAMMA_MULTIPLY:
            ras_ptr->gamma(agg::gamma_multiply(gamma));
            break;
        default:
            ras_ptr->gamma(agg::gamma_power(gamma));
    }
}

} // namespace mapnik

#endif // MAPNIK_AGG_HELPERS_HPP
