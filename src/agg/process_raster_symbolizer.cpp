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

namespace mapnik {


template <typename T>
void agg_renderer<T>::process(raster_symbolizer const& sym,
                              Feature const& feature,
                              proj_transform const& prj_trans)
{
    raster_ptr const& raster=feature.get_raster();
    if (raster)
    {
        // If there's a colorizer defined, use it to color the raster in-place
        raster_colorizer_ptr colorizer = sym.get_colorizer();
        if (colorizer)
            colorizer->colorize(raster);
        
        box2d<double> ext=t_.forward(raster->ext_);
        
        int start_x = rint(ext.minx());
        int start_y = rint(ext.miny());
        int raster_width = rint(ext.width());
        int raster_height = rint(ext.height());
        int end_x = start_x + raster_width;
        int end_y = start_y + raster_height;
        double err_offs_x = (ext.minx()-start_x + ext.maxx()-end_x)/2;
        double err_offs_y = (ext.miny()-start_y + ext.maxy()-end_y)/2;
        
        if ( raster_width > 0 && raster_height > 0)
        {
            image_data_32 target(raster_width,raster_height);
          
            if (sym.get_scaling() == "fast") {
                scale_image<image_data_32>(target,raster->data_);
            } else if (sym.get_scaling() == "bilinear"){
                scale_image_bilinear<image_data_32>(target,raster->data_, err_offs_x, err_offs_y);
            } else if (sym.get_scaling() == "bilinear8"){
                scale_image_bilinear8<image_data_32>(target,raster->data_, err_offs_x, err_offs_y);
            } else {
                scale_image<image_data_32>(target,raster->data_);
            }
            
            if (sym.get_mode() == "normal"){
                if (sym.get_opacity() == 1.0) {
                    pixmap_.set_rectangle(start_x,start_y,target);
                } else {
                    pixmap_.set_rectangle_alpha2(target,start_x,start_y, sym.get_opacity());
                }
            } else if (sym.get_mode() == "grain_merge"){
                pixmap_.template merge_rectangle<MergeGrain> (target,start_x,start_y, sym.get_opacity());
            } else if (sym.get_mode() == "grain_merge2"){
                pixmap_.template merge_rectangle<MergeGrain2> (target,start_x,start_y, sym.get_opacity());
            } else if (sym.get_mode() == "multiply"){
                pixmap_.template merge_rectangle<Multiply> (target,start_x,start_y, sym.get_opacity());
            } else if (sym.get_mode() == "multiply2"){
                pixmap_.template merge_rectangle<Multiply2> (target,start_x,start_y, sym.get_opacity());
            } else if (sym.get_mode() == "divide"){
                pixmap_.template merge_rectangle<Divide> (target,start_x,start_y, sym.get_opacity());
            } else if (sym.get_mode() == "divide2"){
                pixmap_.template merge_rectangle<Divide2> (target,start_x,start_y, sym.get_opacity());
            } else if (sym.get_mode() == "screen"){
                pixmap_.template merge_rectangle<Screen> (target,start_x,start_y, sym.get_opacity());
            } else if (sym.get_mode() == "hard_light"){
                pixmap_.template merge_rectangle<HardLight> (target,start_x,start_y, sym.get_opacity());
            } else {
                if (sym.get_opacity() == 1.0){
                    pixmap_.set_rectangle(start_x,start_y,target);
                } else {
                    pixmap_.set_rectangle_alpha2(target,start_x,start_y, sym.get_opacity());
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
