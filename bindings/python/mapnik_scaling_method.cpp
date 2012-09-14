/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko, Jean-Francois Doyon
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


#include <boost/python.hpp>
#include <mapnik/image_scaling.hpp>
#include "mapnik_enumeration.hpp"

void export_scaling_method()
{
    using namespace boost::python;

    enum_<mapnik::scaling_method_e>("scaling_method")
        .value("NEAR", mapnik::SCALING_NEAR)
        .value("BILINEAR", mapnik::SCALING_BILINEAR)
        .value("BICUBIC", mapnik::SCALING_BICUBIC)
        .value("SPLINE16", mapnik::SCALING_SPLINE16)
        .value("SPLINE36", mapnik::SCALING_SPLINE36)
        .value("HANNING", mapnik::SCALING_HANNING)
        .value("HAMMING", mapnik::SCALING_HAMMING)
        .value("HERMITE", mapnik::SCALING_HERMITE)
        .value("KAISER", mapnik::SCALING_KAISER)
        .value("QUADRIC", mapnik::SCALING_QUADRIC)
        .value("CATROM", mapnik::SCALING_CATROM)
        .value("GAUSSIAN", mapnik::SCALING_GAUSSIAN)
        .value("BESSEL", mapnik::SCALING_BESSEL)
        .value("MITCHELL", mapnik::SCALING_MITCHELL)
        .value("SINC", mapnik::SCALING_SINC)
        .value("LANCZOS", mapnik::SCALING_LANCZOS)
        .value("BLACKMAN", mapnik::SCALING_BLACKMAN)
        .value("BILINEAR8", mapnik::SCALING_BILINEAR8)
        ;
}
