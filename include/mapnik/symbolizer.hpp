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

#ifndef MAPNIK_SYMBOLIZER_HPP
#define MAPNIK_SYMBOLIZER_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/image_compositing.hpp>
#include <mapnik/transform_expression.hpp>
#include <mapnik/simplify.hpp>

// boost
#include <boost/array.hpp>
#include <boost/optional.hpp>

namespace mapnik
{

typedef transform_list_ptr transform_type;

MAPNIK_DECL void evaluate_transform(agg::trans_affine& tr, Feature const& feature,
                                    transform_type const& trans_expr);

class Map;

class MAPNIK_DECL symbolizer_base
{
public:
    symbolizer_base();
    symbolizer_base(symbolizer_base const& other);
    void set_comp_op(composite_mode_e comp_op);
    composite_mode_e comp_op() const;
    void set_transform(transform_type const& );
    transform_type const& get_transform() const;
    std::string get_transform_string() const;
    void set_clip(bool clip);
    bool clip() const;
    void set_simplify_algorithm(simplify_algorithm_e algorithm);
    simplify_algorithm_e simplify_algorithm() const;
    void set_simplify_tolerance(double simplify_tolerance);
    double simplify_tolerance() const;
    void set_smooth(double smooth);
    double smooth() const;
private:
    composite_mode_e comp_op_;
    transform_type affine_transform_;
    bool clip_;
    simplify_algorithm_e simplify_algorithm_value_;
    double simplify_tolerance_value_;
    double smooth_value_;
};


class MAPNIK_DECL symbolizer_with_image
{
public:
    path_expression_ptr const& get_filename() const;
    void set_filename(path_expression_ptr const& filename);
    void set_opacity(float opacity);
    float get_opacity() const;
    void  set_image_transform(transform_type const& tr);
    transform_type const& get_image_transform() const;
    std::string get_image_transform_string() const;
protected:
    symbolizer_with_image(path_expression_ptr filename = path_expression_ptr());
    symbolizer_with_image(symbolizer_with_image const& rhs);
    path_expression_ptr image_filename_;
    float image_opacity_;
    transform_type image_transform_;
};
}

#endif // MAPNIK_SYMBOLIZER_HPP
