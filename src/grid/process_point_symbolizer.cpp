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
#include <mapnik/grid/grid_rasterizer.hpp>
#include <mapnik/grid/grid_renderer.hpp>
#include <mapnik/grid/grid_pixfmt.hpp>
#include <mapnik/grid/grid_pixel.hpp>
#include <mapnik/grid/grid.hpp>
#include <mapnik/point_symbolizer.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>

// stl
#include <string>

namespace mapnik {

template <typename T>
void grid_renderer<T>::process(point_symbolizer const& sym,
                               mapnik::feature_impl & feature,
                               proj_transform const& prj_trans)
{
    std::string filename = path_processor_type::evaluate(*sym.get_filename(), feature);

    boost::optional<mapnik::marker_ptr> marker;
    if ( !filename.empty() )
    {
        marker = marker_cache::instance()->find(filename, true);
    }
    else
    {
        marker.reset(boost::make_shared<mapnik::marker>());
    }

    if (marker)
    {
        agg::trans_affine tr;
        evaluate_transform(tr, feature, sym.get_image_transform());

        for (unsigned i=0; i<feature.num_geometries(); ++i)
        {
            geometry_type & geom = feature.get_geometry(i);
            double x;
            double y;
            double z=0;
            if (sym.get_point_placement() == CENTROID_POINT_PLACEMENT)
                geom.label_position(&x, &y);
            else
                geom.label_interior_position(&x, &y);

            prj_trans.backward(x,y,z);
            t_.forward(&x,&y);

            double w = (*marker)->width() * (1.0/pixmap_.get_resolution());
            double h = (*marker)->height() * (1.0/pixmap_.get_resolution());

            double px = x - 0.5 * w;
            double py = y - 0.5 * h;
            box2d<double> label_ext (px, py, px + w, py + h);
            if (sym.get_allow_overlap() ||
                detector_.has_placement(label_ext))
            {
                render_marker(feature, pixmap_.get_resolution(),
                              pixel_position(px, py),
                              **marker, tr,
                              sym.get_opacity());

                if (!sym.get_ignore_placement())
                    detector_.insert(label_ext);
            }
        }
    }

}

template void grid_renderer<grid>::process(point_symbolizer const&,
                                           mapnik::feature_impl &,
                                           proj_transform const&);

}

