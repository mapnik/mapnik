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

#ifndef MAPNIK_SQL_UTILS_HPP
#define MAPNIK_SQL_UTILS_HPP

// mapnik
#include <mapnik/util/trim.hpp> // for trim

// boost
#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/algorithm/string/replace.hpp>
#pragma GCC diagnostic pop

// stl
#include <sstream>
#include <vector>
#include <string>

namespace mapnik { namespace sql_utils {

    inline std::string unquote_double(std::string const& sql)
    {
        std::string table_name = sql;
        util::unquote_double(table_name);
        return table_name;
    }

    inline std::string unquote(std::string const& sql)
    {
        std::string table_name = sql;
        util::unquote(table_name);
        return table_name;
    }

    inline void quote_attr(std::ostringstream & s, std::string const& field)
    {
        s << ",\"" << field << "\"";
    }

    inline std::string table_from_sql(std::string const& sql)
    {
        std::string table_name = sql;
        boost::algorithm::replace_all(table_name,"\n"," ");
        boost::algorithm::ireplace_all(table_name," from "," FROM ");

        std::string::size_type idx = table_name.rfind(" FROM ");
        if (idx!=std::string::npos)
        {
            idx = table_name.find_first_not_of(" ",idx+5);
            if (idx != std::string::npos)
            {
                table_name=table_name.substr(idx);
            }
            idx = table_name.find_first_of(", )");
            if (idx != std::string::npos)
            {
                table_name = table_name.substr(0,idx);
            }
        }
        return table_name;
    }
}}

#endif // MAPNIK_SQL_UTILS_HPP
