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

#ifndef MAPNIK_RENDERER_COMMON_RENDER_GROUP_SYMBOLIZER_HPP
#define MAPNIK_RENDERER_COMMON_RENDER_GROUP_SYMBOLIZER_HPP

// mapnik
#include <mapnik/pixel_position.hpp>
#include <mapnik/renderer_common.hpp>
#include <mapnik/renderer_common/render_thunk.hpp>
#include <mapnik/symbolizer_base.hpp>
#include <mapnik/text/glyph_positions.hpp>

namespace mapnik {

struct render_thunk_list_dispatch
{
    virtual void operator()(vector_marker_render_thunk const& thunk) = 0;
    virtual void operator()(raster_marker_render_thunk const& thunk) = 0;
    virtual void operator()(text_render_thunk const& thunk) = 0;

    void render_list(render_thunk_list const& thunks, pixel_position const& offset)
    {
        offset_ = offset;

        for (render_thunk const& thunk : thunks)
        {
            util::apply_visitor(std::ref(*this), thunk);
        }
    }

  protected:
    pixel_position offset_;
};

MAPNIK_DECL
void render_group_symbolizer(group_symbolizer const& sym,
                             feature_impl& feature,
                             attributes const& vars,
                             proj_transform const& prj_trans,
                             box2d<double> const& clipping_extent,
                             renderer_common& common,
                             render_thunk_list_dispatch& render_thunks);

} // namespace mapnik

#endif // MAPNIK_RENDERER_COMMON_RENDER_GROUP_SYMBOLIZER_HPP
