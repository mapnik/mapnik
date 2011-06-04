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
#include <mapnik/grid/grid_rasterizer.hpp>
#include <mapnik/grid/grid_renderer.hpp>
#include <mapnik/grid/grid_pixfmt.hpp>
#include <mapnik/grid/grid_pixel.hpp>
#include <mapnik/grid/grid.hpp>

#include <mapnik/marker_cache.hpp>
#include <mapnik/svg/svg_renderer.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/markers_placement.hpp>
#include <mapnik/arrow.hpp>

// agg
#include "agg_rasterizer_scanline_aa.h"
#include "agg_renderer_scanline.h"
#include "agg_scanline_bin.h"
#include "agg_path_storage.h"
#include "agg_ellipse.h"
#include "agg_conv_stroke.h"

// stl
#include <algorithm>



namespace mapnik {

template <typename T>
void grid_renderer<T>::process(markers_symbolizer const& sym,
                              Feature const& feature,
                              proj_transform const& prj_trans)
{
    typedef coord_transform2<CoordTransform,geometry_type> path_type;
    typedef agg::renderer_base<mapnik::pixfmt_gray16> ren_base;
    typedef agg::renderer_scanline_bin_solid<ren_base> renderer;
    agg::scanline_bin sl;

    grid_rendering_buffer buf(pixmap_.raw_data(), width_, height_, width_);
    mapnik::pixfmt_gray16 pixf(buf);

    ren_base renb(pixf);
    renderer ren(renb);

    ras_ptr->reset();

    agg::trans_affine tr;
    boost::array<double,6> const& m = sym.get_transform();
    tr.load_from(&m[0]);
    tr = agg::trans_affine_scaling(scale_factor_*(1.0/pixmap_.get_resolution())) * tr;
    std::string filename = path_processor_type::evaluate(*sym.get_filename(), feature);
    marker_placement_e placement_method = sym.get_marker_placement();
    marker_type_e marker_type = sym.get_marker_type();

    if (!filename.empty())
    {
        boost::optional<marker_ptr> mark = mapnik::marker_cache::instance()->find(filename, true);
        if (mark && *mark && (*mark)->is_vector())
        {
            boost::optional<path_ptr> marker = (*mark)->get_vector_data();
            box2d<double> const& bbox = (*marker)->bounding_box();
            double x1 = bbox.minx();
            double y1 = bbox.miny();
            double x2 = bbox.maxx();
            double y2 = bbox.maxy();
            
            agg::trans_affine recenter = agg::trans_affine_translation(-0.5*(x1+x2),-0.5*(y1+y2));
            tr.transform(&x1,&y1);
            tr.transform(&x2,&y2);
            box2d<double> extent(x1,y1,x2,y2);
            using namespace mapnik::svg;
            vertex_stl_adapter<svg_path_storage> stl_storage((*marker)->source());
            svg_path_adapter svg_path(stl_storage);
            svg_renderer<svg_path_adapter, 
                         agg::pod_bvector<path_attributes>,
                         renderer,
                         mapnik::pixfmt_gray16 > svg_renderer(svg_path,(*marker)->attributes());

            bool placed = false;
            for (unsigned i=0; i<feature.num_geometries(); ++i)
            {
                geometry_type const& geom = feature.get_geometry(i);
                if (geom.num_points() <= 1)
                {
                    std::clog << "### Warning svg markers not supported yet for points within markers_symbolizer\n";
                    continue;
                } 
                
                path_type path(t_,geom,prj_trans);
                markers_placement<path_type, label_collision_detector4> placement(path, extent, detector_, 
                                                                                  sym.get_spacing() * scale_factor_, 
                                                                                  sym.get_max_error(), 
                                                                                  sym.get_allow_overlap());        
                double x, y, angle;
            
                while (placement.get_point(&x, &y, &angle))
                {
                    placed = true;
                    agg::trans_affine matrix = recenter * tr *agg::trans_affine_rotation(angle) * agg::trans_affine_translation(x, y);
                    svg_renderer.render_id(*ras_ptr, sl, renb, feature.id(), matrix, sym.get_opacity(),bbox);
                }
            }
            if (placed)
                pixmap_.add_feature(feature);
        }
    }
    else
    {
        stroke const& stroke_ = sym.get_stroke();
        double strk_width = stroke_.get_width();
        
        double w;
        double h;
        unsigned int res = pixmap_.get_resolution();
        if (res != 1) {
            // clamp to at least 4 px otherwise interactive pixels can be too small
            double min = static_cast<double>(4/pixmap_.get_resolution());
            w = std::max(sym.get_width()/res,min);
            h = std::max(sym.get_height()/res,min);
        } else {
            w = sym.get_width()/res;
            h = sym.get_height()/res;
        }
    
        arrow arrow_;
        box2d<double> extent;

        double dx = w + (2*strk_width);
        double dy = h + (2*strk_width);

        if (marker_type == ARROW)
        {
            extent = arrow_.extent();
            double x1 = extent.minx();
            double y1 = extent.miny();
            double x2 = extent.maxx();
            double y2 = extent.maxy();
            tr.transform(&x1,&y1);
            tr.transform(&x2,&y2);
            extent.init(x1,y1,x2,y2);
        }
        else
        {
            double x1 = -1 *(dx);
            double y1 = -1 *(dy);
            double x2 = dx;
            double y2 = dy;
            tr.transform(&x1,&y1);
            tr.transform(&x2,&y2);
            extent.init(x1,y1,x2,y2);
        }
    
        double x;
        double y;
        double z=0;

        for (unsigned i=0; i<feature.num_geometries(); ++i)
        {
            geometry_type const& geom = feature.get_geometry(i);
            if (placement_method == MARKER_POINT_PLACEMENT || geom.num_points() <= 1)
            {
                geom.label_position(&x,&y);
                prj_trans.backward(x,y,z);
                t_.forward(&x,&y);
                int px = int(floor(x - 0.5 * dx));
                int py = int(floor(y - 0.5 * dy));
                box2d<double> label_ext (px, py, px + dx +1, py + dy +1);

                if (sym.get_allow_overlap() ||
                    detector_.has_placement(label_ext))
                {
                    agg::ellipse c(x, y, w, h);
                    agg::path_storage marker;
                    marker.concat_path(c);
                    ras_ptr->add_path(marker);
                    
                    // outline
                    if (strk_width)
                    {
                        agg::conv_stroke<agg::path_storage>  outline(marker);
                        outline.generator().width(strk_width * scale_factor_);
                        ras_ptr->add_path(outline);
                    }

                    detector_.insert(label_ext);
                }
            }
            else
            {
                
                agg::path_storage marker;
                if (marker_type == ARROW)
                    marker.concat_path(arrow_);

                path_type path(t_,geom,prj_trans);
                markers_placement<path_type, label_collision_detector4> placement(path, extent, detector_, 
                                                                                  sym.get_spacing() * scale_factor_, 
                                                                                  sym.get_max_error(), 
                                                                                  sym.get_allow_overlap());        
                double x_t, y_t, angle;
            
                while (placement.get_point(&x_t, &y_t, &angle))
                {
                    agg::trans_affine matrix;

                    if (marker_type == ELLIPSE)
                    {
                        // todo proper bbox - this is buggy
                        agg::ellipse c(x_t, y_t, w, h);
                        marker.concat_path(c);
                        agg::trans_affine matrix;
                        matrix *= agg::trans_affine_translation(-x_t,-y_t);
                        matrix *= agg::trans_affine_rotation(angle);
                        matrix *= agg::trans_affine_translation(x_t,y_t);
                        marker.transform(matrix);
                    }
                    else
                    {
                        matrix = tr * agg::trans_affine_rotation(angle) * agg::trans_affine_translation(x_t, y_t);
                    }

                    agg::conv_transform<agg::path_storage, agg::trans_affine> trans(marker, matrix);

                    // fill
                    ras_ptr->add_path(trans);

                    // outline
                    if (strk_width)
                    {
                        agg::conv_stroke<agg::conv_transform<agg::path_storage, agg::trans_affine> >  outline(trans);
                        outline.generator().width(strk_width * scale_factor_);
                        ras_ptr->add_path(outline);
                    }
                }
            }

        }
        ren.color(mapnik::gray16(feature.id()));
        agg::render_scanlines(*ras_ptr, sl, ren);
        pixmap_.add_feature(feature);
    }
}

template void grid_renderer<grid>::process(markers_symbolizer const&,
                                              Feature const&,
                                              proj_transform const&);
}
