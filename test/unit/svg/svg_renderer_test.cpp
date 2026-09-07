/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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

#include "catch.hpp"

#include <mapnik/debug.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/image_compositing.hpp>
#include <mapnik/svg/svg_parser.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/svg/svg_renderer_agg.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>
#include <boost/range/combine.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_rasterizer_scanline_aa.h"
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_renderer_base.h"
#include "agg_pixfmt_rgba.h"
#include "agg_scanline_u.h"
MAPNIK_DISABLE_WARNING_POP

namespace {

mapnik::image_rgba8 render_svg(std::string const& filename, double scale_factor)
{
    using color_type = agg::rgba8;
    using order_type = agg::order_rgba;
    using blender_type = agg::comp_op_adaptor_rgba_pre<color_type, order_type>; // comp blender
    using buf_type = agg::rendering_buffer;
    using pixfmt = agg::pixfmt_custom_blend_rgba<blender_type, buf_type>;
    using renderer_base = agg::renderer_base<pixfmt>;
    using renderer_solid = agg::renderer_scanline_aa_solid<renderer_base>;

    agg::rasterizer_scanline_aa<> ras_ptr;
    agg::scanline_u8 sl;

    std::shared_ptr<mapnik::marker const> marker = mapnik::marker_cache::instance().find(filename, false);
    mapnik::marker_svg const& svg = mapnik::util::get<mapnik::marker_svg>(*marker);
    double svg_width, svg_height;
    std::tie(svg_width, svg_height) = svg.dimensions();
    int image_width = static_cast<int>(std::round(svg_width));
    int image_height = static_cast<int>(std::round(svg_height));

    mapnik::image_rgba8 im(image_width, image_height, true, true);
    agg::rendering_buffer buf(im.bytes(), im.width(), im.height(), im.row_size());
    pixfmt pixf(buf);
    renderer_base renb(pixf);

    agg::trans_affine mtx = agg::trans_affine_translation(-0.5 * svg_width, -0.5 * svg_height);
    mtx.scale(scale_factor);
    mtx.translate(0.5 * svg_width, 0.5 * svg_height);

    mapnik::svg::vertex_stl_adapter<mapnik::svg::svg_path_storage> stl_storage(svg.get_data()->source());
    mapnik::svg::svg_path_adapter svg_path(stl_storage);
    mapnik::svg::renderer_agg<mapnik::svg_path_adapter, mapnik::svg_attribute_type, renderer_solid, pixfmt> renderer(
      svg_path,
      svg.get_data()->svg_group());
    double opacity = 1.0;
    renderer.render(ras_ptr, sl, renb, mtx, opacity, {0, 0, svg_width, svg_height});
    return im;
}

bool equal(mapnik::image_rgba8 const& im1, mapnik::image_rgba8 const& im2)
{
    if (im1.width() != im2.width() || im1.height() != im2.height())
        return false;

    for (auto tup : boost::combine(im1, im2))
    {
        if (boost::get<0>(tup) != boost::get<1>(tup))
            return false;
    }
    return true;
}

mapnik::image_rgba8 render_svg_string(std::string const& content,
                                      double opacity = 1.0,
                                      mapnik::composite_mode_e comp_op = mapnik::src_over,
                                      bool background = false,
                                      agg::trans_affine const& transform = {})
{
    using pixfmt =
      agg::pixfmt_custom_blend_rgba<agg::comp_op_adaptor_rgba_pre<agg::rgba8, agg::order_rgba>, agg::rendering_buffer>;
    using renderer_base = agg::renderer_base<pixfmt>;
    using renderer_solid = agg::renderer_scanline_aa_solid<renderer_base>;

    mapnik::svg_storage_type storage;
    mapnik::svg::vertex_stl_adapter<mapnik::svg::svg_path_storage> adapter(storage.source());
    mapnik::svg_path_adapter path(adapter);
    mapnik::svg::svg_converter_type converter(path, storage.svg_group());
    mapnik::svg::svg_parser parser(converter);
    parser.parse_from_string("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"100\" height=\"100\">" + content +
                             "</svg>");

    mapnik::image_rgba8 image(100, 100, true, true);
    if (background)
        mapnik::fill(image, 0xffffffff);
    agg::rendering_buffer buffer(image.bytes(), image.width(), image.height(), image.row_size());
    pixfmt pixels(buffer);
    pixels.comp_op(comp_op);
    renderer_base base(pixels);
    agg::rasterizer_scanline_aa<> rasterizer;
    agg::scanline_u8 scanline;
    mapnik::svg::renderer_agg<mapnik::svg_path_adapter, mapnik::svg_attribute_type, renderer_solid, pixfmt> renderer(
      path,
      storage.svg_group());
    renderer.render(rasterizer, scanline, base, transform, opacity, {0, 0, 100, 100});
    return image;
}
} // namespace

TEST_CASE("SVG renderer")
{
    SECTION("SVG octocat inline/css")
    {
        double scale_factor = 1.0;
        std::string octocat_inline("./test/data/svg/octocat.svg");
        std::string octocat_css("./test/data/svg/octocat-css.svg");
        auto image1 = render_svg(octocat_inline, scale_factor);
        auto image2 = render_svg(octocat_css, scale_factor);
        REQUIRE(equal(image1, image2));
    }
}

TEST_CASE("SVG opacity preserves object bounding box gradients")
{
    for (std::string const type : {"linearGradient", "radialGradient"})
    {
        CAPTURE(type);
        std::string const definitions =
          "<defs><" + type +
          " id=\"gradient\">"
          "<stop offset=\"0\" stop-color=\"red\"/><stop offset=\"1\" stop-color=\"blue\"/>"
          "</" +
          type + "></defs>";
        std::string const rectangle = "<rect x=\"40\" y=\"40\" width=\"40\" height=\"40\" fill=\"url(#gradient)\"/>";
        for (auto const& transform : {agg::trans_affine(), agg::trans_affine(0.8, 0, 0, 0.8, 7, 9)})
        {
            auto source = render_svg_string(definitions + rectangle, 1.0, mapnik::src_over, false, transform);
            mapnik::image_rgba8 expected(100, 100, true, true);
            mapnik::composite(expected, source, mapnik::src_over, 0.5);
            auto actual = render_svg_string(definitions + rectangle, 0.5, mapnik::src_over, false, transform);
            CHECK(equal(actual, expected));

            // The invisible path gives the outer group a different buffer origin from the inner group.
            std::string const nested = definitions +
                                       "<g opacity=\"0.5\"><path d=\"M20 20 L90 90\" fill=\"none\"/>"
                                       "<g opacity=\"0.5\">" +
                                       rectangle + "</g></g>";
            mapnik::image_rgba8 nested_expected(100, 100, true, true);
            mapnik::composite(nested_expected, expected, mapnik::src_over, 0.5);
            actual = render_svg_string(nested, 1.0, mapnik::src_over, false, transform);
            CHECK(equal(actual, nested_expected));
        }
    }
}

TEST_CASE("SVG opacity composites transparent pixels outside marker bounds")
{
    for (auto mode : {mapnik::src, mapnik::src_in, mapnik::dst_in, mapnik::dst_atop})
    {
        for (int x : {40, 140})
        {
            CAPTURE(mode, x);
            std::string const rectangle =
              "<rect x=\"" + std::to_string(x) + "\" y=\"40\" width=\"40\" height=\"40\" fill=\"red\"/>";
            auto source = render_svg_string(rectangle);
            mapnik::image_rgba8 expected(100, 100, true, true);
            mapnik::fill(expected, 0xffffffff);
            mapnik::composite(expected, source, mode, 0.5);
            CHECK(equal(render_svg_string(rectangle, 0.5, mode, true), expected));
            CHECK(equal(render_svg_string("<g opacity=\"0.5\">" + rectangle + "</g>", 1.0, mode, true), expected));
        }
    }
}
