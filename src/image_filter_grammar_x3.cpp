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

#include <mapnik/image_filter_grammar_x3_def.hpp>

namespace mapnik {
namespace image_filter {

namespace x3 = boost::spirit::x3;
using iterator_type = std::string::const_iterator;
using context_type = x3::phrase_parse_context<x3::ascii::space_type>::type;

BOOST_SPIRIT_INSTANTIATE(image_filter_grammar_type, iterator_type, context_type);

} // namespace image_filter
} // namespace mapnik
