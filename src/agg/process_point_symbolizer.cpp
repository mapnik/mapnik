/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2010 Artem Pavlenko
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
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/metawriter.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/expression_evaluator.hpp>

#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"

// stl
#include <string>

// boost
#include <boost/make_shared.hpp>

namespace mapnik {

template <typename T>
void agg_renderer<T>::process(point_symbolizer const& sym,
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
        marker.reset(boost::make_shared<mapnik::marker>());
    }

    if (marker)
    {
        int w = (*marker)->width();
        int h = (*marker)->height();    
        agg::trans_affine tr;
        boost::array<double,6> const& m = sym.get_transform();
        tr.load_from(&m[0]);
        double px0 = - 0.5 * w;
        double py0 = - 0.5 * h;
        double px1 = 0.5 * w;
        double py1 = 0.5 * h;
        double px2 = px1;
        double py2 = py0;
        double px3 = px0;
        double py3 = py1;
        tr.transform(&px0,&py0);
        tr.transform(&px1,&py1);
        tr.transform(&px2,&py2);
        tr.transform(&px3,&py3);
        box2d<double> label_ext (px0, py0, px1, py1);
        label_ext.expand_to_include(px2, py2);
        label_ext.expand_to_include(px3, py3);
        
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
            label_ext.re_center(x,y);
            
            if (sym.get_allow_overlap() ||
                detector_.has_placement(label_ext))
            {
                
                render_marker(floor(x - 0.5 * w),floor(y - 0.5 * h) ,**marker,tr, sym.get_opacity());

                if (!sym.get_ignore_placement())
                    detector_.insert(label_ext);
                metawriter_with_properties writer = sym.get_metawriter();
                if (writer.first) writer.first->add_box(label_ext, feature, t_, writer.second);
            }
        }
    }

}

template void agg_renderer<image_32>::process(point_symbolizer const&,
                                              Feature const&,
                                              proj_transform const&);

}
 
