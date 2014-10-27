/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko, Jean-Francois Doyon
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

#include <mapnik/config.hpp>

// boost
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#include <boost/python.hpp>
#pragma GCC diagnostic pop

#include <mapnik/symbolizer_enumerations.hpp>
#include "mapnik_enumeration.hpp"

void export_gamma_method()
{
    using namespace boost::python;

    mapnik::enumeration_<mapnik::gamma_method_e>("gamma_method")
        .value("POWER", mapnik::GAMMA_POWER)
        .value("LINEAR",mapnik::GAMMA_LINEAR)
        .value("NONE", mapnik::GAMMA_NONE)
        .value("THRESHOLD", mapnik::GAMMA_THRESHOLD)
        .value("MULTIPLY", mapnik::GAMMA_MULTIPLY)
        ;

}
