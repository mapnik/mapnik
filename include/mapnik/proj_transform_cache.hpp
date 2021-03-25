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


#ifndef MAPNIK_PROJ_TRANSFORM_CACHE_HPP
#define MAPNIK_PROJ_TRANSFORM_CACHE_HPP

#include <mapnik/util/noncopyable.hpp>
#include <mapnik/proj_transform.hpp>

MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility/string_view.hpp>
MAPNIK_DISABLE_WARNING_POP

namespace mapnik  {

struct proj_transform_cache : util::noncopyable
{
    using key_type = std::pair<std::string, std::string>;
    using compatible_key_type = std::pair<boost::string_view, boost::string_view>;

    struct compatible_hash
    {
        template <typename KeyType>
        std::size_t operator() (KeyType const& key) const
        {
            using hash_type = boost::hash<typename KeyType::first_type>;
            std::size_t seed = hash_type{}(key.first);
            seed ^= hash_type{}(key.second) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };

    struct compatible_predicate
    {
        bool operator()(compatible_key_type const& k1,
                        compatible_key_type const& k2) const
        {
            return k1 == k2;
        }
    };

    using cache_type = boost::unordered_map<key_type, std::unique_ptr<proj_transform>, compatible_hash>;

    proj_transform_cache() = default;

    thread_local static cache_type cache_;
    void init(std::string const& source, std::string const& dest) const;
    proj_transform const* get(std::string const& source, std::string const& dest) const;
};

}

#endif // MAPNIK_PROJ_TRANSFORM_CACHE_HPP
