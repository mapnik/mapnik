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

#ifndef MAPNIK_MARKERS_SYMBOLIZER_HPP
#define MAPNIK_MARKERS_SYMBOLIZER_HPP

//mapnik
#include <mapnik/symbolizer.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/color.hpp>
#include <mapnik/stroke.hpp>
#include <mapnik/enumeration.hpp>
#include <mapnik/expression.hpp>

// boost
#include <boost/optional.hpp>

namespace mapnik {

// TODO - consider merging with text_symbolizer label_placement_e
enum marker_placement_enum {
    MARKER_POINT_PLACEMENT,
    MARKER_INTERIOR_PLACEMENT,
    MARKER_LINE_PLACEMENT,
    marker_placement_enum_MAX
};

DEFINE_ENUM( marker_placement_e, marker_placement_enum );

struct MAPNIK_DECL markers_symbolizer :
        public symbolizer_with_image, public symbolizer_base
{
public:
    markers_symbolizer();
    markers_symbolizer(path_expression_ptr const& filename);
    markers_symbolizer(markers_symbolizer const& rhs);

    void set_width(expression_ptr const& width);
    expression_ptr const& get_width() const;
    void set_height(expression_ptr const& height);
    expression_ptr const& get_height() const;
    void set_ignore_placement(bool ignore_placement);
    bool get_ignore_placement() const;
    void set_allow_overlap(bool overlap);
    bool get_allow_overlap() const;
    void set_spacing(double spacing);
    double get_spacing() const;
    void set_max_error(double max_error);
    double get_max_error() const;
    void set_fill(color const& fill);
    boost::optional<color> get_fill() const;
    void set_fill_opacity(float opacity);
    boost::optional<float> get_fill_opacity() const;
    void set_stroke(stroke const& stroke);
    boost::optional<stroke> get_stroke() const;
    void set_marker_placement(marker_placement_e marker_p);
    marker_placement_e get_marker_placement() const;
private:
    expression_ptr width_;
    expression_ptr height_;
    bool ignore_placement_;
    bool allow_overlap_;
    double spacing_;
    double max_error_;
    boost::optional<color> fill_;
    boost::optional<float> fill_opacity_;
    boost::optional<float> opacity_;
    boost::optional<stroke> stroke_;
    marker_placement_e marker_p_;
};

}

#endif // MAPNIK_MARKERS_SYMBOLIZER_HPP
