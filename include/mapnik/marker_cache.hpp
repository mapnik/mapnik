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
#include <mapnik/marker.hpp>
#include <mapnik/config.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/svg/svg_storage.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
// agg
#include "agg_path_storage.h"
// boost
#include <boost/utility.hpp>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#ifdef MAPNIK_THREADSAFE
#include <boost/thread/mutex.hpp>
#endif

namespace mapnik
{

using namespace mapnik::svg;

typedef boost::shared_ptr<marker> marker_ptr;


struct MAPNIK_DECL marker_cache :
        public singleton <marker_cache, CreateStatic>,
        private boost::noncopyable
{

    friend class CreateStatic<marker_cache>;
#ifdef MAPNIK_THREADSAFE
    static boost::mutex mutex_;
#endif
    static boost::unordered_map<std::string,marker_ptr> cache_;
    static bool insert(std::string const& key, marker_ptr);
    static boost::optional<marker_ptr> find(std::string const& key, bool update_cache = false);
};

}

#endif // MAPNIK_MARKER_CACHE_HPP
