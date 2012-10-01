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

//mapnik
#include <mapnik/symbolizer.hpp>
#include <mapnik/map.hpp>
#include <mapnik/transform_processor.hpp>

namespace mapnik {

void evaluate_transform(agg::trans_affine& tr, Feature const& feature,
                        transform_list_ptr const& trans_expr)
{
    if (trans_expr)
    {
#ifdef MAPNIK_LOG
    MAPNIK_LOG_DEBUG(transform) << "transform: evaluate "
                                << transform_processor_type::to_string(*trans_expr);
#endif
        transform_processor_type::evaluate(tr, feature, *trans_expr);
    }
}

// default ctor
symbolizer_base::symbolizer_base()
    : comp_op_(src_over),
      clip_(true),
      simplify_algorithm_value_(radial_distance),
      simplify_tolerance_value_(0.0),
      smooth_value_(0.0)
{
}

// copy ctor
symbolizer_base::symbolizer_base(symbolizer_base const& other)
    : comp_op_(other.comp_op_),
      affine_transform_(other.affine_transform_),
      clip_(other.clip_),
      simplify_algorithm_value_(other.simplify_algorithm_value_),
      simplify_tolerance_value_(other.simplify_tolerance_value_),
      smooth_value_(other.smooth_value_) {}

void symbolizer_base::set_comp_op(composite_mode_e comp_op)
{
    comp_op_ = comp_op;
}

composite_mode_e symbolizer_base::comp_op() const
{
    return comp_op_;
}

void symbolizer_base::set_transform(transform_type const& affine_transform)
{
    affine_transform_ = affine_transform;

    #ifdef MAPNIK_LOG
    MAPNIK_LOG_DEBUG(load_map) << "map_parser: set_transform: "
        << (affine_transform_
            ? transform_processor_type::to_string(*affine_transform_)
            : std::string("null"));
    #endif
}

transform_type const& symbolizer_base::get_transform() const
{
    return affine_transform_;
}

std::string symbolizer_base::get_transform_string() const
{
    if (affine_transform_)
        return transform_processor_type::to_string(*affine_transform_);
    else
        return std::string();
}

void symbolizer_base::set_clip(bool clip)
{
    clip_ = clip;
}

bool symbolizer_base::clip() const
{
    return clip_;
}

void symbolizer_base::set_simplify_algorithm(simplify_algorithm_e algo)
{
    simplify_algorithm_value_ = algo;
}

simplify_algorithm_e symbolizer_base::simplify_algorithm() const
{
    return simplify_algorithm_value_;
}

void symbolizer_base::set_simplify_tolerance(double simplify_tolerance)
{
    simplify_tolerance_value_ = simplify_tolerance;
}

double symbolizer_base::simplify_tolerance() const
{
    return simplify_tolerance_value_;
}

void symbolizer_base::set_smooth(double smooth)
{
    smooth_value_ = smooth;
}

double symbolizer_base::smooth() const
{
    return smooth_value_;
}

///////////////////////////////////////////////////////////////////////////////////////

symbolizer_with_image::symbolizer_with_image(path_expression_ptr file)
    : image_filename_( file ),
      image_opacity_(1.0f)
{
}

symbolizer_with_image::symbolizer_with_image( symbolizer_with_image const& rhs)
    : image_filename_(rhs.image_filename_),
      image_opacity_(rhs.image_opacity_),
      image_transform_(rhs.image_transform_)
{
}

path_expression_ptr const& symbolizer_with_image::get_filename() const
{
    return image_filename_;
}

void symbolizer_with_image::set_filename(path_expression_ptr const& image_filename)
{
    image_filename_ = image_filename;
}

void symbolizer_with_image::set_opacity(float opacity)
{
    image_opacity_ = opacity;
}

float symbolizer_with_image::get_opacity() const
{
    return image_opacity_;
}

void symbolizer_with_image::set_image_transform(transform_type const& tr)
{
    image_transform_ = tr;

    #ifdef MAPNIK_LOG
    MAPNIK_LOG_DEBUG(load_map) << "map_parser: set_image_transform: "
        << (image_transform_
            ? transform_processor_type::to_string(*image_transform_)
            : std::string("null"));
    #endif
}

transform_type const& symbolizer_with_image::get_image_transform() const
{
    return image_transform_;
}

std::string symbolizer_with_image::get_image_transform_string() const
{
    if (image_transform_)
        return transform_processor_type::to_string(*image_transform_);
    else
        return std::string();
}

} // end of namespace mapnik
