/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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

#include <mapnik/expression_node.hpp>

namespace mapnik
{

#if defined(BOOST_REGEX_HAS_ICU)

regex_match_node::regex_match_node (expr_node const& a, UnicodeString const& ustr)
    : expr(a),
      pattern(boost::make_u32regex(ustr)) {}

regex_replace_node::regex_replace_node (expr_node const& a, UnicodeString const& ustr, UnicodeString const& f)
    : expr(a),
      pattern(boost::make_u32regex(ustr)),
      format(f) {}

#else

regex_match_node::regex_match_node (expr_node const& a, std::string const& str)
    : expr(a),
      pattern(str) {}

regex_replace_node::regex_replace_node (expr_node const& a, std::string const& str, std::string const& f)
    : expr(a),
      pattern(str),
      format(f) {}
#endif

}