/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#ifndef MAPNIK_MARKER_CACHE_HPP
#define MAPNIK_MARKER_CACHE_HPP

// mapnik
#include <mapnik/utils.hpp>
#include <mapnik/config.hpp>
#include <mapnik/noncopyable.hpp>

// boost
#include <boost/unordered_map.hpp>
#include <memory>
#include <boost/optional.hpp>

namespace mapnik
{

class marker;

using marker_ptr = std::shared_ptr<marker>;


class MAPNIK_DECL marker_cache :
        public singleton <marker_cache, CreateUsingNew>,
        private mapnik::noncopyable
{
    friend class CreateUsingNew<marker_cache>;
private:
    marker_cache();
    ~marker_cache();
    bool insert_marker(std::string const& key, marker_ptr path);
    boost::unordered_map<std::string,marker_ptr> marker_cache_;
    bool insert_svg(std::string const& name, std::string const& svg_string);
    boost::unordered_map<std::string,std::string> svg_cache_;
public:
    std::string known_svg_prefix_;
    std::string known_image_prefix_;
    inline bool is_uri(std::string const& path) { return is_svg_uri(path) || is_image_uri(path); }
    bool is_svg_uri(std::string const& path);
    bool is_image_uri(std::string const& path);
    boost::optional<marker_ptr> find(std::string const& key, bool update_cache = false);
    void clear();
};

}

#endif // MAPNIK_MARKER_CACHE_HPP
