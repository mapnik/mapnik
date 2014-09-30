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

#ifndef MAPNIK_RENDERER_COMMON_PROCESS_POINT_SYMBOLIZER_HPP
#define MAPNIK_RENDERER_COMMON_PROCESS_POINT_SYMBOLIZER_HPP

#include <mapnik/geom_util.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/label_collision_detector.hpp>

namespace mapnik {

template <typename F,typename RendererType>
void render_point_symbolizer(point_symbolizer const &sym,
                             mapnik::feature_impl &feature,
                             proj_transform const &prj_trans,
                             RendererType &common,
                             F render_marker)
{
    std::string filename = get<std::string>(sym, keys::file, feature, common.vars_);
    boost::optional<mapnik::marker_ptr> marker;
    if (!filename.empty())
    {
        marker = marker_cache::instance().find(filename, true);
    }
    else
    {
        marker.reset(std::make_shared<mapnik::marker>());
    }
    if (marker)
    {
        double opacity = get<double>(sym,keys::opacity,feature, common.vars_, 1.0);
        bool allow_overlap = get<bool>(sym, keys::allow_overlap, feature, common.vars_, false);
        bool ignore_placement = get<bool>(sym, keys::ignore_placement, feature, common.vars_, false);
        point_placement_enum placement= get<point_placement_enum>(sym, keys::point_placement_type, feature, common.vars_, CENTROID_POINT_PLACEMENT);

        box2d<double> const& bbox = (*marker)->bounding_box();
        coord2d center = bbox.center();

        agg::trans_affine tr;
        auto image_transform = get_optional<transform_type>(sym, keys::image_transform);
        if (image_transform) evaluate_transform(tr, feature, common.vars_, *image_transform);

        agg::trans_affine_translation recenter(-center.x, -center.y);
        agg::trans_affine recenter_tr = recenter * tr;
        box2d<double> label_ext = bbox * recenter_tr * agg::trans_affine_scaling(common.scale_factor_);

        for (std::size_t i=0; i<feature.num_geometries(); ++i)
        {
            geometry_type const& geom = feature.get_geometry(i);
            double x;
            double y;
            double z=0;
            if (placement == CENTROID_POINT_PLACEMENT)
            {
                if (!label::centroid(geom, x, y))
                    return;
            }
            else
            {
                if (!label::interior_position(geom ,x, y))
                    return;
            }

            prj_trans.backward(x,y,z);
            common.t_.forward(&x,&y);
            label_ext.re_center(x,y);
            if (allow_overlap ||
                common.detector_->has_placement(label_ext))
            {

                render_marker(pixel_position(x, y),
                              **marker,
                              tr,
                              opacity);

                if (!ignore_placement)
                    common.detector_->insert(label_ext);
            }
        }
    }
}

} // namespace mapnik

#endif /* MAPNIK_RENDERER_COMMON_PROCESS_POINT_SYMBOLIZER_HPP */
