/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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
#include <mapnik/proj_transform.hpp>
#include <mapnik/image_compositing.hpp>

// agg
#include "agg_ellipse.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_color_rgba.h"
#include "agg_renderer_base.h"

namespace mapnik { namespace detail {

template <typename Rasterizer, typename Renderer, typename Common, typename ProjTransform>
struct render_dot_symbolizer : util::noncopyable
{
    render_dot_symbolizer(double rx, double ry, Rasterizer & ras, Renderer & ren, Common & common, ProjTransform const& prj_trans)
        : ras_(ras),
          ren_(ren),
          common_(common),
          prj_trans_(prj_trans),
          rx_(rx),
          ry_(ry),
          el_(0, 0, rx, ry) {}

    template <typename Adapter>
    void operator() (Adapter const& va)
    {
        double x,y,z = 0;
        unsigned cmd = SEG_END;
        va.rewind(0);
        while ((cmd = va.vertex(&x, &y)) != mapnik::SEG_END)
        {
            if (cmd == SEG_CLOSE) continue;
            prj_trans_.backward(x, y, z);
            common_.t_.forward(&x, &y);
            el_.init(x, y, rx_, ry_, el_.num_steps());
            ras_.add_path(el_);
            agg::render_scanlines(ras_, sl_, ren_);
        }
    }
    Rasterizer & ras_;
    Renderer & ren_;
    Common & common_;
    ProjTransform const& prj_trans_;
    double rx_;
    double ry_;
    agg::ellipse el_;
    agg::scanline_u8 sl_;
};

} // ns detail

template <typename T0, typename T1>
void agg_renderer<T0,T1>::process(dot_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    double width = 0.0;
    double height = 0.0;
    bool has_width = has_key(sym,keys::width);
    bool has_height = has_key(sym,keys::height);
    if (has_width && has_height)
    {
        width = get<double>(sym, keys::width, feature, common_.vars_, 0.0);
        height = get<double>(sym, keys::height, feature, common_.vars_, 0.0);
    }
    else if (has_width)
    {
        width = height = get<double>(sym, keys::width, feature, common_.vars_, 0.0);
    }
    else if (has_height)
    {
        width = height = get<double>(sym, keys::height, feature, common_.vars_, 0.0);
    }
    double rx = width/2.0;
    double ry = height/2.0;
    double opacity = get<double>(sym, keys::opacity, feature, common_.vars_, 1.0);
    color const& fill = get<mapnik::color>(sym, keys::fill, feature, common_.vars_, mapnik::color(128,128,128));
    ras_ptr->reset();
    agg::rendering_buffer buf(current_buffer_->bytes(),current_buffer_->width(),current_buffer_->height(),current_buffer_->row_size());
    using blender_type = agg::comp_op_adaptor_rgba_pre<agg::rgba8, agg::order_rgba>;
    using pixfmt_comp_type = agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer>;
    using renderer_base = agg::renderer_base<pixfmt_comp_type>;
    using renderer_type = agg::renderer_scanline_aa_solid<renderer_base>;
    pixfmt_comp_type pixf(buf);
    pixf.comp_op(static_cast<agg::comp_op_e>(get<composite_mode_e>(sym, keys::comp_op, feature, common_.vars_, src_over)));
    renderer_base renb(pixf);
    renderer_type ren(renb);

    ren.color(agg::rgba8_pre(fill.red(), fill.green(), fill.blue(), int(fill.alpha() * opacity)));
    using render_dot_symbolizer_type = detail::render_dot_symbolizer<rasterizer, renderer_type, renderer_common, proj_transform>;
    render_dot_symbolizer_type apply(rx, ry, *ras_ptr, ren, common_, prj_trans);
    mapnik::util::apply_visitor(geometry::vertex_processor<render_dot_symbolizer_type>(apply), feature.get_geometry());
}

template void agg_renderer<image_rgba8>::process(dot_symbolizer const&,
                                              mapnik::feature_impl &,
                                              proj_transform const&);

}
