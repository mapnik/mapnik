/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
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
#include <mapnik/marker.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/svg/svg_parser.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/image_reader.hpp>

// agg
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"

// boost
#include <boost/make_shared.hpp>

namespace mapnik
{

mapnik::svg_path_ptr read_svg_marker(std::string const& uri, bool from_string)
{
    using namespace mapnik::svg;
    svg_path_ptr marker_path(boost::make_shared<svg_storage_type>());
    vertex_stl_adapter<svg_path_storage> stl_storage(marker_path->source());
    svg_path_adapter svg_path(stl_storage);
    svg_converter_type svg(svg_path, marker_path->attributes());
    svg_parser p(svg);
    if (from_string)
    {
        p.parse_from_string(uri);
    }
    else
    {
        p.parse(uri);
    }
    double lox,loy,hix,hiy;
    svg.bounding_rect(&lox, &loy, &hix, &hiy);
    marker_path->set_bounding_box(lox,loy,hix,hiy);
    marker_path->set_dimensions(svg.width(),svg.height());
    return marker_path;
}

mapnik::image_ptr read_bitmap_marker(std::string const& uri, bool from_string)
{
    mapnik::image_reader * reader = NULL;
    mapnik::image_reader_guard guard(reader);
    if (from_string)
    {
        reader = mapnik::get_image_reader(uri.data(),uri.size());
    }
    else
    {
        reader = mapnik::get_image_reader(uri);
    }
    if (!reader) throw std::runtime_error("could not intialize reader for: '" + uri + "'");
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
    return image;
}


marker::marker()
{
    // create default OGC 4x4 black pixel
    bitmap_data_ = boost::optional<mapnik::image_ptr>(boost::make_shared<image_data_32>(4,4));
    (*bitmap_data_)->set(0xff000000);
}

marker::marker(boost::optional<mapnik::image_ptr> const& data)
    : bitmap_data_(data)
{
}

marker::marker(boost::optional<mapnik::svg_path_ptr> const& data)
    : vector_data_(data)
{
}

marker::marker(marker const& rhs)
    : bitmap_data_(rhs.bitmap_data_),
      vector_data_(rhs.vector_data_)
{}

box2d<double> marker::bounding_box() const
{
    if (is_vector())
    {
        return (*vector_data_)->bounding_box();
    }
    if (is_bitmap())
    {
        double width = (*bitmap_data_)->width();
        double height = (*bitmap_data_)->height();
        return box2d<double>(0, 0, width, height);
    }
    return box2d<double>();
}


double marker::width() const
{
    if (is_bitmap())
        return (*bitmap_data_)->width();
    else if (is_vector())
        return (*vector_data_)->bounding_box().width();
    return 0;
}

double marker::height() const
{
    if (is_bitmap())
        return (*bitmap_data_)->height();
    else if (is_vector())
        return (*vector_data_)->bounding_box().height();
    return 0;
}

bool marker::is_bitmap() const
{
    return bitmap_data_;
}

bool marker::is_vector() const
{
    return vector_data_;
}

boost::optional<mapnik::image_ptr> marker::get_bitmap_data() const
{
    return bitmap_data_;
}

boost::optional<mapnik::svg_path_ptr> marker::get_vector_data() const
{
    return vector_data_;
}

}
