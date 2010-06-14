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

#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/image_cache.hpp>
#include <mapnik/svg/marker_cache.hpp>
#include <mapnik/svg/svg_renderer.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/markers_placement.hpp>
#include <mapnik/arrow.hpp>

#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"

namespace mapnik {

template <typename T>
void agg_renderer<T>::process(markers_symbolizer const& sym,
                              Feature const& feature,
                              proj_transform const& prj_trans)
{
    typedef coord_transform2<CoordTransform,geometry2d> path_type;
    typedef agg::pixfmt_rgba32 pixfmt;
    typedef agg::renderer_base<pixfmt> renderer_base;
    typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_solid;
    
    ras_ptr->reset();
    ras_ptr->gamma(agg::gamma_linear());
    agg::scanline_u8 sl;
    agg::rendering_buffer buf(pixmap_.raw_data(), width_, height_, width_ * 4);
    pixfmt pixf(buf);
    renderer_base renb(pixf);
    renderer_solid ren(renb);
    agg::trans_affine tr;
    boost::array<double,6> const& m = sym.get_transform();
    tr.load_from(&m[0]);
    tr = agg::trans_affine_scaling(scale_factor_) * tr;
    std::string filename = path_processor_type::evaluate(*sym.get_filename(), feature);
    
    if (!filename.empty())
    {
        boost::optional<path_ptr> marker = mapnik::marker_cache::instance()->find(filename, true);
        if (marker && *marker)
        {
            box2d<double> const& bbox = (*marker)->bounding_box();
            double x1 = bbox.minx();
            double y1 = bbox.miny();
            double x2 = bbox.maxx();
            double y2 = bbox.maxy();
            tr.transform(&x1,&y1);
            tr.transform(&x2,&y2);
            box2d<double> extent(x1,y1,x2,y2);
            using namespace mapnik::svg;
            vertex_stl_adapter<svg_path_storage> stl_storage((*marker)->source());
            svg_path_adapter svg_path(stl_storage);
            svg_renderer<svg_path_adapter, 
                         agg::pod_bvector<path_attributes> > svg_renderer(svg_path,(*marker)->attributes());

            for (unsigned i=0; i<feature.num_geometries(); ++i)
            {
                geometry2d const& geom = feature.get_geometry(i);
                if (geom.num_points() <= 1) continue;
                
                path_type path(t_,geom,prj_trans);
                markers_placement<path_type, label_collision_detector4> placement(path, extent, detector_, 
                                                                                  sym.get_spacing(), 
                                                                                  sym.get_max_error(), 
                                                                                  sym.get_allow_overlap());        
                double x, y, angle;
            
                while (placement.get_point(&x, &y, &angle))
                {
                    agg::trans_affine matrix = tr *agg::trans_affine_rotation(angle) * agg::trans_affine_translation(x, y);
                    svg_renderer.render(*ras_ptr, sl, ren, matrix, renb.clip_box(), 1.0 /*sym.get_opacity()*/);
                }
            }
        }
    }
    else // FIXME: should default marker be stored in marker_cache ???
    {
        arrow arrow_;
        color const& fill_ = sym.get_fill();
        unsigned r = fill_.red();
        unsigned g = fill_.green();
        unsigned b = fill_.blue();
        unsigned a = fill_.alpha();
    
        box2d<double> extent = arrow_.extent();    
        double x1 = extent.minx();
        double y1 = extent.miny();
        double x2 = extent.maxx();
        double y2 = extent.maxy();
        tr.transform(&x1,&y1);
        tr.transform(&x2,&y2);
        extent.init(x1,y1,x2,y2);
        
        for (unsigned i=0; i<feature.num_geometries(); ++i)
        {
            geometry2d const& geom = feature.get_geometry(i);
            if (geom.num_points() <= 1) continue;
            
            path_type path(t_,geom,prj_trans);
            markers_placement<path_type, label_collision_detector4> placement(path, extent, detector_, 
                                                                              sym.get_spacing(), 
                                                                              sym.get_max_error(), 
                                                                              sym.get_allow_overlap());        
            double x, y, angle;
        
            while (placement.get_point(&x, &y, &angle))
            {
                agg::trans_affine matrix = tr * agg::trans_affine_rotation(angle) * agg::trans_affine_translation(x, y);
                agg::conv_transform<arrow, agg::trans_affine> trans(arrow_, matrix);
                ras_ptr->add_path(trans);
                ren.color(agg::rgba8(r, g, b, a));
                agg::render_scanlines(*ras_ptr, sl, ren);
            }
        }
    }
}

template void agg_renderer<image_32>::process(markers_symbolizer const&,
                                              Feature const&,
                                              proj_transform const&);
}
