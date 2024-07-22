/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
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

#ifndef MAPNIK_RENDERER_COMMON_RENDER_THUNK_EXTRACTOR_HPP
#define MAPNIK_RENDERER_COMMON_RENDER_THUNK_EXTRACTOR_HPP

// mapnik
#include <mapnik/renderer_common.hpp>
#include <mapnik/renderer_common/render_thunk.hpp>
#include <mapnik/symbolizer_base.hpp>
#include <mapnik/util/noncopyable.hpp>

namespace mapnik {

// The approach here is to run the normal symbolizers, but in
// a 'virtual' blank environment where the changes that they
// make are recorded (the detector, the render_* calls).
//
// The recorded boxes are then used to lay out the items and
// the offsets from old to new positions can be used to perform
// the actual rendering calls.

// This should allow us to re-use as much as possible of the
// existing symbolizer layout and rendering code while still
// being able to interpose our own decisions about whether
// a collision has occurred or not.

struct virtual_renderer_common : renderer_common
{
    explicit virtual_renderer_common(renderer_common const& other);
};

// Base class for extracting the bounding boxes associated with placing
// a symbolizer at a fake, virtual point - not real geometry.
//
// The bounding boxes can be used for layout, and the thunks are
// used to re-render at locations according to the group layout.

struct render_thunk_extractor
{
    render_thunk_extractor(box2d<double>& box,
                           render_thunk_list& thunks,
                           feature_impl& feature,
                           attributes const& vars,
                           proj_transform const& prj_trans,
                           virtual_renderer_common& common,
                           box2d<double> const& clipping_extent);

    void operator()(markers_symbolizer const& sym) const;

    void operator()(text_symbolizer const& sym) const;

    void operator()(shield_symbolizer const& sym) const;

    template<typename T>
    void operator()(T const&) const
    {
        // TODO: warning if unimplemented?
    }

  private:
    void extract_text_thunk(text_render_thunk::helper_ptr&& helper, text_symbolizer const& sym) const;

    box2d<double>& box_;
    render_thunk_list& thunks_;
    feature_impl& feature_;
    attributes const& vars_;
    proj_transform const& prj_trans_;
    virtual_renderer_common& common_;
    box2d<double> clipping_extent_;

    void update_box() const;
};

} // namespace mapnik

#endif // MAPNIK_RENDERER_COMMON_RENDER_THUNK_EXTRACTOR_HPP
