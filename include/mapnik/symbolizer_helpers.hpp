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

//mapnik
#include <mapnik/text_symbolizer.hpp>
#include <mapnik/shield_symbolizer.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/text/placement_finder_ng.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/ctrans.hpp>

//boost
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
                           CoordTransform const& t,
                           FaceManagerT &font_manager,
                           DetectorT &detector,
                           box2d<double> const& query_extent);

    /** Return next placement.
     * If no more placements are found false is returned.
     */
    glyph_positions_ptr next();

protected:
    glyph_positions_ptr next_point_placement();
    glyph_positions_ptr next_line_placement();
    void initialize_geometries();
    void initialize_points();
    void update_detector(glyph_positions_ptr glyphs);

    //Input
    text_symbolizer const& sym_;
    Feature const& feature_;
    proj_transform const& prj_trans_;
    CoordTransform const& t_;
    DetectorT & detector_;
    metawriter_with_properties writer_;
    box2d<double> dims_;
    box2d<double> const& query_extent_;
    //Processing
    /* Using list instead of vector, because we delete random elements and need iterators to stay valid. */
    /** Remaining geometries to be processed. */
    std::list<geometry_type*> geometries_to_process_;
    /** Geometry currently being processed. */
    std::list<geometry_type*>::iterator geo_itr_;
    /** Remaining points to be processed. */
    std::list<pixel_position> points_;
    /** Point currently being processed. */
    std::list<pixel_position>::iterator point_itr_;
    /** Use point placement. Otherwise line placement is used. */
    bool point_placement_;
    /** Place text at points on a line instead of following the line (used for ShieldSymbolizer) .*/
    bool points_on_line_;

    text_placement_info_ptr placement_;
    placement_finder_ng finder_;
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
                             DetectorT &detector,
                             box2d<double> const& query_extent);

    box2d<double> const& get_marker_extent() const
    {
        return marker_ext_;
    }

    pixel_position get_marker_size() const
    {
        return marker_size_;
    }

    bool next();
    pixel_position get_marker_position(glyph_positions_ptr p);
    marker & get_marker() const;
    agg::trans_affine const& get_image_transform() const;
protected:
    bool next_point_placement();
    bool next_line_placement();
    void init_marker();
    shield_symbolizer const& sym_;
    box2d<double> marker_ext_;
    boost::optional<marker_ptr> marker_;
    agg::trans_affine image_transform_;
    pixel_position marker_size_;
    pixel_position marker_pos_;
    
    using text_symbolizer_helper<FaceManagerT, DetectorT>::geometries_to_process_;
    using text_symbolizer_helper<FaceManagerT, DetectorT>::placement_;
    using text_symbolizer_helper<FaceManagerT, DetectorT>::geo_itr_;
    using text_symbolizer_helper<FaceManagerT, DetectorT>::point_itr_;
    using text_symbolizer_helper<FaceManagerT, DetectorT>::points_;
    using text_symbolizer_helper<FaceManagerT, DetectorT>::writer_;
    using text_symbolizer_helper<FaceManagerT, DetectorT>::feature_;
    using text_symbolizer_helper<FaceManagerT, DetectorT>::t_;
    using text_symbolizer_helper<FaceManagerT, DetectorT>::detector_;
    using text_symbolizer_helper<FaceManagerT, DetectorT>::dims_;
    using text_symbolizer_helper<FaceManagerT, DetectorT>::prj_trans_;
    using text_symbolizer_helper<FaceManagerT, DetectorT>::point_placement_;
    using text_symbolizer_helper<FaceManagerT, DetectorT>::finder_;
};
} //namespace
#endif // SYMBOLIZER_HELPERS_HPP
