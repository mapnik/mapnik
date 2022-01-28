/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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
#include <mapnik/feature.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/image_any.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/transform_path_adapter.hpp>
#include <mapnik/agg_helpers.hpp>
#include <mapnik/util/is_clockwise.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_color_rgba.h"
#include "agg_pixfmt_rgba.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_conv_stroke.h"
MAPNIK_DISABLE_WARNING_POP

namespace mapnik {

namespace {
template<typename T>
void draw_rect(T& pixmap, box2d<double> const& box)
{
    int x0 = static_cast<int>(box.minx());
    int x1 = static_cast<int>(box.maxx());
    int y0 = static_cast<int>(box.miny());
    int y1 = static_cast<int>(box.maxy());
    unsigned color1 = 0xff0000ff;
    for (int x = x0; x < x1; x++)
    {
        mapnik::set_pixel(pixmap, x, y0, color1);
        mapnik::set_pixel(pixmap, x, y1, color1);
    }
    for (int y = y0; y < y1; y++)
    {
        mapnik::set_pixel(pixmap, x0, y, color1);
        mapnik::set_pixel(pixmap, x1, y, color1);
    }
}

template<typename Pixmap>
struct apply_vertex_mode
{
    apply_vertex_mode(Pixmap& pixmap, view_transform const& t, proj_transform const& prj_trans)
        : pixmap_(pixmap)
        , t_(t)
        , prj_trans_(prj_trans)
    {}

    template<typename Adapter>
    void operator()(Adapter const& va) const
    {
        double x;
        double y;
        double z = 0;
        va.rewind(0);
        unsigned cmd = SEG_END;
        while ((cmd = va.vertex(&x, &y)) != mapnik::SEG_END)
        {
            if (cmd == SEG_CLOSE)
                continue;
            prj_trans_.backward(x, y, z);
            t_.forward(&x, &y);
            mapnik::set_pixel(pixmap_, x, y, 0xff0000ff);
            mapnik::set_pixel(pixmap_, x - 1, y - 1, 0xff0000ff);
            mapnik::set_pixel(pixmap_, x + 1, y + 1, 0xff0000ff);
            mapnik::set_pixel(pixmap_, x - 1, y + 1, 0xff0000ff);
            mapnik::set_pixel(pixmap_, x + 1, y - 1, 0xff0000ff);
        }
    }

    Pixmap& pixmap_;
    view_transform const& t_;
    proj_transform const& prj_trans_;
};

template<typename BufferType>
struct RingRenderer
{
    using color_type = agg::rgba8;
    using order_type = agg::order_rgba;
    using blender_type = agg::comp_op_adaptor_rgba_pre<color_type, order_type>; // comp blender
    using pixfmt_comp_type = agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer>;
    using renderer_base = agg::renderer_base<pixfmt_comp_type>;
    using renderer_type = agg::renderer_scanline_aa_solid<renderer_base>;

    using path_type = transform_path_adapter<view_transform, geometry::ring_vertex_adapter<double>>;

    RingRenderer(rasterizer& ras_ptr, BufferType& im, view_transform const& tr, proj_transform const& prj_trans)
        : ras_ptr_(ras_ptr)
        , im_(im)
        , tr_(tr)
        , prj_trans_(prj_trans)
        , sl_()
    {}

    void draw_ring(geometry::linear_ring<double> const& ring, agg::rgba8 const& color)
    {
        ras_ptr_.reset();
        agg::rendering_buffer buf(im_.bytes(), im_.width(), im_.height(), im_.row_size());
        pixfmt_comp_type pixf(buf);
        renderer_base renb(pixf);
        renderer_type ren(renb);
        geometry::ring_vertex_adapter<double> va(ring);
        path_type path(tr_, va, prj_trans_);
        ras_ptr_.add_path(path);
        ras_ptr_.filling_rule(agg::fill_non_zero);
        ren.color(color);
        agg::render_scanlines(ras_ptr_, sl_, ren);
    }

    void draw_outline(geometry::linear_ring<double> const& ring, agg::rgba8 const& color, double stroke_width = 3)
    {
        ras_ptr_.reset();
        agg::rendering_buffer buf(im_.bytes(), im_.width(), im_.height(), im_.row_size());
        pixfmt_comp_type pixf(buf);
        renderer_base renb(pixf);
        renderer_type ren(renb);
        geometry::ring_vertex_adapter<double> va(ring);
        path_type path(tr_, va, prj_trans_);
        agg::conv_stroke<path_type> stroke(path);
        stroke.width(stroke_width);
        ras_ptr_.add_path(stroke);
        ras_ptr_.filling_rule(agg::fill_non_zero);
        ren.color(color);
        agg::render_scanlines(ras_ptr_, sl_, ren);
    }

    rasterizer& ras_ptr_;
    BufferType& im_;
    view_transform const& tr_;
    proj_transform const& prj_trans_;
    agg::scanline_u8 sl_;
};

template<typename BufferType>
struct render_ring_visitor
{
    render_ring_visitor(RingRenderer<BufferType>& renderer)
        : renderer_(renderer)
    {}

    void operator()(mapnik::geometry::multi_polygon<double> const& geom) const
    {
        for (auto const& poly : geom)
        {
            (*this)(poly);
        }
    }

    void operator()(mapnik::geometry::polygon<double> const& poly) const
    {
        agg::rgba8 red(255, 0, 0, 255);
        agg::rgba8 green(0, 255, 255, 255);
        agg::rgba8 black(0, 0, 0, 255);
        bool exterior = true;
        for (auto const& ring : poly)
        {
            if (exterior)
            {
                exterior = false;
                renderer_.draw_ring(ring, red);
                if (mapnik::util::is_clockwise(ring))
                {
                    renderer_.draw_outline(ring, black);
                }
            }
            else
            {
                renderer_.draw_ring(ring, green);
                if (!mapnik::util::is_clockwise(ring))
                {
                    renderer_.draw_outline(ring, black);
                }
            }
        }
    }

    template<typename GeomType>
    void operator()(GeomType const&) const
    {}

    RingRenderer<BufferType>& renderer_;
};

} // anonymous namespace

template<typename T0, typename T1>
void agg_renderer<T0, T1>::process(debug_symbolizer const& sym,
                                   mapnik::feature_impl& feature,
                                   proj_transform const& prj_trans)
{
    debug_symbolizer_mode_enum mode =
      get<debug_symbolizer_mode_enum>(sym, keys::mode, feature, common_.vars_, DEBUG_SYM_MODE_COLLISION);

    ras_ptr->reset();
    if (gamma_method_ != GAMMA_POWER || gamma_ != 1.0)
    {
        ras_ptr->gamma(agg::gamma_power());
        gamma_method_ = GAMMA_POWER;
        gamma_ = 1.0;
    }

    if (mode == DEBUG_SYM_MODE_RINGS)
    {
        RingRenderer<buffer_type> renderer(*ras_ptr, buffers_.top().get(), common_.t_, prj_trans);
        render_ring_visitor<buffer_type> apply(renderer);
        mapnik::util::apply_visitor(apply, feature.get_geometry());
    }
    else if (mode == DEBUG_SYM_MODE_COLLISION)
    {
        for (auto const& n : *common_.detector_)
        {
            draw_rect(buffers_.top().get(), n.get().box);
        }
    }
    else if (mode == DEBUG_SYM_MODE_VERTEX)
    {
        using apply_vertex_mode = apply_vertex_mode<buffer_type>;
        apply_vertex_mode apply(buffers_.top().get(), common_.t_, prj_trans);
        util::apply_visitor(geometry::vertex_processor<apply_vertex_mode>(apply), feature.get_geometry());
    }
}

template void agg_renderer<image_rgba8>::process(debug_symbolizer const&, mapnik::feature_impl&, proj_transform const&);
} // namespace mapnik
