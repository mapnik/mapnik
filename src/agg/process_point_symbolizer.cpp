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
#include <mapnik/feature.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/image_util.hpp>

#include <mapnik/geom_util.hpp>
#include <mapnik/point_symbolizer.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/pixel_position.hpp>


// agg
#include "agg_trans_affine.h"

// stl
#include <string>

// boost
#include <boost/make_shared.hpp>

namespace mapnik {

template <typename T>
void agg_renderer<T>::process(point_symbolizer const& sym,
                              mapnik::feature_impl & feature,
                              proj_transform const& prj_trans)
{
    std::string filename = path_processor_type::evaluate(*sym.get_filename(), feature);

    boost::optional<mapnik::marker_ptr> marker;
    if ( !filename.empty() )
    {
        marker = marker_cache::instance().find(filename, true);
    }
    else
    {
        marker.reset(boost::make_shared<mapnik::marker>());
    }

    if (marker)
    {
        box2d<double> const& bbox = (*marker)->bounding_box();
        coord2d center = bbox.center();

        agg::trans_affine tr;
        evaluate_transform(tr, feature, sym.get_image_transform());
        agg::trans_affine_translation recenter(-center.x, -center.y);
        agg::trans_affine recenter_tr = recenter * tr;
        box2d<double> label_ext = bbox * recenter_tr * agg::trans_affine_scaling(scale_factor_);

        for (unsigned i=0; i<feature.num_geometries(); ++i)
        {
            geometry_type const& geom = feature.get_geometry(i);
            double x;
            double y;
            double z=0;
            if (sym.get_point_placement() == CENTROID_POINT_PLACEMENT)
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
            t_.forward(&x,&y);
            label_ext.re_center(x,y);
            if (sym.get_allow_overlap() ||
                detector_->has_placement(label_ext))
            {

                render_marker(pixel_position(x, y),
                              **marker,
                              tr,
                              sym.get_opacity(),
                              sym.comp_op());

                if (!sym.get_ignore_placement())
                    detector_->insert(label_ext);
            }
        }
    }

}

template void agg_renderer<image_32>::process(point_symbolizer const&,
                                              mapnik::feature_impl &,
                                              proj_transform const&);

}
