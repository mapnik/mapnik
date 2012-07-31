/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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

#ifndef MAPNIK_MARKER_HELPERS_HPP
#define MAPNIK_MARKER_HELPERS_HPP

#include <mapnik/color.hpp>
#include <mapnik/markers_symbolizer.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>

// boost
#include <boost/optional.hpp>

namespace mapnik {

template <typename Attr>
bool push_explicit_style(Attr const& src, Attr & dst, markers_symbolizer const& sym)
{
    boost::optional<stroke> const& strk = sym.get_stroke();
    boost::optional<color> const& fill = sym.get_fill();
    boost::optional<float> const& opacity = sym.get_opacity();
    boost::optional<float> const& fill_opacity = sym.get_fill_opacity();
    if (strk || fill || opacity || fill_opacity)
    {
        for(unsigned i = 0; i < src.size(); ++i)
        {
            mapnik::svg::path_attributes attr = src[i];

            if (attr.stroke_flag)
            {
                // TODO - stroke attributes need to be boost::optional
                // for this to work properly
                if (strk)
                {
                    attr.stroke_width = strk->get_width();
                    color const& s_color = strk->get_color();
                    attr.stroke_color = agg::rgba(s_color.red()/255.0,
                                                  s_color.green()/255.0,
                                                  s_color.blue()/255.0,
                                                  s_color.alpha()/255.0);
                }
                if (opacity)
                {
                    attr.stroke_opacity = *opacity;
                }
                else if (strk)
                {
                    attr.stroke_opacity = strk->get_opacity();
                }
            }
            if (attr.fill_flag)
            {
                if (fill)
                {
                    color const& f_color = *fill;
                    attr.fill_color = agg::rgba(f_color.red()/255.0,
                                                f_color.green()/255.0,
                                                f_color.blue()/255.0,
                                                f_color.alpha()/255.0);
                }
                if (opacity)
                {
                    attr.fill_opacity = *opacity;
                }
                else if (fill_opacity)
                {
                    attr.fill_opacity = *fill_opacity;
                }
            }
            dst.push_back(attr);
        }
        return true;
    }
    return false;
}

template <typename T>
void setup_label_transform(agg::trans_affine & tr, box2d<double> const& bbox, mapnik::feature_impl const& feature, T const& sym)
{
    double width = 0;
    double height = 0;

    expression_ptr const& width_expr = sym.get_width();
    if (width_expr)
        width = boost::apply_visitor(evaluate<Feature,value_type>(feature), *width_expr).to_double();

    expression_ptr const& height_expr = sym.get_height();
    if (height_expr)
        height = boost::apply_visitor(evaluate<Feature,value_type>(feature), *height_expr).to_double();

    if (width > 0 && height > 0)
    {
        double sx = width/bbox.width();
        double sy = height/bbox.height();
        tr *= agg::trans_affine_scaling(sx,sy);
    }
    else if (width > 0)
    {
        double sx = width/bbox.width();
        tr *= agg::trans_affine_scaling(sx);
    }
    else if (height > 0)
    {
        double sy = height/bbox.height();
        tr *= agg::trans_affine_scaling(sy);
    }
    else
    {
        evaluate_transform(tr, feature, sym.get_image_transform());
    }
}

}

#endif //MAPNIK_MARKER_HELPERS_HPP
