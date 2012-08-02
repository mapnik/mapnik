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

/*

porting notes -->

 - grid includes
 - detector
 - no gamma
 - mapnik::pixfmt_gray32
 - agg::scanline_bin sl
 - grid_rendering_buffer
 - agg::renderer_scanline_bin_solid
 - clamping:
    // - clamp sizes to > 4 pixels of interactivity
    if (tr.scale() < 0.5)
    {
        agg::trans_affine tr2;
        tr2 *= agg::trans_affine_scaling(0.5);
        tr = tr2;
    }
    tr *= agg::trans_affine_scaling(scale_factor_*(1.0/pixmap_.get_resolution()));
 - svg_renderer.render_id
 - only encode feature if placements are found:
    if (placed)
    {
        pixmap_.add_feature(feature);
    }

*/

// mapnik
#include <mapnik/grid/grid_rasterizer.hpp>
#include <mapnik/grid/grid_renderer.hpp>
#include <mapnik/grid/grid_pixfmt.hpp>
#include <mapnik/grid/grid_pixel.hpp>
#include <mapnik/grid/grid.hpp>

#include <mapnik/debug.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/marker_helpers.hpp>
#include <mapnik/svg/svg_renderer.hpp>
#include <mapnik/svg/svg_storage.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/markers_placement.hpp>
#include <mapnik/markers_symbolizer.hpp>

// agg
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_path_storage.h"
#include "agg_conv_clip_polyline.h"
#include "agg_conv_transform.h"
#include "agg_image_filters.h"
#include "agg_trans_bilinear.h"
#include "agg_span_allocator.h"
#include "agg_image_accessors.h"
#include "agg_span_image_filter_rgba.h"

// boost
#include <boost/optional.hpp>

// stl
#include <algorithm>


namespace mapnik {

template <typename T>
void grid_renderer<T>::process(markers_symbolizer const& sym,
                               mapnik::feature_impl & feature,
                               proj_transform const& prj_trans)
{
    typedef agg::renderer_base<mapnik::pixfmt_gray32> renderer_base;
    typedef agg::renderer_scanline_bin_solid<renderer_base> renderer_type;

    std::string filename = path_processor_type::evaluate(*sym.get_filename(), feature);

    if (!filename.empty())
    {
        boost::optional<marker_ptr> mark = mapnik::marker_cache::instance()->find(filename, true);
        if (mark && *mark)
        {
            if (!(*mark)->is_vector())
            {
                MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: markers_symbolizer does not yet support non-SVG markers";
                return;
            }

            ras_ptr->reset();
            agg::scanline_bin sl;
            grid_rendering_buffer buf(pixmap_.raw_data(), width_, height_, width_);
            mapnik::pixfmt_gray32 pixf(buf);
            renderer_base renb(pixf);
            renderer_type ren(renb);

            agg::trans_affine geom_tr;
            evaluate_transform(geom_tr, feature, sym.get_transform());

            boost::optional<svg_path_ptr> marker = (*mark)->get_vector_data();
            box2d<double> const& bbox = (*marker)->bounding_box();

            agg::trans_affine tr;
            setup_label_transform(tr, bbox, feature, sym);
            // - clamp sizes to > 4 pixels of interactivity
            if (tr.scale() < 0.5)
            {
                agg::trans_affine tr2;
                tr2 *= agg::trans_affine_scaling(0.5);
                tr = tr2;
            }
            tr *= agg::trans_affine_scaling(scale_factor_*(1.0/pixmap_.get_resolution()));

            coord2d center = bbox.center();
            agg::trans_affine_translation recenter(-center.x, -center.y);
            agg::trans_affine marker_trans = recenter * tr;

            using namespace mapnik::svg;
            vertex_stl_adapter<svg_path_storage> stl_storage((*marker)->source());
            svg_path_adapter svg_path(stl_storage);

            agg::pod_bvector<path_attributes> attributes;
            bool result = push_explicit_style( (*marker)->attributes(), attributes, sym);

            svg_renderer<svg_path_adapter,
                         agg::pod_bvector<path_attributes>,
                         renderer_type,
                         mapnik::pixfmt_gray32 > svg_renderer(svg_path, result ? attributes : (*marker)->attributes());

            marker_placement_e placement_method = sym.get_marker_placement();

            bool placed = false;
            BOOST_FOREACH( geometry_type & geom, feature.paths())
            {
                // TODO - merge this code with point_symbolizer rendering
                if (placement_method == MARKER_POINT_PLACEMENT || geom.size() <= 1)
                {
                    double x;
                    double y;
                    double z=0;
                    label::interior_position(geom, x, y);
                    prj_trans.backward(x,y,z);
                    t_.forward(&x,&y);
                    geom_tr.transform(&x,&y);
                    agg::trans_affine matrix = marker_trans;
                    matrix.translate(x,y);
                    box2d<double> transformed_bbox = bbox * matrix;

                    if (sym.get_allow_overlap() ||
                        detector_->has_placement(transformed_bbox))
                    {
                        placed = true;
                        svg_renderer.render_id(*ras_ptr, sl, renb, feature.id(), matrix, 1, bbox);
                        if (!sym.get_ignore_placement())
                            detector_->insert(transformed_bbox);
                    }
                }
                else if (sym.clip())
                {
                    typedef agg::conv_clip_polyline<geometry_type> clipped_geometry_type;
                    typedef coord_transform<CoordTransform,clipped_geometry_type> path_type;
                    typedef agg::conv_transform<path_type, agg::trans_affine> transformed_path_type;

                    clipped_geometry_type clipped(geom);
                    clipped.clip_box(query_extent_.minx(),query_extent_.miny(),query_extent_.maxx(),query_extent_.maxy());
                    path_type path(t_,clipped,prj_trans);
                    transformed_path_type path_transformed(path,geom_tr);
                    markers_placement<transformed_path_type, label_collision_detector4> placement(path_transformed, bbox, marker_trans, *detector_,
                                                                                                  sym.get_spacing() * scale_factor_,
                                                                                                  sym.get_max_error(),
                                                                                                  sym.get_allow_overlap());
                    double x, y, angle;
                    while (placement.get_point(x, y, angle))
                    {
                        placed = true;
                        agg::trans_affine matrix = marker_trans;
                        matrix.rotate(angle);
                        matrix.translate(x, y);
                        svg_renderer.render_id(*ras_ptr, sl, renb, feature.id(), matrix, 1, bbox);
                    }
                }
                else
                {
                    typedef coord_transform<CoordTransform,geometry_type> path_type;
                    typedef agg::conv_transform<path_type, agg::trans_affine> transformed_path_type;
                    path_type path(t_,geom,prj_trans);
                    transformed_path_type path_transformed(path,geom_tr);
                    markers_placement<transformed_path_type, label_collision_detector4> placement(path_transformed, bbox, marker_trans, *detector_,
                                                                                                  sym.get_spacing() * scale_factor_,
                                                                                                  sym.get_max_error(),
                                                                                                  sym.get_allow_overlap());
                    double x, y, angle;
                    while (placement.get_point(x, y, angle))
                    {
                        placed = true;
                        agg::trans_affine matrix = marker_trans;
                        matrix.rotate(angle);
                        matrix.translate(x, y);
                        svg_renderer.render_id(*ras_ptr, sl, renb, feature.id(), matrix, 1, bbox);
                    }
                }
            }
            if (placed)
            {
                pixmap_.add_feature(feature);
            }
        }
    }
}

template void grid_renderer<grid>::process(markers_symbolizer const&,
                                           mapnik::feature_impl &,
                                           proj_transform const&);
}
