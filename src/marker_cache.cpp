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
#include <mapnik/svg/svg_parser.hpp>
#include <mapnik/svg/svg_storage.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/image_reader.hpp>

// boost
#include <boost/assert.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>

// agg
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"

namespace mapnik
{

marker_cache::marker_cache()
    : known_svg_prefix_("shape://")
{
    insert_svg("ellipse",
               "<?xml version='1.0' standalone='no'?>"
               "<svg width='100%' height='100%' version='1.1' xmlns='http://www.w3.org/2000/svg'>"
               "<ellipse rx='5' ry='5' fill='#0000FF' stroke='black' stroke-width='.5'/>"
               "</svg>");
    insert_svg("arrow",
               "<?xml version='1.0' standalone='no'?>"
               "<svg width='100%' height='100%' version='1.1' xmlns='http://www.w3.org/2000/svg'>"
               "<path fill='#0000FF' stroke='black' stroke-width='.5' d='m 31.698405,7.5302648 -8.910967,-6.0263712 0.594993,4.8210971 -18.9822542,0 0,2.4105482 18.9822542,0 -0.594993,4.8210971 z'/>"
               "</svg>");
}

marker_cache::~marker_cache() {}

void marker_cache::clear()
{
#ifdef MAPNIK_THREADSAFE
    mutex::scoped_lock lock(mutex_);
#endif
    typedef boost::unordered_map<std::string, marker_ptr>::const_iterator iterator_type;
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

bool marker_cache::is_uri(std::string const& path)
{
    return boost::algorithm::starts_with(path,known_svg_prefix_);
}

bool marker_cache::insert_svg(std::string const& name, std::string const& svg_string)
{
    std::string key = known_svg_prefix_ + name;
    typedef boost::unordered_map<std::string, std::string>::const_iterator iterator_type;
    iterator_type itr = svg_cache_.find(key);
    if (itr == svg_cache_.end())
    {
        return svg_cache_.insert(std::make_pair(key,svg_string)).second;
    }
    return false;
}

bool marker_cache::insert_marker(std::string const& uri, marker_ptr path)
{
#ifdef MAPNIK_THREADSAFE
    mutex::scoped_lock lock(mutex_);
#endif
    return marker_cache_.insert(std::make_pair(uri,path)).second;
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
    typedef boost::unordered_map<std::string, marker_ptr>::const_iterator iterator_type;
    iterator_type itr = marker_cache_.find(uri);
    if (itr != marker_cache_.end())
    {
        result.reset(itr->second);
        return result;
    }

    try
    {
        // if uri references a built-in marker
        if (is_uri(uri))
        {
            boost::unordered_map<std::string, std::string>::const_iterator mark_itr = svg_cache_.find(uri);
            if (mark_itr == svg_cache_.end())
            {
                MAPNIK_LOG_ERROR(marker_cache) << "Marker does not exist: " << uri;
                return result;
            }
            std::string known_svg_string = mark_itr->second;
            using namespace mapnik::svg;
            svg_path_ptr marker_path(boost::make_shared<svg_storage_type>());
            vertex_stl_adapter<svg_path_storage> stl_storage(marker_path->source());
            svg_path_adapter svg_path(stl_storage);
            svg_converter_type svg(svg_path, marker_path->attributes());
            svg_parser p(svg);
            p.parse_from_string(known_svg_string);
            //svg.arrange_orientations();
            double lox,loy,hix,hiy;
            svg.bounding_rect(&lox, &loy, &hix, &hiy);
            marker_path->set_bounding_box(lox,loy,hix,hiy);
            marker_ptr mark(boost::make_shared<marker>(marker_path));
            result.reset(mark);
            if (update_cache)
            {
                marker_cache_.insert(std::make_pair(uri,*result));
            }
        }
        // otherwise assume file-based
        else
        {
            boost::filesystem::path path(uri);
            if (!exists(path))
            {
                MAPNIK_LOG_ERROR(marker_cache) << "Marker does not exist: " << uri;
                return result;
            }
            if (is_svg(uri))
            {
                using namespace mapnik::svg;
                svg_path_ptr marker_path(boost::make_shared<svg_storage_type>());
                vertex_stl_adapter<svg_path_storage> stl_storage(marker_path->source());
                svg_path_adapter svg_path(stl_storage);
                svg_converter_type svg(svg_path, marker_path->attributes());
                svg_parser p(svg);
                p.parse(uri);
                //svg.arrange_orientations();
                double lox,loy,hix,hiy;
                svg.bounding_rect(&lox, &loy, &hix, &hiy);
                marker_path->set_bounding_box(lox,loy,hix,hiy);
                marker_ptr mark(boost::make_shared<marker>(marker_path));
                result.reset(mark);
                if (update_cache)
                {
                    marker_cache_.insert(std::make_pair(uri,*result));
                }
            }
            else
            {
                // TODO - support reading images from string
                std::auto_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(uri));
                if (reader.get())
                {
                    unsigned width = reader->width();
                    unsigned height = reader->height();
                    BOOST_ASSERT(width > 0 && height > 0);
                    mapnik::image_ptr image(boost::make_shared<mapnik::image_data_32>(width,height));
                    reader->read(0,0,*image);
                    if (!reader->premultiplied_alpha())
                    {
                        agg::rendering_buffer buffer(image->getBytes(),image->width(),image->height(),image->width() * 4);
                        agg::pixfmt_rgba32 pixf(buffer);
                        pixf.premultiply();
                    }
                    marker_ptr mark(boost::make_shared<marker>(image));
                    result.reset(mark);
                    if (update_cache)
                    {
                        marker_cache_.insert(std::make_pair(uri,*result));
                    }
                }
                else
                {
                    MAPNIK_LOG_ERROR(marker_cache) << "could not intialize reader for: '" << uri << "'";
                }
            }
        }
    }
    catch (std::exception const& ex)
    {
        MAPNIK_LOG_ERROR(marker_cache) << "Exception caught while loading: '" << uri << "' (" << ex.what() << ")";
    }
    catch (...)
    {
        MAPNIK_LOG_ERROR(marker_cache) << "Exception caught while loading: '" << uri << "'";
    }
    return result;
}

}
