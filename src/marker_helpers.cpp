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

// mapnik
#include <mapnik/marker_helpers.hpp>

namespace mapnik {

void build_ellipse(symbolizer_base const& sym, mapnik::feature_impl & feature, attributes const& vars, svg_storage_type & marker_ellipse, svg::svg_path_adapter & svg_path)
{
    double width = 0.0;
    double height = 0.0;
    if (has_key(sym,keys::width) && has_key(sym,keys::height))
    {
        width = get<double>(sym, keys::width, feature, vars, 0.0);
        height = get<double>(sym, keys::height, feature, vars, 0.0);
    }
    else if (has_key(sym,keys::width))
    {
        width = height = get<double>(sym, keys::width, feature, vars, 0.0);
    }
    else if (has_key(sym,keys::height))
    {
        width = height = get<double>(sym, keys::height, feature, vars, 0.0);
    }
    svg::svg_converter_type styled_svg(svg_path, marker_ellipse.attributes());
    styled_svg.push_attr();
    styled_svg.begin_path();
    agg::ellipse c(0, 0, width/2.0, height/2.0);
    styled_svg.storage().concat_path(c);
    styled_svg.end_path();
    styled_svg.pop_attr();
    double lox,loy,hix,hiy;
    styled_svg.bounding_rect(&lox, &loy, &hix, &hiy);
    styled_svg.set_dimensions(width,height);
    marker_ellipse.set_dimensions(width,height);
    marker_ellipse.set_bounding_box(lox,loy,hix,hiy);
}

bool push_explicit_style(svg_attribute_type const& src,
                         svg_attribute_type & dst,
                         symbolizer_base const& sym,
                         feature_impl & feature,
                         attributes const& vars)
{
    auto fill_color = get_optional<color>(sym, keys::fill, feature, vars);
    auto fill_opacity = get_optional<double>(sym, keys::fill_opacity, feature, vars);
    auto stroke_color = get_optional<color>(sym, keys::stroke, feature, vars);
    auto stroke_width = get_optional<double>(sym, keys::stroke_width, feature, vars);
    auto stroke_opacity = get_optional<double>(sym, keys::stroke_opacity, feature, vars);
    if (fill_color ||
        fill_opacity ||
        stroke_color ||
        stroke_width ||
        stroke_opacity)
    {
        bool success = false;
        for(unsigned i = 0; i < src.size(); ++i)
        {
            success = true;
            dst.push_back(src[i]);
            mapnik::svg::path_attributes & attr = dst.last();
            if (attr.stroke_flag)
            {
                if (stroke_width)
                {
                    attr.stroke_width = *stroke_width;
                }
                if (stroke_color)
                {
                    color const& s_color = *stroke_color;
                    attr.stroke_color = agg::rgba(s_color.red()/255.0,
                                                  s_color.green()/255.0,
                                                  s_color.blue()/255.0,
                                                  s_color.alpha()/255.0);
                }
                if (stroke_opacity)
                {
                    attr.stroke_opacity = *stroke_opacity;
                }
            }
            if (attr.fill_flag)
            {
                if (fill_color)
                {
                    color const& f_color = *fill_color;
                    attr.fill_color = agg::rgba(f_color.red()/255.0,
                                                f_color.green()/255.0,
                                                f_color.blue()/255.0,
                                                f_color.alpha()/255.0);
                }
                if (fill_opacity)
                {
                    attr.fill_opacity = *fill_opacity;
                }
            }
        }
        return success;
    }
    return false;
}

void setup_transform_scaling(agg::trans_affine & tr,
                             double svg_width,
                             double svg_height,
                             mapnik::feature_impl & feature,
                             attributes const& vars,
                             symbolizer_base const& sym)
{
    double width = get<double>(sym, keys::width, feature, vars, 0.0);
    double height = get<double>(sym, keys::height, feature, vars, 0.0);
    if (width > 0 && height > 0)
    {
        double sx = width/svg_width;
        double sy = height/svg_height;
        tr *= agg::trans_affine_scaling(sx,sy);
    }
    else if (width > 0)
    {
        double sx = width/svg_width;
        tr *= agg::trans_affine_scaling(sx);
    }
    else if (height > 0)
    {
        double sy = height/svg_height;
        tr *= agg::trans_affine_scaling(sy);
    }
}


} // end namespace mapnik
