/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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
#include <mapnik/vertex_adapters.hpp>
#include <mapnik/path.hpp>
#include <mapnik/transform_path_adapter.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_conv_transform.h"
MAPNIK_DISABLE_WARNING_POP

namespace mapnik {

struct render_building_symbolizer
{
    using vertex_adapter_type = geometry::polygon_vertex_adapter<double>;
    using transform_path_type = transform_path_adapter<view_transform, vertex_adapter_type>;
    using roof_type = agg::conv_transform<transform_path_type>;

    template <typename F1, typename F2, typename F3>
    static void apply(feature_impl const& feature,
                      proj_transform const& prj_trans,
                      view_transform const& view_trans,
                      double height,
                      F1 face_func, F2 frame_func, F3 roof_func)
    {
        auto const& geom = feature.get_geometry();
        if (geom.is<geometry::polygon<double>>())
        {
            auto const& poly = geom.get<geometry::polygon<double>>();
            vertex_adapter_type va(poly);
            transform_path_type transformed(view_trans, va, prj_trans);
            make_building(transformed, height, face_func, frame_func, roof_func);
        }
        else if (geom.is<geometry::multi_polygon<double>>())
        {
            auto const& multi_poly = geom.get<geometry::multi_polygon<double>>();
            for (auto const& poly : multi_poly)
            {
                vertex_adapter_type va(poly);
                transform_path_type transformed(view_trans, va, prj_trans);
                make_building(transformed, height, face_func, frame_func, roof_func);
            }
        }
    }

private:
    template <typename F>
    static void render_face(double x0, double y0, double x, double y, double height, F const& face_func, path_type & frame)
    {
        path_type faces(path_type::types::Polygon);
        faces.move_to(x0, y0);
        faces.line_to(x, y);
        faces.line_to(x, y - height);
        faces.line_to(x0, y0 - height);
        face_func(faces);

        frame.move_to(x0, y0);
        frame.line_to(x, y);
        frame.line_to(x, y - height);
        frame.line_to(x0, y0 - height);
    }

    template <typename Geom, typename F1, typename F2, typename F3>
    static void make_building(Geom & poly, double height, F1 const& face_func, F2 const& frame_func, F3 const& roof_func)
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
                render_face(x0, y0, x, y, height, face_func, frame);
            }
            else if (cm == SEG_CLOSE)
            {
                render_face(x0, y0, ring_begin_x, ring_begin_y, height, face_func, frame);
            }
            x0 = x;
            y0 = y;
        }

        frame_func(frame);

        agg::trans_affine_translation tr(0, -height);
        roof_type roof(poly, tr);
        roof_func(roof);
    }
};

} // namespace mapnik

#endif // MAPNIK_RENDERER_COMMON_PROCESS_BUILDING_SYMBOLIZER_HPP
