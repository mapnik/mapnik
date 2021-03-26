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

#include <mapnik/proj_transform_cache.hpp>

namespace mapnik {

thread_local proj_transform_cache::cache_type proj_transform_cache::cache_ = cache_type();

void proj_transform_cache::init(std::string const& source, std::string const& dest) const
{
    compatible_key_type key = std::make_pair<boost::string_view, boost::string_view>(source, dest);
    auto itr = cache_.find(key, compatible_hash{}, compatible_predicate{});
    if (itr == cache_.end())
    {
        mapnik::projection p0(source, true);
        mapnik::projection p1(dest, true);
        cache_.emplace(std::make_pair(source, dest),
                       std::make_unique<proj_transform>(p0, p1));
    }
}

proj_transform const* proj_transform_cache::get(std::string const& source, std::string const& dest) const
{

    compatible_key_type key = std::make_pair<boost::string_view, boost::string_view>(source, dest);
    auto itr = cache_.find(key, compatible_hash{}, compatible_predicate{});
    if (itr == cache_.end())
    {
        mapnik::projection srs1(source, true);
        mapnik::projection srs2(dest, true);
        return cache_.emplace(std::make_pair(source, dest),
                              std::make_unique<proj_transform>(srs1, srs2)).first->second.get();
    }
    return itr->second.get();
}


} // namespace mapnik
