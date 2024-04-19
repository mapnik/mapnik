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
#include <mapnik/proj_transform.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>
MAPNIK_DISABLE_WARNING_POP
#include <string_view>

namespace mapnik {
namespace proj_transform_cache {
namespace {
using key_type = std::pair<std::string, std::string>;
using compatible_key_type = std::pair<std::string_view, std::string_view>;

struct compatible_hash
{
    template<typename KeyType>
    std::size_t operator()(KeyType const& key) const
    {
        using hash_type = boost::hash<typename KeyType::first_type>;
        std::size_t seed = hash_type{}(key.first);
        seed ^= hash_type{}(key.second) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

struct compatible_predicate
{
    bool operator()(compatible_key_type const& k1, compatible_key_type const& k2) const { return k1 == k2; }
};

using cache_type = boost::unordered_map<key_type, std::unique_ptr<proj_transform>, compatible_hash>;
thread_local static cache_type cache_ = {};

} // namespace

void init(std::string const& source, std::string const& dest)
{
    compatible_key_type key = std::make_pair<std::string_view, std::string_view>(source, dest);
    auto itr = cache_.find(key, compatible_hash{}, compatible_predicate{});
    if (itr == cache_.end())
    {
        mapnik::projection p0(source, true);
        mapnik::projection p1(dest, true);
        cache_.emplace(std::make_pair(source, dest), std::make_unique<proj_transform>(p0, p1));
    }
}

proj_transform const* get(std::string const& source, std::string const& dest)
{
    compatible_key_type key = std::make_pair<std::string_view, std::string_view>(source, dest);
    auto itr = cache_.find(key, compatible_hash{}, compatible_predicate{});
    if (itr == cache_.end())
    {
        mapnik::projection srs1(source, true);
        mapnik::projection srs2(dest, true);
        return cache_.emplace(std::make_pair(source, dest), std::make_unique<proj_transform>(srs1, srs2))
          .first->second.get();
    }
    return itr->second.get();
}

} // namespace proj_transform_cache
} // namespace mapnik
