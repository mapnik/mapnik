/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
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
#include <mapnik/cairo/cairo_renderer.hpp>
#include <mapnik/cairo/cairo_render_vector.hpp>
#include <mapnik/cairo/cairo_context.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>

namespace mapnik {

namespace {
struct group_renderer
{
    group_renderer(agg::trans_affine const& transform,
                   cairo_context& context,
                   svg_path_adapter& svg_path,
                   box2d<double> const& bbox)
        : transform_(transform)
        , context_(context)
        , svg_path_(svg_path)
        , bbox_(bbox)
    {}

    void operator()(svg::group const& g) const
    {
        double opacity = g.opacity;
        if (opacity < 1.0)
        {
            context_.push_group();
            for (auto const& elem : g.elements)
            {
                mapbox::util::apply_visitor(group_renderer(transform_, context_, svg_path_, bbox_), elem);
            }
            context_.pop_group();
            context_.paint(opacity);
        }
        else
        {
            for (auto const& elem : g.elements)
            {
                mapbox::util::apply_visitor(group_renderer(transform_, context_, svg_path_, bbox_), elem);
            }
        }
    }

    void operator()(svg::path_attributes const& attr) const
    {
        if (!attr.visibility_flag)
            return;
        cairo_save_restore guard(context_);
        agg::trans_affine transform = attr.transform;
        transform *= transform_;

        // TODO - this 'is_valid' check is not used in the AGG renderer and also
        // appears to lead to bogus results with
        // tests/data/good_maps/markers_symbolizer_lines_file.xml
        // if (transform.is_valid() && !transform.is_identity())
        if (!transform.is_identity())
        {
            double m[6];
            transform.store_to(m);
            cairo_matrix_t matrix;
            cairo_matrix_init(&matrix, m[0], m[1], m[2], m[3], m[4], m[5]);
            context_.transform(matrix);
        }

        if (attr.fill_flag || attr.fill_gradient.get_gradient_type() != NO_GRADIENT)
        {
            context_.add_agg_path(svg_path_, attr.index);
            if (attr.even_odd_flag)
            {
                context_.set_fill_rule(CAIRO_FILL_RULE_EVEN_ODD);
            }
            else
            {
                context_.set_fill_rule(CAIRO_FILL_RULE_WINDING);
            }
            if (attr.fill_gradient.get_gradient_type() != NO_GRADIENT)
            {
                cairo_gradient g(attr.fill_gradient, attr.fill_opacity * attr.opacity);

                context_.set_gradient(g, bbox_);
                context_.fill();
            }
            else if (attr.fill_flag)
            {
                double fill_opacity = attr.fill_opacity * attr.opacity * attr.fill_color.opacity();
                context_.set_color(attr.fill_color.r / 255.0,
                                   attr.fill_color.g / 255.0,
                                   attr.fill_color.b / 255.0,
                                   fill_opacity);
                context_.fill();
            }
        }

        if (attr.stroke_gradient.get_gradient_type() != NO_GRADIENT || attr.stroke_flag)
        {
            context_.add_agg_path(svg_path_, attr.index);
            if (attr.stroke_gradient.get_gradient_type() != NO_GRADIENT)
            {
                context_.set_line_width(attr.stroke_width);
                context_.set_line_cap(line_cap_enum(attr.line_cap));
                context_.set_line_join(line_join_enum(attr.line_join));
                context_.set_miter_limit(attr.miter_limit);
                cairo_gradient g(attr.stroke_gradient, attr.fill_opacity * attr.opacity);
                context_.set_gradient(g, bbox_);
                context_.stroke();
            }
            else if (attr.stroke_flag)
            {
                double stroke_opacity = attr.stroke_opacity * attr.opacity * attr.stroke_color.opacity();
                context_.set_color(attr.stroke_color.r / 255.0,
                                   attr.stroke_color.g / 255.0,
                                   attr.stroke_color.b / 255.0,
                                   stroke_opacity);
                context_.set_line_width(attr.stroke_width);
                context_.set_line_cap(line_cap_enum(attr.line_cap));
                context_.set_line_join(line_join_enum(attr.line_join));
                context_.set_miter_limit(attr.miter_limit);
                context_.stroke();
            }
        }
    }
    agg::trans_affine const& transform_;
    cairo_context& context_;
    svg_path_adapter& svg_path_;
    box2d<double> const& bbox_;
};

} // namespace

void render_vector_marker(cairo_context& context,
                          svg_path_adapter& svg_path,
                          svg::group const& group_attrs,
                          box2d<double> const& bbox,
                          agg::trans_affine const& tr,
                          double opacity)
{
    double adjusted_opacity = opacity * group_attrs.opacity; // adjust top level opacity
    if (adjusted_opacity < 1.0)
    {
        context.push_group();
        for (auto const& elem : group_attrs.elements)
        {
            mapbox::util::apply_visitor(group_renderer(tr, context, svg_path, bbox), elem);
        }
        context.pop_group();
        context.paint(opacity);
    }
    else
    {
        for (auto const& elem : group_attrs.elements)
        {
            mapbox::util::apply_visitor(group_renderer(tr, context, svg_path, bbox), elem);
        }
    }
}

} // namespace mapnik

#endif // HAVE_CAIRO
