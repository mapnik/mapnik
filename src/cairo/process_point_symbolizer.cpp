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
#include <mapnik/symbolizer.hpp>
#include <mapnik/symbolizer_keys.hpp>
#include <mapnik/image_compositing.hpp>
#include <mapnik/cairo/cairo_context.hpp>
#include <mapnik/cairo/cairo_renderer.hpp>
#include <mapnik/renderer_common.hpp>
#include <mapnik/renderer_common/process_point_symbolizer.hpp>

namespace agg { struct trans_affine; }

namespace mapnik
{

class feature_impl;
class proj_transform;

template <typename T>
void cairo_renderer<T>::process(point_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    composite_mode_e comp_op = get<composite_mode_e>(sym, keys::comp_op, feature, common_.vars_, src_over);

    cairo_save_restore guard(context_);
    context_.set_operator(comp_op);

    render_point_symbolizer(
        sym, feature, prj_trans, common_,
        [this](pixel_position const& pos, marker const& marker,
            agg::trans_affine const& tr, double opacity) {
            render_marker(pos, marker, tr, opacity);
        });
}

template void cairo_renderer<cairo_ptr>::process(point_symbolizer const&,
                                                 mapnik::feature_impl &,
                                                 proj_transform const&);

}

#endif // HAVE_CAIRO
