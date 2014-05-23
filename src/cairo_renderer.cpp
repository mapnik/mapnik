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

#if defined(HAVE_CAIRO)

// mapnik
#include <mapnik/std.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/raster.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/cairo_renderer.hpp>
#include <mapnik/cairo_context.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/markers_placement.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/font_set.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/map.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/segment.hpp>
#include <mapnik/text/symbolizer_helpers.hpp>
#include <mapnik/raster_colorizer.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/warp.hpp>
#include <mapnik/config.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/marker_helpers.hpp>
#include <mapnik/noncopyable.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/attribute_collector.hpp>
#include <mapnik/group/group_layout_manager.hpp>
#include <mapnik/group/group_symbolizer_helper.hpp>
#include <mapnik/attribute.hpp>

// mapnik symbolizer generics
#include <mapnik/renderer_common/process_building_symbolizer.hpp>
#include <mapnik/renderer_common/process_point_symbolizer.hpp>
#include <mapnik/renderer_common/process_raster_symbolizer.hpp>
#include <mapnik/renderer_common/process_markers_symbolizer.hpp>
#include <mapnik/renderer_common/process_polygon_symbolizer.hpp>
#include <mapnik/renderer_common/process_group_symbolizer.hpp>
// cairo
#include <cairo.h>
#include <cairo-ft.h>
#include <cairo-version.h>

// boost
#include <boost/math/special_functions/round.hpp>

// agg
#include "agg_conv_clip_polyline.h"
#include "agg_conv_clip_polygon.h"
#include "agg_conv_smooth_poly1.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"

// markers
#include "agg_path_storage.h"
#include "agg_ellipse.h"

// stl
#include <deque>
#include <cmath>

namespace mapnik
{

struct cairo_save_restore
{
    cairo_save_restore(cairo_context & context)
        : context_(context)
    {
        context_.save();
    }
    ~cairo_save_restore()
    {
        context_.restore();
    }
    cairo_context & context_;
};

cairo_renderer_base::cairo_renderer_base(Map const& m,
                                         cairo_ptr const& cairo,
                                         attributes const& vars,
                                         double scale_factor,
                                         unsigned offset_x,
                                         unsigned offset_y)
    : m_(m),
      context_(cairo),
      common_(m, vars, offset_x, offset_y, m.width(), m.height(), scale_factor),
      face_manager_(common_.shared_font_engine_)
{
    setup(m);
}

cairo_renderer_base::cairo_renderer_base(Map const& m,
                                         request const& req,
                                         cairo_ptr const& cairo,
                                         attributes const& vars,
                                         double scale_factor,
                                         unsigned offset_x,
                                         unsigned offset_y)
    : m_(m),
      context_(cairo),
      common_(req, vars, offset_x, offset_y, req.width(), req.height(), scale_factor),
      face_manager_(common_.shared_font_engine_)
{
    setup(m);
}

cairo_renderer_base::cairo_renderer_base(Map const& m,
                                         cairo_ptr const& cairo,
                                         attributes const& vars,
                                         std::shared_ptr<label_collision_detector4> detector,
                                         double scale_factor,
                                         unsigned offset_x,
                                         unsigned offset_y)
    : m_(m),
      context_(cairo),
      common_(m, vars, offset_x, offset_y, m.width(), m.height(), scale_factor, detector),
      face_manager_(common_.shared_font_engine_)
{
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer_base: Scale=" << m.scale();
}

template <>
cairo_renderer<cairo_ptr>::cairo_renderer(Map const& m, cairo_ptr const& cairo, double scale_factor, unsigned offset_x, unsigned offset_y)
    : feature_style_processor<cairo_renderer>(m,scale_factor),
      cairo_renderer_base(m,cairo,attributes(),scale_factor,offset_x,offset_y) {}

template <>
cairo_renderer<cairo_surface_ptr>::cairo_renderer(Map const& m, cairo_surface_ptr const& surface, double scale_factor, unsigned offset_x, unsigned offset_y)
    : feature_style_processor<cairo_renderer>(m,scale_factor),
      cairo_renderer_base(m,create_context(surface),attributes(),scale_factor,offset_x,offset_y) {}

template <>
cairo_renderer<cairo_ptr>::cairo_renderer(Map const& m, request const& req, attributes const& vars, cairo_ptr const& cairo, double scale_factor, unsigned offset_x, unsigned offset_y)
    : feature_style_processor<cairo_renderer>(m,scale_factor),
      cairo_renderer_base(m,req,cairo,vars,scale_factor,offset_x,offset_y) {}

template <>
cairo_renderer<cairo_surface_ptr>::cairo_renderer(Map const& m, request const& req, attributes const& vars, cairo_surface_ptr const& surface, double scale_factor, unsigned offset_x, unsigned offset_y)
    : feature_style_processor<cairo_renderer>(m,scale_factor),
      cairo_renderer_base(m,req,create_context(surface),attributes(),scale_factor,offset_x,offset_y) {}

template <>
cairo_renderer<cairo_ptr>::cairo_renderer(Map const& m, cairo_ptr const& cairo, std::shared_ptr<label_collision_detector4> detector, double scale_factor, unsigned offset_x, unsigned offset_y)
    : feature_style_processor<cairo_renderer>(m,scale_factor),
      cairo_renderer_base(m,cairo,attributes(),detector,scale_factor,offset_x,offset_y) {}

template <>
cairo_renderer<cairo_surface_ptr>::cairo_renderer(Map const& m, cairo_surface_ptr const& surface, std::shared_ptr<label_collision_detector4> detector, double scale_factor, unsigned offset_x, unsigned offset_y)
    : feature_style_processor<cairo_renderer>(m,scale_factor),
      cairo_renderer_base(m,create_context(surface),attributes(),detector,scale_factor,offset_x,offset_y) {}

cairo_renderer_base::~cairo_renderer_base() {}

void cairo_renderer_base::setup(Map const& map)
{
    boost::optional<color> bg = m_.background();
    if (bg)
    {
        cairo_save_restore guard(context_);
        context_.set_color(*bg);
        context_.paint();
    }
    boost::optional<std::string> const& image_filename = map.background_image();
    if (image_filename)
    {
        // NOTE: marker_cache returns premultiplied image, if needed
        boost::optional<mapnik::marker_ptr> bg_marker = mapnik::marker_cache::instance().find(*image_filename,true);
        if (bg_marker && (*bg_marker)->is_bitmap())
        {
            mapnik::image_ptr bg_image = *(*bg_marker)->get_bitmap_data();
            int w = bg_image->width();
            int h = bg_image->height();
            if ( w > 0 && h > 0)
            {
                // repeat background-image both vertically and horizontally
                unsigned x_steps = unsigned(std::ceil(common_.width_/double(w)));
                unsigned y_steps = unsigned(std::ceil(common_.height_/double(h)));
                for (unsigned x=0;x<x_steps;++x)
                {
                    for (unsigned y=0;y<y_steps;++y)
                    {
                        agg::trans_affine matrix = agg::trans_affine_translation(
                                                       x*w,
                                                       y*h);
                        context_.add_image(matrix, *bg_image, 1.0f);
                    }
                }
            }
        }
    }
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer_base: Scale=" << map.scale();
}

void cairo_renderer_base::start_map_processing(Map const& map)
{
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer_base: Start map processing bbox=" << map.get_current_extent();
    box2d<double> bounds = common_.t_.forward(common_.t_.extent());
    context_.rectangle(bounds.minx(), bounds.miny(), bounds.maxx(), bounds.maxy());
    context_.clip();
}

template <>
void cairo_renderer<cairo_ptr>::end_map_processing(Map const& )
{
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer_base: End map processing";
}

template <>
void cairo_renderer<cairo_surface_ptr>::end_map_processing(Map const& )
{
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer_base: End map processing";

    context_.show_page();
}

void cairo_renderer_base::start_layer_processing(layer const& lay, box2d<double> const& query_extent)
{
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer_base: Start processing layer=" << lay.name() ;
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer_base: -- datasource=" << lay.datasource().get();
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer_base: -- query_extent=" << query_extent;

    if (lay.clear_label_cache())
    {
        common_.detector_->clear();
    }
    common_.query_extent_ = query_extent;
}

void cairo_renderer_base::end_layer_processing(layer const&)
{
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer_base: End layer processing";
}

void cairo_renderer_base::start_style_processing(feature_type_style const& st)
{
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer:start style processing";
}

void cairo_renderer_base::end_style_processing(feature_type_style const& st)
{
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer:end style processing";
}

void cairo_renderer_base::process(polygon_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    typedef boost::mpl::vector<clip_poly_tag,transform_tag,affine_transform_tag,simplify_tag,smooth_tag> conv_types;
    typedef vertex_converter<box2d<double>, cairo_context, polygon_symbolizer,
                             CoordTransform, proj_transform, agg::trans_affine,
                             conv_types, feature_impl> vertex_converter_type;

    cairo_save_restore guard(context_);
    composite_mode_e comp_op = get<composite_mode_e>(sym, keys::comp_op, feature, common_.vars_, src_over);
    context_.set_operator(comp_op);

    render_polygon_symbolizer<vertex_converter_type>(
        sym, feature, prj_trans, common_, common_.query_extent_, context_,
        [&](color const &fill, double opacity) {
            context_.set_color(fill, opacity);
            // fill polygon
            context_.set_fill_rule(CAIRO_FILL_RULE_EVEN_ODD);
            context_.fill();
        });
}

void cairo_renderer_base::process(building_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    typedef coord_transform<CoordTransform,geometry_type> path_type;
    cairo_save_restore guard(context_);
    composite_mode_e comp_op = get<composite_mode_e>(sym, keys::comp_op, feature, common_.vars_, src_over);
    mapnik::color fill = get<mapnik::color>(sym, keys::fill, feature, common_.vars_, mapnik::color(128,128,128));
    double opacity = get<double>(sym, keys::fill_opacity, feature, common_.vars_, 1.0);
    double height = get<double>(sym, keys::height, feature, common_.vars_, 0.0);

    context_.set_operator(comp_op);

    render_building_symbolizer(
        feature, height,
        [&](geometry_type &faces) {
            path_type faces_path(common_.t_, faces, prj_trans);
            context_.set_color(fill.red()  * 0.8 / 255.0, fill.green() * 0.8 / 255.0,
                               fill.blue() * 0.8 / 255.0, fill.alpha() * opacity / 255.0);
            context_.add_path(faces_path);
            context_.fill();
        },
        [&](geometry_type &frame) {
            path_type path(common_.t_, frame, prj_trans);
            context_.set_color(fill.red()  * 0.8 / 255.0, fill.green() * 0.8/255.0,
                              fill.blue() * 0.8 / 255.0, fill.alpha() * opacity / 255.0);
            context_.set_line_width(common_.scale_factor_);
            context_.add_path(path);
            context_.stroke();
        },
        [&](geometry_type &roof) {
            path_type roof_path(common_.t_, roof, prj_trans);
            context_.set_color(fill, opacity);
            context_.add_path(roof_path);
            context_.fill();
        });
}

void cairo_renderer_base::process(line_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    typedef boost::mpl::vector<clip_line_tag, transform_tag,
                               affine_transform_tag,
                               simplify_tag, smooth_tag,
                               offset_transform_tag,
                               dash_tag, stroke_tag> conv_types;
    cairo_save_restore guard(context_);
    composite_mode_e comp_op = get<composite_mode_e>(sym, keys::comp_op, feature, common_.vars_, src_over);
    bool clip = get<bool>(sym, keys::clip, feature, common_.vars_, true);
    double offset = get<double>(sym, keys::offset, feature, common_.vars_, 0.0);
    double simplify_tolerance = get<double>(sym, keys::simplify_tolerance, feature, common_.vars_, 0.0);
    double smooth = get<double>(sym, keys::smooth, feature, common_.vars_, 0.0);

    mapnik::color stroke = get<mapnik::color>(sym, keys::stroke, feature, common_.vars_, mapnik::color(0,0,0));
    double stroke_opacity = get<double>(sym, keys::stroke_opacity, feature, common_.vars_, 1.0);
    line_join_enum stroke_join = get<line_join_enum>(sym, keys::stroke_linejoin, feature, common_.vars_, MITER_JOIN);
    line_cap_enum stroke_cap = get<line_cap_enum>(sym, keys::stroke_linecap, feature, common_.vars_, BUTT_CAP);
    auto dash = get_optional<dash_array>(sym, keys::stroke_dasharray);
    double miterlimit = get<double>(sym, keys::stroke_miterlimit, feature, common_.vars_, 4.0);
    double width = get<double>(sym, keys::stroke_width, feature, common_.vars_, 1.0);

    context_.set_operator(comp_op);
    context_.set_color(stroke, stroke_opacity);
    context_.set_line_join(stroke_join);
    context_.set_line_cap(stroke_cap);
    context_.set_miter_limit(miterlimit);
    context_.set_line_width(width * common_.scale_factor_);
    if (dash)
    {
        context_.set_dash(*dash, common_.scale_factor_);
    }

    agg::trans_affine tr;
    auto geom_transform = get_optional<transform_type>(sym, keys::geometry_transform);
    if (geom_transform) { evaluate_transform(tr, feature, common_.vars_, *geom_transform, common_.scale_factor_); }

    box2d<double> clipping_extent = common_.query_extent_;
    if (clip)
    {
        double padding = (double)(common_.query_extent_.width()/common_.width_);
        double half_stroke = width/2.0;
        if (half_stroke > 1)
            padding *= half_stroke;
        if (std::fabs(offset) > 0)
            padding *= std::fabs(offset) * 1.2;
        padding *= common_.scale_factor_;
        clipping_extent.pad(padding);
    }
    vertex_converter<box2d<double>, cairo_context, line_symbolizer,
                     CoordTransform, proj_transform, agg::trans_affine, conv_types, feature_impl>
        converter(clipping_extent,context_,sym,common_.t_,prj_trans,tr,feature,common_.vars_,common_.scale_factor_);

    if (clip) converter.set<clip_line_tag>(); // optional clip (default: true)
    converter.set<transform_tag>(); // always transform
    if (std::fabs(offset) > 0.0) converter.set<offset_transform_tag>(); // parallel offset
    converter.set<affine_transform_tag>(); // optional affine transform
    if (simplify_tolerance > 0.0) converter.set<simplify_tag>(); // optional simplify converter
    if (smooth > 0.0) converter.set<smooth_tag>(); // optional smooth converter

    for (geometry_type & geom : feature.paths())
    {
        if (geom.size() > 1)
        {
            converter.apply(geom);
        }
    }
    // stroke
    context_.set_fill_rule(CAIRO_FILL_RULE_WINDING);
    context_.stroke();
}

void cairo_renderer_base::render_box(box2d<double> const& b)
{
    cairo_save_restore guard(context_);
    context_.move_to(b.minx(), b.miny());
    context_.line_to(b.minx(), b.maxy());
    context_.line_to(b.maxx(), b.maxy());
    context_.line_to(b.maxx(), b.miny());
    context_.close_path();
    context_.stroke();
}

void render_vector_marker(cairo_context & context, pixel_position const& pos, mapnik::svg_storage_type & vmarker,
                          agg::pod_bvector<svg::path_attributes> const & attributes,
                          agg::trans_affine const& tr, double opacity, bool recenter)
{
    using namespace mapnik::svg;
    box2d<double> bbox = vmarker.bounding_box();

    agg::trans_affine mtx = tr;

    if (recenter)
    {
        coord<double,2> c = bbox.center();
        mtx = agg::trans_affine_translation(-c.x,-c.y);
        mtx *= tr;
        mtx.translate(pos.x, pos.y);
    }

    agg::trans_affine transform;

    for(unsigned i = 0; i < attributes.size(); ++i)
    {
        mapnik::svg::path_attributes const& attr = attributes[i];
        if (!attr.visibility_flag)
            continue;
        cairo_save_restore guard(context);
        transform = attr.transform;
        transform *= mtx;

        // TODO - this 'is_valid' check is not used in the AGG renderer and also
        // appears to lead to bogus results with
        // tests/data/good_maps/markers_symbolizer_lines_file.xml
        if (/*transform.is_valid() && */ !transform.is_identity())
        {
            double m[6];
            transform.store_to(m);
            cairo_matrix_t matrix;
            cairo_matrix_init(&matrix,m[0],m[1],m[2],m[3],m[4],m[5]);
            context.transform(matrix);
        }

        vertex_stl_adapter<svg_path_storage> stl_storage(vmarker.source());
        svg_path_adapter svg_path(stl_storage);

        if (attr.fill_flag || attr.fill_gradient.get_gradient_type() != NO_GRADIENT)
        {
            context.add_agg_path(svg_path,attr.index);
            if (attr.even_odd_flag)
            {
                context.set_fill_rule(CAIRO_FILL_RULE_EVEN_ODD);
            }
            else
            {
                context.set_fill_rule(CAIRO_FILL_RULE_WINDING);
            }
            if(attr.fill_gradient.get_gradient_type() != NO_GRADIENT)
            {
                cairo_gradient g(attr.fill_gradient,attr.fill_opacity * attr.opacity * opacity);

                context.set_gradient(g,bbox);
                context.fill();
            }
            else if(attr.fill_flag)
            {
                double fill_opacity = attr.fill_opacity * attr.opacity * opacity * attr.fill_color.opacity();
                context.set_color(attr.fill_color.r/255.0,attr.fill_color.g/255.0,
                                  attr.fill_color.b/255.0, fill_opacity);
                context.fill();
            }
        }

        if (attr.stroke_gradient.get_gradient_type() != NO_GRADIENT || attr.stroke_flag)
        {
            context.add_agg_path(svg_path,attr.index);
            if(attr.stroke_gradient.get_gradient_type() != NO_GRADIENT)
            {
                context.set_line_width(attr.stroke_width);
                context.set_line_cap(line_cap_enum(attr.line_cap));
                context.set_line_join(line_join_enum(attr.line_join));
                context.set_miter_limit(attr.miter_limit);
                cairo_gradient g(attr.stroke_gradient,attr.fill_opacity * attr.opacity * opacity);
                context.set_gradient(g,bbox);
                context.stroke();
            }
            else if (attr.stroke_flag)
            {
                double stroke_opacity = attr.stroke_opacity * attr.opacity * opacity * attr.stroke_color.opacity();
                context.set_color(attr.stroke_color.r/255.0,attr.stroke_color.g/255.0,
                                  attr.stroke_color.b/255.0, stroke_opacity);
                context.set_line_width(attr.stroke_width);
                context.set_line_cap(line_cap_enum(attr.line_cap));
                context.set_line_join(line_join_enum(attr.line_join));
                context.set_miter_limit(attr.miter_limit);
                context.stroke();
            }
        }
    }
}


void cairo_renderer_base::render_marker(pixel_position const& pos,
                                        marker const& marker,
                                        agg::trans_affine const& tr,
                                        double opacity,
                                        bool recenter)

{
    cairo_save_restore guard(context_);
    if (marker.is_vector())
    {
        mapnik::svg_path_ptr vmarker = *marker.get_vector_data();
        if (vmarker)
        {
            agg::trans_affine marker_tr = tr;
            marker_tr *=agg::trans_affine_scaling(common_.scale_factor_);
            agg::pod_bvector<svg::path_attributes> const & attributes = vmarker->attributes();
            render_vector_marker(context_, pos, *vmarker, attributes, marker_tr, opacity, recenter);
        }
    }
    else if (marker.is_bitmap())
    {
        double width = (*marker.get_bitmap_data())->width();
        double height = (*marker.get_bitmap_data())->height();
        double cx = 0.5 * width;
        double cy = 0.5 * height;
        agg::trans_affine marker_tr;
        marker_tr *= agg::trans_affine_translation(-cx,-cy);
        marker_tr *= tr;
        marker_tr *= agg::trans_affine_scaling(common_.scale_factor_);
        marker_tr *= agg::trans_affine_translation(pos.x,pos.y);
        context_.add_image(marker_tr, **marker.get_bitmap_data(), opacity);
    }
}

void cairo_renderer_base::process(point_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    composite_mode_e comp_op = get<composite_mode_e>(sym, keys::comp_op, feature, common_.vars_, src_over);

    cairo_save_restore guard(context_);
    context_.set_operator(comp_op);

    render_point_symbolizer(
        sym, feature, prj_trans, common_,
        [&](pixel_position const& pos, marker const& marker, 
            agg::trans_affine const& tr, double opacity) {
            render_marker(pos, marker, tr, opacity);
        });
}

void cairo_renderer_base::process(shield_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    text_symbolizer_helper helper(
            sym, feature, common_.vars_, prj_trans,
            common_.width_, common_.height_,
            common_.scale_factor_,
            common_.t_, common_.font_manager_, *common_.detector_,
            common_.query_extent_);

    cairo_save_restore guard(context_);
    composite_mode_e comp_op = get<composite_mode_e>(sym, keys::comp_op, feature, common_.vars_, src_over);
    double opacity = get<double>(sym,keys::opacity,feature, common_.vars_, 1.0);

    context_.set_operator(comp_op);

    placements_list const &placements = helper.get();
    for (glyph_positions_ptr glyphs : placements)
    {
        if (glyphs->marker()) {
            pixel_position pos = glyphs->marker_pos();
            render_marker(pos,
                          *(glyphs->marker()->marker),
                          glyphs->marker()->transform,
                          opacity);
        }

        context_.add_text(glyphs, face_manager_, common_.font_manager_, common_.scale_factor_);
    }
}

void cairo_renderer_base::process(line_pattern_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    typedef agg::conv_clip_polyline<geometry_type> clipped_geometry_type;
    typedef coord_transform<CoordTransform,clipped_geometry_type> path_type;

    std::string filename = get<std::string>(sym, keys::file, feature, common_.vars_);
    composite_mode_e comp_op = get<composite_mode_e>(sym, keys::comp_op, feature, common_.vars_, src_over);

    boost::optional<marker_ptr> marker;
    if ( !filename.empty() )
    {
        marker = marker_cache::instance().find(filename, true);
    }
    else
    {
        marker.reset(std::make_shared<mapnik::marker>());
    }
    if (!marker && !(*marker)->is_bitmap()) return;

    unsigned width((*marker)->width());
    unsigned height((*marker)->height());

    cairo_save_restore guard(context_);
    context_.set_operator(comp_op);
    cairo_pattern pattern(**((*marker)->get_bitmap_data()));

    pattern.set_extend(CAIRO_EXTEND_REPEAT);
    pattern.set_filter(CAIRO_FILTER_BILINEAR);
    context_.set_line_width(height * common_.scale_factor_);

    for (std::size_t i = 0; i < feature.num_geometries(); ++i)
    {
        geometry_type & geom = feature.get_geometry(i);

        if (geom.size() > 1)
        {
            clipped_geometry_type clipped(geom);
            clipped.clip_box(common_.query_extent_.minx(),common_.query_extent_.miny(),common_.query_extent_.maxx(),common_.query_extent_.maxy());
            path_type path(common_.t_,clipped,prj_trans);

            double length(0);
            double x0(0), y0(0);
            double x, y;

            for (unsigned cm = path.vertex(&x, &y); cm != SEG_END; cm = path.vertex(&x, &y))
            {
                if (cm == SEG_MOVETO)
                {
                    length = 0.0;
                }
                else if (cm == SEG_LINETO)
                {
                    double dx = x - x0;
                    double dy = y - y0;
                    double angle = std::atan2(dy, dx);
                    double offset = std::fmod(length, width);

                    cairo_matrix_t matrix;
                    cairo_matrix_init_identity(&matrix);
                    cairo_matrix_translate(&matrix,x0,y0);
                    cairo_matrix_rotate(&matrix,angle);
                    cairo_matrix_translate(&matrix,-offset,0.5*height);
                    cairo_matrix_invert(&matrix);

                    pattern.set_matrix(matrix);

                    context_.set_pattern(pattern);

                    context_.move_to(x0, y0);
                    context_.line_to(x, y);
                    context_.stroke();

                    length = length + hypot(x - x0, y - y0);
                }

                x0 = x;
                y0 = y;
            }
        }
    }
}

void cairo_renderer_base::process(polygon_pattern_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    //typedef agg::conv_clip_polygon<geometry_type> clipped_geometry_type;
    //typedef coord_transform<CoordTransform,clipped_geometry_type> path_type;

    cairo_save_restore guard(context_);
    composite_mode_e comp_op = get<composite_mode_e>(sym, keys::comp_op, feature, common_.vars_, src_over);
    std::string filename = get<std::string>(sym, keys::file, feature, common_.vars_);
    bool clip = get<bool>(sym, keys::clip, feature, common_.vars_, true);
    double simplify_tolerance = get<double>(sym, keys::simplify_tolerance, feature, common_.vars_, 0.0);
    double smooth = get<double>(sym, keys::smooth, feature, common_.vars_, 0.0);

    context_.set_operator(comp_op);

    boost::optional<mapnik::marker_ptr> marker = mapnik::marker_cache::instance().find(filename,true);
    if (!marker && !(*marker)->is_bitmap()) return;

    cairo_pattern pattern(**((*marker)->get_bitmap_data()));

    pattern.set_extend(CAIRO_EXTEND_REPEAT);

    context_.set_pattern(pattern);

    //pattern_alignment_e align = sym.get_alignment();
    //unsigned offset_x=0;
    //unsigned offset_y=0;

    //if (align == LOCAL_ALIGNMENT)
    //{
    //    double x0 = 0;
    //    double y0 = 0;
    //    if (feature.num_geometries() > 0)
    //    {
    //        clipped_geometry_type clipped(feature.get_geometry(0));
    //        clipped.clip_box(query_extent_.minx(),query_extent_.miny(),query_extent_.maxx(),query_extent_.maxy());
    //        path_type path(t_,clipped,prj_trans);
    //        path.vertex(&x0,&y0);
    //    }
    //    offset_x = unsigned(width_ - x0);
    //    offset_y = unsigned(height_ - y0);
    //}

    agg::trans_affine tr;
    auto geom_transform = get_optional<transform_type>(sym, keys::geometry_transform);
    if (geom_transform) { evaluate_transform(tr, feature, common_.vars_, *geom_transform, common_.scale_factor_); }

    typedef boost::mpl::vector<clip_poly_tag,transform_tag,affine_transform_tag,simplify_tag,smooth_tag> conv_types;
    vertex_converter<box2d<double>, cairo_context, polygon_pattern_symbolizer,
                     CoordTransform, proj_transform, agg::trans_affine, conv_types, feature_impl>
        converter(common_.query_extent_,context_,sym,common_.t_,prj_trans,tr,feature,common_.vars_,common_.scale_factor_);

    if (prj_trans.equal() && clip) converter.set<clip_poly_tag>(); //optional clip (default: true)
    converter.set<transform_tag>(); //always transform
    converter.set<affine_transform_tag>();
    if (simplify_tolerance > 0.0) converter.set<simplify_tag>(); // optional simplify converter
    if (smooth > 0.0) converter.set<smooth_tag>(); // optional smooth converter

    for ( geometry_type & geom : feature.paths())
    {
        if (geom.size() > 2)
        {
            converter.apply(geom);
        }
    }
    // fill polygon
    context_.set_fill_rule(CAIRO_FILL_RULE_EVEN_ODD);
    context_.fill();
}

void cairo_renderer_base::process(raster_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    cairo_save_restore guard(context_);
    render_raster_symbolizer(
        sym, feature, prj_trans, common_,
        [&](image_data_32 &target, composite_mode_e comp_op, double opacity,
            int start_x, int start_y) {
            context_.set_operator(comp_op);
            context_.add_image(start_x, start_y, target, opacity);
        }
    );
}

namespace detail {

template <typename Context, typename SvgPath, typename Attributes, typename Detector>
struct markers_dispatch
{
    markers_dispatch(Context & ctx,
                     SvgPath & marker,
                     Attributes const& attributes,
                     Detector & detector,
                     markers_symbolizer const& sym,
                     box2d<double> const& bbox,
                     agg::trans_affine const& marker_trans,
                     feature_impl const& feature,
                     mapnik::attributes const& vars,
                     double scale_factor)
        :ctx_(ctx),
         marker_(marker),
         attributes_(attributes),
         detector_(detector),
         sym_(sym),
         bbox_(bbox),
         marker_trans_(marker_trans),
         feature_(feature),
         vars_(vars),
         scale_factor_(scale_factor) {}


    template <typename T>
    void add_path(T & path)
    {
        marker_placement_enum placement_method = get<marker_placement_enum>(sym_, keys::markers_placement_type, feature_, vars_, MARKER_POINT_PLACEMENT);
        bool ignore_placement = get<bool>(sym_, keys::ignore_placement, feature_, vars_, false);
        bool allow_overlap = get<bool>(sym_, keys::allow_overlap, feature_, vars_, false);
        double opacity = get<double>(sym_, keys::opacity, feature_, vars_, 1.0);
        double spacing = get<double>(sym_, keys::spacing, feature_, vars_, 100.0);
        double max_error = get<double>(sym_, keys::max_error, feature_, vars_, 0.2);

        if (placement_method != MARKER_LINE_PLACEMENT ||
            path.type() == geometry_type::types::Point)
        {
            double x = 0;
            double y = 0;
            if (path.type() == geometry_type::types::LineString)
            {
                if (!label::middle_point(path, x, y))
                    return;
            }
            else if (placement_method == MARKER_INTERIOR_PLACEMENT)
            {
                if (!label::interior_position(path, x, y))
                    return;
            }
            else
            {
                if (!label::centroid(path, x, y))
                    return;
            }
            coord2d center = bbox_.center();
            agg::trans_affine matrix = agg::trans_affine_translation(-center.x, -center.y);
            matrix *= marker_trans_;
            matrix *=agg::trans_affine_translation(x, y);

            box2d<double> transformed_bbox = bbox_ * matrix;

            if (allow_overlap ||
                detector_.has_placement(transformed_bbox))
            {
                render_vector_marker(ctx_, pixel_position(x,y), marker_, attributes_, marker_trans_, opacity, true);

                if (!ignore_placement)
                {
                    detector_.insert(transformed_bbox);
                }
            }
        }
        else
        {
            markers_placement<T, label_collision_detector4> placement(path, bbox_, marker_trans_, detector_,
                                                                      spacing * scale_factor_,
                                                                      max_error,
                                                                      allow_overlap);
            double x, y, angle;
            while (placement.get_point(x, y, angle, ignore_placement))
            {
                agg::trans_affine matrix = marker_trans_;
                matrix.rotate(angle);
                render_vector_marker(ctx_, pixel_position(x,y),marker_, attributes_, matrix, opacity, true);

            }
        }
    }

    Context & ctx_;
    SvgPath & marker_;
    Attributes const& attributes_;
    Detector & detector_;
    markers_symbolizer const& sym_;
    box2d<double> const& bbox_;
    agg::trans_affine const& marker_trans_;
    feature_impl const& feature_;
    attributes const& vars_;
    double scale_factor_;
};

template <typename Context, typename ImageMarker, typename Detector>
struct markers_dispatch_2
{
    markers_dispatch_2(Context & ctx,
                       ImageMarker & marker,
                       Detector & detector,
                       markers_symbolizer const& sym,
                       box2d<double> const& bbox,
                       agg::trans_affine const& marker_trans,
                       feature_impl const& feature,
                       mapnik::attributes const& vars,
                       double scale_factor)
        :ctx_(ctx),
         marker_(marker),
         detector_(detector),
         sym_(sym),
         bbox_(bbox),
         marker_trans_(marker_trans),
         feature_(feature),
         vars_(vars),
         scale_factor_(scale_factor) {}


    template <typename T>
    void add_path(T & path)
    {
        marker_placement_enum placement_method = get<marker_placement_enum>(sym_, keys::markers_placement_type, feature_, vars_, MARKER_POINT_PLACEMENT);
        double opacity = get<double>(sym_, keys::opacity, feature_, vars_,  1.0);
        double spacing = get<double>(sym_, keys::spacing, feature_, vars_,  100.0);
        double max_error = get<double>(sym_, keys::max_error, feature_, vars_,  0.2);
        bool allow_overlap = get<bool>(sym_, keys::allow_overlap, feature_, vars_,  false);
        bool ignore_placement = get<bool>(sym_, keys::ignore_placement, feature_, vars_,  false);

        if (placement_method != MARKER_LINE_PLACEMENT ||
            path.type() == geometry_type::types::Point)
        {
            double x = 0;
            double y = 0;
            if (path.type() == geometry_type::types::LineString)
            {
                if (!label::middle_point(path, x, y))
                    return;
            }
            else if (placement_method == MARKER_INTERIOR_PLACEMENT)
            {
                if (!label::interior_position(path, x, y))
                    return;
            }
            else
            {
                if (!label::centroid(path, x, y))
                    return;
            }
            coord2d center = bbox_.center();
            agg::trans_affine matrix = agg::trans_affine_translation(-center.x, -center.y);
            matrix *= marker_trans_;
            matrix *=agg::trans_affine_translation(x, y);

            box2d<double> transformed_bbox = bbox_ * matrix;

            if (allow_overlap ||
                detector_.has_placement(transformed_bbox))
            {
                ctx_.add_image(matrix, marker_, opacity);
                if (!ignore_placement)
                {
                    detector_.insert(transformed_bbox);
                }
            }
        }
        else
        {
            markers_placement<T, label_collision_detector4> placement(path, bbox_, marker_trans_, detector_,
                                                                      spacing * scale_factor_,
                                                                      max_error,
                                                                      allow_overlap);
            double x, y, angle;
            while (placement.get_point(x, y, angle, ignore_placement))
            {
                coord2d center = bbox_.center();
                agg::trans_affine matrix = agg::trans_affine_translation(-center.x, -center.y);
                matrix *= marker_trans_;
                matrix *= agg::trans_affine_rotation(angle);
                matrix *= agg::trans_affine_translation(x, y);
                ctx_.add_image(matrix, marker_, opacity);
            }
        }
    }

    Context & ctx_;
    ImageMarker & marker_;
    Detector & detector_;
    markers_symbolizer const& sym_;
    box2d<double> const& bbox_;
    agg::trans_affine const& marker_trans_;
    feature_impl const& feature_;
    attributes const& vars_;
    double scale_factor_;
};

}
void cairo_renderer_base::process(markers_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    typedef agg::pod_bvector<svg::path_attributes> svg_attribute_type;
    typedef detail::markers_dispatch_2<cairo_context, mapnik::image_data_32,
                                       label_collision_detector4> raster_dispatch_type;
    typedef detail::markers_dispatch<cairo_context, mapnik::svg_storage_type, svg_attribute_type,
                                     label_collision_detector4> vector_dispatch_type;

    cairo_save_restore guard(context_);
    composite_mode_e comp_op = get<composite_mode_e>(sym, keys::comp_op, feature, common_.vars_, src_over);
    context_.set_operator(comp_op);
    box2d<double> clip_box = common_.query_extent_;

    render_markers_symbolizer(
        sym, feature, prj_trans, common_, clip_box,
        [&](svg::svg_path_adapter &, svg_attribute_type const &attr, svg_storage_type &marker,
            box2d<double> const &bbox, agg::trans_affine const &marker_trans,
            bool) -> vector_dispatch_type {
            return vector_dispatch_type(context_, marker, attr, *common_.detector_, sym, bbox, 
                                        marker_trans, feature, common_.vars_, common_.scale_factor_);
        },
        [&](image_data_32 &marker, agg::trans_affine const &marker_trans,
            box2d<double> const &bbox) -> raster_dispatch_type {
            return raster_dispatch_type(context_, marker, *common_.detector_, sym, bbox, 
                                        marker_trans, feature, common_.vars_, common_.scale_factor_);
        });
}

void cairo_renderer_base::process(text_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    text_symbolizer_helper helper(
            sym, feature, common_.vars_, prj_trans,
            common_.width_, common_.height_,
            common_.scale_factor_,
            common_.t_, common_.font_manager_, *common_.detector_,
            common_.query_extent_);

    cairo_save_restore guard(context_);
    composite_mode_e comp_op = get<composite_mode_e>(sym, keys::comp_op, feature, common_.vars_,  src_over);
    context_.set_operator(comp_op);

    placements_list const &placements = helper.get();
    for (glyph_positions_ptr glyphs : placements)
    {
        context_.add_text(glyphs, face_manager_, common_.font_manager_, common_.scale_factor_);
    }
}

namespace {

/**
 * Render a thunk which was frozen from a previous call to 
 * extract_bboxes. We should now have a new offset at which
 * to render it, and the boxes themselves should already be
 * in the detector from the placement_finder.
 */
struct thunk_renderer : public boost::static_visitor<>
{
    typedef cairo_renderer_base renderer_type;

    thunk_renderer(renderer_type &ren,
                   cairo_context &context,
                   cairo_face_manager &face_manager,
                   renderer_common &common,
                   pixel_position const &offset)
        : ren_(ren), context_(context), face_manager_(face_manager),
          common_(common), offset_(offset)
    {}

    void operator()(point_render_thunk const &thunk) const
    {
        pixel_position new_pos(thunk.pos_.x + offset_.x, thunk.pos_.y + offset_.y);
        ren_.render_marker(new_pos, *thunk.marker_, thunk.tr_, thunk.opacity_,
                           thunk.comp_op_);
    }

    void operator()(text_render_thunk const &thunk) const
    {
        cairo_save_restore guard(context_);
        context_.set_operator(thunk.comp_op_);

        render_offset_placements(
            thunk.placements_,
            offset_,
            [&] (glyph_positions_ptr glyphs)
            {
                if (glyphs->marker())
                {
                    ren_.render_marker(glyphs->marker_pos(),
                                       *(glyphs->marker()->marker),
                                       glyphs->marker()->transform,
                                       thunk.opacity_, thunk.comp_op_);
                }
                context_.add_text(glyphs, face_manager_, common_.font_manager_, common_.scale_factor_);
            });
    }

    template <typename T>
    void operator()(T const &) const
    {
        // TODO: warning if unimplemented?
    }

private:
    renderer_type &ren_;
    cairo_context &context_;
    cairo_face_manager &face_manager_;
    renderer_common &common_;
    pixel_position offset_;
};

} // anonymous namespace

void cairo_renderer_base::process(group_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    render_group_symbolizer(
        sym, feature, common_.vars_, prj_trans, common_.query_extent_, common_,
        [&](render_thunk_list const& thunks, pixel_position const& render_offset)
        {
            thunk_renderer ren(*this, context_, face_manager_, common_, render_offset);
            for (render_thunk_ptr const& thunk : thunks)
            {
                boost::apply_visitor(ren, *thunk);
            }
        });
}

namespace {

// special implementation of the box drawing so that it's pixel-aligned
void render_debug_box(cairo_context &context, box2d<double> const& b)
{
    cairo_save_restore guard(context);
    double minx = std::floor(b.minx()) + 0.5;
    double miny = std::floor(b.miny()) + 0.5;
    double maxx = std::floor(b.maxx()) + 0.5;
    double maxy = std::floor(b.maxy()) + 0.5;
    context.move_to(minx, miny);
    context.line_to(minx, maxy);
    context.line_to(maxx, maxy);
    context.line_to(maxx, miny);
    context.close_path();
    context.stroke();
}

} // anonymous namespace

void cairo_renderer_base::process(debug_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    typedef label_collision_detector4 detector_type;
    cairo_save_restore guard(context_);

    debug_symbolizer_mode_enum mode = get<debug_symbolizer_mode_enum>(sym, keys::mode, feature, common_.vars_, DEBUG_SYM_MODE_COLLISION);

    context_.set_operator(src_over);
    context_.set_color(mapnik::color(255, 0, 0), 1.0);
    context_.set_line_join(MITER_JOIN);
    context_.set_line_cap(BUTT_CAP);
    context_.set_miter_limit(4.0);
    context_.set_line_width(1.0);

    if (mode == DEBUG_SYM_MODE_COLLISION)
    {
        typename detector_type::query_iterator itr = common_.detector_->begin();
        typename detector_type::query_iterator end = common_.detector_->end();
        for ( ;itr!=end; ++itr)
        {
            render_debug_box(context_, itr->box);
        }
    }
    else if (mode == DEBUG_SYM_MODE_VERTEX)
    {
        for (auto const& geom : feature.paths())
        {
            double x;
            double y;
            double z = 0;
            geom.rewind(0);
            unsigned cmd = 1;
            while ((cmd = geom.vertex(&x, &y)) != mapnik::SEG_END)
            {
                if (cmd == SEG_CLOSE) continue;
                prj_trans.backward(x,y,z);
                common_.t_.forward(&x,&y);
                context_.move_to(std::floor(x) - 0.5, std::floor(y) + 0.5);
                context_.line_to(std::floor(x) + 1.5, std::floor(y) + 0.5);
                context_.move_to(std::floor(x) + 0.5, std::floor(y) - 0.5);
                context_.line_to(std::floor(x) + 0.5, std::floor(y) + 1.5);
                context_.stroke();
            }
        }
    }
}

template class cairo_renderer<cairo_surface_ptr>;
template class cairo_renderer<cairo_ptr>;
}

#endif // HAVE_CAIRO
