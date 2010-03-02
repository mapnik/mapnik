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

// mapnik
#include <mapnik/image_cache.hpp>
#include <mapnik/image_reader.hpp>

// boost
#include <boost/assert.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>

namespace mapnik 
{

boost::unordered_map<std::string, image_ptr> image_cache::cache_;

bool image_cache::insert (std::string const& uri, image_ptr img)
{
#ifdef MAPNIK_THREADSAFE
    mutex::scoped_lock lock(mutex_);
#endif
    return cache_.insert(std::make_pair(uri,img)).second;
}

boost::optional<image_ptr> image_cache::find(std::string const& uri, bool update_cache)
{
#ifdef MAPNIK_THREADSAFE
    mutex::scoped_lock lock(mutex_);
#endif
    typedef boost::unordered_map<std::string, image_ptr>::const_iterator iterator_type;
    boost::optional<image_ptr> result;
    iterator_type itr = cache_.find(uri);
    if (itr != cache_.end())
    {
	result.reset(itr->second);
	return result;
    }

    // we can't find image in cache, lets try to load it from filesystem
    boost::filesystem::path path(uri);
    if (exists(path))
    {
	try 
	{
	    std::auto_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(uri));
	    if (reader.get())
	    {
		unsigned width = reader->width();
		unsigned height = reader->height();
		BOOST_ASSERT(width > 0 && height > 0);
		mapnik::image_ptr image(new mapnik::image_data_32(width,height));
		reader->read(0,0,*image);
		result.reset(image);
		if (update_cache)
		{
		    cache_.insert(std::make_pair(uri,image));
		}
	    }
	}
	
	catch (...)
	{
	    std::cerr << "Exception caught while loading image: " << uri << std::endl;
	}
    }
    else
    {
        std::cerr << "### WARNING image does not exist: " << uri << std::endl;
    }
    return result;
}

#ifdef MAPNIK_THREADSAFE
   boost::mutex image_cache::mutex_;
#endif

}
