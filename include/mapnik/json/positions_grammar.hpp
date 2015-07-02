/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

#ifndef MAPNIK_JSON_POSITIONS_GRAMMAR_HPP
#define MAPNIK_JSON_POSITIONS_GRAMMAR_HPP

// mapnik
#include <mapnik/util/variant.hpp>
#include <mapnik/json/generic_json.hpp>
#include <mapnik/json/error_handler.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geometry_fusion_adapted.hpp>
// boost
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#pragma GCC diagnostic pop

// stl
#include <tuple>

namespace mapnik { namespace json {

struct empty {};

using position = mapnik::geometry::point<double>;
using positions = std::vector<position>;
using coordinates = util::variant<empty, position, positions, std::vector<positions>, std::vector<std::vector<positions> > > ;

namespace qi = boost::spirit::qi;

struct set_position_impl
{
    using result_type = void;
    template <typename T0,typename T1>
    result_type operator() (T0 & coords, T1 const& pos) const
    {
        if (pos) coords = *pos;
    }
};

struct push_position_impl
{
    using result_type = void;
    template <typename T0,typename T1>
    result_type operator() (T0 & coords, T1 const& pos) const
    {
        if (pos) coords.push_back(*pos);
    }
};

template <typename Iterator, typename ErrorHandler = error_handler<Iterator> >
struct positions_grammar :
        qi::grammar<Iterator,coordinates(),space_type>
{
    positions_grammar();
    qi::rule<Iterator, coordinates(),space_type> coords;
    qi::rule<Iterator, boost::optional<position>(), space_type> pos;
    qi::rule<Iterator, positions(), space_type> ring;
    qi::rule<Iterator, std::vector<positions>(), space_type> rings;
    qi::rule<Iterator, std::vector<std::vector<positions> >(), space_type> rings_array;
    boost::phoenix::function<set_position_impl> set_position;
    boost::phoenix::function<push_position_impl> push_position;
    // error handler
    boost::phoenix::function<ErrorHandler> const error_handler;
};

}}

#endif // MAPNIK_JSON_POSITIONS_GRAMMAR_HPP
