/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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
#include <mapnik/symbolizer_helpers.hpp>
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/placement_finder.hpp>
#include <mapnik/geom_util.hpp>

// agg
#include "agg_conv_clip_polyline.h"

namespace mapnik {

template <typename FaceManagerT, typename DetectorT>
bool text_symbolizer_helper<FaceManagerT, DetectorT>::next()
{
    if (!placement_valid_) return false;
    if (point_placement_)
        return next_point_placement();
    else if (sym_.clip())
        return next_line_placement_clipped();
    else
        return next_line_placement();
}

template <typename FaceManagerT, typename DetectorT>
bool text_symbolizer_helper<FaceManagerT, DetectorT>::next_line_placement()
{
    while (!geometries_to_process_.empty())
    {
        if (geo_itr_ == geometries_to_process_.end())
        {
            //Just processed the last geometry. Try next placement.
            if (!next_placement()) return false; //No more placements
            //Start again from begin of list
            geo_itr_ = geometries_to_process_.begin();
            continue; //Reexecute size check
        }

        typedef coord_transform<CoordTransform,geometry_type> path_type;
        path_type path(t_, **geo_itr_, prj_trans_);

        finder_->clear_placements();
        if (points_on_line_) {
            finder_->find_point_placements(path);
        } else {
            finder_->find_line_placements(path);
        }
        if (!finder_->get_results().empty())
        {
            //Found a placement
            if (points_on_line_)
            {
                finder_->update_detector();
            }
            geo_itr_ = geometries_to_process_.erase(geo_itr_);
            return true;
        }
        //No placement for this geometry. Keep it in geometries_to_process_ for next try.
        geo_itr_++;
    }
    return false;
}

template <typename FaceManagerT, typename DetectorT>
bool text_symbolizer_helper<FaceManagerT, DetectorT>::next_line_placement_clipped()
{
    while (!geometries_to_process_.empty())
    {
        if (geo_itr_ == geometries_to_process_.end())
        {
            //Just processed the last geometry. Try next placement.
            if (!next_placement()) return false; //No more placements
            //Start again from begin of list
            geo_itr_ = geometries_to_process_.begin();
            continue; //Reexecute size check
        }

        typedef agg::conv_clip_polyline<geometry_type> clipped_geometry_type;
        typedef coord_transform<CoordTransform,clipped_geometry_type> path_type;
        clipped_geometry_type clipped(**geo_itr_);
        clipped.clip_box(query_extent_.minx(),query_extent_.miny(),query_extent_.maxx(),query_extent_.maxy());
        path_type path(t_, clipped, prj_trans_);

        finder_->clear_placements();
        if (points_on_line_) {
            finder_->find_point_placements(path);
        } else {
            finder_->find_line_placements(path);
        }
        if (!finder_->get_results().empty())
        {
            //Found a placement
            if (points_on_line_)
            {
                finder_->update_detector();
            }
            geo_itr_ = geometries_to_process_.erase(geo_itr_);
            return true;
        }
        //No placement for this geometry. Keep it in geometries_to_process_ for next try.
        geo_itr_++;
    }
    return false;
}

template <typename FaceManagerT, typename DetectorT>
bool text_symbolizer_helper<FaceManagerT, DetectorT>::next_point_placement()
{
    while (!points_.empty())
    {
        if (point_itr_ == points_.end())
        {
            //Just processed the last point. Try next placement.
            if (!next_placement()) return false; //No more placements
            //Start again from begin of list
            point_itr_ = points_.begin();
            continue; //Reexecute size check
        }
        finder_->clear_placements();
        finder_->find_point_placement(point_itr_->first, point_itr_->second, angle_);
        if (!finder_->get_results().empty())
        {
            //Found a placement
            point_itr_ = points_.erase(point_itr_);
            finder_->update_detector();
            return true;
        }
        //No placement for this point. Keep it in points_ for next try.
        point_itr_++;
    }
    return false;
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

template <typename FaceManagerT, typename DetectorT>
void text_symbolizer_helper<FaceManagerT, DetectorT>::initialize_geometries()
{
    bool largest_box_only = false;
    unsigned num_geom = feature_.num_geometries();
    for (unsigned i=0; i<num_geom; ++i)
    {
        geometry_type const& geom = feature_.get_geometry(i);

        // don't bother with empty geometries
        if (geom.size() == 0) continue;
        eGeomType type = geom.type();
        if (type == Polygon)
        {
            largest_box_only = sym_.largest_bbox_only();
            if (sym_.get_minimum_path_length() > 0)
            {
                box2d<double> gbox = t_.forward(geom.envelope(), prj_trans_);
                if (gbox.width() < sym_.get_minimum_path_length())
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

template <typename FaceManagerT, typename DetectorT>
void text_symbolizer_helper<FaceManagerT, DetectorT>::initialize_points()
{
    label_placement_enum how_placed = placement_->properties.label_placement;
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

    std::list<geometry_type*>::const_iterator itr = geometries_to_process_.begin();
    std::list<geometry_type*>::const_iterator end = geometries_to_process_.end();
    for (; itr != end; itr++)
    {
        geometry_type const& geom = **itr;
        if (how_placed == VERTEX_PLACEMENT)
        {
            geom.rewind(0);
            for(unsigned i = 0; i < geom.size(); i++)
            {
                geom.vertex(&label_x, &label_y);
                prj_trans_.backward(label_x, label_y, z);
                t_.forward(&label_x, &label_y);
                points_.push_back(std::make_pair(label_x, label_y));
            }
        }
        else
        {
            // https://github.com/mapnik/mapnik/issues/1423
            bool success = false;
            // https://github.com/mapnik/mapnik/issues/1350
            if (geom.type() == LineString)
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
                points_.push_back(std::make_pair(label_x, label_y));
            }
        }
    }
    point_itr_ = points_.begin();
}


template <typename FaceManagerT, typename DetectorT>
bool text_symbolizer_helper<FaceManagerT, DetectorT>::next_placement()
{
    if (!placement_->next()) {
        placement_valid_ = false;
        return false;
    }
    placement_->properties.process(text_, feature_);
    info_ = &(text_.get_string_info());
    if (placement_->properties.orientation)
    {
        // https://github.com/mapnik/mapnik/issues/1352
        mapnik::evaluate<Feature, value_type> evaluator(feature_);
        angle_ = boost::apply_visitor(
            evaluator,
            *(placement_->properties.orientation)).to_double();
    } else {
        angle_ = 0.0;
    }


    finder_ = boost::shared_ptr<placement_finder<DetectorT> >(new placement_finder<DetectorT>(feature_, *placement_, *info_, detector_, dims_));
//    boost::make_shared<placement_finder<DetectorT> >(feature_, *placement_, *info_, detector_, dims_);

    placement_valid_ = true;
    return true;
}

template <typename FaceManagerT, typename DetectorT>
placements_type const& text_symbolizer_helper<FaceManagerT, DetectorT>::placements() const
{
    return finder_->get_results();
}


/*****************************************************************************/


template <typename FaceManagerT, typename DetectorT>
bool shield_symbolizer_helper<FaceManagerT, DetectorT>::next()
{
    if (!placement_valid_ || !marker_) return false;
    if (point_placement_)
        return next_point_placement();
    else
        return next_line_placement();
}

template <typename FaceManagerT, typename DetectorT>
bool shield_symbolizer_helper<FaceManagerT, DetectorT>::next_point_placement()
{
    position const& shield_pos = sym_.get_shield_displacement();
    while (!points_.empty())
    {
        if (point_itr_ == points_.end())
        {
            //Just processed the last point. Try next placement.
            if (!next_placement()) return false; //No more placements
            //Start again from begin of list
            point_itr_ = points_.begin();
            continue; //Reexecute size check
        }
        position const& text_disp = placement_->properties.displacement;
        double label_x = point_itr_->first + shield_pos.first;
        double label_y = point_itr_->second + shield_pos.second;

        finder_->clear_placements();
        finder_->find_point_placement(label_x, label_y, angle_);
        if (finder_->get_results().empty())
        {
            //No placement for this point. Keep it in points_ for next try.
            point_itr_++;
            continue;
        }
        //Found a label placement but not necessarily also a marker placement
        // check to see if image overlaps anything too, there is only ever 1 placement found for points and verticies
        if (!sym_.get_unlock_image())
        {
            // center image at text center position
            // remove displacement from image label
            placements_type const& p = finder_->get_results();
            double lx = p[0].center.x - text_disp.first;
            double ly = p[0].center.y - text_disp.second;
            marker_x_ = lx - 0.5 * marker_w_;
            marker_y_ = ly - 0.5 * marker_h_;
            marker_ext_.re_center(lx, ly);
        }
        else
        {  // center image at reference location
            marker_x_ = label_x - 0.5 * marker_w_;
            marker_y_ = label_y - 0.5 * marker_h_;
            marker_ext_.re_center(label_x, label_y);
        }

        if (placement_->properties.allow_overlap || detector_.has_placement(marker_ext_))
        {
            detector_.insert(marker_ext_);
            finder_->update_detector();
            point_itr_ = points_.erase(point_itr_);
            return true;
        }
        //No placement found. Try again
        point_itr_++;
    }
    return false;
}


template <typename FaceManagerT, typename DetectorT>
bool shield_symbolizer_helper<FaceManagerT, DetectorT>::next_line_placement()
{
    position const& pos = placement_->properties.displacement;
    finder_->additional_boxes().clear();
    //Markers are automatically centered
    finder_->additional_boxes().push_back(
        box2d<double>(-0.5 * marker_ext_.width()  - pos.first,
                      -0.5 * marker_ext_.height() - pos.second,
                      0.5 * marker_ext_.width()  - pos.first,
                      0.5 * marker_ext_.height() - pos.second));
    if ( sym_.clip())
        return text_symbolizer_helper<FaceManagerT, DetectorT>::next_line_placement_clipped();
    else
        return text_symbolizer_helper<FaceManagerT, DetectorT>::next_line_placement();
}


template <typename FaceManagerT, typename DetectorT>
void shield_symbolizer_helper<FaceManagerT, DetectorT>::init_marker()
{
    std::string filename = path_processor_type::evaluate(*sym_.get_filename(), this->feature_);
    evaluate_transform(image_transform_, feature_, sym_.get_image_transform());
    marker_.reset();
    if (!filename.empty())
    {
        marker_ = marker_cache::instance().find(filename, true);
    }
    if (!marker_) {
        marker_w_ = 0;
        marker_h_ = 0;
        marker_ext_.init(0, 0, 0, 0);
        return;
    }
    marker_w_ = (*marker_)->width();
    marker_h_ = (*marker_)->height();
    double px0 = - 0.5 * marker_w_;
    double py0 = - 0.5 * marker_h_;
    double px1 = 0.5 * marker_w_;
    double py1 = 0.5 * marker_h_;
    double px2 = px1;
    double py2 = py0;
    double px3 = px0;
    double py3 = py1;
    image_transform_.transform(&px0,&py0);
    image_transform_.transform(&px1,&py1);
    image_transform_.transform(&px2,&py2);
    image_transform_.transform(&px3,&py3);
    marker_ext_.init(px0, py0, px1, py1);
    marker_ext_.expand_to_include(px2, py2);
    marker_ext_.expand_to_include(px3, py3);
}

template <typename FaceManagerT, typename DetectorT>
pixel_position shield_symbolizer_helper<FaceManagerT, DetectorT>::get_marker_position(text_path const& p)
{
    position const& pos = placement_->properties.displacement;
    if (placement_->properties.label_placement == LINE_PLACEMENT) {
        double lx = p.center.x - pos.first;
        double ly = p.center.y - pos.second;
        double px = lx - 0.5*marker_w_;
        double py = ly - 0.5*marker_h_;
        marker_ext_.re_center(lx, ly);
        //label is added to detector by get_line_placement(), but marker isn't
        detector_.insert(marker_ext_);
        return pixel_position(px, py);
    } else {
        //collision_detector is already updated for point placement in get_point_placement()
        return pixel_position(marker_x_, marker_y_);
    }
}


template <typename FaceManagerT, typename DetectorT>
marker& shield_symbolizer_helper<FaceManagerT, DetectorT>::get_marker() const
{
    return **marker_;
}

template <typename FaceManagerT, typename DetectorT>
agg::trans_affine const& shield_symbolizer_helper<FaceManagerT, DetectorT>::get_image_transform() const
{
    return image_transform_;
}

template class text_symbolizer_helper<face_manager<freetype_engine>, label_collision_detector4>;
template class shield_symbolizer_helper<face_manager<freetype_engine>, label_collision_detector4>;
} //namespace
