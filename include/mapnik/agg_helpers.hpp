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

#ifndef MAPNIK_AGG_HELPERS_HPP
#define MAPNIK_AGG_HELPERS_HPP

// mapnik
#include <mapnik/symbolizer_enumerations.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_gamma_functions.h"
MAPNIK_DISABLE_WARNING_POP

namespace mapnik {

template<typename T>
void set_gamma_method(T& ras_ptr, double gamma, gamma_method_enum method)
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

} // namespace mapnik

#endif // MAPNIK_AGG_HELPERS_HPP
