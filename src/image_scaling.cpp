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
#include <mapnik/image_data.hpp>
#include <mapnik/image_scaling.hpp>
#include <mapnik/span_image_filter.hpp>

// boost
#include <boost/assign/list_of.hpp>
#include <boost/bimap.hpp>

// agg
#include "agg_image_accessors.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_renderer_scanline.h"
#include "agg_rendering_buffer.h"
#include "agg_scanline_u.h"
#include "agg_span_allocator.h"
#include "agg_span_image_filter_rgba.h"
#include "agg_span_interpolator_linear.h"
#include "agg_trans_affine.h"
#include "agg_image_filters.h"

namespace mapnik
{

typedef boost::bimap<scaling_method_e, std::string> scaling_method_lookup_type;
static const scaling_method_lookup_type scaling_lookup = boost::assign::list_of<scaling_method_lookup_type::relation>
    (SCALING_NEAR,"near")
    (SCALING_BILINEAR,"bilinear")
    (SCALING_BICUBIC,"bicubic")
    (SCALING_SPLINE16,"spline16")
    (SCALING_SPLINE36,"spline36")
    (SCALING_HANNING,"hanning")
    (SCALING_HAMMING,"hamming")
    (SCALING_HERMITE,"hermite")
    (SCALING_KAISER,"kaiser")
    (SCALING_QUADRIC,"quadric")
    (SCALING_CATROM,"catrom")
    (SCALING_GAUSSIAN,"gaussian")
    (SCALING_BESSEL,"bessel")
    (SCALING_MITCHELL,"mitchell")
    (SCALING_SINC,"sinc")
    (SCALING_LANCZOS,"lanczos")
    (SCALING_BLACKMAN,"blackman")
    (SCALING_BILINEAR8,"bilinear8")
    ;

boost::optional<scaling_method_e> scaling_method_from_string(std::string const& name)
{
    boost::optional<scaling_method_e> mode;
    scaling_method_lookup_type::right_const_iterator right_iter = scaling_lookup.right.find(name);
    if (right_iter != scaling_lookup.right.end())
    {
        mode.reset(right_iter->second);
    }
    return mode;
}

boost::optional<std::string> scaling_method_to_string(scaling_method_e scaling_method)
{
    boost::optional<std::string> mode;
    scaling_method_lookup_type::left_const_iterator left_iter = scaling_lookup.left.find(scaling_method);
    if (left_iter != scaling_lookup.left.end())
    {
        mode.reset(left_iter->second);
    }
    return mode;
}

// this has been replaced by agg impl - see https://github.com/mapnik/mapnik/issues/656
template <typename Image>
void scale_image_bilinear_old (Image & target,Image const& source, double x_off_f, double y_off_f)
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


template <typename Image>
void scale_image_bilinear8 (Image & target,Image const& source, double x_off_f, double y_off_f)
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
void scale_image_agg(Image & target,
                     Image const& source,
                     scaling_method_e scaling_method,
                     double image_ratio,
                     double x_off_f,
                     double y_off_f,
                     double filter_radius,
                     double ratio)
{
    // "the image filters should work namely in the premultiplied color space"
    // http://old.nabble.com/Re:--AGG--Basic-image-transformations-p1110665.html
    // "Yes, you need to use premultiplied images only. Only in this case the simple weighted averaging works correctly in the image fitering."
    // http://permalink.gmane.org/gmane.comp.graphics.agg/3443
    typedef agg::pixfmt_rgba32_pre pixfmt_pre;
    typedef agg::renderer_base<pixfmt_pre> renderer_base_pre;

    // define some stuff we'll use soon
    agg::rasterizer_scanline_aa<> ras;
    agg::scanline_u8 sl;
    agg::span_allocator<agg::rgba8> sa;
    agg::image_filter_lut filter;

    // initialize source AGG buffer
    agg::rendering_buffer rbuf_src((unsigned char*)source.getBytes(), source.width(), source.height(), source.width() * 4);
    pixfmt_pre pixf_src(rbuf_src);
    typedef agg::image_accessor_clone<pixfmt_pre> img_src_type;
    img_src_type img_src(pixf_src);

    // initialize destination AGG buffer (with transparency)
    agg::rendering_buffer rbuf_dst((unsigned char*)target.getBytes(), target.width(), target.height(), target.width() * 4);
    pixfmt_pre pixf_dst(rbuf_dst);
    renderer_base_pre rb_dst_pre(pixf_dst);
    rb_dst_pre.clear(agg::rgba(0, 0, 0, 0));

    // create a scaling matrix
    agg::trans_affine img_mtx;
    img_mtx /= agg::trans_affine_scaling(image_ratio * ratio, image_ratio * ratio);

    // create a linear interpolator for our scaling matrix
    typedef agg::span_interpolator_linear<> interpolator_type;
    interpolator_type interpolator(img_mtx);

    // draw an anticlockwise polygon to render our image into
    double scaled_width = target.width();
    double scaled_height = target.height();
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
        agg::render_scanlines_aa(ras, sl, rb_dst_pre, sa, sg);
        return;
    }
    case SCALING_BILINEAR:
    case SCALING_BILINEAR8:
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
    // details on various resampling considerations
    // http://old.nabble.com/Re%3A-Newbie---texture-p5057255.html

    // high quality resampler
    //typedef agg::span_image_resample_rgba_affine<img_src_type> span_gen_type;

    // faster, lower quality
    //typedef agg::span_image_filter_rgba<img_src_type,interpolator_type> span_gen_type;

    // local, modified agg::span_image_resample_rgba_affine
    // not convinced we need this
    // https://github.com/mapnik/mapnik/issues/1489
    typedef mapnik::span_image_resample_rgba_affine<img_src_type> span_gen_type;
    span_gen_type sg(img_src, interpolator, filter);
    agg::render_scanlines_aa(ras, sl, rb_dst_pre, sa, sg);
}

template void scale_image_agg<image_data_32> (image_data_32& target,const image_data_32& source, scaling_method_e scaling_method, double scale_factor, double x_off_f, double y_off_f, double filter_radius, double ratio);

template void scale_image_bilinear_old<image_data_32> (image_data_32& target,const image_data_32& source, double x_off_f, double y_off_f);

template void scale_image_bilinear8<image_data_32> (image_data_32& target,const image_data_32& source, double x_off_f, double y_off_f);


}
