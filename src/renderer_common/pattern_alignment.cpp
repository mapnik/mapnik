/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

#include <mapnik/geometry.hpp>
#include <mapnik/view_transform.hpp>
#include <mapnik/vertex_adapters.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/transform_path_adapter.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/renderer_common.hpp>

namespace mapnik {

namespace {

struct apply_local_alignment
{
    apply_local_alignment(view_transform const& t, proj_transform const& prj_trans, double& x, double& y)
        : t_(t)
        , prj_trans_(prj_trans)
        , x_(x)
        , y_(y)
    {}

    void operator()(geometry::polygon_vertex_adapter<double>& va)
    {
        using path_type = transform_path_adapter<view_transform, decltype(va)>;
        path_type path(t_, va, prj_trans_);
        path.rewind(0);
        path.vertex(&x_, &y_);
    }

    template<typename Adapter>
    void operator()(Adapter&)
    {
        // no-op
    }

    view_transform const& t_;
    proj_transform const& prj_trans_;
    double& x_;
    double& y_;
};

} // namespace

coord<double, 2> pattern_offset(symbolizer_base const& sym,
                                mapnik::feature_impl const& feature,
                                proj_transform const& prj_trans,
                                renderer_common const& common,
                                unsigned pattern_width,
                                unsigned pattern_height)
{
    coord<double, 2> reference_position(0, 0);
    pattern_alignment_enum alignment_type = get<pattern_alignment_enum, keys::alignment>(sym, feature, common.vars_);

    if (alignment_type == LOCAL_ALIGNMENT)
    {
        apply_local_alignment apply(common.t_, prj_trans, reference_position.x, reference_position.y);
        util::apply_visitor(geometry::vertex_processor<apply_local_alignment>(apply), feature.get_geometry());
    }
    else
    {
        common.t_.forward(&reference_position.x, &reference_position.y);
    }

    return coord<double, 2>(std::fmod(-reference_position.x, pattern_width) + pattern_width,
                            std::fmod(-reference_position.y, pattern_height) + pattern_height);
}

} // namespace mapnik
