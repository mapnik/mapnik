/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
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

// mapnik
#include <mapnik/text/symbolizer_helpers.hpp>
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/text/text_layout.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/text/placement_finder_impl.hpp>
#include <mapnik/text/placements/base.hpp>
#include <mapnik/text/placements/dummy.hpp>

//agg
#include "agg_conv_clip_polyline.h"

namespace mapnik {

base_symbolizer_helper::base_symbolizer_helper(
        symbolizer_base const& sym,
        feature_impl const& feature,
        attributes const& vars,
        proj_transform const& prj_trans,
        unsigned width, unsigned height, double scale_factor,
        view_transform const& t,
        box2d<double> const& query_extent)
    : sym_(sym),
      feature_(feature),
      vars_(vars),
      prj_trans_(prj_trans),
      t_(t),
      dims_(0, 0, width, height),
      query_extent_(query_extent),
      scale_factor_(scale_factor),
      info_ptr_(mapnik::get<text_placements_ptr>(sym_, keys::text_placements_)->get_placement_info(scale_factor)),
      text_props_(evaluate_text_properties(info_ptr_->properties,feature_,vars_))
{
    initialize_geometries();
    if (!geometries_to_process_.size()) return; // FIXME - bad practise
    initialize_points();
}

struct largest_bbox_first
{
    bool operator() (geometry_type const* g0, geometry_type const* g1) const
    {
        box2d<double> b0 = g0->envelope();
        box2d<double> b1 = g1->envelope();
        return b0.width()*b0.height() > b1.width()*b1.height();
    }
};

void base_symbolizer_helper::initialize_geometries() const
{
    bool largest_box_only = text_props_->largest_bbox_only;
    double minimum_path_length = text_props_->minimum_path_length;
    for ( auto const& geom :  feature_.paths())
    {
        // don't bother with empty geometries
        if (geom.size() == 0) continue;
        mapnik::geometry_type::types type = geom.type();
        if (type == geometry_type::types::Polygon)
        {
            if (minimum_path_length > 0)
            {
                box2d<double> gbox = t_.forward(geom.envelope(), prj_trans_);
                if (gbox.width() < minimum_path_length)
                {
                    continue;
                }
            }
        }
        // TODO - calculate length here as well
        geometries_to_process_.push_back(const_cast<geometry_type*>(&geom));
    }

    if (largest_box_only)
    {
        geometries_to_process_.sort(largest_bbox_first());
        geo_itr_ = geometries_to_process_.begin();
        geometries_to_process_.erase(++geo_itr_,geometries_to_process_.end());
    }
    geo_itr_ = geometries_to_process_.begin();
}

void base_symbolizer_helper::initialize_points() const
{
    label_placement_enum how_placed = text_props_->label_placement;
    if (how_placed == LINE_PLACEMENT)
    {
        point_placement_ = false;
        return;
    }
    else
    {
        point_placement_ = true;
    }

    double label_x=0.0;
    double label_y=0.0;
    double z=0.0;

    for (auto * geom_ptr : geometries_to_process_)
    {
        geometry_type const& geom = *geom_ptr;
        if (how_placed == VERTEX_PLACEMENT)
        {
            geom.rewind(0);
            for(unsigned i = 0; i < geom.size(); ++i)
            {
                geom.vertex(&label_x, &label_y);
                prj_trans_.backward(label_x, label_y, z);
                t_.forward(&label_x, &label_y);
                points_.emplace_back(label_x, label_y);
            }
        }
        else
        {
            // https://github.com/mapnik/mapnik/issues/1423
            bool success = false;
            // https://github.com/mapnik/mapnik/issues/1350
            if (geom.type() == geometry_type::types::LineString)
            {
                success = label::middle_point(geom, label_x,label_y);
            }
            else if (how_placed == POINT_PLACEMENT)
            {
                success = label::centroid(geom, label_x, label_y);
            }
            else if (how_placed == INTERIOR_PLACEMENT)
            {
                success = label::interior_position(geom, label_x, label_y);
            }
            else
            {
                MAPNIK_LOG_ERROR(symbolizer_helpers) << "ERROR: Unknown placement type in initialize_points()";
            }
            if (success)
            {
                prj_trans_.backward(label_x, label_y, z);
                t_.forward(&label_x, &label_y);
                points_.emplace_back(label_x, label_y);
            }
        }
    }
    point_itr_ = points_.begin();
}

template <typename FaceManagerT, typename DetectorT>
text_symbolizer_helper::text_symbolizer_helper(
        text_symbolizer const& sym,
        feature_impl const& feature,
        attributes const& vars,
        proj_transform const& prj_trans,
        unsigned width, unsigned height, double scale_factor,
        view_transform const& t, FaceManagerT & font_manager,
        DetectorT &detector, box2d<double> const& query_extent,
        agg::trans_affine const& affine_trans)
    : base_symbolizer_helper(sym, feature, vars, prj_trans, width, height, scale_factor, t, query_extent),
      finder_(feature, vars, detector, dims_, *info_ptr_, font_manager, scale_factor),
    adapter_(finder_,false),
    converter_(query_extent_, adapter_, sym_, t, prj_trans, affine_trans, feature, vars, scale_factor)
{

    // setup vertex converter
    value_bool clip = mapnik::get<value_bool, keys::clip>(sym_, feature_, vars_);
    value_double simplify_tolerance = mapnik::get<value_double, keys::simplify_tolerance>(sym_, feature_, vars_);
    value_double smooth = mapnik::get<value_double, keys::smooth>(sym_, feature_, vars_);

    if (clip) converter_.template set<clip_line_tag>(); //optional clip (default: true)
    converter_.template set<transform_tag>(); //always transform
    converter_.template set<affine_transform_tag>();
    if (simplify_tolerance > 0.0) converter_.template set<simplify_tag>(); // optional simplify converter
    if (smooth > 0.0) converter_.template set<smooth_tag>(); // optional smooth converter

    if (geometries_to_process_.size()) finder_.next_position();
}

placements_list const& text_symbolizer_helper::get() const
{
    if (point_placement_)
    {
        while (next_point_placement());
    }
    else
    {
        while (next_line_placement());
    }
    return finder_.placements();
}

bool text_symbolizer_helper::next_line_placement() const
{
    while (!geometries_to_process_.empty())
    {
        if (geo_itr_ == geometries_to_process_.end())
        {
            //Just processed the last geometry. Try next placement.
            if (!finder_.next_position()) return false; //No more placements
            //Start again from begin of list
            geo_itr_ = geometries_to_process_.begin();
            continue; //Reexecute size check
        }

        converter_.apply(**geo_itr_);
        if (adapter_.status())
        {
            //Found a placement
            geo_itr_ = geometries_to_process_.erase(geo_itr_);
            return true;
        }
        // No placement for this geometry. Keep it in geometries_to_process_ for next try.
        ++geo_itr_;
    }
    return false;
}

bool text_symbolizer_helper::next_point_placement() const
{
    while (!points_.empty())
    {
        if (point_itr_ == points_.end())
        {
            //Just processed the last point. Try next placement.
            if (!finder_.next_position()) return false; //No more placements
            //Start again from begin of list
            point_itr_ = points_.begin();
            continue; //Reexecute size check
        }
        if (finder_.find_point_placement(*point_itr_))
        {
            //Found a placement
            point_itr_ = points_.erase(point_itr_);
            return true;
        }
        //No placement for this point. Keep it in points_ for next try.
        point_itr_++;
    }
    return false;
}

template <typename FaceManagerT, typename DetectorT>
text_symbolizer_helper::text_symbolizer_helper(
        shield_symbolizer const& sym,
        feature_impl const& feature,
        attributes const& vars,
        proj_transform const& prj_trans,
        unsigned width, unsigned height, double scale_factor,
        view_transform const& t, FaceManagerT & font_manager,
        DetectorT & detector, box2d<double> const& query_extent, agg::trans_affine const& affine_trans)
    : base_symbolizer_helper(sym, feature, vars, prj_trans, width, height, scale_factor, t, query_extent),
      finder_(feature, vars, detector, dims_, *info_ptr_, font_manager, scale_factor),
      adapter_(finder_,true),
      converter_(query_extent_, adapter_, sym_, t, prj_trans, affine_trans, feature, vars, scale_factor)
{
   // setup vertex converter
    value_bool clip = mapnik::get<value_bool, keys::clip>(sym_, feature_, vars_);
    value_double simplify_tolerance = mapnik::get<value_double, keys::simplify_tolerance>(sym_, feature_, vars_);
    value_double smooth = mapnik::get<value_double, keys::smooth>(sym_, feature_, vars_);

    if (clip) converter_.template set<clip_line_tag>(); //optional clip (default: true)
    converter_.template set<transform_tag>(); //always transform
    converter_.template set<affine_transform_tag>();
    if (simplify_tolerance > 0.0) converter_.template set<simplify_tag>(); // optional simplify converter
    if (smooth > 0.0) converter_.template set<smooth_tag>(); // optional smooth converter
    if (geometries_to_process_.size())
    {
        init_marker();
        finder_.next_position();
    }
}


void text_symbolizer_helper::init_marker() const
{
    std::string filename = mapnik::get<std::string,keys::file>(sym_, feature_, vars_);
    if (filename.empty()) return;
    boost::optional<mapnik::marker_ptr> marker = marker_cache::instance().find(filename, true);
    if (!marker) return;
    agg::trans_affine trans;
    auto image_transform = get_optional<transform_type>(sym_, keys::image_transform);
    if (image_transform) evaluate_transform(trans, feature_, vars_, *image_transform);
    double width = (*marker)->width();
    double height = (*marker)->height();
    double px0 = - 0.5 * width;
    double py0 = - 0.5 * height;
    double px1 = 0.5 * width;
    double py1 = 0.5 * height;
    double px2 = px1;
    double py2 = py0;
    double px3 = px0;
    double py3 = py1;
    trans.transform(&px0, &py0);
    trans.transform(&px1, &py1);
    trans.transform(&px2, &py2);
    trans.transform(&px3, &py3);
    box2d<double> bbox(px0, py0, px1, py1);
    bbox.expand_to_include(px2, py2);
    bbox.expand_to_include(px3, py3);
    value_bool unlock_image = mapnik::get<value_bool, keys::unlock_image>(sym_, feature_, vars_);
    value_double shield_dx = mapnik::get<value_double, keys::shield_dx>(sym_, feature_, vars_);
    value_double shield_dy = mapnik::get<value_double, keys::shield_dy>(sym_, feature_, vars_);
    pixel_position marker_displacement;
    marker_displacement.set(shield_dx,shield_dy);
    finder_.set_marker(std::make_shared<marker_info>(*marker, trans), bbox, unlock_image, marker_displacement);
}


template text_symbolizer_helper::text_symbolizer_helper(
    text_symbolizer const& sym,
    feature_impl const& feature,
    attributes const& vars,
    proj_transform const& prj_trans,
    unsigned width,
    unsigned height,
    double scale_factor,
    view_transform const& t,
    face_manager_freetype & font_manager,
    label_collision_detector4 &detector,
    box2d<double> const& query_extent,
    agg::trans_affine const&);

template text_symbolizer_helper::text_symbolizer_helper(
    shield_symbolizer const& sym,
    feature_impl const& feature,
    attributes const& vars,
    proj_transform const& prj_trans,
    unsigned width,
    unsigned height,
    double scale_factor,
    view_transform const& t,
    face_manager_freetype & font_manager,
    label_collision_detector4 &detector,
    box2d<double> const& query_extent,
    agg::trans_affine const&);
} //namespace
