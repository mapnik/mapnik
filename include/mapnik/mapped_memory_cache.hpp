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

//$Id$

#ifndef MEMORY_REGION_CACHE_HPP
#define MEMORY_REGION_CACHE_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/utils.hpp>
// boost
#include <boost/utility.hpp>
#include <boost/unordered_map.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>

#ifdef MAPNIK_THREADSAFE
#include <boost/thread/mutex.hpp>
#endif

namespace mapnik
{

using namespace boost::interprocess;

typedef boost::shared_ptr<mapped_region> mapped_region_ptr;

struct MAPNIK_DECL mapped_memory_cache :
        public singleton <mapped_memory_cache, CreateStatic>,
        private boost::noncopyable
{
    friend class CreateStatic<mapped_memory_cache>;
#ifdef MAPNIK_THREADSAFE
    static boost::mutex mutex_;
#endif
    static boost::unordered_map<std::string,mapped_region_ptr> cache_;
    static bool insert(std::string const& key, mapped_region_ptr);
    static boost::optional<mapped_region_ptr> find(std::string const& key, bool update_cache = false);
};

}

#endif // MAPNIK_MARKER_CACHE_HPP

