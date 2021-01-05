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

#ifndef MAPNIK_JSON_ERROR_HANDLER_HPP
#define MAPNIK_JSON_ERROR_HANDLER_HPP

#include <mapnik/config.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/home/qi/nonterminal/error_handler.hpp>
namespace boost { namespace spirit { struct info; } }
#pragma GCC diagnostic pop

// mapnik
#ifdef MAPNIK_LOG
#include <mapnik/debug.hpp>
#endif

// stl
#include <cassert>
#include <string>
#include <sstream>


namespace mapnik { namespace json {

template <typename Iterator>
struct error_handler
{
    using result_type = boost::spirit::qi::error_handler_result;
    result_type operator() (
        Iterator,
        Iterator end,
        Iterator err_pos,
        boost::spirit::info const& what) const
    {
#ifdef MAPNIK_LOG
        std::stringstream s;
        using difference_type = typename std::iterator_traits<Iterator>::difference_type;
        auto start_err = err_pos;
        std::advance(err_pos, std::min(std::distance(err_pos, end), difference_type(16)));
        auto end_err = err_pos;
        assert(end_err <= end);
        s << "Mapnik GeoJSON parsing error:" << what << " expected but got: " << std::string(start_err, end_err);
        MAPNIK_LOG_ERROR(error_handler) << s.str();
#endif
        return boost::spirit::qi::fail;
    }
};

}}

#endif // MAPNIK_JSON_ERROR_HANDLER_HPP
