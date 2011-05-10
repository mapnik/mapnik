/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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

//$Id: image_data.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef MARKER_HPP
#define MARKER_HPP

#include <mapnik/global.hpp>
#include <mapnik/image_data.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/svg/svg_storage.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>

#include "agg_path_storage.h"
// boost
#include <boost/utility.hpp>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <boost/make_shared.hpp>

#include <cassert>
#include <cstring>

namespace mapnik 
{

typedef agg::pod_bvector<mapnik::svg::path_attributes> attr_storage;
typedef mapnik::svg::svg_storage<mapnik::svg::svg_path_storage,attr_storage> svg_storage_type;
typedef boost::shared_ptr<svg_storage_type> path_ptr;
typedef boost::shared_ptr<image_data_32> image_ptr;
/**
 * A class to hold either vector or bitmap marker data. This allows these to be treated equally
 * in the image caches and most of the render paths.
 */
class marker
{
public:
    marker()
    {
        // create default OGC 4x4 black pixel
        bitmap_data_ = boost::optional<mapnik::image_ptr>(boost::make_shared<image_data_32>(4,4));
        (*bitmap_data_)->set(0xff000000);
    }

    marker(const boost::optional<mapnik::image_ptr> &data) : bitmap_data_(data)
    {

    }

    marker(const boost::optional<mapnik::path_ptr> &data) : vector_data_(data)
    {

    }

    marker(const marker& rhs) : bitmap_data_(rhs.bitmap_data_), vector_data_(rhs.vector_data_)
    {
    }

    inline unsigned width() const
    {
        if (is_bitmap())
            return (*bitmap_data_)->width();
        else if (is_vector())
            return (*vector_data_)->bounding_box().width();
        return 0;
    }
    inline unsigned height() const
    {
        if (is_bitmap())
            return (*bitmap_data_)->height();
        else if (is_vector())
            return (*vector_data_)->bounding_box().height();
        return 0;
    }

    inline bool is_bitmap() const
    {
        return bitmap_data_;
    }

    inline bool is_vector() const
    {
        return vector_data_;
    }

    boost::optional<mapnik::image_ptr> get_bitmap_data()
    {
        return bitmap_data_;
    }

    boost::optional<mapnik::path_ptr> get_vector_data()
    {
        return vector_data_;
    }


private:
    marker& operator=(const marker&);

    boost::optional<mapnik::image_ptr> bitmap_data_;
    boost::optional<mapnik::path_ptr> vector_data_;

};

}

#endif //MARKER_HPP
