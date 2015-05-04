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

#include <mapnik/feature.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/geometry_centroid.hpp>
#include <mapnik/geometry_type.hpp>
#include <mapnik/geometry_types.hpp>
#include <mapnik/vertex_adapters.hpp>
#include <mapnik/geom_util.hpp>

namespace mapnik {

template <typename F,typename RendererType>
void render_point_symbolizer(point_symbolizer const &sym,
                             mapnik::feature_impl &feature,
                             proj_transform const &prj_trans,
                             RendererType &common,
                             F render_marker)
{
    std::string filename = get<std::string,keys::file>(sym,feature, common.vars_);
    std::shared_ptr<mapnik::marker const> mark = filename.empty()
       ? std::make_shared<mapnik::marker const>(std::move(mapnik::marker_rgba8()))
       : marker_cache::instance().find(filename, true);
    
    if (!mark->is<mapnik::marker_null>())
    {
        value_double opacity = get<value_double,keys::opacity>(sym, feature, common.vars_);
        value_bool allow_overlap = get<value_bool, keys::allow_overlap>(sym, feature, common.vars_);
        value_bool ignore_placement = get<value_bool, keys::ignore_placement>(sym, feature, common.vars_);
        point_placement_enum placement= get<point_placement_enum, keys::point_placement_type>(sym, feature, common.vars_);

        box2d<double> const& bbox = mark->bounding_box();
        coord2d center = bbox.center();

        agg::trans_affine tr;
        auto image_transform = get_optional<transform_type>(sym, keys::image_transform);
        if (image_transform) evaluate_transform(tr, feature, common.vars_, *image_transform);

        agg::trans_affine_translation recenter(-center.x, -center.y);
        agg::trans_affine recenter_tr = recenter * tr;
        box2d<double> label_ext = bbox * recenter_tr * agg::trans_affine_scaling(common.scale_factor_);

        mapnik::geometry::geometry<double> const& geometry = feature.get_geometry();
        mapnik::geometry::point<double> pt;
        if (placement == CENTROID_POINT_PLACEMENT)
        {
            if (!geometry::centroid(geometry, pt)) return;
        }
        else if (mapnik::geometry::geometry_type(geometry) == mapnik::geometry::geometry_types::Polygon)
        {
            auto const& poly = mapnik::util::get<geometry::polygon<double> >(geometry);
            geometry::polygon_vertex_adapter<double> va(poly);
            if (!label::interior_position(va ,pt.x, pt.y))
                return;
        }
        double x = pt.x;
        double y = pt.y;
        double z = 0;
        prj_trans.backward(x,y,z);
        common.t_.forward(&x,&y);
        label_ext.re_center(x,y);
        if (allow_overlap ||
            common.detector_->has_placement(label_ext))
        {

            render_marker(pixel_position(x, y),
                          *mark,
                          tr,
                          opacity);

            if (!ignore_placement)
                common.detector_->insert(label_ext);
        }
    }
}

} // namespace mapnik

#endif // MAPNIK_RENDERER_COMMON_PROCESS_POINT_SYMBOLIZER_HPP
