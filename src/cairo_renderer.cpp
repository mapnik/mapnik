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
#include <mapnik/symbolizer_helpers.hpp>
#include <mapnik/raster_colorizer.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/warp.hpp>
#include <mapnik/config.hpp>
#include <mapnik/text_path.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/marker_helpers.hpp>
#include <mapnik/noncopyable.hpp>
#include <mapnik/pixel_position.hpp>

// cairo
#include <cairo.h>
#include <cairo-ft.h>
#include <cairo-version.h>

// boost
#include <boost/make_shared.hpp>
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


cairo_face_manager::cairo_face_manager(boost::shared_ptr<freetype_engine> engine)
    : font_engine_(engine)
{
}

cairo_face_ptr cairo_face_manager::get_face(face_ptr face)
{
    cairo_face_cache::iterator itr = cache_.find(face);
    cairo_face_ptr entry;

    if (itr != cache_.end())
    {
        entry = itr->second;
    }
    else
    {
        entry = boost::make_shared<cairo_face>(font_engine_, face);
        cache_.insert(std::make_pair(face, entry));
    }

    return entry;
}


cairo_renderer_base::cairo_renderer_base(Map const& m,
                                         cairo_ptr const& cairo,
                                         double scale_factor,
                                         unsigned offset_x,
                                         unsigned offset_y)
    : m_(m),
      context_(cairo),
      width_(m.width()),
      height_(m.height()),
      scale_factor_(scale_factor),
      t_(m.width(),m.height(),m.get_current_extent(),offset_x,offset_y),
      font_engine_(boost::make_shared<freetype_engine>()),
      font_manager_(*font_engine_,m.get_font_file_mapping()),
      face_manager_(font_engine_),
      detector_(boost::make_shared<label_collision_detector4>(
          box2d<double>(-m.buffer_size(), -m.buffer_size(),
          m.width() + m.buffer_size(), m.height() + m.buffer_size())))
{
    setup(m);
}

cairo_renderer_base::cairo_renderer_base(Map const& m,
                                         request const& req,
                                         cairo_ptr const& cairo,
                                         double scale_factor,
                                         unsigned offset_x,
                                         unsigned offset_y)
    : m_(m),
      context_(cairo),
      width_(req.width()),
      height_(req.height()),
      scale_factor_(scale_factor),
      t_(req.width(),req.height(),req.extent(),offset_x,offset_y),
      font_engine_(boost::make_shared<freetype_engine>()),
      font_manager_(*font_engine_,m.get_font_file_mapping()),
      face_manager_(font_engine_),
      detector_(boost::make_shared<label_collision_detector4>(
          box2d<double>(-req.buffer_size(), -req.buffer_size(),
          req.width() + req.buffer_size(), req.height() + req.buffer_size())))
{
    setup(m);
}

cairo_renderer_base::cairo_renderer_base(Map const& m,
                                         cairo_ptr const& cairo,
                                         boost::shared_ptr<label_collision_detector4> detector,
                                         double scale_factor,
                                         unsigned offset_x,
                                         unsigned offset_y)
    : m_(m),
      context_(cairo),
      width_(m.width()),
      height_(m.height()),
      scale_factor_(scale_factor),
      t_(m.width(),m.height(),m.get_current_extent(),offset_x,offset_y),
      font_engine_(boost::make_shared<freetype_engine>()),
      font_manager_(*font_engine_,m.get_font_file_mapping()),
      face_manager_(font_engine_),
      detector_(detector)
{
    MAPNIK_LOG_DEBUG(cairo_renderer) << "cairo_renderer_base: Scale=" << m.scale();
}

template <>
cairo_renderer<cairo_ptr>::cairo_renderer(Map const& m, cairo_ptr const& cairo, double scale_factor, unsigned offset_x, unsigned offset_y)
    : feature_style_processor<cairo_renderer>(m,scale_factor),
      cairo_renderer_base(m,cairo,scale_factor,offset_x,offset_y) {}

template <>
cairo_renderer<cairo_surface_ptr>::cairo_renderer(Map const& m, cairo_surface_ptr const& surface, double scale_factor, unsigned offset_x, unsigned offset_y)
    : feature_style_processor<cairo_renderer>(m,scale_factor),
      cairo_renderer_base(m,create_context(surface),scale_factor,offset_x,offset_y) {}

template <>
cairo_renderer<cairo_ptr>::cairo_renderer(Map const& m, request const& req, cairo_ptr const& cairo, double scale_factor, unsigned offset_x, unsigned offset_y)
    : feature_style_processor<cairo_renderer>(m,scale_factor),
      cairo_renderer_base(m,req,cairo,scale_factor,offset_x,offset_y) {}

template <>
cairo_renderer<cairo_surface_ptr>::cairo_renderer(Map const& m, request const& req, cairo_surface_ptr const& surface, double scale_factor, unsigned offset_x, unsigned offset_y)
    : feature_style_processor<cairo_renderer>(m,scale_factor),
      cairo_renderer_base(m,req, create_context(surface),scale_factor,offset_x,offset_y) {}

template <>
cairo_renderer<cairo_ptr>::cairo_renderer(Map const& m, cairo_ptr const& cairo, boost::shared_ptr<label_collision_detector4> detector, double scale_factor, unsigned offset_x, unsigned offset_y)
    : feature_style_processor<cairo_renderer>(m,scale_factor),
      cairo_renderer_base(m,cairo,detector,scale_factor,offset_x,offset_y) {}

template <>
cairo_renderer<cairo_surface_ptr>::cairo_renderer(Map const& m, cairo_surface_ptr const& surface, boost::shared_ptr<label_collision_detector4> detector, double scale_factor, unsigned offset_x, unsigned offset_y)
    : feature_style_processor<cairo_renderer>(m,scale_factor),
      cairo_renderer_base(m,create_context(surface),detector,scale_factor,offset_x,offset_y) {}

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
                unsigned x_steps = unsigned(std::ceil(width_/double(w)));
                unsigned y_steps = unsigned(std::ceil(height_/double(h)));
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
    box2d<double> bounds = t_.forward(t_.extent());
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
        detector_->clear();
    }
    query_extent_ = query_extent;
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
    cairo_save_restore guard(context_);
    context_.set_operator(sym.comp_op());
    context_.set_color(sym.get_fill(), sym.get_opacity());

    agg::trans_affine tr;
    evaluate_transform(tr, feature, sym.get_transform(), scale_factor_);

    typedef boost::mpl::vector<clip_poly_tag,transform_tag,affine_transform_tag,simplify_tag,smooth_tag> conv_types;
    vertex_converter<box2d<double>, cairo_context, polygon_symbolizer,
                     CoordTransform, proj_transform, agg::trans_affine, conv_types>
        converter(query_extent_,context_,sym,t_,prj_trans,tr,1.0);

    if (prj_trans.equal() && sym.clip()) converter.set<clip_poly_tag>(); //optional clip (default: true)
    converter.set<transform_tag>(); //always transform
    converter.set<affine_transform_tag>();
    if (sym.simplify_tolerance() > 0.0) converter.set<simplify_tag>(); // optional simplify converter
    if (sym.smooth() > 0.0) converter.set<smooth_tag>(); // optional smooth converter

    BOOST_FOREACH( geometry_type & geom, feature.paths())
    {
        if (geom.size() > 2)
        {
            converter.apply(geom);
        }
    }
    // fill polygon
    context_.fill();
}

void cairo_renderer_base::process(building_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    typedef coord_transform<CoordTransform,geometry_type> path_type;
    cairo_save_restore guard(context_);
    context_.set_operator(sym.comp_op());
    color const& fill = sym.get_fill();
    double height = 0.0;
    expression_ptr height_expr = sym.height();
    if (height_expr)
    {
        value_type result = boost::apply_visitor(evaluate<feature_impl,value_type>(feature), *height_expr);
        height = result.to_double() * scale_factor_;
    }

    for (std::size_t i = 0; i < feature.num_geometries(); ++i)
    {
        geometry_type const& geom = feature.get_geometry(i);

        if (geom.size() > 2)
        {
            boost::scoped_ptr<geometry_type> frame(new geometry_type(LineString));
            boost::scoped_ptr<geometry_type> roof(new geometry_type(Polygon));
            std::deque<segment_t> face_segments;
            double x0 = 0;
            double y0 = 0;
            double x, y;
            geom.rewind(0);
            for (unsigned cm = geom.vertex(&x, &y); cm != SEG_END;
                 cm = geom.vertex(&x, &y))
            {
                if (cm == SEG_MOVETO)
                {
                    frame->move_to(x,y);
                }
                else if (cm == SEG_LINETO)
                {
                    frame->line_to(x,y);
                    face_segments.push_back(segment_t(x0,y0,x,y));
                }
                else if (cm == SEG_CLOSE)
                {
                    frame->close_path();
                }
                x0 = x;
                y0 = y;
            }

            std::sort(face_segments.begin(), face_segments.end(), y_order);
            std::deque<segment_t>::const_iterator itr = face_segments.begin();
            std::deque<segment_t>::const_iterator end=face_segments.end();
            for (; itr != end; ++itr)
            {
                boost::scoped_ptr<geometry_type> faces(new geometry_type(Polygon));
                faces->move_to(itr->get<0>(), itr->get<1>());
                faces->line_to(itr->get<2>(), itr->get<3>());
                faces->line_to(itr->get<2>(), itr->get<3>() + height);
                faces->line_to(itr->get<0>(), itr->get<1>() + height);

                path_type faces_path(t_, *faces, prj_trans);
                context_.set_color(fill.red()  * 0.8 / 255.0, fill.green() * 0.8 / 255.0,
                                  fill.blue() * 0.8 / 255.0, fill.alpha() * sym.get_opacity() / 255.0);
                context_.add_path(faces_path);
                context_.fill();

                frame->move_to(itr->get<0>(), itr->get<1>());
                frame->line_to(itr->get<0>(), itr->get<1>() + height);
            }

            geom.rewind(0);
            for (unsigned cm = geom.vertex(&x, &y); cm != SEG_END;
                 cm = geom.vertex(&x, &y))
            {
                if (cm == SEG_MOVETO)
                {
                    frame->move_to(x,y+height);
                    roof->move_to(x,y+height);
                }
                else if (cm == SEG_LINETO)
                {
                    frame->line_to(x,y+height);
                    roof->line_to(x,y+height);
                }
                else if (cm == SEG_CLOSE)
                {
                    frame->close_path();
                    roof->close_path();
                }
            }

            path_type path(t_, *frame, prj_trans);
            context_.set_color(fill.red()  * 0.8 / 255.0, fill.green() * 0.8/255.0,
                              fill.blue() * 0.8 / 255.0, fill.alpha() * sym.get_opacity() / 255.0);
            context_.set_line_width(scale_factor_);
            context_.add_path(path);
            context_.stroke();

            path_type roof_path(t_, *roof, prj_trans);
            context_.set_color(fill, sym.get_opacity());
            context_.add_path(roof_path);
            context_.fill();
        }
    }
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
    mapnik::stroke const& stroke_ = sym.get_stroke();
    context_.set_operator(sym.comp_op());
    context_.set_color(stroke_.get_color(), stroke_.get_opacity());
    context_.set_line_join(stroke_.get_line_join());
    context_.set_line_cap(stroke_.get_line_cap());
    context_.set_miter_limit(stroke_.get_miterlimit());
    context_.set_line_width(stroke_.get_width() * scale_factor_);
    if (stroke_.has_dash())
    {
        context_.set_dash(stroke_.get_dash_array(), scale_factor_);
    }

    agg::trans_affine tr;
    evaluate_transform(tr, feature, sym.get_transform(), scale_factor_);

    box2d<double> clipping_extent = query_extent_;
    if (sym.clip())
    {
        double padding = (double)(query_extent_.width()/width_);
        double half_stroke = stroke_.get_width()/2.0;
        if (half_stroke > 1)
            padding *= half_stroke;
        if (std::fabs(sym.offset()) > 0)
            padding *= std::fabs(sym.offset()) * 1.2;
        padding *= scale_factor_;
        clipping_extent.pad(padding);
    }
    vertex_converter<box2d<double>, cairo_context, line_symbolizer,
                     CoordTransform, proj_transform, agg::trans_affine, conv_types>
        converter(clipping_extent,context_,sym,t_,prj_trans,tr,scale_factor_);

    if (sym.clip()) converter.set<clip_line_tag>(); // optional clip (default: true)
    converter.set<transform_tag>(); // always transform
    if (std::fabs(sym.offset()) > 0.0) converter.set<offset_transform_tag>(); // parallel offset
    converter.set<affine_transform_tag>(); // optional affine transform
    if (sym.simplify_tolerance() > 0.0) converter.set<simplify_tag>(); // optional simplify converter
    if (sym.smooth() > 0.0) converter.set<smooth_tag>(); // optional smooth converter

    BOOST_FOREACH( geometry_type & geom, feature.paths())
    {
        if (geom.size() > 1)
        {
            converter.apply(geom);
        }
    }
    // stroke
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
            marker_tr *=agg::trans_affine_scaling(scale_factor_);
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
        marker_tr *= agg::trans_affine_scaling(scale_factor_);
        marker_tr *= agg::trans_affine_translation(pos.x,pos.y);
        context_.add_image(marker_tr, **marker.get_bitmap_data(), opacity);
    }
}

void cairo_renderer_base::process(point_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    std::string filename = path_processor_type::evaluate( *sym.get_filename(), feature);

    boost::optional<marker_ptr> marker;
    if ( !filename.empty() )
    {
        marker = marker_cache::instance().find(filename, true);
    }
    else
    {
        marker.reset(boost::make_shared<mapnik::marker>());
    }


    if (marker)
    {
        box2d<double> const& bbox = (*marker)->bounding_box();
        coord2d center = bbox.center();

        agg::trans_affine tr;
        evaluate_transform(tr, feature, sym.get_image_transform());
        agg::trans_affine_translation recenter(-center.x, -center.y);
        agg::trans_affine recenter_tr = recenter * tr;
        box2d<double> label_ext = bbox * recenter_tr * agg::trans_affine_scaling(scale_factor_);

        for (std::size_t i = 0; i < feature.num_geometries(); ++i)
        {
            geometry_type const& geom = feature.get_geometry(i);
            double x;
            double y;
            double z = 0;

            if (sym.get_point_placement() == CENTROID_POINT_PLACEMENT)
            {
                if (!label::centroid(geom, x, y))
                    return;
            }
            else
            {
                if (!label::interior_position(geom ,x, y))
                    return;
            }

            prj_trans.backward(x, y, z);
            t_.forward(&x, &y);
            label_ext.re_center(x,y);
            if (sym.get_allow_overlap() ||
                detector_->has_placement(label_ext))
            {
                render_marker(pixel_position(x,y),**marker, tr, sym.get_opacity());

                if (!sym.get_ignore_placement())
                    detector_->insert(label_ext);
            }
        }
    }
}

void cairo_renderer_base::process(shield_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    shield_symbolizer_helper<face_manager<freetype_engine>,
        label_collision_detector4> helper(
            sym, feature, prj_trans,
            width_, height_,
            scale_factor_,
            t_, font_manager_, *detector_, query_extent_);

    cairo_save_restore guard(context_);
    context_.set_operator(sym.comp_op());

    while (helper.next())
    {
        placements_type const& placements = helper.placements();
        for (unsigned int ii = 0; ii < placements.size(); ++ii)
        {
            pixel_position pos = helper.get_marker_position(placements[ii]);
            pos.x += 0.5 * helper.get_marker_width();
            pos.y += 0.5 * helper.get_marker_height();
            render_marker(pos,
                          helper.get_marker(),
                          helper.get_image_transform(),
                          sym.get_opacity());

            context_.add_text(placements[ii], face_manager_, font_manager_, scale_factor_);
        }
    }
}

void cairo_renderer_base::process(line_pattern_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    typedef agg::conv_clip_polyline<geometry_type> clipped_geometry_type;
    typedef coord_transform<CoordTransform,clipped_geometry_type> path_type;

    std::string filename = path_processor_type::evaluate( *sym.get_filename(), feature);
    boost::optional<mapnik::marker_ptr> marker = mapnik::marker_cache::instance().find(filename,true);
    if (!marker && !(*marker)->is_bitmap()) return;

    unsigned width((*marker)->width());
    unsigned height((*marker)->height());

    cairo_save_restore guard(context_);
    context_.set_operator(sym.comp_op());
    cairo_pattern pattern(**((*marker)->get_bitmap_data()));

    pattern.set_extend(CAIRO_EXTEND_REPEAT);
    pattern.set_filter(CAIRO_FILTER_BILINEAR);
    context_.set_line_width(height * scale_factor_);

    for (std::size_t i = 0; i < feature.num_geometries(); ++i)
    {
        geometry_type & geom = feature.get_geometry(i);

        if (geom.size() > 1)
        {
            clipped_geometry_type clipped(geom);
            clipped.clip_box(query_extent_.minx(),query_extent_.miny(),query_extent_.maxx(),query_extent_.maxy());
            path_type path(t_,clipped,prj_trans);

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
    context_.set_operator(sym.comp_op());

    std::string filename = path_processor_type::evaluate( *sym.get_filename(), feature);
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
    evaluate_transform(tr, feature, sym.get_transform(), scale_factor_);

    typedef boost::mpl::vector<clip_poly_tag,transform_tag,affine_transform_tag,simplify_tag,smooth_tag> conv_types;
    vertex_converter<box2d<double>, cairo_context, polygon_pattern_symbolizer,
                     CoordTransform, proj_transform, agg::trans_affine, conv_types>
        converter(query_extent_,context_,sym,t_,prj_trans,tr, scale_factor_);

    if (prj_trans.equal() && sym.clip()) converter.set<clip_poly_tag>(); //optional clip (default: true)
    converter.set<transform_tag>(); //always transform
    converter.set<affine_transform_tag>();
    if (sym.simplify_tolerance() > 0.0) converter.set<simplify_tag>(); // optional simplify converter
    if (sym.smooth() > 0.0) converter.set<smooth_tag>(); // optional smooth converter

    BOOST_FOREACH( geometry_type & geom, feature.paths())
    {
        if (geom.size() > 2)
        {
            converter.apply(geom);
        }
    }
    // fill polygon
    context_.fill();
}

void cairo_renderer_base::process(raster_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    raster_ptr const& source = feature.get_raster();
    if (source)
    {
        // If there's a colorizer defined, use it to color the raster in-place
        raster_colorizer_ptr colorizer = sym.get_colorizer();
        if (colorizer)
            colorizer->colorize(source,feature);

        box2d<double> target_ext = box2d<double>(source->ext_);
        prj_trans.backward(target_ext, PROJ_ENVELOPE_POINTS);
        box2d<double> ext = t_.forward(target_ext);
        int start_x = static_cast<int>(std::floor(ext.minx()+.5));
        int start_y = static_cast<int>(std::floor(ext.miny()+.5));
        int end_x = static_cast<int>(std::floor(ext.maxx()+.5));
        int end_y = static_cast<int>(std::floor(ext.maxy()+.5));
        int raster_width = end_x - start_x;
        int raster_height = end_y - start_y;
        if (raster_width > 0 && raster_height > 0)
        {
            scaling_method_e scaling_method = sym.get_scaling_method();
            double filter_radius = sym.calculate_filter_factor();
            bool premultiply_source = !source->premultiplied_alpha_;
            boost::optional<bool> is_premultiplied = sym.premultiplied();
            if (is_premultiplied)
            {
                if (*is_premultiplied) premultiply_source = false;
                else premultiply_source = true;
            }
            if (premultiply_source)
            {
                agg::rendering_buffer buffer(source->data_.getBytes(),
                                             source->data_.width(),
                                             source->data_.height(),
                                             source->data_.width() * 4);
                agg::pixfmt_rgba32 pixf(buffer);
                pixf.premultiply();
            }
            if (!prj_trans.equal())
            {
                double offset_x = ext.minx() - start_x;
                double offset_y = ext.miny() - start_y;
                raster target(target_ext, raster_width,raster_height);
                reproject_and_scale_raster(target, *source, prj_trans,
                                 offset_x, offset_y,
                                 sym.get_mesh_size(),
                                 filter_radius,
                                 scaling_method);
                cairo_save_restore guard(context_);
                context_.set_operator(sym.comp_op());
                context_.add_image(start_x, start_y, target.data_, sym.get_opacity());
            }
            else
            {
                double image_ratio_x = ext.width() / source->data_.width();
                double image_ratio_y = ext.height() / source->data_.height();
                double eps = 1e-5;
                if ( (std::fabs(image_ratio_x - 1.0) <= eps) &&
                     (std::fabs(image_ratio_y - 1.0) <= eps) &&
                     (std::abs(start_x) <= eps) &&
                     (std::abs(start_y) <= eps) )
                {
                    cairo_save_restore guard(context_);
                    context_.set_operator(sym.comp_op());
                    context_.add_image(start_x, start_y, source->data_, sym.get_opacity());
                }
                else
                {
                    raster target(target_ext, raster_width,raster_height);
                    if (scaling_method == SCALING_BILINEAR8)
                    {
                        scale_image_bilinear8<image_data_32>(target.data_,
                                                             source->data_,
                                                             0.0,
                                                             0.0);
                    }
                    else
                    {
                        double image_ratio_x = ext.width() / source->data_.width();
                        double image_ratio_y = ext.height() / source->data_.height();
                        scale_image_agg<image_data_32>(target.data_,
                                                       source->data_,
                                                       scaling_method,
                                                       image_ratio_x,
                                                       image_ratio_y,
                                                       0.0,
                                                       0.0,
                                                       filter_radius);
                    }
                    cairo_save_restore guard(context_);
                    context_.set_operator(sym.comp_op());
                    context_.add_image(start_x, start_y, target.data_, sym.get_opacity());
                }
            }
        }
    }
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
                     double scale_factor)
        :ctx_(ctx),
         marker_(marker),
         attributes_(attributes),
         detector_(detector),
         sym_(sym),
         bbox_(bbox),
         marker_trans_(marker_trans),
         scale_factor_(scale_factor) {}


    template <typename T>
    void add_path(T & path)
    {
        marker_placement_e placement_method = sym_.get_marker_placement();

        if (placement_method != MARKER_LINE_PLACEMENT ||
            path.type() == Point)
        {
            double x = 0;
            double y = 0;
            if (path.type() == LineString)
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

            if (sym_.get_allow_overlap() ||
                detector_.has_placement(transformed_bbox))
            {
                render_vector_marker(ctx_, pixel_position(x,y), marker_, attributes_, marker_trans_, sym_.get_opacity(), true);

                if (!sym_.get_ignore_placement())
                {
                    detector_.insert(transformed_bbox);
                }
            }
        }
        else
        {
            markers_placement<T, label_collision_detector4> placement(path, bbox_, marker_trans_, detector_,
                                                                      sym_.get_spacing() * scale_factor_,
                                                                      sym_.get_max_error(),
                                                                      sym_.get_allow_overlap());
            double x, y, angle;
            while (placement.get_point(x, y, angle, sym_.get_ignore_placement()))
            {
                agg::trans_affine matrix = marker_trans_;
                matrix.rotate(angle);
                render_vector_marker(ctx_, pixel_position(x,y),marker_, attributes_, matrix, sym_.get_opacity(), true);

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
                     double scale_factor)
        :ctx_(ctx),
         marker_(marker),
         detector_(detector),
         sym_(sym),
         bbox_(bbox),
         marker_trans_(marker_trans),
         scale_factor_(scale_factor) {}


    template <typename T>
    void add_path(T & path)
    {
        marker_placement_e placement_method = sym_.get_marker_placement();

        if (placement_method != MARKER_LINE_PLACEMENT ||
            path.type() == Point)
        {
            double x = 0;
            double y = 0;
            if (path.type() == LineString)
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

            if (sym_.get_allow_overlap() ||
                detector_.has_placement(transformed_bbox))
            {
                ctx_.add_image(matrix, *marker_, sym_.get_opacity());
                if (!sym_.get_ignore_placement())
                {
                    detector_.insert(transformed_bbox);
                }
            }
        }
        else
        {
            markers_placement<T, label_collision_detector4> placement(path, bbox_, marker_trans_, detector_,
                                                                      sym_.get_spacing() * scale_factor_,
                                                                      sym_.get_max_error(),
                                                                      sym_.get_allow_overlap());
            double x, y, angle;
            while (placement.get_point(x, y, angle, sym_.get_ignore_placement()))
            {
                coord2d center = bbox_.center();
                agg::trans_affine matrix = agg::trans_affine_translation(-center.x, -center.y);
                matrix *= marker_trans_;
                matrix *= agg::trans_affine_rotation(angle);
                matrix *= agg::trans_affine_translation(x, y);
                ctx_.add_image(matrix, *marker_, sym_.get_opacity());
            }
        }
    }

    Context & ctx_;
    ImageMarker & marker_;
    Detector & detector_;
    markers_symbolizer const& sym_;
    box2d<double> const& bbox_;
    agg::trans_affine const& marker_trans_;
    double scale_factor_;
};

}
void cairo_renderer_base::process(markers_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    typedef boost::mpl::vector<clip_line_tag,clip_poly_tag,transform_tag,smooth_tag> conv_types;

    cairo_save_restore guard(context_);
    context_.set_operator(sym.comp_op());

    agg::trans_affine tr = agg::trans_affine_scaling(scale_factor_);

    std::string filename = path_processor_type::evaluate(*sym.get_filename(), feature);

    if (!filename.empty())
    {
        boost::optional<marker_ptr> mark = mapnik::marker_cache::instance().find(filename, true);
        if (mark && *mark)
        {
            agg::trans_affine geom_tr;
            evaluate_transform(geom_tr, feature, sym.get_transform(), scale_factor_);
            box2d<double> const& bbox = (*mark)->bounding_box();
            setup_transform_scaling(tr, bbox.width(), bbox.height(), feature, sym);
            evaluate_transform(tr, feature, sym.get_image_transform());

            if ((*mark)->is_vector())
            {
                using namespace mapnik::svg;
                typedef agg::pod_bvector<path_attributes> svg_attributes_type;
                typedef detail::markers_dispatch<cairo_context, mapnik::svg_storage_type,
                                             svg_attributes_type,label_collision_detector4> dispatch_type;

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
                    svg_attributes_type attributes;
                    bool result = push_explicit_style( (*stock_vector_marker)->attributes(), attributes, sym);
                    agg::trans_affine marker_tr = agg::trans_affine_scaling(scale_factor_);
                    evaluate_transform(marker_tr, feature, sym.get_image_transform());
                    box2d<double> new_bbox = marker_ellipse.bounding_box();

                    dispatch_type dispatch(context_, marker_ellipse, result?attributes:(*stock_vector_marker)->attributes(),
                                           *detector_, sym, new_bbox, marker_tr, scale_factor_);
                    vertex_converter<box2d<double>, dispatch_type, markers_symbolizer,
                                     CoordTransform, proj_transform, agg::trans_affine, conv_types>
                        converter(query_extent_, dispatch, sym, t_, prj_trans, marker_tr, scale_factor_);

                    if (sym.clip() && feature.paths().size() > 0) // optional clip (default: true)
                    {
                        eGeomType type = feature.paths()[0].type();
                        if (type == Polygon)
                            converter.set<clip_poly_tag>();
                        // line clipping disabled due to https://github.com/mapnik/mapnik/issues/1426
                        //else if (type == LineString)
                        //    converter.template set<clip_line_tag>();
                        // don't clip if type==Point
                    }
                    converter.set<transform_tag>(); //always transform
                    if (sym.smooth() > 0.0) converter.set<smooth_tag>(); // optional smooth converter
                    apply_markers_multi(feature, converter, sym);
                }
                else
                {
                    svg_attributes_type attributes;
                    bool result = push_explicit_style( (*stock_vector_marker)->attributes(), attributes, sym);

                    dispatch_type dispatch(context_, **stock_vector_marker, result?attributes:(*stock_vector_marker)->attributes(),
                                           *detector_, sym, bbox, tr, scale_factor_);
                    vertex_converter<box2d<double>, dispatch_type, markers_symbolizer,
                                     CoordTransform, proj_transform, agg::trans_affine, conv_types>
                        converter(query_extent_, dispatch, sym, t_, prj_trans, tr, scale_factor_);

                    if (sym.clip() && feature.paths().size() > 0) // optional clip (default: true)
                    {
                        eGeomType type = feature.paths()[0].type();
                        if (type == Polygon)
                            converter.set<clip_poly_tag>();
                        // line clipping disabled due to https://github.com/mapnik/mapnik/issues/1426
                        //else if (type == LineString)
                        //    converter.template set<clip_line_tag>();
                        // don't clip if type==Point
                    }
                    converter.set<transform_tag>(); //always transform
                    if (sym.smooth() > 0.0) converter.set<smooth_tag>(); // optional smooth converter
                    apply_markers_multi(feature, converter, sym);
                }
            }
            else // raster markers
            {
                typedef detail::markers_dispatch_2<cairo_context,
                                                   mapnik::image_ptr,
                                                   label_collision_detector4> dispatch_type;
                boost::optional<mapnik::image_ptr> marker = (*mark)->get_bitmap_data();
                if ( marker )
                {
                    dispatch_type dispatch(context_, *marker,
                                           *detector_, sym, bbox, tr, scale_factor_);

                    vertex_converter<box2d<double>, dispatch_type, markers_symbolizer,
                                     CoordTransform, proj_transform, agg::trans_affine, conv_types>
                        converter(query_extent_, dispatch, sym, t_, prj_trans, tr, scale_factor_);

                    if (sym.clip() && feature.paths().size() > 0) // optional clip (default: true)
                    {
                        eGeomType type = feature.paths()[0].type();
                        if (type == Polygon)
                            converter.set<clip_poly_tag>();
                        // line clipping disabled due to https://github.com/mapnik/mapnik/issues/1426
                        //else if (type == LineString)
                        //    converter.template set<clip_line_tag>();
                        // don't clip if type==Point
                    }
                    converter.set<transform_tag>(); //always transform
                    if (sym.smooth() > 0.0) converter.set<smooth_tag>(); // optional smooth converter
                    apply_markers_multi(feature, converter, sym);
                }
            }
        }
    }
}

void cairo_renderer_base::process(text_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    text_symbolizer_helper<face_manager<freetype_engine>,
        label_collision_detector4> helper(
            sym, feature, prj_trans,
            width_, height_,
            scale_factor_,
            t_, font_manager_, *detector_, query_extent_);

    cairo_save_restore guard(context_);
    context_.set_operator(sym.comp_op());

    while (helper.next())
    {
        placements_type const& placements = helper.placements();
        for (unsigned int ii = 0; ii < placements.size(); ++ii)
        {
            context_.add_text(placements[ii], face_manager_, font_manager_, scale_factor_);
        }
    }
}

template class cairo_renderer<cairo_surface_ptr>;
template class cairo_renderer<cairo_ptr>;
}

#endif // HAVE_CAIRO
