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

#include <mapnik/geometry/boost_spirit_karma_adapter.hpp>
#include <mapnik/json/geometry_generator_grammar_impl.hpp>
#include <mapnik/json/properties_generator_grammar_impl.hpp>
#include <mapnik/json/feature_generator_grammar_impl.hpp>

#include <string>

using sink_type = std::back_insert_iterator<std::string>;

template struct mapnik::json::properties_generator_grammar<sink_type, mapnik::kv_store>;
template struct mapnik::json::feature_generator_grammar<sink_type, mapnik::feature_impl>;
template struct mapnik::json::geometry_generator_grammar<sink_type, mapnik::geometry::geometry<double> >;
