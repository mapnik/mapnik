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
#include <mapnik/debug.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/marker_helpers.hpp>
#include <mapnik/svg/svg_renderer.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
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

// boost
#include <boost/optional.hpp>

namespace mapnik {

template <typename T>
void agg_renderer<T>::process(markers_symbolizer const& sym,
                              mapnik::feature_impl & feature,
                              proj_transform const& prj_trans)
{
    typedef agg::conv_clip_polyline<geometry_type> clipped_geometry_type;
    typedef coord_transform<CoordTransform,clipped_geometry_type> path_type;
    typedef agg::conv_transform<path_type, agg::trans_affine> transformed_path_type;
    typedef agg::rgba8 color_type;
    typedef agg::order_rgba order_type;
    typedef agg::pixel32_type pixel_type;
    typedef agg::comp_op_adaptor_rgba_pre<color_type, order_type> blender_type; // comp blender
    typedef agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer> pixfmt_comp_type;
    typedef agg::renderer_base<pixfmt_comp_type> renderer_base;
    typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_type;

    ras_ptr->reset();
    ras_ptr->gamma(agg::gamma_power());
    agg::scanline_u8 sl;
    agg::rendering_buffer buf(current_buffer_->raw_data(), width_, height_, width_ * 4);
    pixfmt_comp_type pixf(buf);
    pixf.comp_op(static_cast<agg::comp_op_e>(sym.comp_op()));
    renderer_base renb(pixf);
    renderer_type ren(renb);

    agg::trans_affine geom_tr;
    evaluate_transform(geom_tr, feature, sym.get_transform());

    std::string filename = path_processor_type::evaluate(*sym.get_filename(), feature);
    marker_placement_e placement_method = sym.get_marker_placement();

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

            boost::optional<path_ptr> marker = (*mark)->get_vector_data();
            box2d<double> const& bbox = (*marker)->bounding_box();

            agg::trans_affine tr;
            setup_label_transform(tr, bbox, feature, sym);
            tr = agg::trans_affine_scaling(scale_factor_) * tr;

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
                         agg::pixfmt_rgba32 > svg_renderer(svg_path, result ? attributes : (*marker)->attributes());

            BOOST_FOREACH( geometry_type & geom, feature.paths())
            {
                // TODO - merge this code with point_symbolizer rendering
                if (placement_method == MARKER_POINT_PLACEMENT || geom.num_points() <= 1)
                {
                    double x;
                    double y;
                    double z=0;
                    geom.label_interior_position(&x, &y);
                    prj_trans.backward(x,y,z);
                    t_.forward(&x,&y);
                    geom_tr.transform(&x,&y);
                    agg::trans_affine matrix = marker_trans;
                    matrix.translate(x,y);
                    box2d<double> transformed_bbox = bbox * matrix;

                    if (sym.get_allow_overlap() ||
                        detector_->has_placement(transformed_bbox))
                    {
                        svg_renderer.render(*ras_ptr, sl, renb, matrix, sym.get_opacity(), bbox);
                        if (/* DEBUG */ 0)
                        {
                            debug_draw_box(buf, transformed_bbox, 0, 0, 0.0);
                        }

                        if (!sym.get_ignore_placement())
                            detector_->insert(transformed_bbox);
                    }
                }
                else
                {
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
                        agg::trans_affine matrix = marker_trans;
                        matrix.rotate(angle);
                        matrix.translate(x, y);
                        svg_renderer.render(*ras_ptr, sl, renb, matrix, sym.get_opacity(), bbox);

                        if (/* DEBUG */ 0)
                        {
                            debug_draw_box(buf, bbox*matrix, 0, 0, 0.0);
                        }
                    }
                }
            }
        }
    }
}


template void agg_renderer<image_32>::process(markers_symbolizer const&,
                                              mapnik::feature_impl &,
                                              proj_transform const&);
}
