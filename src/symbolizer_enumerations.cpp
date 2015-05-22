/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

#include <mapnik/symbolizer_enumerations.hpp>

namespace mapnik {
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
    "miter-revert",
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
    "vertex-first",
    "vertex-last",
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
    "rings",
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
    "grid",
    "alternating-grid",
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
    "adjust",
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
    "reverse",
    ""
};

IMPLEMENT_ENUM( text_transform_e, text_transform_strings )

static const char * text_upright_strings[] = {
    "auto",
    "auto-down",
    "left",
    "right",
    "left-only",
    "right-only",
    ""
};
IMPLEMENT_ENUM(text_upright_e, text_upright_strings)

static const char * direction_strings[] = {
    "left",
    "right",
    "left-only",
    "right-only",
    "auto",
    "auto-down",
    "up",
    "down",
    ""
};
IMPLEMENT_ENUM(direction_e, direction_strings)

static const char * gamma_method_strings[] = {
    "power", //agg::gamma_power
    "linear", //agg::gamma_linear
    "none", //agg::gamma_none
    "threshold", //agg::gamma_threshold
    "multiply", //agg::gamma_multiply",
    ""
};

IMPLEMENT_ENUM( gamma_method_e, gamma_method_strings )

} // namespace mapnik
