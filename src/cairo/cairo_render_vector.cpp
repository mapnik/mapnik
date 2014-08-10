/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

namespace mapnik
{

void render_vector_marker(cairo_context & context, pixel_position const& pos,
                          svg::svg_path_adapter & svg_path, box2d<double> const& bbox,
                          agg::pod_bvector<svg::path_attributes> const & attributes,
                          agg::trans_affine const& tr, double opacity, bool recenter)
{
    using namespace mapnik::svg;
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
        //if (transform.is_valid() && !transform.is_identity())
        if (!transform.is_identity())
        {
            double m[6];
            transform.store_to(m);
            cairo_matrix_t matrix;
            cairo_matrix_init(&matrix,m[0],m[1],m[2],m[3],m[4],m[5]);
            context.transform(matrix);
        }

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

}

#endif // HAVE_CAIRO
