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

#ifndef MAPNIK_SVG_CONVERTER_HPP
#define MAPNIK_SVG_CONVERTER_HPP

// mapnik
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/svg/svg_bounding_box.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/safe_cast.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_path_storage.h"
#include "agg_conv_transform.h"
#include "agg_conv_stroke.h"
#include "agg_conv_contour.h"
#include "agg_conv_curve.h"
#include "agg_color_rgba.h"
MAPNIK_DISABLE_WARNING_POP

// stl
#include <vector>
#include <deque>
#include <stdexcept>

#include <mapbox/variant.hpp>
#include <mapnik/svg/svg_group.hpp>

namespace mapnik {
namespace svg {

template<typename VertexSource, typename AttributeSource>
class svg_converter : util::noncopyable
{
  public:

    svg_converter(VertexSource& source, group& svg_group)
        : source_(source),
          svg_group_(svg_group),
          attr_stack_(),
          svg_width_(0.0),
          svg_height_(0.0)
    {}

    void set_opacity(double opacity) { svg_group_.opacity = opacity; }

    void begin_group()
    {
        current_group_->elements.emplace_back(group {cur_attr().opacity, {}, current_group_});
        current_group_ = &current_group_->elements.back().get<group>();
    }

    void end_group()
    {
        current_group_ = current_group_->parent;
    }

    void begin_path()
    {
        std::size_t idx = source_.start_new_path();
        current_group_->elements.emplace_back(path_attributes {cur_attr(), safe_cast<unsigned>(idx)});
    }

    void end_path()
    {
        if (current_group_->elements.empty())
        {
            throw std::runtime_error("end_path : The path was not begun");
        }
        //auto& elem = current_group_->elements.back();
        //auto& attr = elem.get<path_attributes>();
        //unsigned index = attr.index;
        //attr = cur_attr();
        //attr.index = index;
    }

    void move_to(double x, double y, bool rel = false) // M, m
    {
        if (rel)
            source_.rel_to_abs(&x, &y);
        source_.move_to(x, y);
    }

    void line_to(double x, double y, bool rel = false) // L, l
    {
        if (rel)
            source_.rel_to_abs(&x, &y);
        source_.line_to(x, y);
    }

    void hline_to(double x, bool rel = false) // H, h
    {
        double x2 = 0.0;
        double y2 = 0.0;
        if (source_.total_vertices())
        {
            source_.vertex(safe_cast<unsigned>(source_.total_vertices() - 1), &x2, &y2);
            if (rel)
                x += x2;
            source_.line_to(x, y2);
        }
    }

    void vline_to(double y, bool rel = false) // V, v
    {
        double x2 = 0.0;
        double y2 = 0.0;
        if (source_.total_vertices())
        {
            source_.vertex(safe_cast<unsigned>(source_.total_vertices() - 1), &x2, &y2);
            if (rel)
                y += y2;
            source_.line_to(x2, y);
        }
    }
    void curve3(double x1,
                double y1, // Q, q
                double x,
                double y,
                bool rel = false)
    {
        if (rel)
        {
            source_.rel_to_abs(&x1, &y1);
            source_.rel_to_abs(&x, &y);
        }
        source_.curve3(x1, y1, x, y);
    }

    void curve3(double x, double y, bool rel = false) // T, t
    {
        if (rel)
        {
            source_.curve3_rel(x, y);
        }
        else
        {
            source_.curve3(x, y);
        }
    }

    void curve4(double x1,
                double y1, // C, c
                double x2,
                double y2,
                double x,
                double y,
                bool rel = false)
    {
        if (rel)
        {
            source_.rel_to_abs(&x1, &y1);
            source_.rel_to_abs(&x2, &y2);
            source_.rel_to_abs(&x, &y);
        }
        source_.curve4(x1, y1, x2, y2, x, y);
    }

    void curve4(double x2,
                double y2, // S, s
                double x,
                double y,
                bool rel = false)
    {
        if (rel)
        {
            source_.curve4_rel(x2, y2, x, y);
        }
        else
        {
            source_.curve4(x2, y2, x, y);
        }
    }

    void arc_to(double rx,
                double ry, // A, a
                double angle,
                bool large_arc_flag,
                bool sweep_flag,
                double x,
                double y,
                bool rel = false)
    {
        if (rel)
        {
            source_.arc_rel(rx, ry, angle, large_arc_flag, sweep_flag, x, y);
        }
        else
        {
            source_.arc_to(rx, ry, angle, large_arc_flag, sweep_flag, x, y);
        }
    }

    void close_subpath() // Z, z
    {
        source_.end_poly(agg::path_flags_close);
    }

    void push_attr()
    {
        if (attr_stack_.empty())
            attr_stack_.push_back(path_attributes());
        else
            attr_stack_.push_back(attr_stack_.back());
    }

    void pop_attr()
    {
        if (attr_stack_.empty())
        {
            throw std::runtime_error("pop_attr : Attribute stack is empty");
        }
        attr_stack_.pop_back();
    }

    // Attribute setting functions.
    void fill(agg::rgba8 const& f)
    {
        path_attributes& attr = cur_attr();
        double a = attr.fill_color.opacity();
        attr.fill_color = f;
        attr.fill_color.opacity(a * f.opacity());
        attr.fill_flag = true;
    }

    void add_fill_gradient(mapnik::gradient const& grad)
    {
        path_attributes& attr = cur_attr();
        attr.fill_gradient = grad;
    }

    void add_stroke_gradient(mapnik::gradient const& grad)
    {
        path_attributes& attr = cur_attr();
        attr.stroke_gradient = grad;
    }

    void stroke(agg::rgba8 const& s)
    {
        path_attributes& attr = cur_attr();
        double a = attr.stroke_color.opacity();
        attr.stroke_color = s;
        attr.stroke_color.opacity(a * s.opacity());
        attr.stroke_flag = true;
    }

    void dash_array(dash_array&& dash)
    {
        path_attributes& attr = cur_attr();
        attr.dash = std::move(dash);
    }

    void dash_offset(double offset) { cur_attr().dash_offset = offset; }

    void even_odd(bool flag) { cur_attr().even_odd_flag = flag; }

    void visibility(bool flag) { cur_attr().visibility_flag = flag; }

    bool visibility() { return cur_attr().visibility_flag; }

    void display(bool flag) { cur_attr().display_flag = flag; }

    bool display() { return cur_attr().display_flag; }

    void stroke_width(double w) { cur_attr().stroke_width = w; }

    void fill_none()
    {
        cur_attr().fill_none = true;
        cur_attr().fill_flag = false;
    }

    void stroke_none()
    {
        cur_attr().stroke_none = true;
        cur_attr().stroke_flag = false;
    }

    void fill_opacity(double op) { cur_attr().fill_opacity = op; }

    void stroke_opacity(double op) { cur_attr().stroke_opacity = op; }

    void opacity(double op) { cur_attr().opacity = op; }

    void line_join(agg::line_join_e join) { cur_attr().line_join = join; }

    void line_cap(agg::line_cap_e cap) { cur_attr().line_cap = cap; }
    void miter_limit(double ml) { cur_attr().miter_limit = ml; }

    // Make all polygons CCW-oriented
    void arrange_orientations() { source_.arrange_orientations_all_paths(agg::path_flags_ccw); }

    void bounding_rect(double* x1, double* y1, double* x2, double* y2)
    {
        agg::conv_transform<mapnik::svg::svg_path_adapter> path(source_, transform_);
        bounding_box(path, svg_group_, *x1, *y1, *x2, *y2);
    }

    void set_dimensions(double w, double h)
    {
        svg_width_ = w;
        svg_height_ = h;
    }

    double width() const { return svg_width_; }

    double height() const { return svg_height_; }

    VertexSource& storage() { return source_; }

    agg::trans_affine& transform() { return cur_attr().transform; }

    path_attributes& cur_attr()
    {
        if (attr_stack_.empty())
        {
            throw std::runtime_error("cur_attr : Attribute stack is empty");
        }
        return attr_stack_.back();
    }

  private:

    VertexSource& source_;
    group& svg_group_;
    group* current_group_ = &svg_group_;
    AttributeSource attr_stack_;
    agg::trans_affine transform_;
    double svg_width_;
    double svg_height_;
};

using svg_converter_type = svg_converter<svg_path_adapter, std::deque<path_attributes>>;

} // namespace svg
} // namespace mapnik

#endif // MAPNIK_SVG_CONVERTER_HPP
