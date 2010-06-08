/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2010 Artem Pavlenko
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
#include <mapnik/svg/marker_cache.hpp>
#include <mapnik/svg/svg_parser.hpp>
#include <mapnik/svg/svg_storage.hpp>
#include <mapnik/svg/svg_converter.hpp>

// boost
#include <boost/assert.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>

namespace mapnik 
{

boost::unordered_map<std::string, path_ptr> marker_cache::cache_;

bool marker_cache::insert (std::string const& uri, path_ptr path)
{
#ifdef MAPNIK_THREADSAFE
    mutex::scoped_lock lock(mutex_);
#endif
    return cache_.insert(std::make_pair(uri,path)).second;
}

boost::optional<path_ptr> marker_cache::find(std::string const& uri, bool update_cache)
{
#ifdef MAPNIK_THREADSAFE
    mutex::scoped_lock lock(mutex_);
#endif
    typedef boost::unordered_map<std::string, path_ptr>::const_iterator iterator_type;
    boost::optional<path_ptr> result;
    iterator_type itr = cache_.find(uri);
    if (itr != cache_.end())
    {
        result.reset(itr->second);
        return result;
    }

    // we can't find marker in cache, lets try to load it from filesystem
    boost::filesystem::path path(uri);
    if (exists(path))
    {
        try 
        {
            mapnik::path_ptr marker(new svg_storage_type);
            svg::svg_converter_type svg(marker->source(),marker->attributes());

            svg::svg_parser p(svg);
            p.parse(uri);            
            svg.arrange_orientations();
            double lox,loy,hix,hiy;
            svg.bounding_rect(&lox, &loy, &hix, &hiy); //TODO: store bbox!
            marker->set_bounding_box(lox,loy,hix,hiy);
            if (update_cache)
            {
                cache_.insert(std::make_pair(uri,marker));
            }
            return marker;
        }
        catch (...)
        {
            std::cerr << "Exception caught while loading SVG: " << uri << std::endl;
        }
    }
    else
    {
        std::cerr << "### WARNING SVG does not exist: " << uri << std::endl;
    }
    return result;
}

#ifdef MAPNIK_THREADSAFE
boost::mutex marker_cache::mutex_;
#endif

}
