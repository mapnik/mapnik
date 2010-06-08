/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2009 Artem Pavlenko
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

//$Id$

#ifndef MAPNIK_MARKER_CACHE_HPP
#define MAPNIK_MARKER_CACHE_HPP

// mapnik
#include <mapnik/utils.hpp>
#include <mapnik/config.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/svg/svg_storage.hpp>
// agg
#include "agg_path_storage.h"
// boost
#include <boost/utility.hpp>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <boost/thread/mutex.hpp>

namespace mapnik
{

typedef agg::path_storage path_storage;
typedef agg::pod_bvector<mapnik::svg::path_attributes> attr_storage;
typedef mapnik::svg::svg_storage<path_storage,attr_storage> svg_storage_type;
typedef boost::shared_ptr<svg_storage_type> path_ptr;

struct MAPNIK_DECL marker_cache :
        public singleton <marker_cache, CreateStatic>,
        private boost::noncopyable
{

    friend class CreateStatic<marker_cache>;
    static boost::mutex mutex_;
    static boost::unordered_map<std::string,path_ptr> cache_;
    static bool insert(std::string const& key, path_ptr);
    static boost::optional<path_ptr> find(std::string const& key, bool update_cache = false);
};

}

#endif // MAPNIK_MARKER_CACHE_HPP
