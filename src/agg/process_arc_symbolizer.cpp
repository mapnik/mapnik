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

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_helpers.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/symbolizer_keys.hpp>
#include <mapnik/image.hpp>
#include <mapnik/vertex.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/renderer_common.hpp>
#include <mapnik/renderer_common/arc_symbolizer_properties.hpp>
#include <mapnik/renderer_common/arc_text_placement.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/image_compositing.hpp>
#include <mapnik/text/text_layout.hpp>
#include <mapnik/text/renderer.hpp>
#include <mapnik/text/placements/base.hpp>
#include <mapnik/util/math.hpp>

// stl
#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_arc.h"
#include "agg_path_storage.h"
#include "agg_conv_stroke.h"
#include "agg_conv_dash.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_color_rgba.h"
#include "agg_renderer_base.h"
MAPNIK_DISABLE_WARNING_POP

namespace mapnik {
namespace detail {

template<typename Rasterizer, typename Renderer, typename TextRenderer, typename Common, typename ProjTransform>
struct render_arc_symbolizer : util::noncopyable
{
    render_arc_symbolizer(Rasterizer& ras,
                          Renderer& ren,
                          Common& common,
                          ProjTransform const& prj_trans,
                          arc_symbolizer_properties const& props,
                          agg::rgba8 const& fill_col,
                          agg::rgba8 const& arc_stroke_col,
                          agg::rgba8 const& radius_stroke_col,
                          arc_text_layout const* label,
                          TextRenderer* text_ren)
        : ras_(ras),
          ren_(ren),
          common_(common),
          prj_trans_(prj_trans),
          props_(props),
          fill_col_(fill_col),
          arc_stroke_col_(arc_stroke_col),
          radius_stroke_col_(radius_stroke_col),
          label_(label),
          text_ren_(text_ren)
    {}

    template<typename Adapter>
    void operator()(Adapter const& va)
    {
        double x, y, z = 0;
        unsigned cmd = SEG_END;
        va.rewind(0);
        while ((cmd = va.vertex(&x, &y)) != mapnik::SEG_END)
        {
            if (cmd == SEG_CLOSE)
                continue;
            prj_trans_.backward(x, y, z);
            common_.t_.forward(&x, &y);
            render_one(x, y);
        }
    }

    // Point on the arc at angle a (bearing clockwise from north, screen space
    // with y pointing down).
    void arc_point(double cx, double cy, double a, double& px, double& py) const
    {
        px = cx + props_.radius * std::sin(a);
        py = cy - props_.radius * std::cos(a);
    }

    // The curved part of the arc as an agg::arc vertex generator. agg::arc uses
    // standard math angles (CCW from +x); our bearings are clockwise from north,
    // which maps to (bearing - pi/2). It picks the segment count from the
    // radius so large arcs stay smooth; the radius is already in pixels, so the
    // default approximation scale of 1 is the right one.
    agg::arc make_arc(double cx, double cy) const
    {
        auto [a0, a1] = props_.sweep();
        double const half_pi = util::tau / 4.0;
        return agg::arc(cx, cy, props_.radius, props_.radius, a0 - half_pi, a1 - half_pi, true);
    }

    // Closed pie wedge (center -> arc start, arc, arc end -> center), used for fill.
    void build_wedge(agg::path_storage& path, double cx, double cy) const
    {
        path.move_to(cx, cy);
        agg::arc a = make_arc(cx, cy);
        a.rewind(0);
        double ax, ay;
        unsigned cmd;
        while (!agg::is_stop(cmd = a.vertex(&ax, &ay)))
            path.line_to(ax, ay);
        path.close_polygon();
    }

    // The two radius spokes from the center to the arc endpoints.
    void build_radius_lines(agg::path_storage& path, double cx, double cy) const
    {
        auto [a0, a1] = props_.sweep();
        double px, py;
        path.move_to(cx, cy);
        arc_point(cx, cy, a0, px, py);
        path.line_to(px, py);
        path.move_to(cx, cy);
        arc_point(cx, cy, a1, px, py);
        path.line_to(px, py);
    }

    void render_one(double cx, double cy)
    {
        agg::scanline_u8 sl;

        if (props_.has_fill)
        {
            agg::path_storage path;
            build_wedge(path, cx, cy);
            ras_.reset();
            ras_.add_path(path);
            ren_.color(fill_col_);
            agg::render_scanlines(ras_, sl, ren_);
        }

        if (props_.has_radius_stroke)
        {
            agg::path_storage path;
            build_radius_lines(path, cx, cy);
            dash_stroke_and_render(path,
                                   sl,
                                   radius_stroke_col_,
                                   props_.radius_stroke_width,
                                   props_.radius_dash,
                                   props_.radius_dash_offset);
        }

        if (props_.has_arc_stroke)
        {
            agg::arc arc = make_arc(cx, cy);
            agg::path_storage path;
            path.concat_path(arc);
            dash_stroke_and_render(path,
                                   sl,
                                   arc_stroke_col_,
                                   props_.arc_stroke_width,
                                   props_.arc_dash,
                                   props_.arc_dash_offset);
        }

        // Label bent along the outside of the arc, or running radially outward
        // when the arc is too short for it. Colours and halo come from the
        // glyph formats.
        if (label_ && text_ren_)
        {
            auto [a0, a1] = props_.sweep();
            glyph_positions_ptr glyphs = place_arc_text(*label_,
                                                        cx,
                                                        cy,
                                                        props_.radius,
                                                        a0,
                                                        a1,
                                                        props_.text_offset,
                                                        common_.scale_factor_,
                                                        *common_.detector_);
            if (glyphs)
                text_ren_->render(*glyphs);
        }
    }

    // Stroke the path with the given width/color and render, applying the
    // dash pattern (dash/gap lengths and offset in unscaled pixels) first
    // unless it is empty.
    void dash_stroke_and_render(agg::path_storage& path,
                                agg::scanline_u8& sl,
                                agg::rgba8 const& col,
                                double width,
                                dash_array const& dashes,
                                double dash_offset)
    {
        if (!dashes.empty())
        {
            agg::conv_dash<agg::path_storage> dash(path);
            apply_dash_array(dash, dashes, dash_offset, common_.scale_factor_);
            stroke_and_render(dash, sl, col, width);
        }
        else
        {
            stroke_and_render(path, sl, col, width);
        }
    }

    // Stroke the given vertex source with the given width/color and render.
    template<typename VertexSource>
    void stroke_and_render(VertexSource& vs, agg::scanline_u8& sl, agg::rgba8 const& col, double width)
    {
        agg::conv_stroke<VertexSource> stroke(vs);
        stroke.width(width);
        ras_.reset();
        ras_.add_path(stroke);
        ren_.color(col);
        agg::render_scanlines(ras_, sl, ren_);
    }

    Rasterizer& ras_;
    Renderer& ren_;
    Common& common_;
    ProjTransform const& prj_trans_;
    arc_symbolizer_properties const& props_;
    agg::rgba8 fill_col_;
    agg::rgba8 arc_stroke_col_;
    agg::rgba8 radius_stroke_col_;
    arc_text_layout const* label_;
    TextRenderer* text_ren_;
};

} // namespace detail

template<typename T0, typename T1>
void agg_renderer<T0, T1>::process(arc_symbolizer const& sym,
                                   mapnik::feature_impl& feature,
                                   proj_transform const& prj_trans)
{
    arc_symbolizer_properties const props(sym, feature, common_.vars_, common_.scale_factor_);
    if (!props.drawable())
        return;

    // Shape the optional label once here; it is placed per arc centre below.
    arc_text_layout const label(sym, feature, common_.vars_, common_.font_manager_, common_.scale_factor_);

    ras_ptr->reset();
    if (gamma_method_ != gamma_method_enum::GAMMA_POWER || gamma_ != 1.0)
    {
        ras_ptr->gamma(agg::gamma_power());
        gamma_method_ = gamma_method_enum::GAMMA_POWER;
        gamma_ = 1.0;
    }

    buffer_type& current_buffer = buffers_.top().get();
    agg::rendering_buffer buf(current_buffer.bytes(),
                              current_buffer.width(),
                              current_buffer.height(),
                              current_buffer.row_size());
    using blender_type = agg::comp_op_adaptor_rgba_pre<agg::rgba8, agg::order_rgba>;
    using pixfmt_comp_type = agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer>;
    using renderer_base = agg::renderer_base<pixfmt_comp_type>;
    using renderer_type = agg::renderer_scanline_aa_solid<renderer_base>;
    composite_mode_e const comp_op = get<composite_mode_e>(sym, keys::comp_op, feature, common_.vars_, src_over);
    pixfmt_comp_type pixf(buf);
    pixf.comp_op(static_cast<agg::comp_op_e>(comp_op));
    renderer_base renb(pixf);
    renderer_type ren(renb);

    // Colours must be premultiplied for the comp_op_adaptor_rgba_pre blender.
    auto premultiply = [](color const& c, double opacity) {
        return agg::rgba8_pre(c.red(), c.green(), c.blue(), int(c.alpha() * opacity));
    };
    agg::rgba8 const fill_col = premultiply(props.fill, props.fill_opacity);
    agg::rgba8 const arc_stroke_col = premultiply(props.arc_stroke, props.arc_stroke_opacity);
    agg::rgba8 const radius_stroke_col = premultiply(props.radius_stroke, props.radius_stroke_opacity);

    // The label is rendered straight into the buffer rather than through the
    // rasterizer above, so it only needs to exist when there is a label.
    std::optional<agg_text_renderer<T0>> text_ren;
    if (label)
    {
        text_ren.emplace(current_buffer,
                         halo_rasterizer_enum::HALO_RASTERIZER_FULL,
                         comp_op,
                         src_over,
                         common_.scale_factor_,
                         common_.font_manager_.get_stroker());
    }

    using render_arc_symbolizer_type =
      detail::render_arc_symbolizer<rasterizer, renderer_type, agg_text_renderer<T0>, renderer_common, proj_transform>;
    render_arc_symbolizer_type apply(*ras_ptr,
                                     ren,
                                     common_,
                                     prj_trans,
                                     props,
                                     fill_col,
                                     arc_stroke_col,
                                     radius_stroke_col,
                                     label ? &label : nullptr,
                                     text_ren ? &*text_ren : nullptr);
    mapnik::util::apply_visitor(geometry::vertex_processor<render_arc_symbolizer_type>(apply), feature.get_geometry());
}

template void agg_renderer<image_rgba8>::process(arc_symbolizer const&, mapnik::feature_impl&, proj_transform const&);

} // namespace mapnik
