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

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/params.hpp>
#include <mapnik/util/conversions.hpp>
#include "shape_utils.hpp"

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/algorithm/string.hpp>
MAPNIK_DISABLE_WARNING_POP

void setup_attributes(mapnik::context_ptr const& ctx,
                      std::set<std::string> const& names,
                      std::string const& shape_name,
                      shape_io & shape,
                      std::vector<int> & attr_ids)
{
    std::set<std::string>::const_iterator pos = names.begin();
    std::set<std::string>::const_iterator end = names.end();
    for ( ;pos !=end; ++pos)
    {
        bool found_name = false;
        for (int i = 0; i < shape.dbf().num_fields(); ++i)
        {
            if (shape.dbf().descriptor(i).name_ == *pos)
            {
                ctx->push(*pos);
                attr_ids.push_back(i);
                found_name = true;
                break;
            }
        }

        if (! found_name)
        {
            std::string s("no attribute '");
            std::string pos_string;
            s += *pos + "' in '" + shape_name + "'. Valid attributes are: ";
            std::vector<std::string> list;
            for (int i = 0; i < shape.dbf().num_fields(); ++i)
            {
                list.push_back(shape.dbf().descriptor(i).name_);
            }
            s += boost::algorithm::join(list, ",") + ".";

            throw mapnik::datasource_exception("Shape Plugin: " + s);
        }
    }
}
