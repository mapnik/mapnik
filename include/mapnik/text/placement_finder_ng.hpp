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

//stl
#include <list>

//boost
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

namespace mapnik
{

class label_collision_detector4;
typedef label_collision_detector4 DetectorType;

class feature_impl;
typedef feature_impl Feature;

class text_layout;
typedef boost::shared_ptr<text_layout> text_layout_ptr;

class glyph_positions
{
public:
    glyph_positions(text_layout_ptr layout);
    void point_placement(pixel_position base_point);
private:
    pixel_position base_point_;
    bool point_;
    text_layout_ptr layout_;
};
typedef boost::shared_ptr<glyph_positions> glyph_positions_ptr;

//typedef std::list<placement_positions_ptr> placement_positions_list;

class placement_finder_ng : boost::noncopyable
{
public:
    placement_finder_ng(Feature const& feature,
                        DetectorType & detector,
                        box2d<double> const& extent);

    /** Try to place a single label at the given point. */
    glyph_positions_ptr find_point_placement(text_layout_ptr layout, double pos_x, double pos_y, double angle=0.0);
private:
    Feature const& feature_;
    DetectorType const& detector_;
    box2d<double> const& extent_;
};

}//ns mapnik

#endif // PLACEMENT_FINDER_NG_HPP
