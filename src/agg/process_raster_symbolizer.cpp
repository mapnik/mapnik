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

// agg
#include "agg_image_filters.h"
#include "agg_trans_bilinear.h"
#include "agg_span_interpolator_linear.h"
#include "agg_span_image_filter_rgba.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_basics.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_span_allocator.h"
#include "agg_image_accessors.h"
#include "agg_renderer_scanline.h"

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
        double source_dx = source.data_.width()/double(mesh_nx-1);
        double source_dy = source.data_.height()/double(mesh_ny-1);

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

        /*
        // warp image using projected mesh points using bilinear interpolation
        // inside mesh cell
        typedef agg::pixfmt_rgba32 pixfmt;
        typedef pixfmt::color_type color_type;
        typedef agg::renderer_base<pixfmt> renderer_base;
        typedef agg::pixfmt_rgba32_pre pixfmt_pre;
        typedef agg::renderer_base<pixfmt_pre> renderer_base_pre;

        agg::rasterizer_scanline_aa<> rasterizer;
        agg::scanline_u8  scanline;
        agg::rendering_buffer buf((unsigned char*)target.data_.getData(),
                                  target.data_.width(),
                                  target.data_.height(),
                                  target.data_.width()*4);
        pixfmt_pre pixf_pre(buf);
        renderer_base_pre rb_pre(pixf_pre);
        rasterizer.clip_box(0, 0, target.data_.width(), target.data_.height());

        for(j=0; j<mesh_ny-1; j++) {
            for (i=0; i<mesh_nx-1; i++) {
                box2d<double> win(xs(i,j), ys(i,j), xs(i+1,j+1), ys(i+1,j+1));
                win = ts.forward(win);
                double polygon[8] = {win.minx(), win.miny(),
                                     win.maxx(), win.miny(),
                                     win.maxx(), win.maxy(),
                                     win.minx(), win.maxy()};
                rasterizer.reset();
                rasterizer.move_to_d(polygon[0]-1, polygon[1]-1);
                rasterizer.line_to_d(polygon[2]+1, polygon[3]-1);
                rasterizer.line_to_d(polygon[4]+1, polygon[5]+1);
                rasterizer.line_to_d(polygon[6]-1, polygon[7]+1);

                agg::span_allocator<color_type> sa;
                agg::image_filter_bilinear filter_kernel;
                agg::image_filter_lut filter(filter_kernel, false);

                agg::rendering_buffer buf_tile(
                    (unsigned char*)source.data_.getData(),
                    source.data_.width(),
                    source.data_.height(),
                    source.data_.width() * 4);

                pixfmt pixf_tile(buf_tile);

                typedef agg::image_accessor_clone<pixfmt> img_accessor_type;
                img_accessor_type ia(pixf_tile);

                unsigned x0 = i * source_dx;
                unsigned y0 = j * source_dy;
                unsigned x1 = (i+1) * source_dx;
                unsigned y1 = (j+1) * source_dy;

                agg::trans_bilinear tr(polygon, x0, y0, x1, y1);

                if (tr.is_valid())
                {
                    typedef agg::span_interpolator_linear<agg::trans_bilinear>
                        interpolator_type;
                    interpolator_type interpolator(tr);

                    typedef agg::span_image_filter_rgba_2x2<img_accessor_type,
                        interpolator_type> span_gen_type;

                    span_gen_type sg(ia, interpolator, filter);
                    agg::render_scanlines_aa(rasterizer, scanline, rb_pre,
                                             sa, sg);
                }

            }
        }
        */
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
