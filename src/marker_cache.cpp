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

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/image_util.hpp>

// boost
#include <boost/assert.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>

namespace mapnik
{

marker_cache::marker_cache()
    : known_svg_prefix_("shape://"),
      known_image_prefix_("image://")
{
    init();
}

marker_cache::~marker_cache() {}

void marker_cache::init()
{
    std::string ellipse =
               "<?xml version='1.0' standalone='no'?>"
               "<svg width='100%' height='100%' version='1.1' xmlns='http://www.w3.org/2000/svg'>"
               "<ellipse rx='5' ry='5' fill='#0000FF' stroke='black' stroke-width='.5'/>"
               "</svg>";
    marker_cache_.insert(std::make_pair("shape://ellipse",boost::make_shared<marker>(read_svg_marker(ellipse,true))));
    std::string arrow =
               "<?xml version='1.0' standalone='no'?>"
               "<svg width='100%' height='100%' version='1.1' xmlns='http://www.w3.org/2000/svg'>"
               "<path fill='#0000FF' stroke='black' stroke-width='.5' d='m 31.698405,7.5302648 -8.910967,-6.0263712 0.594993,4.8210971 -18.9822542,0 0,2.4105482 18.9822542,0 -0.594993,4.8210971 z'/>"
               "</svg>";
    marker_cache_.insert(std::make_pair("shape://arrow",boost::make_shared<marker>(read_svg_marker(arrow,true))));
}

void marker_cache::clear()
{
#ifdef MAPNIK_THREADSAFE
    mutex::scoped_lock lock(mutex_);
#endif
    iterator_type itr = marker_cache_.begin();
    while(itr != marker_cache_.end())
    {
        if (!is_uri(itr->first))
        {
           marker_cache_.erase(itr++);
        }
        else
        {
            ++itr;
        }
    }
}

bool marker_cache::remove(std::string const& uri)
{
#ifdef MAPNIK_THREADSAFE
    mutex::scoped_lock lock(mutex_);
#endif
    iterator_type itr = marker_cache_.find(uri);
    if (itr != marker_cache_.end())
    {
        marker_cache_.erase(itr);
        return true;
    }
    return false;
}

bool marker_cache::is_uri(std::string const& uri)
{
    return is_svg_uri(uri) || is_image_uri(uri);
}

bool marker_cache::is_svg_uri(std::string const& uri)
{
    return boost::algorithm::starts_with(uri,known_svg_prefix_);
}

bool marker_cache::is_image_uri(std::string const& uri)
{
    return boost::algorithm::starts_with(uri,known_image_prefix_);
}

bool marker_cache::insert_marker(std::string const& uri, marker_ptr path, bool override)
{
#ifdef MAPNIK_THREADSAFE
    mutex::scoped_lock lock(mutex_);
#endif
    if (!override)
    {
        return marker_cache_.insert(std::make_pair(uri,path)).second;
    }
    else
    {
        typedef boost::unordered_map<std::string, marker_ptr>::iterator non_const_iterator_type;
        std::pair<non_const_iterator_type,bool> result = marker_cache_.insert(std::make_pair(uri,path));
        if (!result.second)
        {
            result.first->second = path;
        }
        return result.second;
    }
}

boost::optional<marker_ptr> marker_cache::find(std::string const& uri,
                                               bool update_cache)
{
    boost::optional<marker_ptr> result;
    if (uri.empty())
    {
        return result;
    }
#ifdef MAPNIK_THREADSAFE
    mutex::scoped_lock lock(mutex_);
#endif
    iterator_type itr = marker_cache_.find(uri);
    if (itr != marker_cache_.end())
    {
        result.reset(itr->second);
        return result;
    }
    try
    {
        if (boost::filesystem::exists(boost::filesystem::path(uri)))
        {
            if (is_svg(uri))
            {
                result.reset(boost::make_shared<marker>(read_svg_marker(uri)));
            }
            else
            {
                result.reset(boost::make_shared<marker>(read_bitmap_marker(uri)));
            }
        }
        if (result)
        {
            if (update_cache)
            {
                marker_cache_.insert(std::make_pair(uri,*result));
            }
            return result;
        }
        else
        {
            MAPNIK_LOG_ERROR(marker_cache) << "Marker does not exist: " << uri;
            return result;
        }
    }
    catch (std::exception const& ex)
    {
        MAPNIK_LOG_ERROR(marker_cache) << ex.what();
    }
    return result;
}

}
