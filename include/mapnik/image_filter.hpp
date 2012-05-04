/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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


#ifndef MAPNIK_IMAGE_FILTER_HPP
#define MAPNIK_IMAGE_FILTER_HPP

//mapnik
#include <mapnik/image_filter_types.hpp>
// boost
#include <boost/gil/gil_all.hpp>
#include <boost/concept_check.hpp>
// agg 
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_scanline_u.h"
#include "agg_blur.h"

        
// 8-bit YUV
//Y = ( (  66 * R + 129 * G +  25 * B + 128) >> 8) +  16
//U = ( ( -38 * R -  74 * G + 112 * B + 128) >> 8) + 128
//V = ( ( 112 * R -  94 * G -  18 * B + 128) >> 8) + 128

//bits_type x_gradient = (0.125f*c2 + 0.25f*c5 + 0.125f*c8)
//    - (0.125f*c0 + 0.25f*c3 + 0.125f*c6);
//bits_type y_gradient = (0.125f*c0 + 0.25f*c1 + 0.125f*c2)
//    - (0.125f*c6 + 0.25f*c7 + 0.125f*c8);

// c0 c1 c2
// c3 c4 c5
// c6 c7 c8

//sharpen     
//  0 -1  0
// -1  5 -1
//  0 -1  0   
//bits_type out_value = -c1 - c3 + 5.0*c4 - c5 - c7; 
    
// edge detect
//  0  1  0
//  1 -4  1
//  0  1  0  
//bits_type out_value = c1 + c3 - 4.0*c4 + c5 + c7; 
       
// 
//if (out_value < 0) out_value = 0;
//if (out_value > 255) out_value = 255;
    
// emboss
// -2 -1  0
// -1  1  1
//  0  1  2
    
// bits_type out_value = -2*c0 - c1 - c3 + c4 + c5 + c7 +  2*c8; 
    
// blur
//float out_value = (0.1f*c0 + 0.1f*c1 + 0.1f*c2 +
//                   0.1f*c3 + 0.1f*c4 + 0.1f*c5 +
//                  0.1f*c6 + 0.1f*c7 + 0.1f*c8);
    
   
//float out_value  = std::sqrt(std::pow(x_gradient,2) + std::pow(y_gradient,2));
//float theta = std::atan2(x_gradient,y_gradient);
//if (out_value < 0.0) out_value = 0.0;
//if (out_value < 1.0) out_value = 1.0;
    

 
    
//float conv_matrix[]={1/3.0,1/3.0,1/3.0};

//float gaussian_1[]={0.00022923296f,0.0059770769f,0.060597949f,0.24173197f,0.38292751f,
//                    0.24173197f,0.060597949f,0.0059770769f,0.00022923296f};
    
//float gaussian_2[]={
//    0.00048869418f,0.0024031631f,0.0092463447f,
//   0.027839607f,0.065602221f,0.12099898f,0.17469721f,
//   0.19744757f,
//   0.17469721f,0.12099898f,0.065602221f,0.027839607f,
//   0.0092463447f,0.0024031631f,0.00048869418f
//};

//  kernel_1d_fixed<float,9> kernel(conv,4);
    
// color_converted_view<rgb8_pixel_t>(src_view);
//typedef kth_channel_view_type< 0, const rgba8_view_t>::type view_t;

//view_t red = kth_channel_view<0>(const_view(src_view));
    
//kernel_1d_fixed<float,3> kernel(sharpen,0);
//convolve_rows_fixed<rgba32f_pixel_t>(src_view,kernel,src_view);    
// convolve_cols_fixed<rgba32f_pixel_t>(src_view,kernel,dst_view);
   
using namespace boost::gil;

namespace mapnik {  namespace filter { namespace detail {

static const float blur_matrix[] = {0.1111,0.1111,0.1111,0.1111,0.1111,0.1111,0.1111,0.1111,0.1111};
static const float emboss_matrix[] = {-2,-1,0,-1,1,1,0,1,2};
static const float sharpen_matrix[] = {0,-1,0,-1,5,-1,0,-1,0 };
static const float edge_detect_matrix[] = {0,1,0,1,-4,1,0,1,0 };

}


template <typename Src, typename Dst, typename Conv>
void process_channel_impl (Src const& src, Dst & dst, Conv const& k)
{
    using namespace boost::gil;                       
    typedef boost::gil::bits32f  bits_type;
    bits32f out_value = 
        k[0]*src[0] + k[1]*src[1] + k[2]*src[2] +
        k[3]*src[3] + k[4]*src[4] + k[5]*src[5] +
        k[6]*src[6] + k[7]*src[7] + k[8]*src[8]
        ;
    if (out_value < 0) out_value = 0;
    if (out_value > 255) out_value = 255;  
    dst = out_value;
}

template <typename Src, typename Dst, typename Conv>
void process_channel (Src const& src, Dst & dst, Conv const& k) 
{
    boost::ignore_unused_variable_warning(src);
    boost::ignore_unused_variable_warning(dst);
    boost::ignore_unused_variable_warning(k);
}

template <typename Src, typename Dst>
void process_channel (Src const& src, Dst & dst, mapnik::filter::blur)
{
    process_channel_impl(src,dst,mapnik::filter::detail::blur_matrix);
}

template <typename Src, typename Dst>
void process_channel (Src const& src, Dst & dst, mapnik::filter::emboss)
{
    process_channel_impl(src,dst,mapnik::filter::detail::emboss_matrix);
}

template <typename Src, typename Dst>
void process_channel (Src const& src, Dst & dst, mapnik::filter::sharpen)
{
    process_channel_impl(src,dst,mapnik::filter::detail::sharpen_matrix);
}

template <typename Src, typename Dst>
void process_channel (Src const& src, Dst & dst, mapnik::filter::edge_detect)
{
    process_channel_impl(src,dst,mapnik::filter::detail::edge_detect_matrix);
}


template <typename Src, typename Dst>
void process_channel (Src const& src, Dst & dst, mapnik::filter::sobel)
{
    using namespace boost::gil;                       
    typedef boost::gil::bits32f  bits_type;
    
    bits32f x_gradient = (src[2] + 2*src[5] + src[8])
        - (src[0] + 2*src[3] + src[6]);
    
    bits32f y_gradient = (src[0] + 2*src[1] + src[2])
        - (src[6] + 2*src[7] + src[8]);
    
    bits32f  out_value  = std::sqrt(std::pow(x_gradient,2) + std::pow(y_gradient,2));   
    //bts32f theta = std::atan2(x_gradient,y_gradient);    
    if (out_value < 0) out_value = 0;
    if (out_value > 255) out_value = 255;  
    dst = out_value;
}



template <typename Src, typename Dst, typename FilterTag>
void apply_convolution_3x3(Src const& src_view, Dst & dst_view, FilterTag filter_tag)
{
    typename Src::xy_locator src_loc = src_view.xy_at(1,1);
    typename Src::xy_locator::cached_location_t loc00 = src_loc.cache_location(-1,-1);
    typename Src::xy_locator::cached_location_t loc10 = src_loc.cache_location( 0,-1);
    typename Src::xy_locator::cached_location_t loc20 = src_loc.cache_location( 1,-1);        
    typename Src::xy_locator::cached_location_t loc01 = src_loc.cache_location(-1, 0);
    typename Src::xy_locator::cached_location_t loc11 = src_loc.cache_location( 0, 0);
    typename Src::xy_locator::cached_location_t loc21 = src_loc.cache_location( 1, 0);        
    typename Src::xy_locator::cached_location_t loc02 = src_loc.cache_location(-1, 1);
    typename Src::xy_locator::cached_location_t loc12 = src_loc.cache_location( 0, 1);
    typename Src::xy_locator::cached_location_t loc22 = src_loc.cache_location( 1, 1);
    
    for (int y = 1; y<src_view.height()-1; ++y) 
    {
        typename Src::x_iterator dst_it = dst_view.row_begin(y);
        for (int x = 0; x<src_view.width(); ++x) 
        {                       
            *dst_it = src_loc[loc11];
            for (int i = 0; i < 3; ++i)
            {
                bits32f p[9];
                p[0] = src_loc[loc00][i];
                p[1] = src_loc[loc10][i];
                p[2] = src_loc[loc20][i];
                p[3] = src_loc[loc01][i];            
                p[4] = src_loc[loc11][i];
                p[5] = src_loc[loc21][i];
                p[6] = src_loc[loc02][i];            
                p[7] = src_loc[loc12][i];
                p[8] = src_loc[loc22][i];                
                process_channel(p, (*dst_it)[i], filter_tag);
            }
            
            ++dst_it;
            ++src_loc.x();     
        }
        src_loc += point2<std::ptrdiff_t>(-src_view.width(),1);
    }
}


template <typename Src, typename Dst,typename FilterTag> 
void apply_filter(Src const& src, Dst & dst, FilterTag filter_tag)
{
    using namespace boost::gil;
    rgba8_view_t src_view = interleaved_view(src.width(),src.height(),
                                             (rgba8_pixel_t*) src.raw_data(),
                                             src.width()*4);
    rgba8_view_t dst_view = interleaved_view(dst.width(),dst.height(),
                                             (rgba8_pixel_t*) dst.raw_data(),
                                             dst.width()*4);            

    typedef boost::mpl::vector<red_t,green_t,blue_t> channels;
    
    apply_convolution_3x3(src_view,dst_view,filter_tag);
}

template <typename Src, typename FilterTag> 
void apply_filter(Src & src, FilterTag filter_tag)
{
    using namespace boost::gil;
    rgba8_view_t src_view = interleaved_view(src.width(),src.height(),
                                             (rgba8_pixel_t*) src.raw_data(),
                                             src.width()*4);    
    rgba8_image_t temp_buffer(src_view.dimensions());
    apply_convolution_3x3(src_view,boost::gil::view(temp_buffer), filter_tag);
    boost::gil::copy_pixels(view(temp_buffer), src_view);    
}

template <typename Src> 
void apply_filter(Src & src, agg_stack_blur const& op)
{
    agg::rendering_buffer buf(src.raw_data(),src.width(),src.height(), src.width() * 4);
    agg::pixfmt_rgba32 pixf(buf);
    agg::stack_blur_rgba32(pixf,op.rx,op.ry);     
}


template <typename Src> 
void apply_filter(Src & src, gray)
{
    using namespace boost::gil;
    typedef pixel<channel_type<rgba8_view_t>::type, gray_layout_t> gray_pixel_t;  
    
    rgba8_view_t src_view = interleaved_view(src.width(),src.height(),
                                             (rgba8_pixel_t*) src.raw_data(),
                                             src.width()*4);
    boost::gil::copy_and_convert_pixels(color_converted_view<gray_pixel_t>(src_view), src_view); 
}


template <typename Src> 
void apply_filter(Src & src, x_gradient)
{
    using namespace boost::gil;
    
    rgba8_view_t src_view = interleaved_view(src.width(),src.height(),
                                             (rgba8_pixel_t*) src.raw_data(),
                                             src.width()*4);

    rgba8_image_t temp_buffer(src_view.dimensions());
    rgba8_view_t dst_view = view(temp_buffer);
    for (int y=0; y<src_view.height(); ++y) 
    {
        rgba8_view_t::x_iterator src_it = src_view.row_begin(y);
        rgba8_view_t::x_iterator dst_it = dst_view.row_begin(y);
        
        for (int x=1; x<src_view.width()-1; ++x)
        {
            dst_it[x][0] = 127 + (src_it[x-1][0] - src_it[x+1][0]) / 2;
            dst_it[x][1] = 127 + (src_it[x-1][1] - src_it[x+1][1]) / 2;
            dst_it[x][2] = 127 + (src_it[x-1][2] - src_it[x+1][2]) / 2;
            dst_it[x][3] = 255;
        }
    }
    
    boost::gil::copy_pixels(view(temp_buffer), src_view); 
}

template <typename Src> 
void apply_filter(Src & src, y_gradient)
{
    using namespace boost::gil;
    
    rgba8_view_t in = interleaved_view(src.width(),src.height(),
                                       (rgba8_pixel_t*) src.raw_data(),
                                       src.width()*4);
    rgba8_image_t temp_buffer(in.dimensions());
    dynamic_xy_step_type<rgba8_view_t>::type src_view = rotated90ccw_view(in);
    dynamic_xy_step_type<rgba8_view_t>::type dst_view = rotated90ccw_view(view(temp_buffer));
    
    for (int y=0; y<src_view.height(); ++y) 
    {
        dynamic_xy_step_type<rgba8_view_t>::type::x_iterator src_it = src_view.row_begin(y);
        dynamic_xy_step_type<rgba8_view_t>::type::x_iterator dst_it = dst_view.row_begin(y);
        
        for (int x=1; x<src_view.width()-1; ++x)
        {
            dst_it[x][0] = 127 + (src_it[x-1][0] - src_it[x+1][0]) / 2;
            dst_it[x][1] = 127 + (src_it[x-1][1] - src_it[x+1][1]) / 2;
            dst_it[x][2] = 127 + (src_it[x-1][2] - src_it[x+1][2]) / 2;
            dst_it[x][3] = 255;
        }
    }
    
    boost::gil::copy_pixels(view(temp_buffer), in); 
}

template <typename Src> 
void apply_filter(Src & src, invert)
{    
    using namespace boost::gil;
    
    rgba8_view_t src_view = interleaved_view(src.width(),src.height(),
                                             (rgba8_pixel_t*) src.raw_data(),
                                             src.width()*4);        
    for (int y=0; y<src_view.height(); ++y) 
    {
        rgba8_view_t::x_iterator src_itr = src_view.row_begin(y);        
        for (int x=0; x<src_view.width(); ++x)
        {
            get_color(src_itr[x],red_t()) = channel_invert(get_color(src_itr[x],red_t()));
            get_color(src_itr[x],green_t()) = channel_invert(get_color(src_itr[x],green_t()));
            get_color(src_itr[x],blue_t()) =  channel_invert(get_color(src_itr[x],blue_t()));
            get_color(src_itr[x],alpha_t()) = get_color(src_itr[x],alpha_t());
        }
    }
}

template <typename Src>
struct filter_visitor : boost::static_visitor<void>
{
    filter_visitor(Src & src)
        : src_(src) {} 
    
    template <typename T>
    void operator () (T filter_tag)
    {
        apply_filter(src_,filter_tag);
    }   
    
    Src & src_;
};


}}

#endif // MAPNIK_IMAGE_FILTER_HPP
