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
#include <mapnik/graphics.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_rasterizer.hpp>

#include <mapnik/debug.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/marker_helpers.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/svg/svg_renderer_agg.hpp>
#include <mapnik/svg/svg_storage.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/markers_symbolizer.hpp>
#include <mapnik/parse_path.hpp>

// agg
#include "agg_basics.h"
#include "agg_renderer_base.h"
#include "agg_renderer_scanline.h"
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
                              feature_impl & feature,
                              proj_transform const& prj_trans)
{
    typedef agg::rgba8 color_type;
    typedef agg::order_rgba order_type;
    typedef agg::pixel32_type pixel_type;
    typedef agg::comp_op_adaptor_rgba_pre<color_type, order_type> blender_type; // comp blender
    typedef agg::rendering_buffer buf_type;
    typedef agg::pixfmt_custom_blend_rgba<blender_type, buf_type> pixfmt_comp_type;
    typedef agg::renderer_base<pixfmt_comp_type> renderer_base;
    typedef label_collision_detector4 detector_type;
    typedef boost::mpl::vector<clip_line_tag,clip_poly_tag,transform_tag,smooth_tag> conv_types;

    std::string filename = path_processor_type::evaluate(*sym.get_filename(), feature);

    // https://github.com/mapnik/mapnik/issues/1316
    bool snap_pixels = !mapnik::marker_cache::instance().is_uri(filename);

    if (!filename.empty())
    {
        boost::optional<marker_ptr> mark = mapnik::marker_cache::instance().find(filename, true);
        if (mark && *mark)
        {
            ras_ptr->reset();
            if (gamma_method_ != GAMMA_POWER || gamma_ != 1.0)
            {
                ras_ptr->gamma(agg::gamma_power());
                gamma_method_ = GAMMA_POWER;
                gamma_ = 1.0;
            }
            agg::trans_affine geom_tr;
            evaluate_transform(geom_tr, feature, sym.get_transform());
            agg::trans_affine tr = agg::trans_affine_scaling(scale_factor_);

            if ((*mark)->is_vector())
            {
                using namespace mapnik::svg;
                typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_type;
                typedef agg::pod_bvector<path_attributes> svg_attribute_type;
                typedef svg_renderer_agg<svg_path_adapter,
                                     svg_attribute_type,
                                     renderer_type,
                                     pixfmt_comp_type > svg_renderer_type;
                typedef vector_markers_rasterizer_dispatch<buf_type,
                                     svg_renderer_type,
                                     rasterizer,
                                     detector_type > dispatch_type;
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
                    build_ellipse(sym, feature, marker_ellipse, svg_path);
                    svg_attribute_type attributes;
                    bool result = push_explicit_style( (*stock_vector_marker)->attributes(), attributes, sym);
                    svg_renderer_type svg_renderer(svg_path, result ? attributes : (*stock_vector_marker)->attributes());
                    evaluate_transform(tr, feature, sym.get_image_transform());
                    box2d<double> bbox = marker_ellipse.bounding_box();
                    coord2d center = bbox.center();
                    agg::trans_affine_translation recenter(-center.x, -center.y);
                    agg::trans_affine marker_trans = recenter * tr;
                    buf_type render_buffer(current_buffer_->raw_data(), width_, height_, width_ * 4);
                    dispatch_type rasterizer_dispatch(render_buffer,
                                                      svg_renderer,
                                                      *ras_ptr,
                                                      bbox,
                                                      marker_trans,
                                                      sym,
                                                      *detector_,
                                                      scale_factor_,
                                                      snap_pixels);
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
                    apply_markers_multi(feature, converter, sym);
                }
                else
                {
                    box2d<double> const& bbox = (*mark)->bounding_box();
                    setup_transform_scaling(tr, bbox.width(), bbox.height(), feature, sym);
                    evaluate_transform(tr, feature, sym.get_image_transform());
                    coord2d center = bbox.center();
                    agg::trans_affine_translation recenter(-center.x, -center.y);
                    agg::trans_affine marker_trans = recenter * tr;
                    vertex_stl_adapter<svg_path_storage> stl_storage((*stock_vector_marker)->source());
                    svg_path_adapter svg_path(stl_storage);
                    svg_attribute_type attributes;
                    bool result = push_explicit_style( (*stock_vector_marker)->attributes(), attributes, sym);
                    svg_renderer_type svg_renderer(svg_path, result ? attributes : (*stock_vector_marker)->attributes());
                    buf_type render_buffer(current_buffer_->raw_data(), width_, height_, width_ * 4);
                    dispatch_type rasterizer_dispatch(render_buffer,
                                                      svg_renderer,
                                                      *ras_ptr,
                                                      bbox,
                                                      marker_trans,
                                                      sym,
                                                      *detector_,
                                                      scale_factor_,
                                                      snap_pixels);
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
                    apply_markers_multi(feature, converter, sym);
                }
            }
            else // raster markers
            {
                setup_transform_scaling(tr, (*mark)->width(), (*mark)->height(), feature, sym);
                evaluate_transform(tr, feature, sym.get_image_transform());
                box2d<double> const& bbox = (*mark)->bounding_box();
                coord2d center = bbox.center();
                agg::trans_affine_translation recenter(-center.x, -center.y);
                agg::trans_affine marker_trans = recenter * tr;
                boost::optional<mapnik::image_ptr> marker = (*mark)->get_bitmap_data();
                typedef raster_markers_rasterizer_dispatch<buf_type,rasterizer, detector_type> dispatch_type;
                buf_type render_buffer(current_buffer_->raw_data(), width_, height_, width_ * 4);
                dispatch_type rasterizer_dispatch(render_buffer,
                                                  *ras_ptr,
                                                  **marker,
                                                  marker_trans,
                                                  sym,
                                                  *detector_,
                                                  scale_factor_,
                                                  true /*snap rasters no matter what*/);
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
                apply_markers_multi(feature, converter, sym);
            }
        }
    }
}

template void agg_renderer<image_32>::process(markers_symbolizer const&,
                                              mapnik::feature_impl &,
                                              proj_transform const&);
}
