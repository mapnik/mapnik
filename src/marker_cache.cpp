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
#include <mapnik/marker_cache.hpp>
#include <mapnik/svg/svg_parser.hpp>
#include <mapnik/svg/svg_storage.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/image_reader.hpp>

// boost
#include <boost/assert.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>

namespace mapnik 
{

boost::unordered_map<std::string, marker_ptr> marker_cache::cache_;

bool marker_cache::insert (std::string const& uri, marker_ptr path)
{
#ifdef MAPNIK_THREADSAFE
    mutex::scoped_lock lock(mutex_);
#endif
    return cache_.insert(std::make_pair(uri,path)).second;
}

boost::optional<marker_ptr> marker_cache::find(std::string const& uri, bool update_cache)
{
#ifdef MAPNIK_THREADSAFE
    mutex::scoped_lock lock(mutex_);
#endif
    typedef boost::unordered_map<std::string, marker_ptr>::const_iterator iterator_type;
    boost::optional<marker_ptr> result;
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
        if (is_svg(uri))
        {
            using namespace mapnik::svg;
            try
            {
                path_ptr marker_path(new svg_storage_type);
                vertex_stl_adapter<svg_path_storage> stl_storage(marker_path->source());
                svg_path_adapter svg_path(stl_storage);
                svg_converter_type svg(svg_path, marker_path->attributes());

                svg_parser p(svg);
                p.parse(uri);
                //svg.arrange_orientations();
                double lox,loy,hix,hiy;
                svg.bounding_rect(&lox, &loy, &hix, &hiy);
                marker_path->set_bounding_box(lox,loy,hix,hiy);

                marker_ptr mark(new marker(marker_path));
                result.reset(mark);
                if (update_cache)
                {
                    cache_.insert(std::make_pair(uri,*result));
                }
                return result;
            }
            catch (...)
            {
                std::cerr << "Exception caught while loading SVG: " << uri << std::endl;
            }
        }
        else
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
                    marker_ptr mark(new marker(image));
                    result.reset(mark);
                    if (update_cache)
                    {
                        cache_.insert(std::make_pair(uri,*result));
                    }
                }
            }

            catch (...)
            {
                std::cerr << "Exception caught while loading image: " << uri << std::endl;
            }
        }
    }
    else
    {
        std::cerr << "### WARNING Marker does not exist: " << uri << std::endl;
    }
    return result;
}

#ifdef MAPNIK_THREADSAFE
boost::mutex marker_cache::mutex_;
#endif

}
