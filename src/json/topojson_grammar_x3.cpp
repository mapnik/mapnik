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

#include <mapnik/json/json_grammar_config.hpp>
#include <mapnik/json/topojson_grammar_x3_def.hpp>

namespace mapnik { namespace topojson { namespace grammar {

using json::grammar::iterator_type;
using json::grammar::phrase_parse_context_type;

MAPNIK_SPIRIT_INSTANTIATE(start_rule, iterator_type, phrase_parse_context_type);

MAPNIK_SPIRIT_RULE_NAME(arc) = "Arc";
MAPNIK_SPIRIT_RULE_NAME(arcs) = "Arcs";
MAPNIK_SPIRIT_RULE_NAME(bbox) = "Bounding box";
MAPNIK_SPIRIT_RULE_NAME(empty_array) = "Empty array";
MAPNIK_SPIRIT_RULE_NAME(geometry_collection) = "Geometry collection";
MAPNIK_SPIRIT_RULE_NAME(geometry) = "Geometry object";
MAPNIK_SPIRIT_RULE_NAME(geometry_tuple) = "Geometry tuple";
MAPNIK_SPIRIT_RULE_NAME(line_indices) = "Array of Line/Ring arc indices";
MAPNIK_SPIRIT_RULE_NAME(mpoly_indices) = "Array of MultiPolygon arc indices";
MAPNIK_SPIRIT_RULE_NAME(objects) = "Objects";
MAPNIK_SPIRIT_RULE_NAME(poly_indices) = "Array of MultiLine/Polygon arc indices";
MAPNIK_SPIRIT_RULE_NAME(position) = "Position";
MAPNIK_SPIRIT_RULE_NAME(positions) = "Array of positions";
MAPNIK_SPIRIT_RULE_NAME(properties) = "Properties";
MAPNIK_SPIRIT_RULE_NAME(property) = "Property";
MAPNIK_SPIRIT_RULE_NAME(rings_array) = "Array of arc indices";
MAPNIK_SPIRIT_RULE_NAME(start_rule) = "Topology";
MAPNIK_SPIRIT_RULE_NAME(transform) = "Transform";

}}}
