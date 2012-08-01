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
#include <mapnik/geom_util.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_rasterizer.hpp>
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

namespace mapnik {

template <typename BufferType, typename SvgRenderer, typename Rasterizer, typename Detector>
struct vector_markers_rasterizer_dispatch
{
    typedef agg::rgba8 color_type;
    typedef agg::order_rgba order_type;
    typedef agg::pixel32_type pixel_type;
    typedef agg::comp_op_adaptor_rgba_pre<color_type, order_type> blender_type; // comp blender
    typedef agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer> pixfmt_comp_type;
    typedef agg::renderer_base<pixfmt_comp_type> renderer_base;

    vector_markers_rasterizer_dispatch(BufferType & image_buffer,
                                SvgRenderer & svg_renderer,
                                Rasterizer & ras,
                                box2d<double> const& bbox,
                                agg::trans_affine const& marker_trans,
                                markers_symbolizer const& sym,
                                Detector & detector,
                                double scale_factor)
        : buf_(image_buffer.raw_data(), image_buffer.width(), image_buffer.height(), image_buffer.width() * 4),
        pixf_(buf_),
        renb_(pixf_),
        svg_renderer_(svg_renderer),
        ras_(ras),
        bbox_(bbox),
        marker_trans_(marker_trans),
        sym_(sym),
        detector_(detector),
        scale_factor_(scale_factor)
    {
        pixf_.comp_op(static_cast<agg::comp_op_e>(sym_.comp_op()));
    }

    template <typename T>
    void add_path(T & path)
    {
        marker_placement_e placement_method = sym_.get_marker_placement();

        if (placement_method != MARKER_LINE_PLACEMENT)
        {
            double x,y;
            path.rewind(0);
            if (placement_method == MARKER_INTERIOR_PLACEMENT)
            {
                label::interior_position(path, x, y);
            }
            else
            {
                label::centroid(path, x, y);
            }
            agg::trans_affine matrix = marker_trans_;
            matrix.translate(x,y);
            box2d<double> transformed_bbox = bbox_ * matrix;

            if (sym_.get_allow_overlap() ||
                detector_.has_placement(transformed_bbox))
            {
                svg_renderer_.render(ras_, sl_, renb_, matrix, 1, bbox_);

                if (!sym_.get_ignore_placement())
                    detector_.insert(transformed_bbox);
            }
        }
        else
        {
            markers_placement<T, label_collision_detector4> placement(path, bbox_, marker_trans_, detector_,
                                                                      sym_.get_spacing() * scale_factor_,
                                                                      sym_.get_max_error(),
                                                                      sym_.get_allow_overlap());
            double x, y, angle;
            while (placement.get_point(x, y, angle))
            {
                agg::trans_affine matrix = marker_trans_;
                matrix.rotate(angle);
                matrix.translate(x, y);
                svg_renderer_.render(ras_, sl_, renb_, matrix, 1, bbox_);
            }
        }
    }
private:
    agg::scanline_u8 sl_;
    agg::rendering_buffer buf_;
    pixfmt_comp_type pixf_;
    renderer_base renb_;
    SvgRenderer & svg_renderer_;
    Rasterizer & ras_;
    box2d<double> const& bbox_;
    agg::trans_affine const& marker_trans_;
    markers_symbolizer const& sym_;
    Detector & detector_;
    double scale_factor_;
};

template <typename Rasterizer, typename RendererBuffer>
void render_raster_marker(Rasterizer & ras, RendererBuffer & renb,
                          agg::scanline_u8 & sl, image_data_32 const& src,
                          agg::trans_affine const& marker_tr, double opacity)
{
    double width  = src.width();
    double height = src.height();
    double p[8];
    p[0] = 0;     p[1] = 0;
    p[2] = width; p[3] = 0;
    p[4] = width; p[5] = height;
    p[6] = 0;     p[7] = height;

    marker_tr.transform(&p[0], &p[1]);
    marker_tr.transform(&p[2], &p[3]);
    marker_tr.transform(&p[4], &p[5]);
    marker_tr.transform(&p[6], &p[7]);

    ras.move_to_d(p[0],p[1]);
    ras.line_to_d(p[2],p[3]);
    ras.line_to_d(p[4],p[5]);
    ras.line_to_d(p[6],p[7]);

    typedef agg::rgba8 color_type;
    agg::span_allocator<color_type> sa;
    agg::image_filter_bilinear filter_kernel;
    agg::image_filter_lut filter(filter_kernel, false);

    agg::rendering_buffer marker_buf((unsigned char *)src.getBytes(),
                                     src.width(),
                                     src.height(),
                                     src.width()*4);
    agg::pixfmt_rgba32_pre pixf(marker_buf);

    typedef agg::image_accessor_clone<agg::pixfmt_rgba32_pre> img_accessor_type;
    typedef agg::span_interpolator_linear<agg::trans_affine> interpolator_type;
    typedef agg::span_image_filter_rgba_2x2<img_accessor_type,
                                            interpolator_type> span_gen_type;
    typedef agg::order_rgba order_type;
    typedef agg::pixel32_type pixel_type;
    typedef agg::comp_op_adaptor_rgba_pre<color_type, order_type> blender_type; // comp blender
    typedef agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer> pixfmt_comp_type;
    typedef agg::renderer_base<pixfmt_comp_type> renderer_base;
    typedef agg::renderer_scanline_aa_alpha<renderer_base,
            agg::span_allocator<agg::rgba8>,
            span_gen_type> renderer_type;
    img_accessor_type ia(pixf);
    interpolator_type interpolator(agg::trans_affine(p, 0, 0, width, height) );
    span_gen_type sg(ia, interpolator, filter);
    renderer_type rp(renb,sa, sg, unsigned(opacity*255));
    agg::render_scanlines(ras, sl, rp);
}

template <typename BufferType, typename Rasterizer, typename Detector>
struct raster_markers_rasterizer_dispatch
{
    typedef agg::rgba8 color_type;
    typedef agg::order_rgba order_type;
    typedef agg::pixel32_type pixel_type;
    typedef agg::comp_op_adaptor_rgba_pre<color_type, order_type> blender_type; // comp blender
    typedef agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer> pixfmt_comp_type;
    typedef agg::renderer_base<pixfmt_comp_type> renderer_base;

    raster_markers_rasterizer_dispatch(BufferType & image_buffer,
                                       Rasterizer & ras,
                                       image_data_32 const& src,
                                       agg::trans_affine const& marker_trans,
                                       markers_symbolizer const& sym,
                                       Detector & detector,
                                       double scale_factor)
        : buf_(image_buffer.raw_data(), image_buffer.width(), image_buffer.height(), image_buffer.width() * 4),
        pixf_(buf_),
        renb_(pixf_),
        ras_(ras),
        src_(src),
        marker_trans_(marker_trans),
        sym_(sym),
        detector_(detector),
        scale_factor_(scale_factor)
    {
        pixf_.comp_op(static_cast<agg::comp_op_e>(sym_.comp_op()));
    }

    template <typename T>
    void add_path(T & path)
    {
        marker_placement_e placement_method = sym_.get_marker_placement();
        box2d<double> bbox_(0,0, src_.width(),src_.height());

        if (placement_method != MARKER_LINE_PLACEMENT)
        {
            double x,y;
            path.rewind(0);
            if (placement_method == MARKER_INTERIOR_PLACEMENT)
            {
                label::interior_position(path, x, y);
            }
            else
            {
                label::centroid(path, x, y);
            }
            agg::trans_affine matrix = marker_trans_;
            matrix.translate(x,y);
            box2d<double> transformed_bbox = bbox_ * matrix;

            if (sym_.get_allow_overlap() ||
                detector_.has_placement(transformed_bbox))
            {

                float opacity = sym_.get_opacity() ? *sym_.get_opacity() : 1;
                render_raster_marker(ras_, renb_, sl_, src_,
                                     matrix, opacity);
                if (!sym_.get_ignore_placement())
                    detector_.insert(transformed_bbox);
            }
        }
        else
        {
            markers_placement<T, label_collision_detector4> placement(path, bbox_, marker_trans_, detector_,
                                                                      sym_.get_spacing() * scale_factor_,
                                                                      sym_.get_max_error(),
                                                                      sym_.get_allow_overlap());
            double x, y, angle;
            while (placement.get_point(x, y, angle))
            {
                agg::trans_affine matrix = marker_trans_;
                matrix.rotate(angle);
                matrix.translate(x,y);
                float opacity = sym_.get_opacity() ? *sym_.get_opacity() : 1;
                render_raster_marker(ras_, renb_, sl_, src_,
                                     matrix, opacity);
            }
        }
    }
private:
    agg::scanline_u8 sl_;
    agg::rendering_buffer buf_;
    pixfmt_comp_type pixf_;
    renderer_base renb_;
    Rasterizer & ras_;
    image_data_32 const& src_;
    agg::trans_affine const& marker_trans_;
    markers_symbolizer const& sym_;
    Detector & detector_;
    double scale_factor_;
};


template <typename T>
void agg_renderer<T>::process(markers_symbolizer const& sym,
                              feature_impl & feature,
                              proj_transform const& prj_trans)
{
    typedef agg::rgba8 color_type;
    typedef agg::order_rgba order_type;
    typedef agg::pixel32_type pixel_type;
    typedef agg::comp_op_adaptor_rgba_pre<color_type, order_type> blender_type; // comp blender
    typedef agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer> pixfmt_comp_type;
    typedef agg::renderer_base<pixfmt_comp_type> renderer_base;
    typedef label_collision_detector4 detector_type;
    typedef boost::mpl::vector<clip_line_tag,clip_poly_tag,transform_tag,smooth_tag> conv_types;

    std::string filename = path_processor_type::evaluate(*sym.get_filename(), feature);

    if (!filename.empty())
    {
        boost::optional<marker_ptr> mark = mapnik::marker_cache::instance()->find(filename, true);
        if (mark && *mark)
        {
            ras_ptr->reset();
            ras_ptr->gamma(agg::gamma_power());
            agg::trans_affine geom_tr;
            evaluate_transform(geom_tr, feature, sym.get_transform());
            agg::trans_affine tr;
            tr *= agg::trans_affine_scaling(scale_factor_);

            if ((*mark)->is_vector())
            {
                using namespace mapnik::svg;
                typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_type;
                typedef agg::pod_bvector<path_attributes> svg_attribute_type;
                typedef svg_renderer<svg_path_adapter,
                                     svg_attribute_type,
                                     renderer_type,
                                     agg::pixfmt_rgba32 > svg_renderer_type;
                typedef vector_markers_rasterizer_dispatch<buffer_type,
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
                    dispatch_type rasterizer_dispatch(*current_buffer_,svg_renderer,*ras_ptr,
                                                      bbox, marker_trans, sym, *detector_, scale_factor_);
                    vertex_converter<box2d<double>, dispatch_type, markers_symbolizer,
                                     CoordTransform, proj_transform, agg::trans_affine, conv_types>
                        converter(query_extent_, rasterizer_dispatch, sym,t_,prj_trans,tr,scale_factor_);
                    if (sym.clip() && feature.paths().size() > 0) // optional clip (default: true)
                    {
                        eGeomType type = feature.paths()[0].type();
                        if (type == Polygon)
                            converter.template set<clip_poly_tag>();
                        else if (type == LineString)
                            converter.template set<clip_line_tag>();
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
                    setup_label_transform(tr, bbox, feature, sym);
                    coord2d center = bbox.center();
                    agg::trans_affine_translation recenter(-center.x, -center.y);
                    agg::trans_affine marker_trans = recenter * tr;
                    vertex_stl_adapter<svg_path_storage> stl_storage((*stock_vector_marker)->source());
                    svg_path_adapter svg_path(stl_storage);
                    svg_attribute_type attributes;
                    bool result = push_explicit_style( (*stock_vector_marker)->attributes(), attributes, sym);
                    svg_renderer_type svg_renderer(svg_path, result ? attributes : (*stock_vector_marker)->attributes());
                    dispatch_type rasterizer_dispatch(*current_buffer_,svg_renderer,*ras_ptr,
                                                      bbox, marker_trans, sym, *detector_, scale_factor_);
                    vertex_converter<box2d<double>, dispatch_type, markers_symbolizer,
                                     CoordTransform, proj_transform, agg::trans_affine, conv_types>
                        converter(query_extent_, rasterizer_dispatch, sym,t_,prj_trans,tr,scale_factor_);
                    if (sym.clip() && feature.paths().size() > 0) // optional clip (default: true)
                    {
                        eGeomType type = feature.paths()[0].type();
                        if (type == Polygon)
                            converter.template set<clip_poly_tag>();
                        else if (type == LineString)
                            converter.template set<clip_line_tag>();
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
                setup_label_transform(tr, bbox, feature, sym);
                coord2d center = bbox.center();
                agg::trans_affine_translation recenter(-center.x, -center.y);
                agg::trans_affine marker_trans = recenter * tr;
                boost::optional<mapnik::image_ptr> marker = (*mark)->get_bitmap_data();
                typedef raster_markers_rasterizer_dispatch<buffer_type,rasterizer, detector_type> dispatch_type;
                dispatch_type rasterizer_dispatch(*current_buffer_,*ras_ptr, **marker,
                                                  marker_trans, sym, *detector_, scale_factor_);
                vertex_converter<box2d<double>, dispatch_type, markers_symbolizer,
                                 CoordTransform, proj_transform, agg::trans_affine, conv_types>
                    converter(query_extent_, rasterizer_dispatch, sym,t_,prj_trans,tr,scale_factor_);

                if (sym.clip() && feature.paths().size() > 0) // optional clip (default: true)
                {
                    eGeomType type = feature.paths()[0].type();
                    if (type == Polygon)
                        converter.template set<clip_poly_tag>();
                    else if (type == LineString)
                        converter.template set<clip_line_tag>();
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

template void agg_renderer<image_32>::process(markers_symbolizer const&,
                                              mapnik::feature_impl &,
                                              proj_transform const&);
}
