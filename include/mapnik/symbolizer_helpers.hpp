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
#ifndef SYMBOLIZER_HELPERS_HPP
#define SYMBOLIZER_HELPERS_HPP

#include <mapnik/text_symbolizer.hpp>
#include <mapnik/shield_symbolizer.hpp>
#include <mapnik/text_processing.hpp>
#include <mapnik/placement_finder.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>

#include <boost/shared_ptr.hpp>


namespace mapnik {

template <typename FaceManagerT, typename DetectorT>
class text_symbolizer_helper
{
public:
    text_symbolizer_helper(text_symbolizer const& sym,
                           Feature const& feature,
                           proj_transform const& prj_trans,
                           unsigned width,
                           unsigned height,
                           double scale_factor,
                           CoordTransform const &t,
                           FaceManagerT &font_manager,
                           DetectorT &detector) :
        sym_(sym),
        feature_(feature),
        prj_trans_(prj_trans),
        width_(width),
        height_(height),
        scale_factor_(scale_factor),
        t_(t),
        font_manager_(font_manager),
        detector_(detector),
        text_(),
        angle_(0.0)
    {
        initialize_geometries();
        if (!geometries_to_process_.size()) return;
        text_ = boost::shared_ptr<processed_text>(new processed_text(font_manager_, scale_factor_));
        placement_ = sym_.get_placement_options()->get_placement_info();
        placement_->init(scale_factor_, width_, height_);
        metawriter_with_properties writer = sym_.get_metawriter();
        if (writer.first)
            placement_->collect_extents = true;
        itr_ = geometries_to_process_.begin();
        next_placement();
    }

    bool next_placement()
    {
        if (!placement_->next()) return false;
        /* TODO: Simplify this. */
        text_->clear();
        placement_->properties.processor.process(*text_, feature_);
        info_ = &(text_->get_string_info());
        if (placement_->properties.orientation)
        {
            angle_ = boost::apply_visitor(
                        evaluate<Feature, value_type>(feature_),
                        *(placement_->properties.orientation)).to_double();
        } else {
            angle_ = 0.0;
        }
        /* END TODO */
        return true;
    }

    /** Return next placement.
      * If no more placements are found returns null pointer.
      * TODO: Currently stops returning placements to early.
      */
    text_placement_info_ptr get_placement();
private:
    void initialize_geometries();

    text_symbolizer const& sym_;
    Feature const& feature_;
    proj_transform const& prj_trans_;
    unsigned width_;
    unsigned height_;
    double scale_factor_;
    CoordTransform const &t_;
    FaceManagerT &font_manager_;
    DetectorT &detector_;
    boost::shared_ptr<processed_text> text_; /*TODO: Use shared pointers for text placement so we don't need to keep a reference here! */
    std::vector<geometry_type*> geometries_to_process_;
    std::vector<geometry_type*>::iterator itr_;
    text_placement_info_ptr placement_;
    double angle_;
    string_info *info_;
};


template <typename FaceManagerT, typename DetectorT>
text_placement_info_ptr text_symbolizer_helper<FaceManagerT, DetectorT>::get_placement()
{
    if (!geometries_to_process_.size()) return text_placement_info_ptr();
    metawriter_with_properties writer = sym_.get_metawriter();
    box2d<double> dims(0, 0, width_, height_);

    while (geometries_to_process_.size())
    {
        if (itr_ == geometries_to_process_.end()) {
            //Just processed the last geometry. Try next placement.
            if (!next_placement()) return text_placement_info_ptr(); //No more placements
            //Start again from begin of list
            itr_ = geometries_to_process_.begin();
        }
        //TODO: Avoid calling constructor repeatedly
        placement_finder<DetectorT> finder(*placement_, *info_, detector_, dims);
        finder.find_placement(angle_, **itr_, t_, prj_trans_);
        if (placement_->placements.size())
        {
            //Found a placement
            geometries_to_process_.erase(itr_); //Remove current geometry
            if (writer.first) writer.first->add_text(*placement_, font_manager_, feature_, t_, writer.second);
            itr_++;
            return placement_;
        }
        //No placement for this geometry. Keep it in geometries_to_process_ for next try.
        itr_++;

    }
    return text_placement_info_ptr();
}

template <typename FaceManagerT, typename DetectorT>
void text_symbolizer_helper<FaceManagerT, DetectorT>::initialize_geometries()
{
    unsigned num_geom = feature_.num_geometries();
    for (unsigned i=0; i<num_geom; ++i)
    {
        geometry_type const& geom = feature_.get_geometry(i);

        // don't bother with empty geometries
        if (geom.num_points() == 0) continue;

        if ((geom.type() == Polygon) && sym_.get_minimum_path_length() > 0)
        {
            // TODO - find less costly method than fetching full envelope
            box2d<double> gbox = t_.forward(geom.envelope(), prj_trans_);
            if (gbox.width() < sym_.get_minimum_path_length())
            {
                continue;
            }
        }
        // TODO - calculate length here as well
        geometries_to_process_.push_back(const_cast<geometry_type*>(&geom));
    }
}


/*****************************************************************************/

template <typename FaceManagerT, typename DetectorT>
class shield_symbolizer_helper
{
public:
    shield_symbolizer_helper(unsigned width,
                           unsigned height,
                           double scale_factor,
                           CoordTransform const &t,
                           FaceManagerT &font_manager,
                           DetectorT &detector) :
        width_(width),
        height_(height),
        scale_factor_(scale_factor),
        t_(t),
        font_manager_(font_manager),
        detector_(detector),
        text_()
    {

    }

    text_placement_info_ptr get_placement(shield_symbolizer const& sym,
                                          Feature const& feature,
                                          proj_transform const& prj_trans);
private:
    void init_marker(shield_symbolizer const& sym, Feature const& feature);
    unsigned width_;
    unsigned height_;
    double scale_factor_;
    CoordTransform const &t_;
    FaceManagerT &font_manager_;
    DetectorT &detector_;
    boost::shared_ptr<processed_text> text_;
    box2d<double> label_ext_;
    boost::optional<marker_ptr> marker_;
    agg::trans_affine transform_;
    int marker_w;
    int marker_h;
};


template <typename FaceManagerT, typename DetectorT>
text_placement_info_ptr shield_symbolizer_helper<FaceManagerT, DetectorT>::get_placement(
    shield_symbolizer const& sym,
    Feature const& feature,
    proj_transform const& prj_trans)
{
    init_marker(sym);
    if (!marker_) return text_placement_info_ptr();
}

template <typename FaceManagerT, typename DetectorT>
void shield_symbolizer_helper<FaceManagerT, DetectorT>::init_marker(shield_symbolizer const& sym, Feature const& feature)
{
    std::string filename = path_processor_type::evaluate(*sym.get_filename(), feature);
    boost::array<double,6> const& m = sym.get_transform();
    transform_.load_from(&m[0]);
    marker_.reset();
    if (!filename.empty())
    {
        marker_ = marker_cache::instance()->find(filename, true);
    }
    if (!marker_) {
        marker_w = 0;
        marker_h = 0;
        label_ext_.init(0, 0, 0, 0);
        return;
    }
    marker_w = (*marker_)->width();
    marker_h = (*marker_)->height();
    double px0 = - 0.5 * marker_w;
    double py0 = - 0.5 * marker_h;
    double px1 = 0.5 * marker_w;
    double py1 = 0.5 * marker_h;
    double px2 = px1;
    double py2 = py0;
    double px3 = px0;
    double py3 = py1;
    transform_.transform(&px0,&py0);
    transform_.transform(&px1,&py1);
    transform_.transform(&px2,&py2);
    transform_.transform(&px3,&py3);
    label_ext_.init(px0, py0, px1, py1);
    label_ext_.expand_to_include(px2, py2);
    label_ext_.expand_to_include(px3, py3);
}

} //namespace
#endif // SYMBOLIZER_HELPERS_HPP
