/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

#include <string>
#include <sstream>
#include <boost/spirit/home/support/info.hpp>

namespace mapnik { namespace json {

template <typename Iterator>
struct error_handler
{
    using result_type = void;
    void operator() (
        Iterator, Iterator,
        Iterator err_pos, boost::spirit::info const& what) const
    {
        std::stringstream s;
        auto start = err_pos;
        std::advance(err_pos,16);
        auto end = err_pos;
        s << what << " expected but got: " << std::string(start, end);
        throw std::runtime_error(s.str());
    }
};

}}

#endif // MAPNIK_JSON_ERROR_HANDLER_HPP
