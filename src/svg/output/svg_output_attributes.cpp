
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

// mapnik
#include <mapnik/svg/output/svg_output_attributes.hpp>

namespace mapnik { namespace svg {

    // path_output_attributes

    void path_output_attributes::set_fill_color(color const& fill_color)
    {
        fill_color_ = fill_color.to_hex_string();
    }

    void path_output_attributes::set_fill_opacity(const double fill_opacity)
    {
        fill_opacity_ = fill_opacity;
    }

    void path_output_attributes::set_stroke_color(color const& stroke_color)
    {
        stroke_color_ = stroke_color.to_hex_string();
    }

    void path_output_attributes::set_stroke_opacity(const double stroke_opacity)
    {
        stroke_opacity_ = stroke_opacity;
    }

    void path_output_attributes::set_stroke_width(const double stroke_width)
    {
        stroke_width_ = stroke_width;
    }

    void path_output_attributes::set_stroke_linecap(const line_cap_e stroke_linecap)
    {
        switch(stroke_linecap)
        {
        case BUTT_CAP:
            stroke_linecap_ = "butt";
            break;
        case SQUARE_CAP:
            stroke_linecap_ = "square";
            break;
        case ROUND_CAP:
            stroke_linecap_ = "round";
            break;
        default:
            stroke_linecap_ = "butt";
        }
    }

    void path_output_attributes::set_stroke_linejoin(const line_join_e stroke_linejoin)
    {
        switch(stroke_linejoin)
        {
        case MITER_JOIN:
            stroke_linejoin_ = "miter";
            break;
        case MITER_REVERT_JOIN:
            stroke_linejoin_ = "miter";
            break;
        case ROUND_JOIN:
            stroke_linejoin_ = "round";
            break;
        case BEVEL_JOIN:
            stroke_linejoin_ = "bevel";
            break;
        default:
            stroke_linejoin_ = "miter";
        }
    }

    void path_output_attributes::set_stroke_dasharray(const dash_array stroke_dasharray)
    {
        stroke_dasharray_ = stroke_dasharray;
    }

    void path_output_attributes::set_stroke_dashoffset(const double stroke_dashoffset)
    {
        stroke_dashoffset_ = stroke_dashoffset;
    }

    const std::string path_output_attributes::fill_color() const
    {
        return fill_color_;
    }

    const double path_output_attributes::fill_opacity() const
    {
        return fill_opacity_;
    }

    const std::string path_output_attributes::stroke_color() const
    {
        return stroke_color_;
    }

    const double path_output_attributes::stroke_opacity() const
    {
        return stroke_opacity_;
    }

    const double path_output_attributes::stroke_width() const
    {
        return stroke_width_;
    }

    const std::string path_output_attributes::stroke_linecap() const
    {
        return stroke_linecap_;
    }

    const std::string path_output_attributes::stroke_linejoin() const
    {
        return stroke_linejoin_;
    }

    const dash_array path_output_attributes::stroke_dasharray() const
    {
        return stroke_dasharray_;
    }

    const double path_output_attributes::stroke_dashoffset() const
    {
        return stroke_dashoffset_;
    }

    void path_output_attributes::reset()
    {
        fill_color_ = "none";
        fill_opacity_ = 1.0;
        stroke_color_ = "none";
        stroke_opacity_ = 1.0;
        stroke_width_ = 0.0;
        stroke_linecap_ = "butt";
        stroke_linejoin_ = "miter";
        stroke_dasharray_.clear();
        stroke_dashoffset_ = 0.0;
    }

    // rect_output_attributes

    void rect_output_attributes::set_x(const int x)
    {
        x_ = x;
    }

    void rect_output_attributes::set_y(const int y)
    {
        y_ = y;
    }

    void rect_output_attributes::set_width(const unsigned width)
    {
        width_ = width;
    }

    void rect_output_attributes::set_height(const unsigned height)
    {
        height_ = height;
    }

    void rect_output_attributes::set_fill_color(color const& fill_color)
    {
        fill_color_ = fill_color.to_hex_string();
    }

    const int rect_output_attributes::x() const
    {
        return x_;
    }

    const int rect_output_attributes::y() const
    {
        return y_;
    }

    const int rect_output_attributes::width() const
    {
        return width_;
    }

    const int rect_output_attributes::height() const
    {
        return height_;
    }

    const std::string rect_output_attributes::fill_color() const
    {
        return fill_color_;
    }

    void rect_output_attributes::reset()
    {
        x_ = 0;
        y_ = 0;
        width_ = 400;
        height_ = 400;
        fill_color_ = "#000000";
    }

    // rect_output_attributes

    const double root_output_attributes::SVG_VERSION = 1.1;
    const std::string root_output_attributes::SVG_NAMESPACE_URL = "http://www.w3.org/2000/svg";

    root_output_attributes::root_output_attributes()
        : width_(400),
          height_(400),
          svg_version_(SVG_VERSION),
          svg_namespace_url_(SVG_NAMESPACE_URL)
    {}

    root_output_attributes::root_output_attributes(const unsigned width, const unsigned height)
        : width_(width),
          height_(height),
          svg_version_(SVG_VERSION),
          svg_namespace_url_(SVG_NAMESPACE_URL)
    {}

    void root_output_attributes::set_width(const unsigned width)
    {
        width_ = width;
    }

    void root_output_attributes::set_height(const unsigned height)
    {
        height_ = height;
    }

    void root_output_attributes::set_svg_version(const double svg_version)
    {
        svg_version_ = svg_version;
    }

    void root_output_attributes::set_svg_namespace_url(std::string const& svg_namespace_url)
    {
        svg_namespace_url_ = svg_namespace_url;
    }

    const unsigned root_output_attributes::width() const
    {
        return width_;
    }

    const unsigned root_output_attributes::height() const
    {
        return height_;
    }

    const double root_output_attributes::svg_version() const
    {
        return svg_version_;
    }

    const std::string root_output_attributes::svg_namespace_url() const
    {
        return svg_namespace_url_;
    }

    void root_output_attributes::reset()
    {
        width_ = 400;
        height_ = 400;
        svg_version_ = SVG_VERSION;
        svg_namespace_url_ = SVG_NAMESPACE_URL;
    }
    }}
