/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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


#ifndef MAPNIK_JSON_PARSE_FEATURE_HPP
#define MAPNIK_JSON_PARSE_FEATURE_HPP

#include <mapnik/feature.hpp>
#include <mapnik/unicode.hpp>

namespace mapnik { namespace json {

template <typename Iterator>
void parse_feature(Iterator start, Iterator end, feature_impl& feature, mapnik::transcoder const& tr = mapnik::transcoder("utf8"));

template <typename Iterator>
void parse_geometry(Iterator start, Iterator end, feature_impl& feature);

}}


#endif // MAPNIK_JSON_PARSE_FEATURE_HPP
