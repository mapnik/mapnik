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
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/symbolizer_keys.hpp>
#include <mapnik/image.hpp>
#include <mapnik/vertex.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/renderer_common.hpp>
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
                          double radius,
                          double start_angle,
                          double end_angle,
                          bool has_fill,
                          agg::rgba8 const& fill,
                          bool has_stroke,
                          agg::rgba8 const& stroke,
                          double stroke_width)
        : ras_(ras),
          ren_(ren),
          common_(common),
          prj_trans_(prj_trans),
          radius_(radius),
          start_angle_(start_angle),
          end_angle_(end_angle),
          has_fill_(has_fill),
          fill_(fill),
          has_stroke_(has_stroke),
          stroke_(stroke),
          stroke_width_(stroke_width)
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

    // Sweep angles in radians, clockwise from north, with wrap-around handled
    // (e.g. 350 -> 10 degrees). Equal angles mean a full circle, as charted
    // for all-round lights. Returns {a0, a1} with a1 > a0.
    std::pair<double, double> sweep() const
    {
        double a0 = util::radians(start_angle_);
        double a1 = util::radians(end_angle_);
        if (a1 <= a0)
            a1 += util::tau;
        return {a0, a1};
    }

    // Point on the arc at angle a (bearing clockwise from north, screen space
    // with y pointing down).
    void arc_point(double cx, double cy, double a, double& px, double& py) const
    {
        px = cx + radius_ * std::sin(a);
        py = cy - radius_ * std::cos(a);
    }

    // The curved part of the arc as an agg::arc vertex generator. agg::arc uses
    // standard math angles (CCW from +x); our bearings are clockwise from north,
    // which maps to (bearing - pi/2). Its adaptive approximation_scale picks the
    // segment count from the radius so large arcs stay smooth.
    agg::arc make_arc(double cx, double cy) const
    {
        auto [a0, a1] = sweep();
        double const half_pi = util::tau / 4.0;
        agg::arc a(cx, cy, radius_, radius_, a0 - half_pi, a1 - half_pi, true);
        a.approximation_scale(common_.scale_factor_);
        return a;
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
        auto [a0, a1] = sweep();
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
        if (radius_ <= 0.0)
            return;

        agg::scanline_u8 sl;

        if (has_fill_)
        {
            agg::path_storage path;
            build_wedge(path, cx, cy);
            ras_.reset();
            ras_.add_path(path);
            ren_.color(fill_);
            agg::render_scanlines(ras_, sl, ren_);
        }

        if (has_stroke_)
        {
            agg::path_storage path;
            build_wedge(path, cx, cy);
	    stroke_and_render(path, sl, stroke_, stroke_width_);
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
    double radius_;
    double start_angle_;
    double end_angle_;
    bool has_fill_;
    agg::rgba8 fill_;
    bool has_stroke_;
    agg::rgba8 stroke_;
    double stroke_width_;
};

} // namespace detail

template<typename T0, typename T1>
void agg_renderer<T0, T1>::process(arc_symbolizer const& sym,
                                   mapnik::feature_impl& feature,
                                   proj_transform const& prj_trans)
{
    double const radius = get<double>(sym, keys::radius, feature, common_.vars_, 0.0) * common_.scale_factor_;
    double const start_angle = get<double>(sym, keys::start_angle, feature, common_.vars_, 0.0);
    double const end_angle = get<double>(sym, keys::end_angle, feature, common_.vars_, 360.0);

    bool const has_fill = has_key(sym, keys::fill);
    color const& fill = get<mapnik::color>(sym, keys::fill, feature, common_.vars_, mapnik::color(128, 128, 128));
    double const fill_opacity = get<double>(sym, keys::fill_opacity, feature, common_.vars_, 1.0);

    bool const has_stroke = has_key(sym, keys::stroke);
    color const& stroke = get<mapnik::color>(sym, keys::stroke, feature, common_.vars_, mapnik::color(0, 0, 0));
    double const stroke_width_raw = get<double>(sym, keys::stroke_width, feature, common_.vars_, 1.0);
    double const stroke_width = stroke_width_raw * common_.scale_factor_;
    double const stroke_opacity = get<double>(sym, keys::stroke_opacity, feature, common_.vars_, 1.0);

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
    pixfmt_comp_type pixf(buf);
    pixf.comp_op(
      static_cast<agg::comp_op_e>(get<composite_mode_e>(sym, keys::comp_op, feature, common_.vars_, src_over)));
    renderer_base renb(pixf);
    renderer_type ren(renb);

    // Colours must be premultiplied for the comp_op_adaptor_rgba_pre blender.
    agg::rgba8 fill_col = agg::rgba8_pre(fill.red(), fill.green(), fill.blue(), int(fill.alpha() * fill_opacity));
    agg::rgba8 stroke_col =
      agg::rgba8_pre(stroke.red(), stroke.green(), stroke.blue(), int(stroke.alpha() * stroke_opacity));

    using render_arc_symbolizer_type =
      detail::render_arc_symbolizer<rasterizer, renderer_type, agg_text_renderer<T0>, renderer_common, proj_transform>;
    render_arc_symbolizer_type apply(*ras_ptr,
                                     ren,
                                     common_,
                                     prj_trans,
                                     radius,
                                     start_angle,
                                     end_angle,
                                     has_fill,
                                     fill_col,
                                     has_stroke,
                                     stroke_col,
                                     stroke_width);
    mapnik::util::apply_visitor(geometry::vertex_processor<render_arc_symbolizer_type>(apply), feature.get_geometry());
}

template void agg_renderer<image_rgba8>::process(arc_symbolizer const&, mapnik::feature_impl&, proj_transform const&);

} // namespace mapnik
