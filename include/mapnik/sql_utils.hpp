/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2010 Artem Pavlenko
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

//$Id: sql_utils.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef SQL_UTILS_HPP
#define SQL_UTILS_HPP

// boost
#include <boost/algorithm/string.hpp>

namespace mapnik
{

MAPNIK_DECL std::string table_from_sql(const std::string& sql)
{
    std::string table_name = boost::algorithm::to_lower_copy(sql);
    boost::algorithm::replace_all(table_name,"\n"," ");
   
    std::string::size_type idx = table_name.rfind(" from ");
    if (idx!=std::string::npos)
    {
      
        idx = table_name.find_first_not_of(" ",idx+5);
        if (idx != std::string::npos)
        {
            table_name=table_name.substr(idx);
        }
        idx = table_name.find_first_of(" )");
        if (idx != std::string::npos)
        {
            table_name = table_name.substr(0,idx);
        }
    }
    return table_name;
}

}

#endif //SQL_UTILS_HPP
