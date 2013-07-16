/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
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

#if defined(HAVE_SKIA)

#include <mapnik/skia/skia_renderer.hpp>
#include <mapnik/vertex_converters.hpp>

// skia
#include <SkCanvas.h>
#include <SkPath.h>
#include <SkPaint.h>
#include <SkDashPathEffect.h>
// agg
//#include "agg_conv_clip_polyline.h"
//#include "agg_conv_clip_polygon.h"
//#include "agg_conv_smooth_poly1.h"
//#include "agg_rendering_buffer.h"
//#include "agg_pixfmt_rgba.h"
#include "agg_trans_affine.h"

namespace mapnik {

struct skia_path_adapter : private mapnik::noncopyable
{
    skia_path_adapter(SkPath & path)
        : sk_path_(path) {}

    template <typename T>
    void add_path(T & path)
    {
        vertex2d vtx(vertex2d::no_init);
        path.rewind(0);
        while ((vtx.cmd = path.vertex(&vtx.x, &vtx.y)) != SEG_END)
        {
            //std::cerr << vtx.x << "," << vtx.y << " cmd=" << vtx.cmd << std::endl;
            switch (vtx.cmd)
            {
            case SEG_MOVETO:
                sk_path_.moveTo(vtx.x, vtx.y);
            case SEG_LINETO:
                sk_path_.lineTo(vtx.x, vtx.y);
                //case SEG_CLOSE:
                //sk_path_.close();
            }
        }
    }

    SkPath & sk_path_;
};

skia_renderer::skia_renderer(Map const& map, SkCanvas & canvas, double scale_factor)
    : feature_style_processor<skia_renderer>(map,scale_factor),
      canvas_(canvas),
      width_(map.width()),
      height_(map.height()),
      t_(map.width(), map.height(), map.get_current_extent()),
      scale_factor_(scale_factor) {}

skia_renderer::~skia_renderer() {}

void skia_renderer::start_map_processing(Map const& map)
{
    MAPNIK_LOG_DEBUG(skia_renderer) << "skia_renderer_base: Start map processing bbox=" << map.get_current_extent();
    boost::optional<color> bg = map.background();
    if (bg)
    {
        canvas_.drawARGB((*bg).alpha(), (*bg).red(), (*bg).green(), (*bg).blue());
    }
}

void skia_renderer::end_map_processing(Map const& map)
{
    MAPNIK_LOG_DEBUG(skia_renderer) << "skia_renderer_base: End map processing";
}

void skia_renderer::start_layer_processing(layer const& lay, box2d<double> const& query_extent)
{
    MAPNIK_LOG_DEBUG(skia_renderer) << "skia_renderer_base: Start processing layer=" << lay.name() ;
    MAPNIK_LOG_DEBUG(skia_renderer) << "skia_renderer_base: -- datasource=" << lay.datasource().get();
    MAPNIK_LOG_DEBUG(skia_renderer) << "skia_renderer_base: -- query_extent=" << query_extent;
    query_extent_ = query_extent;
}
void skia_renderer::end_layer_processing(layer const& lay)
{
    MAPNIK_LOG_DEBUG(skia_renderer) << "skia_renderer_base: End layer processing";
}
void skia_renderer::start_style_processing(feature_type_style const& st)
{
    MAPNIK_LOG_DEBUG(skia_renderer) << "skia_renderer:start style processing";
}
void skia_renderer::end_style_processing(feature_type_style const& st)
{
    MAPNIK_LOG_DEBUG(skia_renderer) << "skia_renderer:end style processing";
}

void skia_renderer::process(line_symbolizer const& sym,
                            mapnik::feature_impl & feature,
                            proj_transform const& prj_trans)
{
     agg::trans_affine tr;
     evaluate_transform(tr, feature, sym.get_transform());

     box2d<double> clipping_extent = query_extent_;
     SkPath path;
     skia_path_adapter adapter(path);

     typedef boost::mpl::vector<clip_line_tag,transform_tag,affine_transform_tag,simplify_tag,smooth_tag> conv_types;
     vertex_converter<box2d<double>, skia_path_adapter, line_symbolizer,
                      CoordTransform, proj_transform, agg::trans_affine, conv_types>
         converter(clipping_extent,adapter,sym,t_,prj_trans,tr,scale_factor_);

     if (prj_trans.equal() && sym.clip()) converter.template set<clip_line_tag>(); //optional clip (default: true)
     converter.template set<transform_tag>(); //always transform
     converter.template set<affine_transform_tag>();
     if (sym.simplify_tolerance() > 0.0) converter.template set<simplify_tag>(); // optional simplify converter
     if (sym.smooth() > 0.0) converter.template set<smooth_tag>(); // optional smooth converter

     BOOST_FOREACH( geometry_type & geom, feature.paths())
     {
         if (geom.size() > 1)
         {
             converter.apply(geom);
         }
     }

     stroke const& strk = sym.get_stroke();
     color const& col = strk.get_color();
     SkPaint paint;
     paint.setStyle(SkPaint::kStroke_Style);
     paint.setAntiAlias(true);
     paint.setARGB(int(col.alpha() * strk.get_opacity()), col.red(), col.green(), col.blue());
     paint.setStrokeWidth(strk.get_width() * scale_factor_);

     switch (strk.get_line_join())
     {
     case MITER_JOIN:
         paint.setStrokeJoin(SkPaint::kMiter_Join);
         break;
     case MITER_REVERT_JOIN:
         break;
     case ROUND_JOIN:
         paint.setStrokeJoin(SkPaint::kRound_Join);
         break;
     case BEVEL_JOIN:
         paint.setStrokeJoin(SkPaint::kBevel_Join);
         break;
     default:
         break;
     }

     switch (strk.get_line_cap())
     {
     case BUTT_CAP:
         paint.setStrokeCap(SkPaint::kButt_Cap);
         break;
     case SQUARE_CAP:
         paint.setStrokeCap(SkPaint::kSquare_Cap);
         break;
     case ROUND_CAP:
         paint.setStrokeCap(SkPaint::kRound_Cap);
         break;
     default:
         break;
     }

     if (strk.has_dash())
     {
         std::vector<SkScalar> dash;
         for (auto p : strk.get_dash_array())
         {
             dash.push_back(p.first * scale_factor_);
             dash.push_back(p.second * scale_factor_);
         }

         SkDashPathEffect dash_effect(&dash[0], dash.size(), strk.dash_offset(), false);
         paint.setPathEffect(&dash_effect);
         canvas_.drawPath(path, paint);
     }
     else
     {
         canvas_.drawPath(path, paint);
     }
}

void skia_renderer::process(polygon_symbolizer const& sym,
                            mapnik::feature_impl & feature,
                            proj_transform const& prj_trans)
{

    agg::trans_affine tr;
    evaluate_transform(tr, feature, sym.get_transform());

    SkPath path;
    skia_path_adapter adapter(path);

    typedef boost::mpl::vector<clip_poly_tag,transform_tag,affine_transform_tag,simplify_tag,smooth_tag> conv_types;
    vertex_converter<box2d<double>, skia_path_adapter, polygon_symbolizer,
                     CoordTransform, proj_transform, agg::trans_affine, conv_types>
        converter(query_extent_, adapter ,sym,t_,prj_trans,tr,scale_factor_);

    if (prj_trans.equal() && sym.clip()) converter.template set<clip_poly_tag>(); //optional clip (default: true)
    converter.template set<transform_tag>(); //always transform
    converter.template set<affine_transform_tag>();
    if (sym.simplify_tolerance() > 0.0) converter.template set<simplify_tag>(); // optional simplify converter
    if (sym.smooth() > 0.0) converter.template set<smooth_tag>(); // optional smooth converter
    BOOST_FOREACH( geometry_type & geom, feature.paths())
    {
        if (geom.size() > 2)
        {
            converter.apply(geom);
        }
    }

    color const& fill = sym.get_fill();
    SkPaint paint;
    paint.setStyle(SkPaint::kFill_Style);
    paint.setAntiAlias(true);
    paint.setARGB(int(fill.alpha() * sym.get_opacity()), fill.red(), fill.green(), fill.blue());
    canvas_.drawPath(path, paint);
}

}

#endif // HAVE_SKIA
