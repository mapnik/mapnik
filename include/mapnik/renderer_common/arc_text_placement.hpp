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

#ifndef MAPNIK_RENDERER_COMMON_ARC_TEXT_PLACEMENT_HPP
#define MAPNIK_RENDERER_COMMON_ARC_TEXT_PLACEMENT_HPP

#include <mapnik/config.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/text/glyph_positions.hpp>
#include <mapnik/text/placements/base.hpp>
#include <mapnik/util/noncopyable.hpp>

#include <memory>

namespace mapnik {

class text_layout;
class face_manager;
using face_manager_freetype = face_manager;
class label_collision_detector4;

class MAPNIK_DECL arc_text_layout : util::noncopyable
{
  public:
    arc_text_layout(arc_symbolizer const& sym,
                    feature_impl const& feature,
                    attributes const& vars,
                    face_manager_freetype& font_manager,
                    double scale_factor);
    ~arc_text_layout();

    explicit operator bool() const { return static_cast<bool>(layout_); }
    text_layout const& get() const { return *layout_; }
    bool allow_overlap() const { return allow_overlap_; }

  private:
    text_placement_info_ptr info_;
    std::unique_ptr<text_layout> layout_;
    bool allow_overlap_ = false;
};

// Places the label around the arc centred at (cx, cy). Like other labels it
// is dropped (null is returned) if it would collide with a label already in
// the detector, unless text-allow-overlap is set, and otherwise its glyph
// boxes are added to the detector.
MAPNIK_DECL glyph_positions_ptr place_arc_text(arc_text_layout const& label,
                                               double cx,
                                               double cy,
                                               double radius,
                                               double a0,
                                               double a1,
                                               double text_offset,
                                               double scale_factor,
                                               label_collision_detector4& detector);

} // namespace mapnik

#endif // MAPNIK_RENDERER_COMMON_ARC_TEXT_PLACEMENT_HPP
