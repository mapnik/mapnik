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
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/svg/svg_renderer.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>

#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"

// stl
#include <string>

namespace mapnik {

template <typename T>
void agg_renderer<T>::render_marker(const int x, const int y, marker &marker, const agg::trans_affine & tr, double opacity)
{
    if (marker.is_vector())
    {
        typedef agg::pixfmt_rgba32_plain pixfmt;
        typedef agg::renderer_base<pixfmt> renderer_base;

        ras_ptr->reset();
        ras_ptr->gamma(agg::gamma_linear());
        agg::scanline_u8 sl;
        agg::rendering_buffer buf(pixmap_.raw_data(), width_, height_, width_ * 4);
        pixfmt pixf(buf);
        renderer_base renb(pixf);

        box2d<double> const& bbox = (*marker.get_vector_data())->bounding_box();
        double x1 = bbox.minx();
        double y1 = bbox.miny();
        double x2 = bbox.maxx();
        double y2 = bbox.maxy();

        agg::trans_affine recenter = agg::trans_affine_translation(0.5*(marker.width()-(x1+x2)),0.5*(marker.height()-(y1+y2)));

        vertex_stl_adapter<svg_path_storage> stl_storage((*marker.get_vector_data())->source());
        svg_path_adapter svg_path(stl_storage);
        svg_renderer<svg_path_adapter,
                     agg::pod_bvector<path_attributes> > svg_renderer(svg_path,
                             (*marker.get_vector_data())->attributes());
        agg::trans_affine mtx = recenter * tr;
        mtx *= agg::trans_affine_scaling(scale_factor_);
        mtx *= agg::trans_affine_translation(x, y);

        svg_renderer.render(*ras_ptr, sl, renb, mtx, opacity, bbox);


    }
    else
    {
        //int px = int(floor(x - 0.5 * marker.width()));
       // int py = int(floor(y - 0.5 * marker.height()));

        pixmap_.set_rectangle_alpha2(**marker.get_bitmap_data(), x, y, opacity);
    }
}


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

            geom.label_position(&x,&y);
            prj_trans.backward(x,y,z);
            t_.forward(&x,&y);

            int w = (*marker)->width();
            int h = (*marker)->height();

            int px = int(floor(x - 0.5 * w));
            int py = int(floor(y - 0.5 * h));
            box2d<double> label_ext (px, py, px + w, py + h);
            if (sym.get_allow_overlap() ||
                detector_.has_placement(label_ext))
            {
                agg::trans_affine tr;
                boost::array<double,6> const& m = sym.get_transform();
                tr.load_from(&m[0]);

                render_marker(px,py,**marker,tr, sym.get_opacity());

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
 
