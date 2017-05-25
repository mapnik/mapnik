/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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
#include <mapnik/util/fs.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/assert.hpp>
#include <boost/algorithm/string.hpp>
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#pragma GCC diagnostic pop

namespace mapnik
{

marker_cache::marker_cache()
    : known_svg_prefix_("shape://"),
      known_image_prefix_("image://")
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
    marker_cache_.emplace("image://square",std::make_shared<mapnik::marker const>(mapnik::marker_rgba8()));
}

marker_cache::~marker_cache() {}

void marker_cache::clear()
{
#ifdef MAPNIK_THREADSAFE
    std::lock_guard<std::mutex> lock(mutex_);
#endif
    auto itr = marker_cache_.begin();
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

bool marker_cache::is_svg_uri(std::string const& path)
{
    return boost::algorithm::starts_with(path,known_svg_prefix_);
}

bool marker_cache::is_image_uri(std::string const& path)
{
    return boost::algorithm::starts_with(path,known_image_prefix_);
}

bool marker_cache::insert_svg(std::string const& name, std::string const& svg_string)
{
    std::string key = known_svg_prefix_ + name;
    auto itr = svg_cache_.find(key);
    if (itr == svg_cache_.end())
    {
        return svg_cache_.emplace(key,svg_string).second;
    }
    return false;
}

bool marker_cache::insert_marker(std::string const& uri, mapnik::marker && path)
{
#ifdef MAPNIK_THREADSAFE
    std::lock_guard<std::mutex> lock(mutex_);
#endif
    return marker_cache_.emplace(uri,std::make_shared<mapnik::marker const>(std::move(path))).second;
}

namespace detail
{

struct visitor_create_marker
{
    marker operator() (image_rgba8 & data) const
    {
        mapnik::premultiply_alpha(data);
        return mapnik::marker(mapnik::marker_rgba8(data));
    }

    marker operator() (image_null &) const
    {
        throw std::runtime_error("Can not make marker from null image data type");
    }

    template <typename T>
    marker operator() (T &) const
    {
        throw std::runtime_error("Can not make marker from this data type");
    }
};

} // end detail ns

std::shared_ptr<mapnik::marker const> marker_cache::find(std::string const& uri,
                                                         bool update_cache, bool strict)
{
    if (uri.empty())
    {
        return std::make_shared<mapnik::marker const>(mapnik::marker_null());
    }

#ifdef MAPNIK_THREADSAFE
    std::lock_guard<std::mutex> lock(mutex_);
#endif
    auto itr = marker_cache_.find(uri);
    if (itr != marker_cache_.end())
    {
        return itr->second;
    }

    try
    {
        // if uri references a built-in marker
        if (is_svg_uri(uri))
        {
            auto mark_itr = svg_cache_.find(uri);
            if (mark_itr == svg_cache_.end())
            {
                MAPNIK_LOG_ERROR(marker_cache) << "Marker does not exist: " << uri;
                return std::make_shared<mapnik::marker const>(mapnik::marker_null());
            }
            std::string known_svg_string = mark_itr->second;
            using namespace mapnik::svg;
            svg_path_ptr marker_path(std::make_shared<svg_storage_type>());
            vertex_stl_adapter<svg_path_storage> stl_storage(marker_path->source());
            svg_path_adapter svg_path(stl_storage);
            svg_converter_type svg(svg_path, marker_path->attributes());
            svg_parser p(svg, strict);

            if (!p.parse_from_string(known_svg_string) && !strict)
            {
                for (auto const& msg : p.error_messages())
                {
                    MAPNIK_LOG_ERROR(marker_cache) <<  "SVG PARSING ERROR:\"" << msg << "\"";
                }
                //return std::make_shared<mapnik::marker const>(mapnik::marker_null());
            }
            //svg.arrange_orientations();
            double lox,loy,hix,hiy;
            svg.bounding_rect(&lox, &loy, &hix, &hiy);
            marker_path->set_bounding_box(lox,loy,hix,hiy);
            marker_path->set_dimensions(svg.width(),svg.height());
            if (update_cache)
            {
                auto emplace_result = marker_cache_.emplace(uri,std::make_shared<mapnik::marker const>(mapnik::marker_svg(marker_path)));
                return emplace_result.first->second;
            }
            else
            {
                return std::make_shared<mapnik::marker const>(mapnik::marker_svg(marker_path));
            }
        }
        // otherwise assume file-based
        else
        {
            if (!mapnik::util::exists(uri))
            {
                MAPNIK_LOG_ERROR(marker_cache) << "Marker does not exist: " << uri;
                return std::make_shared<mapnik::marker const>(mapnik::marker_null());
            }
            if (is_svg(uri))
            {
                using namespace mapnik::svg;
                svg_path_ptr marker_path(std::make_shared<svg_storage_type>());
                vertex_stl_adapter<svg_path_storage> stl_storage(marker_path->source());
                svg_path_adapter svg_path(stl_storage);
                svg_converter_type svg(svg_path, marker_path->attributes());
                svg_parser p(svg, strict);


                if (!p.parse(uri) && !strict)
                {
                    for (auto const& msg : p.error_messages())
                    {
                        MAPNIK_LOG_ERROR(marker_cache) <<  "SVG PARSING ERROR:\"" << msg << "\"";
                    }
                    //return std::make_shared<mapnik::marker const>(mapnik::marker_null());
                }
                //svg.arrange_orientations();
                double lox,loy,hix,hiy;
                svg.bounding_rect(&lox, &loy, &hix, &hiy);
                marker_path->set_bounding_box(lox,loy,hix,hiy);
                marker_path->set_dimensions(svg.width(),svg.height());
                if (update_cache)
                {
                    auto emplace_result = marker_cache_.emplace(uri,std::make_shared<mapnik::marker const>(mapnik::marker_svg(marker_path)));
                    return emplace_result.first->second;
                }
                else
                {
                    return std::make_shared<mapnik::marker const>(mapnik::marker_svg(marker_path));
                }
            }
            else
            {
                // TODO - support reading images from string
                std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(uri));
                if (reader.get())
                {
                    unsigned width = reader->width();
                    unsigned height = reader->height();
                    BOOST_ASSERT(width > 0 && height > 0);
                    image_any im = reader->read(0,0,width,height);
                    if (update_cache)
                    {
                        auto emplace_result = marker_cache_.emplace(uri,
                                std::make_shared<mapnik::marker const>(
                                    util::apply_visitor(detail::visitor_create_marker(), im)
                                ));
                        return emplace_result.first->second;
                    }
                    else
                    {
                        return std::make_shared<mapnik::marker const>(
                            util::apply_visitor(detail::visitor_create_marker(), im)
                        );
                    }
                }
                else
                {
                    MAPNIK_LOG_ERROR(marker_cache) << "could not initialize reader for: '" << uri << "'";
                    return std::make_shared<mapnik::marker const>(mapnik::marker_null());
                }
            }
        }
    }
    catch (std::exception const& ex)
    {
        MAPNIK_LOG_ERROR(marker_cache) << "Exception caught while loading: '" << uri << "' (" << ex.what() << ")";
    }
    return std::make_shared<mapnik::marker const>(mapnik::marker_null());
}

}
