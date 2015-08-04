/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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
#ifndef MAPNIK_PLACEMENT_FINDER_HPP
#define MAPNIK_PLACEMENT_FINDER_HPP

//mapnik
#include <mapnik/box2d.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/text/text_layout.hpp>
#include <mapnik/text/glyph_positions.hpp>
#include <mapnik/text/rotation.hpp>
#include <mapnik/util/noncopyable.hpp>

namespace mapnik
{

class label_collision_detector4;
using DetectorType = label_collision_detector4;

class feature_impl;
class vertex_cache;
class text_placement_info;
struct glyph_info;

class placement_finder : util::noncopyable
{
public:
    placement_finder(feature_impl const& feature,
                     attributes const& attr,
                     DetectorType & detector,
                     box2d<double> const& extent,
                     text_placement_info const& placement_info,
                     face_manager_freetype & font_manager,
                     double scale_factor);

    // Try to place a single label at the given point.
    bool find_point_placement(pixel_position const& pos);
    // Iterate over the given path, placing line-following labels or point labels with respect to label_spacing.
    template <typename T>
    bool find_line_placements(T & path, bool points);
    // Try next position alternative from placement_info.
    bool next_position();

    placements_list const& placements() const { return placements_; }

    void set_marker(marker_info_ptr m, box2d<double> box, bool marker_unlocked, pixel_position const& marker_displacement);
private:
    bool single_line_placement(vertex_cache &pp, text_upright_e orientation);
    // Moves dx pixels but makes sure not to fall of the end.
    void path_move_dx(vertex_cache & pp, double dx);
    // Normalize angle in range [-pi, +pi].
    static double normalize_angle(double angle);
    // Adjusts user defined spacing to place an integer number of labels.
    double get_spacing(double path_length, double layout_width) const;
    // Checks for collision.
    bool collision(box2d<double> const& box, const value_unicode_string &repeat_key, bool line_placement) const;
    // Adds marker to glyph_positions and to collision detector. Returns false if there is a collision.
    bool add_marker(glyph_positions_ptr & glyphs, pixel_position const& pos, std::vector<box2d<double>> & bboxes) const;
    // Maps upright==auto, left-only and right-only to left,right to simplify processing.
    // angle = angle of at start of line (to estimate best option for upright==auto)
    text_upright_e simplify_upright(text_upright_e upright, double angle) const;
    box2d<double> get_bbox(text_layout const& layout, glyph_info const& glyph, pixel_position const& pos, rotation const& rot);
    feature_impl const& feature_;
    attributes const& attr_;
    DetectorType & detector_;
    box2d<double> const& extent_;
    text_placement_info const& info_;
    evaluated_text_properties_ptr text_props_;
    layout_container layouts_;

    double scale_factor_;
    face_manager_freetype &font_manager_;

    placements_list placements_;

    //ShieldSymbolizer
    bool has_marker_;
    marker_info_ptr marker_;
    box2d<double> marker_box_;
    bool marker_unlocked_;
    pixel_position marker_displacement_;
    double move_dx_;
    horizontal_alignment_e horizontal_alignment_;
};

}//ns mapnik

#endif // PLACEMENT_FINDER_HPP
