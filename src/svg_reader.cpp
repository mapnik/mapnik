/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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
#include <mapnik/image_reader.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/svg/svg_parser.hpp>
#include <mapnik/svg/svg_storage.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/svg/svg_renderer_agg.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>

#include "agg_rasterizer_scanline_aa.h"
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_renderer_base.h"
#include "agg_pixfmt_rgba.h"
#include "agg_scanline_u.h"

#include <libxml/parser.h> // for xmlInitParser(), xmlCleanupParser()

extern "C"
{
#include <png.h>
}
// boost
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#pragma GCC diagnostic pop

// stl
#include <cstring>
#include <memory>
#include <algorithm>

namespace mapnik
{

template <typename T>
class svg_reader : public image_reader
{
    using source_type = T;
    using input_stream = boost::iostreams::stream<source_type>;

private:

    source_type source_;
    input_stream stream_;
    unsigned width_;
    unsigned height_;
    std::shared_ptr<marker_svg> marker;
public:
    explicit svg_reader(std::string const& file_name);
    svg_reader(char const* data, std::size_t size);
    ~svg_reader();
    unsigned width() const final;
    unsigned height() const final;
    inline bool has_alpha() const final { return true; }
    boost::optional<box2d<double> > bounding_box() const final;
    void read(unsigned x,unsigned y,image_rgba8& image) final;
    image_any read(unsigned x, unsigned y, unsigned width, unsigned height) final;
private:
    void init();
};

namespace
{

image_reader* create_svg_reader(std::string const& file)
{
      return new svg_reader<boost::iostreams::file_source>(file);
}

image_reader* create_svg_reader2(char const * data, std::size_t size)
{
    return new svg_reader<boost::iostreams::array_source>(data, size);
}

const bool registered = register_image_reader("svg",create_svg_reader);
const bool registered2 = register_image_reader("svg", create_svg_reader2);
}


template <typename T>
svg_reader<T>::svg_reader(std::string const& file_name)
    : source_(file_name,std::ios_base::in | std::ios_base::binary),
      stream_(source_),
      width_(0),
      height_(0)
{
    if (!source_.is_open()) throw image_reader_exception("SVG reader: cannot open file '"+ file_name + "'");
    if (!stream_) throw image_reader_exception("SVG reader: cannot open file '"+ file_name + "'");
    init();
}

template <typename T>
svg_reader<T>::svg_reader(char const* data, std::size_t size)
    : source_(data,size),
      stream_(source_),
      width_(0),
      height_(0)
{

    if (!stream_) throw image_reader_exception("SVG reader: cannot open image stream");
    init();
}


template <typename T>
svg_reader<T>::~svg_reader() {}


template <typename T>
void svg_reader<T>::init()
{
    using namespace mapnik::svg;
    svg_path_ptr marker_path(std::make_shared<svg_storage_type>());
    vertex_stl_adapter<svg_path_storage> stl_storage(marker_path->source());
    svg_path_adapter svg_path(stl_storage);
    svg_converter_type svg(svg_path, marker_path->attributes());
    svg_parser p(svg);
    p.parse_from_stream(stream_);
    double lox,loy,hix,hiy;
    svg.bounding_rect(&lox, &loy, &hix, &hiy);
    marker_path->set_bounding_box(lox,loy,hix,hiy);
    marker_path->set_dimensions(svg.width(),svg.height());

    marker = std::make_shared<mapnik::marker_svg>(marker_path);

    width_=svg.width();
    height_=svg.height();
}

template <typename T>
unsigned svg_reader<T>::width() const
{
    return width_;
}

template <typename T>
unsigned svg_reader<T>::height() const
{
    return height_;
}

template <typename T>
boost::optional<box2d<double> > svg_reader<T>::bounding_box() const
{
    // TODO: does this need to be implemented?
    return boost::optional<box2d<double> >();
}

template <typename T>
void svg_reader<T>::read(unsigned x0, unsigned y0, image_rgba8& image)
{
    using pixfmt = agg::pixfmt_rgba32_pre;
    using renderer_base = agg::renderer_base<pixfmt>;
    using renderer_solid = agg::renderer_scanline_aa_solid<renderer_base>;
    agg::rasterizer_scanline_aa<> ras_ptr;
    agg::scanline_u8 sl;

    double opacity = 1;
    unsigned w = std::min(unsigned(image.width()),width_ - x0);
    unsigned h = std::min(unsigned(image.height()),height_ - y0);

    // 10 pixel buffer to avoid edge clipping of 100% svg's
    agg::rendering_buffer buf(image.bytes(), image.width(), image.height(), image.row_size());
    pixfmt pixf(buf);
    renderer_base renb(pixf);

    mapnik::box2d<double> const& bbox = marker->get_data()->bounding_box();
    mapnik::coord<double,2> c = bbox.center();
    // center the svg marker on '0,0'
    agg::trans_affine mtx = agg::trans_affine_translation(-c.x,-c.y);
    // Scale if necessary
    mtx.scale((double)w / width_, (double)h / height_);
    // render the marker at the center of the marker box
    mtx.translate(0.5 * image.width(), 0.5 * image.height());

    mapnik::svg::vertex_stl_adapter<mapnik::svg::svg_path_storage> stl_storage(marker->get_data()->source());
    mapnik::svg::svg_path_adapter svg_path(stl_storage);
    mapnik::svg::svg_renderer_agg<mapnik::svg::svg_path_adapter,
        agg::pod_bvector<mapnik::svg::path_attributes>,
        renderer_solid,
        agg::pixfmt_rgba32_pre > svg_renderer_this(svg_path,
                                                   marker->get_data()->attributes());

    svg_renderer_this.render(ras_ptr, sl, renb, mtx, opacity, bbox);

    demultiply_alpha(image);
}


template <typename T>
image_any svg_reader<T>::read(unsigned x, unsigned y, unsigned width, unsigned height)
{
    image_rgba8 data(width,height);
    read(x, y, data);
    return image_any(std::move(data));
}

}
