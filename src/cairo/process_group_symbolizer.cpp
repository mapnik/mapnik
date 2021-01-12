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

#if defined(HAVE_CAIRO)

// mapnik
#include <mapnik/marker.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/text/glyph_positions.hpp>
#include <mapnik/cairo/cairo_renderer.hpp>
#include <mapnik/cairo/cairo_render_vector.hpp>

// mapnik symbolizer generics
#include <mapnik/renderer_common/render_group_symbolizer.hpp>

#include <memory>

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
struct thunk_renderer : render_thunk_list_dispatch
{
    using renderer_type = cairo_renderer<T>;

    thunk_renderer(renderer_type & ren,
                   cairo_context & context,
                   cairo_face_manager & face_manager,
                   renderer_common & common)
        : ren_(ren), context_(context), face_manager_(face_manager),
          common_(common)
    {}

    virtual void operator()(vector_marker_render_thunk const& thunk)
    {
        cairo_save_restore guard(context_);
        context_.set_operator(thunk.comp_op_);

        svg::vertex_stl_adapter<svg::svg_path_storage> stl_storage(thunk.src_->source());
        svg::svg_path_adapter svg_path(stl_storage);

        agg::trans_affine offset_tr = thunk.tr_;
        offset_tr.translate(offset_.x, offset_.y);
        mapnik::render_vector_marker(context_,
                                     svg_path,
                                     thunk.attrs_,
                                     thunk.src_->bounding_box(),
                                     offset_tr,
                                     thunk.opacity_);
    }

    virtual void operator()(raster_marker_render_thunk const& thunk)
    {
        cairo_save_restore guard(context_);
        context_.set_operator(thunk.comp_op_);

        agg::trans_affine offset_tr = thunk.tr_;
        offset_tr.translate(offset_.x, offset_.y);
        context_.add_image(offset_tr, thunk.src_, thunk.opacity_);
    }

    virtual void operator()(text_render_thunk const& thunk)
    {
        cairo_save_restore guard(context_);
        context_.set_operator(thunk.comp_op_);

        for (auto const& glyphs : thunk.placements_)
        {
            scoped_glyph_positions_offset tmp_off(*glyphs, offset_);

            if (auto const& mark = glyphs->get_marker())
            {
                ren_.render_marker(glyphs->marker_pos(),
                                   *mark->marker_,
                                   mark->transform_,
                                   thunk.opacity_, thunk.comp_op_);
            }
            context_.add_text(*glyphs, face_manager_, src_over, src_over, common_.scale_factor_);
        }
    }

private:
    renderer_type & ren_;
    cairo_context & context_;
    cairo_face_manager & face_manager_;
    renderer_common & common_;
};

} // anonymous namespace

template <typename T>
void cairo_renderer<T>::process(group_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    thunk_renderer<T> ren(*this, context_, face_manager_, common_);

    render_group_symbolizer(
        sym, feature, common_.vars_, prj_trans, common_.query_extent_, common_,
        ren);
}

template void cairo_renderer<cairo_ptr>::process(group_symbolizer const&,
                                                 mapnik::feature_impl &,
                                                 proj_transform const&);

}

#endif // HAVE_CAIRO
