/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

#ifndef MAPNIK_RENDERER_COMMON_PROCESS_BUILDING_SYMBOLIZER_HPP
#define MAPNIK_RENDERER_COMMON_PROCESS_BUILDING_SYMBOLIZER_HPP

#include <mapnik/feature.hpp>
#include <mapnik/renderer_common.hpp>
#include <mapnik/symbolizer_base.hpp>
#include <mapnik/vertex_adapters.hpp>
#include <mapnik/path.hpp>
#include <mapnik/transform_path_adapter.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_conv_transform.h"
#pragma GCC diagnostic pop

namespace mapnik {

struct render_building_symbolizer
{
    using vertex_adapter_type = geometry::polygon_vertex_adapter<double>;
    using transform_path_type = transform_path_adapter<view_transform, vertex_adapter_type>;
    using roof_type = agg::conv_transform<transform_path_type>;

private:

    using size_t = std::size_t;
    using uint8_t = std::uint8_t;

    renderer_common const& rencom_;
    double const height_;

public:

    color fill_color;
    color wall_fill_color;
    color stroke_color;
    color base_stroke_color;
    double stroke_width = symbolizer_default<double, keys::stroke_width>::value();

    render_building_symbolizer(building_symbolizer const& sym,
                               feature_impl const& feature,
                               renderer_common const& rencom)
      : rencom_(rencom)
      , height_(get<double, keys::height>(sym, feature, rencom_.vars_)
                * rencom_.scale_factor_)
      , stroke_width(get<double, keys::stroke_width>(sym, feature, rencom_.vars_)
                     * rencom_.scale_factor_)
    {
        // colors are not always needed so they're not extracted here
    }

    void setup_colors(building_symbolizer const& sym,
                      feature_impl const& feature)
    {
        auto const& vars = rencom_.vars_;

        // `fill` default is defined in `symbolizer_default` specialization
        fill_color = get<color, keys::fill>(sym, feature, vars);

        // `wall-fill` defaults to dimmed `fill` [backward compatibility]
        if (auto opt = get_optional<color>(sym, keys::wall_fill, feature, vars))
        {
            wall_fill_color = *opt;
        }
        else
        {
            wall_fill_color = fill_color;
            safe_mul(wall_fill_color.red_, 0.8);
            safe_mul(wall_fill_color.green_, 0.8);
            safe_mul(wall_fill_color.blue_, 0.8);
        }

        // `stroke` defaults to dimmed `fill` [backward compatibility]
        if (auto opt = get_optional<color>(sym, keys::stroke, feature, vars))
        {
            stroke_color = *opt;
        }
        else
        {
            stroke_color = fill_color;
            safe_mul(stroke_color.red_, 0.8);
            safe_mul(stroke_color.green_, 0.8);
            safe_mul(stroke_color.blue_, 0.8);
        }

        // `base-stroke` defaults to `stroke` [backward compatibility]
        if (auto opt = get_optional<color>(sym, keys::base_stroke, feature, vars))
        {
            base_stroke_color = *opt;
        }
        else
        {
            base_stroke_color = stroke_color;
        }

        double fill_opacity = get<double, keys::fill_opacity>(sym, feature, vars);
        safe_mul(fill_color.alpha_, fill_opacity);
        safe_mul(wall_fill_color.alpha_, fill_opacity);

        double stroke_opacity = get<double, keys::stroke_opacity>(sym, feature, vars);
        safe_mul(stroke_color.alpha_, stroke_opacity);
        safe_mul(base_stroke_color.alpha_, stroke_opacity);
    }

    template <typename F1, typename F2, typename F3>
    void apply(feature_impl const& feature,
               proj_transform const& prj_trans,
               F1 face_func, F2 frame_func, F3 roof_func)
    {
        auto const& geom = feature.get_geometry();
        if (geom.is<geometry::polygon<double>>())
        {
            auto const& poly = geom.get<geometry::polygon<double>>();
            vertex_adapter_type va(poly);
            transform_path_type transformed(rencom_.t_, va, prj_trans);
            make_building(transformed, face_func, frame_func, roof_func);
        }
        else if (geom.is<geometry::multi_polygon<double>>())
        {
            auto const& multi_poly = geom.get<geometry::multi_polygon<double>>();
            for (auto const& poly : multi_poly)
            {
                vertex_adapter_type va(poly);
                transform_path_type transformed(rencom_.t_, va, prj_trans);
                make_building(transformed, face_func, frame_func, roof_func);
            }
        }
    }

private:
    template <typename F>
    void render_face(double x0, double y0, double x, double y, F & face_func, path_type & frame)
    {
        path_type faces(path_type::types::Polygon);
        faces.move_to(x0, y0);
        faces.line_to(x, y);
        faces.line_to(x, y - height_);
        faces.line_to(x0, y0 - height_);
        face_func(faces, wall_fill_color);

        frame.move_to(x0, y0);
        frame.line_to(x, y);
        frame.line_to(x, y - height_);
        frame.line_to(x0, y0 - height_);
    }

    template <typename Geom, typename F1, typename F2, typename F3>
    void make_building(Geom & poly, F1 & face_func, F2 & frame_func, F3 & roof_func)
    {
        path_type frame(path_type::types::LineString);
        double ring_begin_x, ring_begin_y;
        double x0 = 0;
        double y0 = 0;
        double x, y;
        poly.rewind(0);
        for (unsigned cm = poly.vertex(&x, &y); cm != SEG_END; cm = poly.vertex(&x, &y))
        {
            if (cm == SEG_MOVETO)
            {
                ring_begin_x = x;
                ring_begin_y = y;
            }
            else if (cm == SEG_LINETO)
            {
                render_face(x0, y0, x, y, face_func, frame);
            }
            else if (cm == SEG_CLOSE)
            {
                render_face(x0, y0, ring_begin_x, ring_begin_y, face_func, frame);
            }
            x0 = x;
            y0 = y;
        }

        frame_func(frame, wall_fill_color);

        agg::trans_affine_translation tr(0, -height_);
        roof_type roof(poly, tr);
        roof_func(roof, fill_color);
    }

    static void safe_mul(uint8_t & v, double opacity)
    {
        if (opacity <= 0)
        {
            v = 0;
        }
        else if (opacity < 1)
        {
            v = static_cast<uint8_t>(v * opacity + 0.5);
        }
    }
};

} // namespace mapnik

#endif // MAPNIK_RENDERER_COMMON_PROCESS_BUILDING_SYMBOLIZER_HPP
