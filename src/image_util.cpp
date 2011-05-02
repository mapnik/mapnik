/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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

//$Id: image_util.cpp 36 2005-04-05 14:32:18Z pavlenko $

extern "C"
{
#include <png.h>
}

// mapnik
#include <mapnik/image_util.hpp>
#include <mapnik/png_io.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/memory.hpp>
#include <mapnik/image_view.hpp>
#include <mapnik/map.hpp>

// boost
#include <boost/lexical_cast.hpp>

// jpeg
#if defined(HAVE_JPEG)
#include <mapnik/jpeg_io.hpp>
#endif

#ifdef HAVE_CAIRO
#include <mapnik/cairo_renderer.hpp>
#endif

#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>

// stl
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

// agg
//#include "agg_conv_transform.h"
#include "agg_image_accessors.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_renderer_scanline.h"
#include "agg_rendering_buffer.h"
#include "agg_scanline_u.h"
//#include "agg_scanline_p.h"
#include "agg_span_allocator.h"
#include "agg_span_image_filter_rgba.h"
#include "agg_span_interpolator_linear.h"
#include "agg_trans_affine.h"
#include "agg_image_filters.h"


namespace mapnik
{    
template <typename T>
std::string save_to_string(T const& image,
                           std::string const& type)
{
    std::ostringstream ss(std::ios::out|std::ios::binary);
    save_to_stream(image, ss, type);
    return ss.str();
}

template <typename T>
void save_to_file(T const& image,
                  std::string const& filename,
                  std::string const& type)
{
    std::ofstream file (filename.c_str(), std::ios::out| std::ios::trunc|std::ios::binary);
    if (file)
    {
        save_to_stream(image, file, type);
    }
    else throw ImageWriterException("Could not write file to " + filename );
}

template <typename T>
void save_to_stream(T const& image,
                    std::ostream & stream,
                    std::string const& type)
{
    if (stream)
    {
        //all this should go into image_writer factory
        if (type == "png")  save_as_png(stream, image);
        else if (boost::algorithm::istarts_with(type, std::string("png256")) ||
                 boost::algorithm::istarts_with(type, std::string("png8"))
            ) 
        {
            int colors  = 256;
            int trans_mode = -1;
            double gamma = -1;
            bool use_octree = true;
            if (type.length() > 6){
                boost::char_separator<char> sep(":");
                boost::tokenizer< boost::char_separator<char> > tokens(type, sep);
                BOOST_FOREACH(string t, tokens)
                {
                    if (t == "m=h")
                    {
                        use_octree = false;
                    }
                    if (t == "m=o")
                    {
                        use_octree = true;
                    }
                    if (boost::algorithm::istarts_with(t,std::string("c=")))
                    {
                        try 
                        {
                            colors = boost::lexical_cast<int>(t.substr(2));
                            if (colors < 0 || colors > 256)
                                throw ImageWriterException("invalid color parameter: " + t.substr(2) + " out of bounds");
                        }
                        catch(boost::bad_lexical_cast &)
                        {
                            throw ImageWriterException("invalid color parameter: " + t.substr(2));
                        }
                    }
                    if (boost::algorithm::istarts_with(t, std::string("t=")))
                    {
                        try 
                        {
                            trans_mode= boost::lexical_cast<int>(t.substr(2));
                            if (trans_mode < 0 || trans_mode > 2)
                                throw ImageWriterException("invalid trans_mode parameter: " + t.substr(2) + " out of bounds");
                        }
                        catch(boost::bad_lexical_cast &)
                        {
                            throw ImageWriterException("invalid trans_mode parameter: " + t.substr(2));
                        }
                    }
                    if (boost::algorithm::istarts_with(t, std::string("g=")))
                    {
                        try 
                        {
                            gamma= boost::lexical_cast<double>(t.substr(2));
                            if (gamma < 0)
                                throw ImageWriterException("invalid gamma parameter: " + t.substr(2) + " out of bounds");
                        }
                        catch(boost::bad_lexical_cast &)
                        {
                            throw ImageWriterException("invalid gamma parameter: " + t.substr(2));
                        }
                    }
                }

            }
            if (use_octree)
                save_as_png256(stream, image, colors);
            else
                save_as_png256_hex(stream, image, colors, trans_mode, gamma);
        }
#if defined(HAVE_JPEG)
        else if (boost::algorithm::istarts_with(type,std::string("jpeg")))
        {
            int quality = 85;
            try 
            {
                if(type.substr(4).length() != 0)
                {
                    quality = boost::lexical_cast<int>(type.substr(4));
                    if(quality<0 || quality>100)
                        throw ImageWriterException("invalid jpeg quality: " + type.substr(4) + " out of bounds");
                }
                save_as_jpeg(stream, quality, image); 
            } 
            catch(boost::bad_lexical_cast &)
            {
                throw ImageWriterException("invalid jpeg quality: " + type.substr(4) + " not a number");
            }
        }
#endif
        else throw ImageWriterException("unknown file type: " + type);
    } 
    else throw ImageWriterException("Could not write to empty stream" );
}
        
        
template <typename T>
void save_to_file(T const& image,std::string const& filename)
{
    boost::optional<std::string> type = type_from_filename(filename);
    if (type)
    {
        save_to_file<T>(image,filename,*type);
    }
}

#if defined(HAVE_CAIRO)
// TODO - move to separate cairo_io.hpp
void save_to_cairo_file(mapnik::Map const& map, std::string const& filename)
{
    boost::optional<std::string> type = type_from_filename(filename);
    if (type)
    {
        save_to_cairo_file(map,filename,*type);
    }
}

void save_to_cairo_file(mapnik::Map const& map,
                        std::string const& filename,
                        std::string const& type)
{
    std::ofstream file (filename.c_str(), std::ios::out|std::ios::trunc|std::ios::binary);
    if (file)
    {
        Cairo::RefPtr<Cairo::Surface> surface;
        unsigned width = map.width();
        unsigned height = map.height();
        if (type == "pdf")
            surface = Cairo::PdfSurface::create(filename,width,height);
        else if (type == "svg")
            surface = Cairo::SvgSurface::create(filename,width,height);
        else if (type == "ps")
            surface = Cairo::PsSurface::create(filename,width,height);
        else if (type == "ARGB32")
            surface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32,width,height);
        else if (type == "RGB24")
            surface = Cairo::ImageSurface::create(Cairo::FORMAT_RGB24,width,height);
        else 
            throw ImageWriterException("unknown file type: " + type);    
        Cairo::RefPtr<Cairo::Context> context = Cairo::Context::create(surface);
    
        // TODO - expose as user option
        /*
          if (type == "ARGB32" || type == "RGB24") 
          { 
          context->set_antialias(Cairo::ANTIALIAS_NONE); 
          }
        */
          
    
        mapnik::cairo_renderer<Cairo::Context> ren(map, context);
        ren.apply();
    
        if (type == "ARGB32" || type == "RGB24") 
        { 
            surface->write_to_png(filename);
        }
        surface->finish();
    }
}

#endif

template void save_to_file<image_data_32>(image_data_32 const&,
                                          std::string const&,
                                          std::string const&);

template void save_to_file<image_data_32>(image_data_32 const&,
                                          std::string const&);

template std::string save_to_string<image_data_32>(image_data_32 const&,
                                                   std::string const&);

template void save_to_file<image_view<image_data_32> > (image_view<image_data_32> const&,
                                                        std::string const&,
                                                        std::string const&);
   
template void save_to_file<image_view<image_data_32> > (image_view<image_data_32> const&,
                                                        std::string const&);
   
template std::string save_to_string<image_view<image_data_32> > (image_view<image_data_32> const&,
                                                                 std::string const&);



// Image scaling functions

scaling_method_e get_scaling_method_by_name (std::string name)
{
    // TODO - make into proper ENUMS
    if (name == "fast" || name == "near")
        return SCALING_NEAR;
    else if (name == "bilinear")
        return SCALING_BILINEAR;
    else if (name == "cubic" || name == "bicubic")
        return SCALING_BICUBIC;
    else if (name == "spline16")
        return SCALING_SPLINE16;
    else if (name == "spline36")
        return SCALING_SPLINE36;
    else if (name == "hanning")
        return SCALING_HANNING;
    else if (name == "hamming")
        return SCALING_HAMMING;
    else if (name == "hermite")
        return SCALING_HERMITE;
    else if (name == "kaiser")
        return SCALING_KAISER;
    else if (name == "quadric")
        return SCALING_QUADRIC;
    else if (name == "catrom")
        return SCALING_CATROM;
    else if (name == "gaussian")
        return SCALING_GAUSSIAN;
    else if (name == "bessel")
        return SCALING_BESSEL;
    else if (name == "mitchell")
        return SCALING_MITCHELL;
    else if (name == "sinc")
        return SCALING_SINC;
    else if (name == "lanczos")
        return SCALING_LANCZOS;
    else if (name == "blackman")
        return SCALING_BLACKMAN;
    else
        return SCALING_NEAR;
}

// this has been replaced by agg impl - see https://trac.mapnik.org/ticket/656
/*
inline void scale_image_bilinear (Image& target,const Image& source, double x_off_f=0, double y_off_f=0)
{

    int source_width=source.width();
    int source_height=source.height();

    int target_width=target.width();
    int target_height=target.height();

    if (source_width<1 || source_height<1 ||
        target_width<1 || target_height<1) return;
    int x=0,y=0,xs=0,ys=0;
    int tw2 = target_width/2;
    int th2 = target_height/2;
    int offs_x = rint((source_width-target_width-x_off_f*2*source_width)/2);
    int offs_y = rint((source_height-target_height-y_off_f*2*source_height)/2);
    unsigned yprt, yprt1, xprt, xprt1;

    //no scaling or subpixel offset
    if (target_height == source_height && target_width == source_width && offs_x == 0 && offs_y == 0){
        for (y=0;y<target_height;++y)
            target.setRow(y,source.getRow(y),target_width);
        return;
    }

    for (y=0;y<target_height;++y)
    {
        ys = (y*source_height+offs_y)/target_height;
        int ys1 = ys+1;
        if (ys1>=source_height)
            ys1--;
        if (ys<0)
            ys=ys1=0;
        if (source_height/2<target_height)
            yprt = (y*source_height+offs_y)%target_height;
        else
            yprt = th2;
        yprt1 = target_height-yprt;
        for (x=0;x<target_width;++x)
        {
            xs = (x*source_width+offs_x)/target_width;
            if (source_width/2<target_width)
                xprt = (x*source_width+offs_x)%target_width;
            else
                xprt = tw2;
            xprt1 = target_width-xprt;
            int xs1 = xs+1;
            if (xs1>=source_width)
                xs1--;
            if (xs<0)
                xs=xs1=0;

            unsigned a = source(xs,ys);
            unsigned b = source(xs1,ys);
            unsigned c = source(xs,ys1);
            unsigned d = source(xs1,ys1);
            unsigned out=0;
            unsigned t = 0;

            for(int i=0; i<4; i++){
                unsigned p,r,s;
                // X axis
                p = a&0xff;
                r = b&0xff;
                if (p!=r)
                    r = (r*xprt+p*xprt1+tw2)/target_width;
                p = c&0xff;
                s = d&0xff;
                if (p!=s)
                    s = (s*xprt+p*xprt1+tw2)/target_width;
                // Y axis
                if (r!=s)
                    r = (s*yprt+r*yprt1+th2)/target_height;
                // channel up
                out |= r << t;
                t += 8;
                a >>= 8;
                b >>= 8;
                c >>= 8;
                d >>= 8;
            }
            target(x,y)=out;
        }
    }
}
*/

template <typename Image>
void scale_image_bilinear8 (Image& target,const Image& source, double x_off_f, double y_off_f)
{

    int source_width=source.width();
    int source_height=source.height();

    int target_width=target.width();
    int target_height=target.height();

    if (source_width<1 || source_height<1 ||
        target_width<1 || target_height<1) return;
    int x=0,y=0,xs=0,ys=0;
    int tw2 = target_width/2;
    int th2 = target_height/2;
    int offs_x = rint((source_width-target_width-x_off_f*2*source_width)/2);
    int offs_y = rint((source_height-target_height-y_off_f*2*source_height)/2);
    unsigned yprt, yprt1, xprt, xprt1;

    //no scaling or subpixel offset
    if (target_height == source_height && target_width == source_width && offs_x == 0 && offs_y == 0){
        for (y=0;y<target_height;++y)
            target.setRow(y,source.getRow(y),target_width);
        return;
    }

    for (y=0;y<target_height;++y)
    {
        ys = (y*source_height+offs_y)/target_height;
        int ys1 = ys+1;
        if (ys1>=source_height)
            ys1--;
        if (ys<0)
            ys=ys1=0;
        if (source_height/2<target_height)
            yprt = (y*source_height+offs_y)%target_height;
        else
            yprt = th2;
        yprt1 = target_height-yprt;
        for (x=0;x<target_width;++x)
        {
            xs = (x*source_width+offs_x)/target_width;
            if (source_width/2<target_width)
                xprt = (x*source_width+offs_x)%target_width;
            else
                xprt = tw2;
            xprt1 = target_width-xprt;
            int xs1 = xs+1;
            if (xs1>=source_width)
                xs1--;
            if (xs<0)
                xs=xs1=0;

            unsigned a = source(xs,ys);
            unsigned b = source(xs1,ys);
            unsigned c = source(xs,ys1);
            unsigned d = source(xs1,ys1);
            unsigned p,r,s;
            // X axis
            p = a&0xff;
            r = b&0xff;
            if (p!=r)
                r = (r*xprt+p*xprt1+tw2)/target_width;
            p = c&0xff;
            s = d&0xff;
            if (p!=s)
                s = (s*xprt+p*xprt1+tw2)/target_width;
            // Y axis
            if (r!=s)
                r = (s*yprt+r*yprt1+th2)/target_height;
            target(x,y)=(0xff<<24) | (r<<16) | (r<<8) | r;
        }
    }
}

template <typename Image>
void scale_image_agg (Image& target,const Image& source, scaling_method_e scaling_method, double scale_factor, double x_off_f, double y_off_f, double filter_radius, double ratio)
{
    typedef agg::pixfmt_rgba32 pixfmt;
    typedef agg::renderer_base<pixfmt> renderer_base;
    
    // define some stuff we'll use soon
    agg::rasterizer_scanline_aa<> ras;
    agg::scanline_u8 sl;
    agg::span_allocator<agg::rgba8> sa;
    agg::image_filter_lut filter;
    
    // initialize source AGG buffer
    agg::rendering_buffer rbuf_src((unsigned char*)source.getBytes(), source.width(), source.height(), source.width() * 4);
    pixfmt pixf_src(rbuf_src);
    
    typedef agg::image_accessor_clone<pixfmt> img_src_type;
    img_src_type img_src(pixf_src);
    
    // initialise destination AGG buffer (with transparency)
    agg::rendering_buffer rbuf_dst((unsigned char*)target.getBytes(), target.width(), target.height(), target.width() * 4);
    pixfmt pixf_dst(rbuf_dst);
    renderer_base rb_dst(pixf_dst);
    rb_dst.clear(agg::rgba(0, 0, 0, 0));
    
    // create a scaling matrix
    agg::trans_affine img_mtx;
    img_mtx /= agg::trans_affine_scaling(scale_factor * ratio, scale_factor * ratio);

    // create a linear interpolator for our scaling matrix
    typedef agg::span_interpolator_linear<> interpolator_type;
    interpolator_type interpolator(img_mtx);
    
    // draw an anticlockwise polygon to render our image into
    double scaled_width = source.width() * scale_factor;
    double scaled_height = source.height() * scale_factor;
    ras.reset();
    ras.move_to_d(x_off_f,                y_off_f);
    ras.line_to_d(x_off_f + scaled_width, y_off_f);
    ras.line_to_d(x_off_f + scaled_width, y_off_f + scaled_height);
    ras.line_to_d(x_off_f,                y_off_f + scaled_height);
    
    switch(scaling_method)
    {
        case SCALING_NEAR:
        {
            typedef agg::span_image_filter_rgba_nn<img_src_type, interpolator_type> span_gen_type;
            span_gen_type sg(img_src, interpolator);
            agg::render_scanlines_aa(ras, sl, rb_dst, sa, sg);
            return;
        }
        case SCALING_BILINEAR:
            filter.calculate(agg::image_filter_bilinear(), true); break;
        case SCALING_BICUBIC:
            filter.calculate(agg::image_filter_bicubic(), true); break;
        case SCALING_SPLINE16:
            filter.calculate(agg::image_filter_spline16(), true); break;
        case SCALING_SPLINE36:
            filter.calculate(agg::image_filter_spline36(), true); break;
        case SCALING_HANNING:
            filter.calculate(agg::image_filter_hanning(), true); break;
        case SCALING_HAMMING:
            filter.calculate(agg::image_filter_hamming(), true); break;
        case SCALING_HERMITE:
            filter.calculate(agg::image_filter_hermite(), true); break;
        case SCALING_KAISER:
            filter.calculate(agg::image_filter_kaiser(), true); break;
        case SCALING_QUADRIC:
            filter.calculate(agg::image_filter_quadric(), true); break;
        case SCALING_CATROM:
            filter.calculate(agg::image_filter_catrom(), true); break;
        case SCALING_GAUSSIAN:
            filter.calculate(agg::image_filter_gaussian(), true); break;
        case SCALING_BESSEL:
            filter.calculate(agg::image_filter_bessel(), true); break;
        case SCALING_MITCHELL:
            filter.calculate(agg::image_filter_mitchell(), true); break;
        case SCALING_SINC:
            filter.calculate(agg::image_filter_sinc(filter_radius), true); break;
        case SCALING_LANCZOS:
            filter.calculate(agg::image_filter_lanczos(filter_radius), true); break;
        case SCALING_BLACKMAN:
            filter.calculate(agg::image_filter_blackman(filter_radius), true); break;
    }
    typedef agg::span_image_resample_rgba_affine<img_src_type> span_gen_type;
    span_gen_type sg(img_src, interpolator, filter);
    agg::render_scanlines_aa(ras, sl, rb_dst, sa, sg);
}

template void scale_image_agg<image_data_32> (image_data_32& target,const image_data_32& source, scaling_method_e scaling_method, double scale_factor, double x_off_f, double y_off_f, double filter_radius, double ratio);

template void scale_image_bilinear8<image_data_32> (image_data_32& target,const image_data_32& source, double x_off_f, double y_off_f);

}
