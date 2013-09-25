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

#if defined(SHAPE_MEMORY_MAPPED_FILE)

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/util/fs.hpp>
#include <mapnik/mapped_memory_cache.hpp>

// boost
#include <boost/assert.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/make_shared.hpp>

namespace mapnik
{

void mapped_memory_cache::clear()
{
#ifdef MAPNIK_THREADSAFE
    mutex::scoped_lock lock(mutex_);
#endif
    return cache_.clear();
}

bool mapped_memory_cache::insert(std::string const& uri, mapped_region_ptr mem)
{
#ifdef MAPNIK_THREADSAFE
    mutex::scoped_lock lock(mutex_);
#endif
    return cache_.insert(std::make_pair(uri,mem)).second;
}

boost::optional<mapped_region_ptr> mapped_memory_cache::find(std::string const& uri, bool update_cache)
{
#ifdef MAPNIK_THREADSAFE
    mutex::scoped_lock lock(mutex_);
#endif
    typedef boost::unordered_map<std::string, mapped_region_ptr>::const_iterator iterator_type;
    boost::optional<mapped_region_ptr> result;
    iterator_type itr = cache_.find(uri);
    if (itr != cache_.end())
    {
        result.reset(itr->second);
        return result;
    }

    if (mapnik::util::exists(uri))
    {
        try
        {
            boost::interprocess::file_mapping mapping(uri.c_str(),boost::interprocess::read_only);
            mapped_region_ptr region(std::make_shared<boost::interprocess::mapped_region>(mapping,boost::interprocess::read_only));

            result.reset(region);

            if (update_cache)
            {
                cache_.insert(std::make_pair(uri,*result));
            }
            return result;
        }
        catch (std::exception const& ex)
        {
            MAPNIK_LOG_ERROR(mapped_memory_cache)
                << "Error loading mapped memory file: '"
                << uri << "' (" << ex.what() << ")";
        }
    }
    /*
    else
    {
        MAPNIK_LOG_WARN(mapped_memory_cache) << "Memory region does not exist file: " << uri;
    }
    */
    return result;
}

}

#endif
