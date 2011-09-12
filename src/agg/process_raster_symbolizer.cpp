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
#include <mapnik/config.hpp>
#include <mapnik/image_data.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/raster.hpp>
#include <mapnik/box2d.hpp>

// stl
#include <cmath>

namespace mapnik {

static inline void resample_raster(raster &target, raster const& source,
                                   proj_transform const& prj_trans,
                                   double offset_x, double offset_y,
                                   double filter_factor,
                                   std::string scaling_method_name)
{
    if (prj_trans.equal()) {
    
        if (scaling_method_name == "bilinear8"){
            scale_image_bilinear8<image_data_32>(target.data_,source.data_,
                                                 offset_x, offset_y);
        } else {
            double scale_factor = target.data_.width() / source.data_.width();
            scaling_method_e scaling_method = get_scaling_method_by_name(scaling_method_name);
            scale_image_agg<image_data_32>(target.data_,source.data_, (scaling_method_e)scaling_method, scale_factor, offset_x, offset_y, filter_factor);
        }
    } else {
        CoordTransform ts(source.data_.width(), source.data_.height(),
                          source.ext_);
        CoordTransform tt(target.data_.width(), target.data_.height(),
                          target.ext_, offset_x, offset_y);
        unsigned i, j, mesh_size=16;
        unsigned mesh_nx = ceil(target.data_.width()/double(mesh_size)+1);
        unsigned mesh_ny = ceil(target.data_.height()/double(mesh_size)+1);
        ImageData<double> xs(mesh_nx, mesh_ny);
        ImageData<double> ys(mesh_nx, mesh_ny);

        // Precalculate reprojected mesh
        for(j=0; j<mesh_ny; j++) {
            for (i=0; i<mesh_nx; i++) {
                xs(i,j) = i*mesh_size+.5;
                ys(i,j) = j*mesh_size+.5;
                tt.backward(&xs(i,j), &ys(i,j));
            }
        }
        prj_trans.forward(xs.getData(), ys.getData(), NULL, mesh_nx*mesh_ny);

        // Interpolate. TODO: Use AGG for this
        for(j=0; j<mesh_ny-1; j++) {
            for (i=0; i<mesh_nx-1; i++) {
                box2d<double> win(xs(i,j), ys(i,j), xs(i+1,j+1), ys(i+1,j+1));
                win = ts.forward(win);
                CoordTransform tw(mesh_size, mesh_size, win);
                unsigned x, y;
                for (y=0; y<mesh_size; y++) {
                    for (x=0; x<mesh_size; x++) {
                        double x2=x, y2=y;
                        tw.backward(&x2,&y2);
                        if (x2>=0 && x2<source.data_.width() &&
                            y2>=0 && y2<source.data_.height())
                        {
                            target.data_(i*mesh_size+x,(j+1)*mesh_size-y) =\
                                source.data_((unsigned)x2,(unsigned)y2);
                        }
                    }
                }
            }
        }
    }
}

template <typename T>
void agg_renderer<T>::process(raster_symbolizer const& sym,
                              Feature const& feature,
                              proj_transform const& prj_trans)
{
    raster_ptr const& source=feature.get_raster();
    if (source)
    {
        // If there's a colorizer defined, use it to color the raster in-place
        raster_colorizer_ptr colorizer = sym.get_colorizer();
        if (colorizer)
            colorizer->colorize(source,feature.props());

        box2d<double> target_ext = box2d<double>(source->ext_);
        prj_trans.backward(target_ext, 20);

        box2d<double> ext=t_.forward(target_ext);
        int start_x = (int)ext.minx();
        int start_y = (int)ext.miny();
        int end_x = (int)ceil(ext.maxx());
        int end_y = (int)ceil(ext.maxy());
        int raster_width = end_x - start_x;
        int raster_height = end_y - start_y;
        double err_offs_x = ext.minx() - start_x;
        double err_offs_y = ext.miny() - start_y;
        
        if (raster_width > 0 && raster_height > 0)
        {
            image_data_32 target_data(raster_width,raster_height);
            raster target(target_ext, target_data);

            resample_raster(target, *source, prj_trans, err_offs_x, err_offs_y,
                            sym.calculate_filter_factor(),
                            sym.get_scaling());
            
            if (sym.get_mode() == "normal"){
                if (sym.get_opacity() == 1.0) {
                    pixmap_.set_rectangle_alpha(start_x,start_y,target.data_);
                } else {
                    pixmap_.set_rectangle_alpha2(target.data_,start_x,start_y, sym.get_opacity());
                }
            } else if (sym.get_mode() == "grain_merge"){
                pixmap_.template merge_rectangle<MergeGrain> (target.data_,start_x,start_y, sym.get_opacity());
            } else if (sym.get_mode() == "grain_merge2"){
                pixmap_.template merge_rectangle<MergeGrain2> (target.data_,start_x,start_y, sym.get_opacity());
            } else if (sym.get_mode() == "multiply"){
                pixmap_.template merge_rectangle<Multiply> (target.data_,start_x,start_y, sym.get_opacity());
            } else if (sym.get_mode() == "multiply2"){
                pixmap_.template merge_rectangle<Multiply2> (target.data_,start_x,start_y, sym.get_opacity());
            } else if (sym.get_mode() == "divide"){
                pixmap_.template merge_rectangle<Divide> (target.data_,start_x,start_y, sym.get_opacity());
            } else if (sym.get_mode() == "divide2"){
                pixmap_.template merge_rectangle<Divide2> (target.data_,start_x,start_y, sym.get_opacity());
            } else if (sym.get_mode() == "screen"){
                pixmap_.template merge_rectangle<Screen> (target.data_,start_x,start_y, sym.get_opacity());
            } else if (sym.get_mode() == "hard_light"){
                pixmap_.template merge_rectangle<HardLight> (target.data_,start_x,start_y, sym.get_opacity());
            } else {
                if (sym.get_opacity() == 1.0){
                    pixmap_.set_rectangle(start_x,start_y,target.data_);
                } else {
                    pixmap_.set_rectangle_alpha2(target.data_,start_x,start_y, sym.get_opacity());
                }
            }
            // TODO: other modes? (add,diff,sub,...)
        }
    }
}

template void agg_renderer<image_32>::process(raster_symbolizer const&,
                                              Feature const&,
                                              proj_transform const&);

}
