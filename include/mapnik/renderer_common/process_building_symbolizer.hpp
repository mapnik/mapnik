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

namespace mapnik {

struct render_building_symbolizer
{
private:

    using size_t = std::size_t;
    using uint8_t = std::uint8_t;

    static constexpr uint8_t SEG_MOVETO_LHR = SEG_MOVETO | 0x10;
    static constexpr uint8_t SEG_MOVETO_RHR = SEG_MOVETO | 0x20;

    renderer_common const& rencom_;
    double const height_;
    bool render_back_side_ = false;
    path_type roof_vertices_{path_type::types::Polygon};

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

    bool has_transparent_fill() const
    {
        return (fill_color.alpha_ & wall_fill_color.alpha_) != 255;
    }

    bool render_back_side() const
    {
        return render_back_side_;
    }

    void render_back_side(bool value)
    {
        render_back_side_ = value;
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

    template <typename Context>
    void apply(feature_impl const& feature,
               proj_transform const& prj_trans,
               Context & painter)
    {
        auto const& geom = feature.get_geometry();

        roof_vertices_->clear();

        if (geom.is<geometry::polygon<double>>())
        {
            auto const& poly = geom.get<geometry::polygon<double>>();
            render_ground(poly, prj_trans, painter);
        }
        else if (geom.is<geometry::multi_polygon<double>>())
        {
            auto const& multi_poly = geom.get<geometry::multi_polygon<double>>();
            for (auto const& poly : multi_poly)
            {
                render_ground(poly, prj_trans, painter);
            }
        }
        if (roof_vertices_.size() > 0)
        {
            render_walls(painter);
            render_roof(painter);
        }
    }

private:

    bool is_visible(int orientation) const
    {
        return orientation > 0 || (orientation < 0 && render_back_side_);
    }

    template <typename T, typename Context>
    void render_ground(geometry::polygon<T> const& poly,
                       proj_transform const& prj_trans,
                       Context & painter)
    {
        using vertex_adapter_type = geometry::polygon_vertex_adapter<T>;
        using transform_path_type = transform_path_adapter<view_transform, vertex_adapter_type>;

        vertex_adapter_type va(poly);
        transform_path_type transformed(rencom_.t_, va, prj_trans);
        painter.set_color(base_stroke_color);
        paint_outline(transformed, painter);
    }

    template <typename Context>
    void render_roof(Context & painter) const
    {
        vertex_adapter va(roof_vertices_);
        painter.set_color(stroke_color);
        painter.stroke_preserve(va);
        painter.set_color(fill_color);
        painter.fill();
    }

    template <typename Context>
    void render_walls(Context & painter)
    {
        size_t const size = roof_vertices_.size();
        size_t strip_begin = size;
        size_t wrap_begin = size;
        size_t wrap_end = size;
        double x = 0;
        double y = 0;
        int poly_orientation = 0;
        int strip_orientation = 0;
        int wrap_orientation = 0;

        painter.set_color(wall_fill_color);

        for (size_t i = 0; i < size; ++i)
        {
            double x0 = x;
            unsigned cmd = roof_vertices_->get_vertex(i, &x, &y);

            // SEG_LINETO is the most likely command
            // SEG_CLOSE contains coordinates from previous MOVETO
            if (cmd == SEG_LINETO || cmd == SEG_CLOSE)
            {
                double dx = (x - x0) * poly_orientation;
                int wall_orientation = (dx < 0 ? -1 : dx > 0 ? 1 : 0);

                if (wall_orientation && wall_orientation != strip_orientation)
                {
                    if (wrap_orientation == 0)
                    {
                        wrap_orientation = strip_orientation;
                        wrap_end = i;
                    }
                    else
                    {
                        paint_wall(strip_orientation,
                                   strip_begin, i, i, i, painter);
                    }
                    strip_orientation = wall_orientation;
                    strip_begin = i - 1;
                }

                if (cmd == SEG_CLOSE)
                {
                    if (wrap_orientation != strip_orientation)
                    {
                        paint_wall(wrap_orientation,
                                   wrap_begin, wrap_end, i, i, painter);
                        wrap_end = wrap_begin;
                    }
                    paint_wall(strip_orientation, strip_begin, i + 1,
                               wrap_begin, wrap_end, painter);
                }
            }
            else if (cmd == SEG_MOVETO_LHR || cmd == SEG_MOVETO_RHR)
            {
                poly_orientation = (cmd == SEG_MOVETO_RHR ? -1 : 1);
                strip_orientation = 0;
                strip_begin = size;
                wrap_orientation = 0;
                wrap_begin = i;
                wrap_end = i;
                // restore plain SEG_MOVETO command for `render_roof`
                roof_vertices_->set_command(i, SEG_MOVETO);
            }
        }
    }

    template <typename VertexSource, typename Context>
    void paint_outline(VertexSource & poly, Context & painter)
    {
        double ring_begin_x, ring_begin_y;
        double x0 = 0;
        double y0 = 0;
        double x, y;
        double area = 0;
        size_t ring_begin = roof_vertices_.size();
        uint8_t ring_begin_cmd = SEG_MOVETO;

        // stroke ground outline, collect transformed points in roof_vertices_
        // and figure out exterior ring orientation along the way
        poly.rewind(0);
        while (unsigned cmd = poly.vertex(&x, &y))
        {
            if (cmd == SEG_LINETO) // the most likely command
            {
                if (ring_begin_cmd == SEG_MOVETO) // exterior ring
                {
                    double ax = x0 - ring_begin_x;
                    double ay = y0 - ring_begin_y;
                    double bx = x - ring_begin_x;
                    double by = y - ring_begin_y;
                    area += ax * by - bx * ay;
                }
                roof_vertices_.line_to(x, y - height_);
                painter.stroke_to(x, y);
                x0 = x;
                y0 = y;
            }
            else if (cmd == SEG_MOVETO)
            {
                ring_begin = roof_vertices_.size();
                ring_begin_x = x0 = x;
                ring_begin_y = y0 = y;
                roof_vertices_.move_to(x, y - height_);
                painter.stroke_from(x, y);
            }
            else if (cmd == SEG_CLOSE)
            {
                if (ring_begin_cmd == SEG_MOVETO) // exterior ring
                {
                    // because in screen coordinates `y` grows downward,
                    // negative area means counter-clockwise orientation
                    // (left-hand-rule, inside is on the left)
                    ring_begin_cmd = (area < 0 ? SEG_MOVETO_LHR : SEG_MOVETO_RHR);
                }
                if (ring_begin < roof_vertices_.size())
                {
                    // modify SEG_MOVETO command so that `render_walls` knows
                    // how front-side and back-side walls are oriented
                    roof_vertices_->set_command(ring_begin, ring_begin_cmd);
                }
                painter.stroke_close();
                painter.stroke();
                x0 = ring_begin_x;
                y0 = ring_begin_y;
                // add close path command including coordinates;
                // we utilize these when rendering walls
                roof_vertices_.push_vertex(x0, y0 - height_, SEG_CLOSE);
            }
        }
    }

    template <typename Context>
    void paint_wall(int orientation,
                    size_t begin1, size_t end1,
                    size_t begin2, size_t end2,
                    Context & painter) const
    {
        if (is_visible(orientation))
        {
            paint_wall_faces(begin1, end1, begin2, end2, painter);
        }
    }

    template <typename Context>
    void paint_wall_faces(size_t begin1, size_t end1,
                          size_t begin2, size_t end2,
                          Context & painter) const
    {
        if (begin1 >= end1)
            return;

        double x, y;
        roof_vertices_->get_vertex(begin1, &x, &y);
        painter.fill_from(x, y);

        double x0 = x;

        for (size_t i = begin1; ++i < end1; )
        {
            roof_vertices_->get_vertex(i, &x, &y);
            painter.fill_to(x, y);
        }
        for (size_t i = begin2; i < end2; ++i)
        {
            roof_vertices_->get_vertex(i, &x, &y);
            painter.fill_to(x, y);
        }

        double sx = stroke_width * (x < x0 ? -0.5 : 0.5);
        painter.fill_to(x + sx, y);
        painter.fill_to(x + sx, y + height_);
        painter.fill_to(x, y + height_);

        for (size_t i = end2; i-- > begin2; )
        {
            roof_vertices_->get_vertex(i, &x, &y);
            painter.fill_to(x, y + height_);
        }
        for (size_t i = end1; i-- > begin1; )
        {
            roof_vertices_->get_vertex(i, &x, &y);
            painter.fill_to(x, y + height_);
        }

        painter.fill_to(x - sx, y + height_);
        painter.fill_to(x - sx, y);
        painter.fill_to(x, y);
        painter.fill_close();
        painter.fill();
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
