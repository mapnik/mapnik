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
#include <mapnik/svg/svg_renderer.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/markers_placement.hpp>
#include <mapnik/arrow.hpp>
#include <mapnik/markers_symbolizer.hpp>

#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_scanline_p.h"
#include "agg_path_storage.h"
#include "agg_ellipse.h"
#include "agg_conv_stroke.h"
#include "agg_conv_clip_polyline.h"
#include "agg_conv_transform.h"

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
    agg::trans_affine tr;    
    evaluate_transform(tr, feature, sym.get_image_transform());
    tr = agg::trans_affine_scaling(scale_factor_) * tr;

    agg::trans_affine geom_tr;    
    evaluate_transform(geom_tr, feature, sym.get_transform());
    
    std::string filename = path_processor_type::evaluate(*sym.get_filename(), feature);
    marker_placement_e placement_method = sym.get_marker_placement();
    marker_type_e marker_type = sym.get_marker_type();
    metawriter_with_properties writer = sym.get_metawriter();

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
            coord2d const center = bbox.center();

            agg::trans_affine_translation const recenter(-center.x, -center.y);
            agg::trans_affine const marker_trans = recenter * tr;
            
            using namespace mapnik::svg;
            vertex_stl_adapter<svg_path_storage> stl_storage((*marker)->source());
            svg_path_adapter svg_path(stl_storage);

            svg_renderer<svg_path_adapter,
                agg::pod_bvector<path_attributes>,
                renderer_type,
                agg::pixfmt_rgba32 > svg_renderer(svg_path,(*marker)->attributes());

            for (unsigned i=0; i<feature.num_geometries(); ++i)
            {
                geometry_type & geom = feature.get_geometry(i);
                // TODO - merge this code with point_symbolizer rendering
                if (placement_method == MARKER_POINT_PLACEMENT || geom.num_points() <= 1)
                {
                    double x;
                    double y;
                    double z=0;
                    geom.label_interior_position(&x, &y);
                    prj_trans.backward(x,y,z);
                    t_.forward(&x,&y);
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
                        //metawriter_with_properties writer = sym.get_metawriter();
                        //if (writer.first) writer.first->add_box(extent, feature, t_, writer.second);
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
                            // note: debug_draw_box(buf, extent, x, y, angle)
                            //      would draw a rotated box showing the proper
                            //      bounds of the marker, while the above will
                            //      draw the box used for collision detection,
                            //      which embraces the rotated extent but isn't
                            //      rotated itself
                        }
                        if (writer.first)
                        {
                            //writer.first->add_box(label_ext, feature, t_, writer.second);
                            //MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: metawriter do not yet supported for line placement";
                        }
                    }
                }
            }
        }
    }
    else // FIXME: should default marker be stored in marker_cache ???
    {
        color const& fill_ = sym.get_fill();
        unsigned r = fill_.red();
        unsigned g = fill_.green();
        unsigned b = fill_.blue();
        unsigned a = fill_.alpha();
        stroke const& stroke_ = sym.get_stroke();
        color const& col = stroke_.get_color();
        double strk_width = stroke_.get_width();
        unsigned s_r=col.red();
        unsigned s_g=col.green();
        unsigned s_b=col.blue();
        unsigned s_a=col.alpha();
        double w = sym.get_width();
        double h = sym.get_height();
        double rx = w/2.0;
        double ry = h/2.0;

        arrow arrow_;
        box2d<double> extent;

        double dx = w + (2*strk_width);
        double dy = h + (2*strk_width);

        if (marker_type == MARKER_ARROW)
        {
            extent = arrow_.extent();
            double x1 = extent.minx();
            double y1 = extent.miny();
            double x2 = extent.maxx();
            double y2 = extent.maxy();
            tr.transform(&x1,&y1);
            tr.transform(&x2,&y2);
            extent.init(x1,y1,x2,y2);

            //MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: " << x1 << " " << y1 << " " << x2 << " " << y2 << "\n";
        }
        else
        {
            double x1 = -1 *(dx);
            double y1 = -1 *(dy);
            double x2 = dx;
            double y2 = dy;
            tr.transform(&x1,&y1);
            tr.transform(&x2,&y2);
            extent.init(x1,y1,x2,y2);

            //MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: " << x1 << " " << y1 << " " << x2 << " " << y2 << "\n";
        }


        double x;
        double y;
        double z=0;

        agg::path_storage marker;

        for (unsigned i=0; i<feature.num_geometries(); ++i)
        {
            geometry_type & geom = feature.get_geometry(i);
            //if (geom.num_points() <= 1) continue;
            if (placement_method == MARKER_POINT_PLACEMENT || geom.num_points() <= 1)
            {
                geom.label_position(&x,&y);
                prj_trans.backward(x,y,z);
                t_.forward(&x,&y);
                int px = int(floor(x - 0.5 * dx));
                int py = int(floor(y - 0.5 * dy));
                box2d<double> label_ext (px, py, px + dx +1, py + dy +1);

                if (sym.get_allow_overlap() ||
                    detector_->has_placement(label_ext))
                {
                    agg::ellipse c(x, y, rx, ry);
                    marker.concat_path(c);
                    ras_ptr->add_path(marker);
                    ren.color(agg::rgba8_pre(r, g, b, int(a*sym.get_opacity())));
                    // TODO - fill with packed scanlines? agg::scanline_p8
                    // and agg::renderer_outline_aa
                    agg::render_scanlines(*ras_ptr, sl, ren);

                    // outline
                    if (strk_width)
                    {
                        ras_ptr->reset();
                        agg::conv_stroke<agg::path_storage>  outline(marker);
                        outline.generator().width(strk_width * scale_factor_);
                        ras_ptr->add_path(outline);

                        ren.color(agg::rgba8_pre(s_r, s_g, s_b, int(s_a*stroke_.get_opacity())));
                        agg::render_scanlines(*ras_ptr, sl, ren);
                    }
                    if (!sym.get_ignore_placement())
                        detector_->insert(label_ext);
                    if (writer.first) writer.first->add_box(label_ext, feature, t_, writer.second);
                }
            }
            else
            {

                if (marker_type == MARKER_ARROW)
                    marker.concat_path(arrow_);

                clipped_geometry_type clipped(geom);
                clipped.clip_box(query_extent_.minx(),query_extent_.miny(),query_extent_.maxx(),query_extent_.maxy());
                path_type path(t_,clipped,prj_trans);
                transformed_path_type path_transformed(path,geom_tr);
                markers_placement<transformed_path_type, label_collision_detector4> placement(path_transformed, extent, agg::trans_affine(), *detector_,
                                                                                  sym.get_spacing() * scale_factor_,
                                                                                  sym.get_max_error(),
                                                                                  sym.get_allow_overlap());
                double x_t, y_t, angle;
                while (placement.get_point(x_t, y_t, angle))
                {
                    agg::trans_affine matrix;

                    if (marker_type == MARKER_ELLIPSE)
                    {
                        // todo proper bbox - this is buggy
                        agg::ellipse c(x_t, y_t, rx, ry);
                        marker.concat_path(c);
                        agg::trans_affine matrix;
                        matrix *= agg::trans_affine_translation(-x_t,-y_t);
                        matrix *= agg::trans_affine_rotation(angle);
                        matrix *= agg::trans_affine_translation(x_t,y_t);
                        marker.transform(matrix);

                    }
                    else
                    {
                        matrix = tr * agg::trans_affine_rotation(angle) * agg::trans_affine_translation(x_t, y_t);
                    }


                    // TODO
                    if (writer.first)
                    {
                        //writer.first->add_box(label_ext, feature, t_, writer.second);

                        MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: metawriter do not yet supported for line placement";
                    }

                    agg::conv_transform<agg::path_storage, agg::trans_affine> trans(marker, matrix);
                    ras_ptr->add_path(trans);

                    // fill
                    ren.color(agg::rgba8_pre(r, g, b, int(a*sym.get_opacity())));
                    agg::render_scanlines(*ras_ptr, sl, ren);

                    // outline
                    if (strk_width)
                    {
                        ras_ptr->reset();
                        agg::conv_stroke<agg::conv_transform<agg::path_storage, agg::trans_affine> >  outline(trans);
                        outline.generator().width(strk_width * scale_factor_);
                        ras_ptr->add_path(outline);
                        ren.color(agg::rgba8_pre(s_r, s_g, s_b, int(s_a*stroke_.get_opacity())));
                        agg::render_scanlines(*ras_ptr, sl, ren);
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
