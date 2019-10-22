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
#include <mapnik/json/feature_grammar_x3_def.hpp>

namespace mapnik { namespace json { namespace grammar {

BOOST_SPIRIT_INSTANTIATE(feature_grammar_type, iterator_type, feature_context_type);
BOOST_SPIRIT_INSTANTIATE(geometry_grammar_type, iterator_type, phrase_parse_context_type);

#if BOOST_VERSION >= 107000
BOOST_SPIRIT_INSTANTIATE(feature_grammar_type, iterator_type, feature_context_const_type);
#else
BOOST_SPIRIT_INSTANTIATE_UNUSED(feature_grammar_type, iterator_type, feature_context_const_type);
#endif

}}}
