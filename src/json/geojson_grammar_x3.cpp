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

#include <mapnik/json/geojson_grammar_x3_def.hpp>
#include <mapnik/json/json_grammar_config.hpp>
#include <mapnik/json/extract_bounding_boxes_x3_config.hpp>

namespace mapnik {
namespace json {
namespace grammar {

BOOST_SPIRIT_INSTANTIATE(geojson_grammar_type, iterator_type, context_type);
BOOST_SPIRIT_INSTANTIATE(geojson_key_value_type, iterator_type, context_type);
BOOST_SPIRIT_INSTANTIATE(geojson_grammar_type, iterator_type, extract_bounding_boxes_context_type);

#if BOOST_VERSION >= 107000
BOOST_SPIRIT_INSTANTIATE(geojson_grammar_type, iterator_type, extract_bounding_boxes_reverse_context_type);
BOOST_SPIRIT_INSTANTIATE(geojson_grammar_type, iterator_type, extract_bounding_boxes_context_type_f);
BOOST_SPIRIT_INSTANTIATE(geojson_grammar_type, iterator_type, extract_bounding_boxes_reverse_context_type_f);
#else
BOOST_SPIRIT_INSTANTIATE_UNUSED(geojson_grammar_type, iterator_type, extract_bounding_boxes_context_type);
BOOST_SPIRIT_INSTANTIATE_UNUSED(geojson_grammar_type, iterator_type, extract_bounding_boxes_reverse_context_type);
BOOST_SPIRIT_INSTANTIATE_UNUSED(geojson_grammar_type, iterator_type, extract_bounding_boxes_context_type_f);
BOOST_SPIRIT_INSTANTIATE_UNUSED(geojson_grammar_type, iterator_type, extract_bounding_boxes_reverse_context_type_f);
#endif

} // namespace grammar
} // namespace json
} // namespace mapnik
