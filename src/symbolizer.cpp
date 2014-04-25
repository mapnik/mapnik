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
#include <mapnik/attribute.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/transform_processor.hpp>

namespace mapnik {

// START FIXME - move to its own compilation unit
void evaluate_transform(agg::trans_affine& tr, feature_impl const& feature,
                        transform_list_ptr const& trans_expr, double scale_factor)
{
    if (trans_expr)
    {
#ifdef MAPNIK_LOG
    MAPNIK_LOG_DEBUG(transform) << "transform: evaluate "
                                << transform_processor_type::to_string(*trans_expr);
#endif
        transform_processor_type::evaluate(tr, feature, *trans_expr, scale_factor);
    }
}
// END FIXME

// stroke
static const char * line_cap_strings[] = {
    "butt",
    "square",
    "round",
    ""
};


IMPLEMENT_ENUM( line_cap_e, line_cap_strings )


static const char * line_join_strings[] = {
    "miter",
    "miter_revert",
    "round",
    "bevel",
    ""
};

IMPLEMENT_ENUM( line_join_e, line_join_strings )

// point symbolizer
static const char * point_placement_strings[] = {
    "centroid",
    "interior",
    ""
};

IMPLEMENT_ENUM( point_placement_e, point_placement_strings )

// line symbolizer
static const char * line_rasterizer_strings[] = {
    "full",
    "fast",
    ""
};
IMPLEMENT_ENUM( line_rasterizer_e, line_rasterizer_strings )

// markers symbolizer
static const char * marker_placement_strings[] = {
    "point",
    "interior",
    "line",
    ""
};

IMPLEMENT_ENUM( marker_placement_e, marker_placement_strings )

static const char * marker_multi_policy_strings[] = {
    "each",
    "whole",
    "largest",
    ""
};

IMPLEMENT_ENUM( marker_multi_policy_e, marker_multi_policy_strings )

// debug symbolizer
static const char * debug_symbolizer_mode_strings[] = {
    "collision",
    "vertex",
    ""
};

IMPLEMENT_ENUM( debug_symbolizer_mode_e, debug_symbolizer_mode_strings )

// polygon pattern symbolizer
static const char * pattern_alignment_strings[] = {
    "local", // feature
    "global", // map
    ""
};

IMPLEMENT_ENUM( pattern_alignment_e, pattern_alignment_strings )


// text
static const char * halo_rasterizer_strings[] = {
    "full",
    "fast",
    ""
};

IMPLEMENT_ENUM( halo_rasterizer_e, halo_rasterizer_strings )


static const char * label_placement_strings[] = {
    "point",
    "line",
    "vertex",
    "interior",
    ""
};


IMPLEMENT_ENUM( label_placement_e, label_placement_strings )

static const char * vertical_alignment_strings[] = {
    "top",
    "middle",
    "bottom",
    "auto",
    ""
};


IMPLEMENT_ENUM( vertical_alignment_e, vertical_alignment_strings )

static const char * horizontal_alignment_strings[] = {
    "left",
    "middle",
    "right",
    "auto",
    ""
};


IMPLEMENT_ENUM( horizontal_alignment_e, horizontal_alignment_strings )

static const char * justify_alignment_strings[] = {
    "left",
    "center", // not 'middle' in order to match CSS
    "right",
    "auto",
    ""
};


IMPLEMENT_ENUM( justify_alignment_e, justify_alignment_strings )

static const char * text_transform_strings[] = {
    "none",
    "uppercase",
    "lowercase",
    "capitalize",
    ""
};


IMPLEMENT_ENUM( text_transform_e, text_transform_strings )

static const char * text_upright_strings[] = {
    "auto",
    "left",
    "right",
    "left_only",
    "right_only",
    ""
};
IMPLEMENT_ENUM(text_upright_e, text_upright_strings)

} // end of namespace mapnik
