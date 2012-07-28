/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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
#ifndef MAPNIK_PLACEMENT_FINDER_NG_HPP
#define MAPNIK_PLACEMENT_FINDER_NG_HPP

//mapnik
#include <mapnik/box2d.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/text/glyph_info.hpp>
#include <mapnik/text/layout.hpp>
#include <mapnik/text_placements/base.hpp>
#include <mapnik/expression_evaluator.hpp>

//stl
#include <vector>

//boost
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

namespace mapnik
{

class label_collision_detector4;
typedef label_collision_detector4 DetectorType;

class feature_impl;
typedef feature_impl Feature;

struct glyph_position
{
    glyph_position(glyph_info const& glyph, pixel_position const& pos, double angle)
        : glyph(&glyph), pos(pos), angle(angle) { }
    glyph_info const* glyph;
    pixel_position pos;
    double angle;
};

/** Stores positions of glphys.
 *
 * The actual glyphs and their format is stored in text_layout.
 */
class glyph_positions
{
public:
    typedef std::vector<glyph_position>::const_iterator const_iterator;
    glyph_positions();

    const_iterator begin() const;
    const_iterator end() const;

    void push_back(glyph_info const& glyph, pixel_position offset, double angle);

    /** Is each character rotated by the same angle?
     * This function is used to avoid costly trigonometric function calls when not necessary. */
    bool is_constant_angle() const;
    double get_angle() const;

    pixel_position const& get_base_point() const;
    void set_base_point(pixel_position base_point);
private:
    std::vector<glyph_position> data_;
    pixel_position base_point_;
    double angle_;
    bool const_angle_;
};
typedef boost::shared_ptr<glyph_positions> glyph_positions_ptr;

struct text_symbolizer_properties;

class placement_finder_ng : boost::noncopyable
{
public:
    placement_finder_ng(Feature const& feature,
                        DetectorType & detector,
                        box2d<double> const& extent,
                        text_placement_info_ptr placement_info,
                        face_manager_freetype & font_manager,
                        double scale_factor);

    /** Try to place a single label at the given point. */
    glyph_positions_ptr find_point_placement(pixel_position pos);

    bool next_position();
private:
    void init_alignment();
    pixel_position alignment_offset() const;
    Feature const& feature_;
    DetectorType const& detector_;
    box2d<double> const& extent_;
    double angle_; //in rad
    // Precalculated values for maximum performance
    double sina_;
    double cosa_;
    text_layout layout_;
    text_placement_info_ptr info_;
    bool valid_;

    vertical_alignment_e valign_;
    horizontal_alignment_e halign_;
    justify_alignment_e jalign_;
    double scale_factor_;
};

typedef boost::shared_ptr<placement_finder_ng> placement_finder_ng_ptr;

}//ns mapnik

#endif // PLACEMENT_FINDER_NG_HPP
