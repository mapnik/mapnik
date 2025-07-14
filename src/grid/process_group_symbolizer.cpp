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

#if defined(GRID_RENDERER)

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/grid/grid_rasterizer.hpp>
#include <mapnik/grid/grid_renderer.hpp>
#include <mapnik/grid/grid_renderer_base.hpp>
#include <mapnik/grid/grid.hpp>
#include <mapnik/grid/grid_render_marker.hpp>
#include <mapnik/text/renderer.hpp>
#include <mapnik/text/glyph_positions.hpp>
#include <mapnik/svg/svg_renderer_agg.hpp>
#include <mapnik/svg/svg_storage.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/renderer_common/render_group_symbolizer.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_trans_affine.h"
MAPNIK_DISABLE_WARNING_POP

namespace mapnik {

/**
 * Render a thunk which was frozen from a previous call to
 * extract_bboxes. We should now have a new offset at which
 * to render it, and the boxes themselves should already be
 * in the detector from the placement_finder.
 */
template<typename T0>
struct thunk_renderer : render_thunk_list_dispatch
{
    using renderer_type = grid_renderer<T0>;
    using buffer_type = typename renderer_type::buffer_type;
    using text_renderer_type = grid_text_renderer<buffer_type>;

    thunk_renderer(renderer_type& ren,
                   grid_rasterizer& ras,
                   buffer_type& pixmap,
                   renderer_common& common,
                   feature_impl& feature)
        : ren_(ren),
          ras_(ras),
          pixmap_(pixmap),
          common_(common),
          feature_(feature),
          tex_(pixmap, src_over, common.scale_factor_)
    {}

    virtual void operator()(vector_marker_render_thunk const& thunk)
    {
        using buf_type = grid_rendering_buffer;
        using pixfmt_type = typename grid_renderer_base_type::pixfmt_type;
        using renderer_type = agg::renderer_scanline_bin_solid<grid_renderer_base_type>;
        using svg_renderer_type = svg::renderer_agg<svg_path_adapter, svg_attribute_type, renderer_type, pixfmt_type>;
        buf_type render_buf(pixmap_.raw_data(), common_.width_, common_.height_, common_.width_);
        ras_.reset();
        pixfmt_type pixf(render_buf);
        grid_renderer_base_type renb(pixf);
        renderer_type ren(renb);
        svg::vertex_stl_adapter<svg::svg_path_storage> stl_storage(thunk.src_->source());
        svg_path_adapter svg_path(stl_storage);
        svg_renderer_type svg_renderer(svg_path, thunk.group_attrs_);
        agg::trans_affine offset_tr = thunk.tr_;
        offset_tr.translate(offset_.x, offset_.y);
        agg::scanline_bin sl;
        svg_renderer.render_id(ras_, sl, renb, feature_.id(), offset_tr, thunk.opacity_, thunk.src_->bounding_box());
        pixmap_.add_feature(feature_);
    }

    virtual void operator()(raster_marker_render_thunk const& thunk)
    {
        using buf_type = grid_rendering_buffer;
        using pixfmt_type = typename grid_renderer_base_type::pixfmt_type;
        using renderer_type = agg::renderer_scanline_bin_solid<grid_renderer_base_type>;
        buf_type render_buf(pixmap_.raw_data(), common_.width_, common_.height_, common_.width_);
        ras_.reset();
        pixfmt_type pixf(render_buf);
        grid_renderer_base_type renb(pixf);
        renderer_type ren(renb);
        agg::trans_affine offset_tr = thunk.tr_;
        offset_tr.translate(offset_.x, offset_.y);
        render_raster_marker(ren, ras_, thunk.src_, feature_, offset_tr, thunk.opacity_);
        pixmap_.add_feature(feature_);
    }

    virtual void operator()(text_render_thunk const& thunk)
    {
        tex_.set_comp_op(thunk.comp_op_);

        value_integer feature_id = feature_.id();

        for (auto const& glyphs : thunk.placements_)
        {
            scoped_glyph_positions_offset tmp_off(*glyphs, offset_);

            if (auto const& mark = glyphs->get_marker())
            {
                ren_.render_marker(feature_,
                                   glyphs->marker_pos(),
                                   *mark->marker_,
                                   mark->transform_,
                                   thunk.opacity_,
                                   thunk.comp_op_);
            }
            tex_.render(*glyphs, feature_id);
        }

        pixmap_.add_feature(feature_);
    }

  private:
    renderer_type& ren_;
    grid_rasterizer& ras_;
    buffer_type& pixmap_;
    renderer_common& common_;
    feature_impl& feature_;
    text_renderer_type tex_;
};

template<typename T>
void grid_renderer<T>::process(group_symbolizer const& sym,
                               mapnik::feature_impl& feature,
                               proj_transform const& prj_trans)
{
    thunk_renderer<T> ren(*this, *ras_ptr, pixmap_, common_, feature);

    render_group_symbolizer(sym, feature, common_.vars_, prj_trans, common_.query_extent_, common_, ren);
}

template void grid_renderer<grid>::process(group_symbolizer const&, mapnik::feature_impl&, proj_transform const&);

} // namespace mapnik

#endif
