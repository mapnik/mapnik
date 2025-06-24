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

#ifndef MAPNIK_BOOST_SPIRIT_INSTANTIATE_HPP
#define MAPNIK_BOOST_SPIRIT_INSTANTIATE_HPP

namespace boost {
namespace spirit {
namespace x3 {
// helper macro
#define BOOST_SPIRIT_INSTANTIATE_UNUSED(rule_type, Iterator, Context)                                                  \
    template bool parse_rule<Iterator, Context, boost::spirit::x3::unused_type const>(                                 \
      rule_type rule_,                                                                                                 \
      Iterator & first,                                                                                                \
      Iterator const& last,                                                                                            \
      Context const& context,                                                                                          \
      boost::spirit::x3::unused_type const&);                                                                          \
    /***/

} // namespace x3
} // namespace spirit
} // namespace boost

#endif // MAPNIK_BOOST_SPIRIT_INSTANTIATE_HPP
