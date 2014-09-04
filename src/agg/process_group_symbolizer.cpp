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

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/util/variant.hpp>
#include <mapnik/text/renderer.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/renderer_common/process_group_symbolizer.hpp>
#include <mapnik/renderer_common/clipping_extent.hpp>
// agg
#include "agg_trans_affine.h"

namespace mapnik {

/**
 * Render a thunk which was frozen from a previous call to
 * extract_bboxes. We should now have a new offset at which
 * to render it, and the boxes themselves should already be
 * in the detector from the placement_finder.
 */
struct thunk_renderer : public util::static_visitor<>
{
    using renderer_type = agg_renderer<image_32>;
    using buffer_type = renderer_type::buffer_type;
    using text_renderer_type = agg_text_renderer<buffer_type>;

    thunk_renderer(renderer_type &ren,
                   buffer_type *buf,
                   renderer_common &common,
                   pixel_position const &offset)
        : ren_(ren), buf_(buf), common_(common), offset_(offset)
    {}

    void operator()(point_render_thunk const &thunk) const
    {
        pixel_position new_pos(thunk.pos_.x + offset_.x, thunk.pos_.y + offset_.y);
        ren_.render_marker(new_pos, *thunk.marker_, thunk.tr_, thunk.opacity_,
                           thunk.comp_op_);
    }

    void operator()(text_render_thunk const &thunk) const
    {
        text_renderer_type ren(*buf_, thunk.halo_rasterizer_, thunk.comp_op_, thunk.comp_op_,
                               common_.scale_factor_, common_.font_manager_.get_stroker());

        render_offset_placements(
            thunk.placements_,
            offset_,
            [&] (glyph_positions_ptr glyphs)
            {
                if (glyphs->marker())
                {
                    ren_.render_marker(glyphs->marker_pos(),
                                       *(glyphs->marker()->marker),
                                       glyphs->marker()->transform,
                                       thunk.opacity_, thunk.comp_op_);
                }
                ren.render(*glyphs);
            });
    }

    template <typename T>
    void operator()(T const &) const
    {
        // TODO: warning if unimplemented?
    }

private:
    renderer_type &ren_;
    buffer_type *buf_;
    renderer_common &common_;
    pixel_position offset_;
};

template <typename T0, typename T1>
void agg_renderer<T0,T1>::process(group_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    render_group_symbolizer(
        sym, feature, common_.vars_, prj_trans, clipping_extent(common_), common_,
        [&](render_thunk_list const& thunks, pixel_position const& render_offset)
        {
            thunk_renderer ren(*this, current_buffer_, common_, render_offset);
            for (render_thunk_ptr const& thunk : thunks)
            {
                util::apply_visitor(ren, *thunk);
            }
        });
}

template void agg_renderer<image_32>::process(group_symbolizer const&,
                                              mapnik::feature_impl &,
                                              proj_transform const&);

}
