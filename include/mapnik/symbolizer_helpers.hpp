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

/** Helper object that does all the TextSymbolizer placment finding
  * work except actually rendering the object. */
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
                           DetectorT &detector)
        : sym_(sym),
          feature_(feature),
          prj_trans_(prj_trans),
          t_(t),
          font_manager_(font_manager),
          detector_(detector),
          writer_(sym.get_metawriter()),
          dims_(0, 0, width, height),
          text_(font_manager, scale_factor),
          angle_(0.0),
          placement_valid_(true)
      {
          initialize_geometries();
          if (!geometries_to_process_.size()) return; //TODO: Test this
          placement_ = sym_.get_placement_options()->get_placement_info();
          placement_->init(scale_factor, width, height);
          if (writer_.first) placement_->collect_extents = true;
          next_placement();
          initialize_points();
      }

    /** Return next placement.
      * If no more placements are found returns null pointer.
      */
    text_placement_info_ptr get_placement();
    text_placement_info_ptr get_point_placement();
    text_placement_info_ptr get_line_placement();
protected:
    bool next_placement();
    void initialize_geometries();
    void initialize_points();

    //Input
    text_symbolizer const& sym_;
    Feature const& feature_;
    proj_transform const& prj_trans_;
    CoordTransform const &t_;
    FaceManagerT &font_manager_;
    DetectorT &detector_;
    metawriter_with_properties writer_;
    box2d<double> dims_;

    //Processing
    processed_text text_;
    /* Using list instead of vector, because we delete random elements and need iterators to stay valid. */
    std::list<geometry_type*> geometries_to_process_;
    std::list<geometry_type*>::iterator geo_itr_;
    std::list<position> points_;
    std::list<position>::iterator point_itr_;
    double angle_;
    string_info *info_;
    bool placement_valid_;
    bool point_placement_;

    //Output
    text_placement_info_ptr placement_;
};

template <typename FaceManagerT, typename DetectorT>
class shield_symbolizer_helper: public text_symbolizer_helper<FaceManagerT, DetectorT>
{
public:
    shield_symbolizer_helper(shield_symbolizer const& sym,
                             Feature const& feature,
                             proj_transform const& prj_trans,
                             unsigned width,
                             unsigned height,
                             double scale_factor,
                             CoordTransform const &t,
                             FaceManagerT &font_manager,
                             DetectorT &detector) :
        text_symbolizer_helper<FaceManagerT, DetectorT>(sym, feature, prj_trans, width, height, scale_factor, t, font_manager, detector),
        sym_(sym)
    {
        init_marker();
    }

    text_placement_info_ptr get_placement();
    std::pair<int, int> get_marker_position(text_path &p);
    marker &get_marker() const;
    agg::trans_affine const& get_transform() const;
protected:
    text_placement_info_ptr get_point_placement();
    text_placement_info_ptr get_line_placement();
    void init_marker();
    shield_symbolizer const& sym_;
    box2d<double> marker_ext_;
    boost::optional<marker_ptr> marker_;
    agg::trans_affine transform_;
    int marker_w_;
    int marker_h_;
    int marker_x_;
    int marker_y_;
    // F***ing templates...
    // http://womble.decadent.org.uk/c++/template-faq.html#base-lookup
    using text_symbolizer_helper<FaceManagerT, DetectorT>::geometries_to_process_;
    using text_symbolizer_helper<FaceManagerT, DetectorT>::placement_;
    using text_symbolizer_helper<FaceManagerT, DetectorT>::next_placement;
    using text_symbolizer_helper<FaceManagerT, DetectorT>::info_;
    using text_symbolizer_helper<FaceManagerT, DetectorT>::geo_itr_;
    using text_symbolizer_helper<FaceManagerT, DetectorT>::point_itr_;
    using text_symbolizer_helper<FaceManagerT, DetectorT>::points_;
    using text_symbolizer_helper<FaceManagerT, DetectorT>::writer_;
    using text_symbolizer_helper<FaceManagerT, DetectorT>::font_manager_;
    using text_symbolizer_helper<FaceManagerT, DetectorT>::feature_;
    using text_symbolizer_helper<FaceManagerT, DetectorT>::t_;
    using text_symbolizer_helper<FaceManagerT, DetectorT>::detector_;
    using text_symbolizer_helper<FaceManagerT, DetectorT>::dims_;
    using text_symbolizer_helper<FaceManagerT, DetectorT>::prj_trans_;
    using text_symbolizer_helper<FaceManagerT, DetectorT>::placement_valid_;
    using text_symbolizer_helper<FaceManagerT, DetectorT>::point_placement_;
    using text_symbolizer_helper<FaceManagerT, DetectorT>::angle_;
};
} //namespace
#endif // SYMBOLIZER_HELPERS_HPP
