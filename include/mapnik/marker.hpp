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

#ifndef MAPNIK_MARKER_HPP
#define MAPNIK_MARKER_HPP

// mapnik
#include <mapnik/global.hpp>
#include <mapnik/image_data.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/svg/svg_storage.hpp>
#include <mapnik/noncopyable.hpp>

// boost
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>

// stl
#include <cassert>
#include <cstring>

namespace mapnik
{

typedef agg::pod_bvector<mapnik::svg::path_attributes> attr_storage;
typedef mapnik::svg::svg_storage<mapnik::svg::svg_path_storage,attr_storage> svg_storage_type;
typedef boost::shared_ptr<svg_storage_type> svg_path_ptr;
typedef boost::shared_ptr<image_data_32> image_ptr;

class marker;

typedef boost::shared_ptr<marker> marker_ptr;

marker_ptr read_svg_marker(std::string const& uri, bool from_string=false);
marker_ptr read_bitmap_marker(std::string const& uri, bool from_string=false);

/**
 * A class to hold either vector or bitmap marker data. This allows these to be treated equally
 * in the image caches and most of the render paths.
 */
class marker: private mapnik::noncopyable
{
public:
    marker();
    marker(boost::optional<mapnik::image_ptr> const& data);
    marker(boost::optional<mapnik::svg_path_ptr> const& data);
    marker(marker const& rhs);
    box2d<double> bounding_box() const;
    double width() const;
    double height() const;
    bool is_bitmap() const;
    bool is_vector() const;
    boost::optional<mapnik::image_ptr> get_bitmap_data() const;
    boost::optional<mapnik::svg_path_ptr> get_vector_data() const;
private:
    boost::optional<mapnik::image_ptr> bitmap_data_;
    boost::optional<mapnik::svg_path_ptr> vector_data_;

};


}

#endif // MAPNIK_MARKER_HPP
