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
#include <mapnik/cairo/cairo_renderer.hpp>
#include <mapnik/cairo/cairo_render_vector.hpp>
#include <mapnik/renderer_common/render_markers_symbolizer.hpp>
#include <mapnik/symbolizer.hpp>

namespace mapnik
{

namespace detail {

struct cairo_markers_renderer_context : markers_renderer_context
{
    explicit cairo_markers_renderer_context(cairo_context & ctx)
      : ctx_(ctx)
    {}

    virtual void render_marker(svg_path_ptr const& src,
                               svg_path_adapter & path,
                               svg_attribute_type const& attrs,
                               markers_dispatch_params const& params,
                               agg::trans_affine const& marker_tr)
    {
        render_vector_marker(ctx_,
                             path,
                             attrs,
                             src->bounding_box(),
                             marker_tr,
                             params.opacity);
    }

    virtual void render_marker(image_rgba8 const& src,
                               markers_dispatch_params const& params,
                               agg::trans_affine const& marker_tr)
    {
        ctx_.add_image(marker_tr, src, params.opacity);
    }

private:
    cairo_context & ctx_;
};

} // namespace detail

template <typename T>
void cairo_renderer<T>::process(markers_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    cairo_save_restore guard(context_);
    composite_mode_e comp_op = get<composite_mode_e>(sym, keys::comp_op, feature, common_.vars_, src_over);
    context_.set_operator(comp_op);
    box2d<double> clip_box = common_.query_extent_;

    using context_type = detail::cairo_markers_renderer_context;
    context_type renderer_context(context_);

    render_markers_symbolizer(
        sym, feature, prj_trans, common_, clip_box,
        renderer_context);
}

template void cairo_renderer<cairo_ptr>::process(markers_symbolizer const&,
                                                 mapnik::feature_impl &,
                                                 proj_transform const&);

} // namespace mapnik

#endif // HAVE_CAIRO
