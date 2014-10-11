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

#if defined(HAVE_CAIRO)

// mapnik
#include <mapnik/make_unique.hpp>
#include <mapnik/cairo/cairo_renderer.hpp>

// mapnik symbolizer generics
#include <mapnik/renderer_common/process_group_symbolizer.hpp>

namespace mapnik
{

class feature_impl;
class proj_transform;
struct group_symbolizer;

namespace {

// Render a thunk which was frozen from a previous call to
// extract_bboxes. We should now have a new offset at which
// to render it, and the boxes themselves should already be
// in the detector from the placement_finder.
template <typename T>
struct thunk_renderer : public util::static_visitor<>
{
    using renderer_type = cairo_renderer<T>;

    thunk_renderer(renderer_type & ren,
                   cairo_context & context,
                   cairo_face_manager & face_manager,
                   renderer_common & common,
                   pixel_position const& offset)
        : ren_(ren), context_(context), face_manager_(face_manager),
          common_(common), offset_(offset)
    {}

    void operator()(point_render_thunk const &thunk) const
    {
        pixel_position new_pos(thunk.pos_.x + offset_.x, thunk.pos_.y + offset_.y);
        ren_.render_marker(new_pos, *thunk.marker_, thunk.tr_, thunk.opacity_,
                           thunk.comp_op_);
    }

    void operator()(text_render_thunk const &thunk) const
    {
        cairo_save_restore guard(context_);
        context_.set_operator(thunk.comp_op_);

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
                context_.add_text(*glyphs, face_manager_, common_.font_manager_, src_over, src_over, common_.scale_factor_);
            });
    }

    template <typename T0>
    void operator()(T0 const &) const
    {
        // TODO: warning if unimplemented?
    }

private:
    renderer_type & ren_;
    cairo_context & context_;
    cairo_face_manager & face_manager_;
    renderer_common & common_;
    pixel_position offset_;
};

} // anonymous namespace

template <typename T>
void cairo_renderer<T>::process(group_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    render_group_symbolizer(
        sym, feature, common_.vars_, prj_trans, common_.query_extent_, common_,
        [&](render_thunk_list const& thunks, pixel_position const& render_offset)
        {
            thunk_renderer<T> ren(*this, context_, face_manager_, common_, render_offset);
            for (render_thunk_ptr const& thunk : thunks)
            {
                util::apply_visitor(ren, *thunk);
            }
        });
}

template void cairo_renderer<cairo_ptr>::process(group_symbolizer const&,
                                                 mapnik::feature_impl &,
                                                 proj_transform const&);

}

#endif // HAVE_CAIRO
