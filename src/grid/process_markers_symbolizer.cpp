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

 - rasterizer -> grid_rasterizer
 - current_buffer_ -> pixmap_
 - agg::rendering_buffer -> grid_renderering_buffer
 - no gamma
 - mapnik::pixfmt_gray32
 - agg::scanline_bin sl
 - grid_rendering_buffer
 - agg::renderer_scanline_bin_solid
 - TODO - clamp sizes to > 4 pixels of interactivity
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
#include <mapnik/grid/grid_marker_helpers.hpp>

#include <mapnik/debug.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/marker_helpers.hpp>
#include <mapnik/svg/svg_renderer_agg.hpp>
#include <mapnik/svg/svg_storage.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/markers_symbolizer.hpp>

// agg
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa.h"

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
    typedef grid_rendering_buffer buf_type;
    typedef mapnik::pixfmt_gray32 pixfmt_type;
    typedef agg::renderer_base<pixfmt_type> renderer_base;
    typedef agg::renderer_scanline_bin_solid<renderer_base> renderer_type;
    typedef label_collision_detector4 detector_type;
    typedef boost::mpl::vector<clip_line_tag,clip_poly_tag,transform_tag,smooth_tag> conv_types;

    std::string filename = path_processor_type::evaluate(*sym.get_filename(), feature);

    if (!filename.empty())
    {
        boost::optional<marker_ptr> mark = mapnik::marker_cache::instance().find(filename, true);
        if (mark && *mark)
        {
            ras_ptr->reset();
            agg::trans_affine geom_tr;
            evaluate_transform(geom_tr, feature, sym.get_transform());
            agg::trans_affine tr = agg::trans_affine_scaling(scale_factor_*(1.0/pixmap_.get_resolution()));

            if ((*mark)->is_vector())
            {
                using namespace mapnik::svg;
                typedef agg::pod_bvector<path_attributes> svg_attribute_type;
                typedef svg_renderer_agg<svg_path_adapter,
                                     svg_attribute_type,
                                     renderer_type,
                                     pixfmt_type > svg_renderer_type;
                typedef vector_markers_rasterizer_dispatch_grid<buf_type,
                                     svg_renderer_type,
                                     grid_rasterizer,
                                     detector_type,
                                     mapnik::grid > dispatch_type;
                boost::optional<svg_path_ptr> const& stock_vector_marker = (*mark)->get_vector_data();
                expression_ptr const& width_expr = sym.get_width();
                expression_ptr const& height_expr = sym.get_height();

                // special case for simple ellipse markers
                // to allow for full control over rx/ry dimensions
                if (filename == "shape://ellipse"
                   && (width_expr || height_expr))
                {
                    svg_storage_type marker_ellipse;
                    vertex_stl_adapter<svg_path_storage> stl_storage(marker_ellipse.source());
                    svg_path_adapter svg_path(stl_storage);
                    // TODO - clamping to >= 4 pixels
                    build_ellipse(sym, feature, marker_ellipse, svg_path);
                    svg_attribute_type attributes;
                    bool result = push_explicit_style( (*stock_vector_marker)->attributes(), attributes, sym);
                    svg_renderer_type svg_renderer(svg_path, result ? attributes : (*stock_vector_marker)->attributes());
                    evaluate_transform(tr, feature, sym.get_image_transform());
                    box2d<double> bbox = marker_ellipse.bounding_box();
                    coord2d center = bbox.center();
                    agg::trans_affine_translation recenter(-center.x, -center.y);
                    agg::trans_affine marker_trans = recenter * tr;
                    buf_type render_buf(pixmap_.raw_data(), width_, height_, width_);
                    dispatch_type rasterizer_dispatch(render_buf,
                                                      svg_renderer,
                                                      *ras_ptr,
                                                      bbox,
                                                      marker_trans,
                                                      sym,
                                                      *detector_,
                                                      scale_factor_,
                                                      feature,
                                                      pixmap_);
                    vertex_converter<box2d<double>, dispatch_type, markers_symbolizer,
                                     CoordTransform, proj_transform, agg::trans_affine, conv_types>
                        converter(query_extent_, rasterizer_dispatch, sym,t_,prj_trans,tr,scale_factor_);
                    if (sym.clip() && feature.paths().size() > 0) // optional clip (default: true)
                    {
                        eGeomType type = feature.paths()[0].type();
                        if (type == Polygon)
                            converter.template set<clip_poly_tag>();
                        // line clipping disabled due to https://github.com/mapnik/mapnik/issues/1426
                        //else if (type == LineString)
                        //    converter.template set<clip_line_tag>();
                        // don't clip if type==Point
                    }
                    converter.template set<transform_tag>(); //always transform
                    if (sym.smooth() > 0.0) converter.template set<smooth_tag>(); // optional smooth converter
                    BOOST_FOREACH(geometry_type & geom, feature.paths())
                    {
                        converter.apply(geom);
                    }
                }
                else
                {
                    box2d<double> const& bbox = (*mark)->bounding_box();
                    setup_transform_scaling(tr, bbox, feature, sym);
                    evaluate_transform(tr, feature, sym.get_image_transform());
                    // TODO - clamping to >= 4 pixels
                    coord2d center = bbox.center();
                    agg::trans_affine_translation recenter(-center.x, -center.y);
                    agg::trans_affine marker_trans = recenter * tr;
                    vertex_stl_adapter<svg_path_storage> stl_storage((*stock_vector_marker)->source());
                    svg_path_adapter svg_path(stl_storage);
                    svg_attribute_type attributes;
                    bool result = push_explicit_style( (*stock_vector_marker)->attributes(), attributes, sym);
                    svg_renderer_type svg_renderer(svg_path, result ? attributes : (*stock_vector_marker)->attributes());
                    buf_type render_buf(pixmap_.raw_data(), width_, height_, width_);
                    dispatch_type rasterizer_dispatch(render_buf,
                                                      svg_renderer,
                                                      *ras_ptr,
                                                      bbox,
                                                      marker_trans,
                                                      sym,
                                                      *detector_,
                                                      scale_factor_,
                                                      feature,
                                                      pixmap_);
                    vertex_converter<box2d<double>, dispatch_type, markers_symbolizer,
                                     CoordTransform, proj_transform, agg::trans_affine, conv_types>
                        converter(query_extent_, rasterizer_dispatch, sym,t_,prj_trans,tr,scale_factor_);
                    if (sym.clip() && feature.paths().size() > 0) // optional clip (default: true)
                    {
                        eGeomType type = feature.paths()[0].type();
                        if (type == Polygon)
                            converter.template set<clip_poly_tag>();
                        // line clipping disabled due to https://github.com/mapnik/mapnik/issues/1426
                        //else if (type == LineString)
                        //    converter.template set<clip_line_tag>();
                        // don't clip if type==Point
                    }
                    converter.template set<transform_tag>(); //always transform
                    if (sym.smooth() > 0.0) converter.template set<smooth_tag>(); // optional smooth converter
                    BOOST_FOREACH(geometry_type & geom, feature.paths())
                    {
                        converter.apply(geom);
                    }
                }
            }
            else // raster markers
            {
                box2d<double> const& bbox = (*mark)->bounding_box();
                setup_transform_scaling(tr, bbox, feature, sym);
                evaluate_transform(tr, feature, sym.get_image_transform());
                // - clamp sizes to > 4 pixels of interactivity
                coord2d center = bbox.center();
                agg::trans_affine_translation recenter(-center.x, -center.y);
                agg::trans_affine marker_trans = recenter * tr;
                boost::optional<mapnik::image_ptr> marker = (*mark)->get_bitmap_data();
                typedef raster_markers_rasterizer_dispatch_grid<buf_type,
                                                            grid_rasterizer,
                                                            pixfmt_type,
                                                            renderer_base,
                                                            renderer_type,
                                                            detector_type,
                                                            mapnik::grid > dispatch_type;
                buf_type render_buf(pixmap_.raw_data(), width_, height_, width_);
                dispatch_type rasterizer_dispatch(render_buf,
                                                  *ras_ptr,
                                                  **marker,
                                                  marker_trans,
                                                  sym,
                                                  *detector_,
                                                  scale_factor_,
                                                  feature,
                                                  pixmap_);
                vertex_converter<box2d<double>, dispatch_type, markers_symbolizer,
                                 CoordTransform, proj_transform, agg::trans_affine, conv_types>
                    converter(query_extent_, rasterizer_dispatch, sym,t_,prj_trans,tr,scale_factor_);
                if (sym.clip() && feature.paths().size() > 0) // optional clip (default: true)
                {
                    eGeomType type = feature.paths()[0].type();
                    if (type == Polygon)
                        converter.template set<clip_poly_tag>();
                    // line clipping disabled due to https://github.com/mapnik/mapnik/issues/1426
                    //else if (type == LineString)
                    //    converter.template set<clip_line_tag>();
                    // don't clip if type==Point
                }
                converter.template set<transform_tag>(); //always transform
                if (sym.smooth() > 0.0) converter.template set<smooth_tag>(); // optional smooth converter
                BOOST_FOREACH(geometry_type & geom, feature.paths())
                {
                    converter.apply(geom);
                }
            }
        }
    }
}

template void grid_renderer<grid>::process(markers_symbolizer const&,
                                           mapnik::feature_impl &,
                                           proj_transform const&);
}
