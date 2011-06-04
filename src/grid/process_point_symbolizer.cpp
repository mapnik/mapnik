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
//$Id$

// mapnik
#include <mapnik/grid/grid_rasterizer.hpp>
#include <mapnik/grid/grid_renderer.hpp>
#include <mapnik/grid/grid_pixfmt.hpp>
#include <mapnik/grid/grid_pixel.hpp>
#include <mapnik/grid/grid.hpp>

#include <mapnik/marker_cache.hpp>

// stl
#include <string>

namespace mapnik {

template <typename T>
void grid_renderer<T>::process(point_symbolizer const& sym,
                              Feature const& feature,
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
        marker.reset(boost::shared_ptr<mapnik::marker> (new mapnik::marker()));
    }

    if (marker)
    {
        for (unsigned i=0; i<feature.num_geometries(); ++i)
        {
            geometry_type const& geom = feature.get_geometry(i);
            double x;
            double y;
            double z=0;
            if (sym.get_point_placement() == CENTROID_POINT_PLACEMENT)
                geom.label_position(&x, &y);
            else
                geom.label_interior_position(&x, &y);

            prj_trans.backward(x,y,z);
            t_.forward(&x,&y);

            int w = (*marker)->width() * (1.0/pixmap_.get_resolution());
            int h = (*marker)->height() * (1.0/pixmap_.get_resolution());

            int px = int(floor(x - 0.5 * w));
            int py = int(floor(y - 0.5 * h));
            box2d<double> label_ext (px, py, px + w, py + h);
            if (sym.get_allow_overlap() ||
                detector_.has_placement(label_ext))
            {
                agg::trans_affine tr;
                boost::array<double,6> const& m = sym.get_transform();
                tr.load_from(&m[0]);

                render_marker(feature,pixmap_.get_resolution(),px,py,**marker,tr, sym.get_opacity());

                if (!sym.get_ignore_placement())
                    detector_.insert(label_ext);
            }
        }
    }

}

template void grid_renderer<grid>::process(point_symbolizer const&,
                                              Feature const&,
                                              proj_transform const&);

}
 
