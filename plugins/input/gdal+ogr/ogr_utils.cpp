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

#include "ogr_utils.hpp"
#include <mapnik/datasource.hpp>

std::vector<ogr_utils::option_ptr> ogr_utils::split_open_options(std::string const& options)
{
    size_t i;
    bool escaped = false; // if previous character was a backslash to escape a backslash or space
    std::vector<ogr_utils::option_ptr> opts;
    std::string unescaped_str; // copy of input string but unescaped
    for (i = 0; i < options.size(); ++i)
    {
        char current = options.at(i);
        if (current == '\\')
        {
            if (escaped)
            {
                unescaped_str.push_back(current);
            }
            escaped = !escaped;
        }
        else if (current != ' ')
        {
            unescaped_str.push_back(current);
        }
        if (current == ' ' || i + 1 == options.size())
        {
            if (!escaped)
            {
                size_t count = unescaped_str.size();
                if (count > 0)
                {
                    option_ptr opt(new char[count + 1], [](char* arr) { delete[] arr; });
                    unescaped_str.copy(opt.get(), count);
                    opt[count] = '\0';
                    opts.push_back(std::move(opt));
                }
                unescaped_str = "";
            }
            else
            {
                escaped = false;
                unescaped_str.push_back(current);
            }
        }
    }
    if (escaped)
    {
        throw mapnik::datasource_exception("<open_options> parameter ends with single backslash");
    }
    opts.emplace_back(nullptr);
    return opts;
}

char** ogr_utils::open_options_for_ogr(std::vector<ogr_utils::option_ptr> const& options)
{
    char** for_ogr = new char*[options.size() + 1];
    for (size_t i = 0; i < options.size(); ++i)
    {
        for_ogr[i] = options.at(i).get();
    }
    for_ogr[options.size()] = nullptr;
    return for_ogr;
}
