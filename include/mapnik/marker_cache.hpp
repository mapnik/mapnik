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

// boost
#include <boost/utility.hpp>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>

namespace mapnik
{

class marker;

typedef boost::shared_ptr<marker> marker_ptr;


class MAPNIK_DECL marker_cache :
        public singleton <marker_cache, CreateUsingNew>,
        private boost::noncopyable
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
    bool is_uri(std::string const& path);
    boost::optional<marker_ptr> find(std::string const& key, bool update_cache = false);
    void clear();
};

}

#endif // MAPNIK_MARKER_CACHE_HPP
